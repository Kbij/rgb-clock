/*
** RgbClock
** File description:
** DABReceiver
*/

#include "DABReceiver.h"
#include "DABReceiverDef.h"
#include "DABCommands.h"
#include "FMReceiver.h"
#include "RadioObserverIf.h"
#include "SI4735.h"
#include "MainboardControl.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/stl_logging.h>
#include <glog/logging.h>
#include <algorithm>
#include <pthread.h>
#include <iomanip>

namespace 
{
const int FIRMWARE_BLOCK_SIZE = 4096;
}

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
	//readPartInfo();
	init();
}

DABReceiver::~DABReceiver()
{
	// powerOff();
	// LOG(INFO) << "DABReceiver destructor exit";
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
	readStatus();
	readSysState();
    //See Page 154 in the manual
    std::vector<uint8_t> powerUpParams;
	powerUpParams.push_back(0x00); // ARG1 CTSIEN is disabled
	powerUpParams.push_back((1 << 4) | (7 << 0)); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7 (19.2Mhz X-Tal)

    //See page 438 Programming Manual)
    //ESR = 70 Ohm
    powerUpParams.push_back(0x48); // ARG3 IBIAS=0x48 (

    //19 200 000 hz = 0x0124F800
	powerUpParams.push_back(0x00); // ARG4 XTAL
	powerUpParams.push_back(0xF8); // ARG5 XTAL (ik 0xF8)
	powerUpParams.push_back(0x24); // ARG6 XTAL
	powerUpParams.push_back(0x01); // ARG7 XTAL 19.2MHz

    //See Page 444 of programming manual (ABM8 Xtal = 10pf)
    //CTUN = 2*(Cl-Clpar) -Cx -> 2*(10pf - 0) - 0= 20pf -> 0x28
	powerUpParams.push_back(0x28); // ARG8 CTUN (ik: 0x28)

	powerUpParams.push_back(0x00 | (1 << 4)); // ARG9
	powerUpParams.push_back(0x00); // ARG10
	powerUpParams.push_back(0x00); // ARG11
	powerUpParams.push_back(0x00); // ARG12

    //See page 443
	powerUpParams.push_back(0x12); // ARG13 IBIAS_RUN (Ik: 0x12)
	powerUpParams.push_back(0x00); // ARG14
	powerUpParams.push_back(0x00); // ARG15    

	VLOG(1) << "Writing POWER_UP";
    sendCommand(SI468X_POWER_UP, powerUpParams, 0);
    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS after PowerUp";
    }

	readStatus();
	//hostload("./firmware/testfirmware.txt");
	hostload("./firmware/rom00_patch.016.bin");
	std::this_thread::sleep_for( std::chrono::milliseconds(1000));
	readStatus();
	hostload("./firmware/dab_radio_4_0_5.bif");
	readStatus();

	std::this_thread::sleep_for( std::chrono::milliseconds(1000));

    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS before Boot";
    }	

	VLOG(1) << "Writing BOOT";
    sendCommand(SI468X_BOOT, std::vector<uint8_t>({0x00}), 0);
	std::this_thread::sleep_for( std::chrono::milliseconds(2000));
    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS after Boot";
    }	
	
	readStatus();
	readSysState();
	readPartInfo();
}

void DABReceiver::readSysState()
{
    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS before reading Sys State";
		return;
    }

	auto sysStateRaw = sendCommand(SI468X_GET_SYS_STATE, std::vector<uint8_t> ({0x00}), 6);
	LOG(INFO) << "Raw sys state: " << vectorToHexString(sysStateRaw, false);
	SysState sysState(sysStateRaw);

	LOG(INFO) << "System State: " << sysState.toString();
}

void DABReceiver::readStatus()
{
	auto status = sendCommand(SI468X_RD_REPLY,  23);
	VLOG(3) << "Raw Status: " << vectorToHexString(status);
	Status statusCmd(status);
	LOG(INFO) << statusCmd.toString();
}

void DABReceiver::readPartInfo()
{
    if (!waitForCTS())
    {
        LOG(ERROR) << "Timeout waiting for CTS before reading PartInfo";
		return;
    }

	auto partInfoRaw = sendCommand(SI468X_GET_PART_INFO, std::vector<uint8_t> ({0x00}), 23);
	PartInfo partInfo(partInfoRaw);
	LOG(INFO) << partInfo.toString();
}

void DABReceiver::hostload(const std::string& fileName)
{
	std::vector<uint8_t> firmware;
	if (readFile(fileName,  firmware))
	{
		VLOG(1) << "Firmware size: " << firmware.size();

		sendCommand(SI468X_LOAD_INIT, std::vector<uint8_t> ({0x00}), 0);
		if (!waitForCTS())
		{
			LOG(ERROR) << "Timeout waiting for CTS after sending load init";
			return;
		}

		int bytesSend = 0;
		auto filePos = firmware.begin();
		while(filePos != firmware.end())
		{
			int amountToSend = (firmware.end() - filePos) >= FIRMWARE_BLOCK_SIZE ? FIRMWARE_BLOCK_SIZE : firmware.end() - filePos;
			std::vector<uint8_t> firmwareBlock(filePos, (filePos + amountToSend));
			std::vector<uint8_t> hostLoadParams({0x00, 0x00, 0x00});

			std::copy (firmwareBlock.begin(),firmwareBlock.end(),back_inserter(hostLoadParams));

			VLOG(3) << "Sending bytes: " << firmwareBlock.size() << ", already send: " << bytesSend;
			sendCommand(SI468X_HOST_LOAD, hostLoadParams, 4);
			VLOG(3) << vectorToHexString(hostLoadParams, true);

			bytesSend += firmwareBlock.size();


			std::advance(filePos , amountToSend);

			if (!waitForCTS())
			{
				LOG(ERROR) << "Timeout waiting for CTS after sending firmware block";
				return;
			}			
		}
		VLOG(1) << "Total bytes send: " << bytesSend;
		
		std::this_thread::sleep_for( std::chrono::milliseconds(50));		
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

		//We only want the first byte of the Status
		Status status(sendCommand(SI468X_RD_REPLY, 1));
		cts = status.CTS;
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

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, uint8_t resultLength)
{
    return sendCommand(command, std::vector<uint8_t>({}), resultLength);
}

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, const std::vector<uint8_t>& param, uint8_t resultLength)
{
    std::vector<uint8_t> result(resultLength);
	std::vector<uint8_t> fullCmd;
	fullCmd.push_back(command);
	fullCmd.insert(fullCmd.end(), param.begin(), param.end());
    mI2C.readWriteData(mAddress, fullCmd, result);
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
