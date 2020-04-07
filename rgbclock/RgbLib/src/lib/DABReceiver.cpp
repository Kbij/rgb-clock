/*
** RgbClock
** File description:
** DABReceiver
*/

#include "DABReceiver.h"
#include "DABReceiverDef.h"
#include "FMReceiver.h"
#include "RadioObserverIf.h"
#include "SI4735.h"
#include "MainboardControl.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/logging.h>
#include <algorithm>
#include <pthread.h>

namespace Hardware
{

DABReceiver::DABReceiver(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl) :
		mI2C(i2c),
		mAddress(address),
		mMainboardControl(mainboardControl),
		mPowerCounter(0),
		mPowerMutex(),
		mPowerState(DABPowerState::Unknown),
        mReceiverMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false),
		mRadioObservers(),
		mRadioObserversMutex()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for(std::chrono::seconds(1));

	if (mMainboardControl) mMainboardControl->selectInput(InputSelection::RadioIn);

    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    mPowerCounter = 0;

	mI2C.registerAddress(address, "DAB Receiver");
}

DABReceiver::~DABReceiver()
{
	powerOff();
	LOG(INFO) << "DABReceiver destructor exit";
}

void DABReceiver::registerRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.insert(observer);
    }
}

void DABReceiver::unRegisterRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.erase(observer);
    }
}

void DABReceiver::init()
{
    //See Page 154 in the manual
    std::vector<uint8_t> powerUpParams;
	powerUpParams.push_back(0x00); // ARG1 CTSIEN is disabled
	powerUpParams.push_back((1 << 4) | (7 << 0)); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7 (19.2Mhz X-Tal)

    //See page 438 Programming Manual)
    //ESR = 70 Ohm
    powerUpParams.push_back(0x48); // ARG3 IBIAS=0x48 (

    //19 200 000 hz = 0x0124F800
	powerUpParams.push_back(0x00); // ARG4 XTAL
	powerUpParams.push_back(0xF8); // ARG5 XTAL 
	powerUpParams.push_back(0x24); // ARG6 XTAL
	powerUpParams.push_back(0x01); // ARG7 XTAL 19.2MHz

    //See Page 444 of programming manual
    //CTUN = 2*(Cl-Clpar) -Cx -> 2*(10pf - 0) - 0= 20pf -> 0x28
	powerUpParams.push_back(0x28); // ARG8 CTUN

	powerUpParams.push_back(0x00 | (1 << 4)); // ARG9
	powerUpParams.push_back(0x00); // ARG10
	powerUpParams.push_back(0x00); // ARG11
	powerUpParams.push_back(0x00); // ARG12

    //See page 443
	powerUpParams.push_back(0x12); // ARG13 IBIAS_RUN
	powerUpParams.push_back(0x00); // ARG14
	powerUpParams.push_back(0x00); // ARG15    

    sendCommand(SI46XX_POWER_UP, powerUpParams, 0);
    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS after PowerUp";
    }


}

bool DABReceiver::powerOn()
{
	LOG(INFO) << "PowerOn";
    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    if (mPowerCounter == 0)
    {
    	internalPowerOn();
    }
    mPowerCounter++;

    return true;
}

bool DABReceiver::powerOff()
{
	LOG(INFO) << "PowerOff";
    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    if (mPowerCounter <= 1)
    {
    	internalPowerOff();
    }
    mPowerCounter--;

    return true;
}

bool DABReceiver::internalPowerOn()
{
	LOG(INFO) << "Internal PowerOn";
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

	startReadThread();
	return true;
}

bool DABReceiver::internalPowerOff()
{
	LOG(INFO) << "Internal PowerOff";
	stopReadThread();
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);
	switch (mPowerState)
	{
		case DABPowerState::PowerOff : return true;
		case DABPowerState::PowerOn :
			// if (!waitForCTS())
			// {
			// 	return false;
			// }; // No break is intended
		default:
			// std::vector<uint8_t> powerdownResponse(1);
			// mI2C.readWriteData(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
			// LOG(INFO) << "POWER_DOWN Status: " << std::hex << "0x" << (int) powerdownResponse[0];

		break;
	}
	return true;
}

bool DABReceiver::setProperty(int property, int value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	std::vector<uint8_t> setPropertyResponse(1);
	uint8_t propH = property >> 8;
	uint8_t propL = property & 0xFF;
	uint8_t valueH = value >> 8;
	uint8_t valueL = value & 0xFF;
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({SET_PROPERTY, 0x00, propH, propL, valueH, valueL}), setPropertyResponse);

	return true;
}

bool DABReceiver::getProperty(int property, int& value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	return true;
}

bool DABReceiver::waitForCTS()
{
	const int MAX_RETRIES = 20;

	int retries = 0;
	bool cts = false;
	while ((retries < MAX_RETRIES) && !cts)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(100));
		cts = readCTS();
		++retries;
	}
	if (retries >= MAX_RETRIES)
	{
		LOG(ERROR) << "Timeout waiting for CTS";
		return false;
	}
	else
	{
		return true;
	}
}

bool DABReceiver::readCTS()
{
	std::vector<uint8_t> ctsReponse = sendCommand(SI46XX_RD_REPLY, 1); 
	return ctsReponse[0] && 0x80;
}

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, uint8_t resultLength)
{
    return sendCommand(command, std::vector<uint8_t>({}), resultLength);
}

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, const std::vector<uint8_t>& param, uint8_t resultLength)
{
    std::vector<uint8_t> result(resultLength);
    mI2C.readWriteData(mAddress, std::vector<uint8_t>({command}), result);
    return result;
}

void DABReceiver::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = std::unique_ptr<std::thread>(new std::thread(&DABReceiver::readThread, this));
}

void DABReceiver::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();
        mReadThread.reset();
    }
}
void DABReceiver::readThread()
{
	pthread_setname_np(pthread_self(), "DAB Receiver");

    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);
    }
}

void DABReceiver::notifyObservers()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
       // observer->radioRdsUpdate(mRDSInfo);
    }
}

}