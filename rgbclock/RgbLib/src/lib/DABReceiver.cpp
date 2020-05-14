/*
** RgbClock
** File description:
** DABReceiver
*/

#include "DABReceiver.h"
#include "Si4684.h"
// #include "Si4684Const.h"
// #include "lib/cmd/DABCommands.h"
// #include "lib/DABCommands.h"
//#include "FMReceiver.h"
#include "RadioObserverIf.h"
//#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/stl_logging.h>
#include <glog/logging.h>
//#include <algorithm>
#include <pthread.h>
#include <iostream>
#include <ostream>
//#include <iomanip>

namespace Hardware
{

DABReceiver::DABReceiver(Si4684* siChip) :
	mSiChip(siChip),
	mPowerCounter(0),
	mPowerMutex(),
	mPowerState(DABPowerState::Unknown),
	mReceiverMutex(),
	mReadThread(nullptr),
	mReadThreadRunning(false),
	mRadioObservers(),
	mRadioObserversMutex()
{
    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    mPowerCounter = 0;
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

void DABReceiver::serviceScan()
{
	std::cout << "Scanning DAB Channels." << std::endl;
	auto frequencyList = mSiChip->getFrequencyList();
	std::cout << frequencyList.mFrequencies.size() << " Channels found." << std::endl;

	for(int index = 0;index < frequencyList.mFrequencies.size() ; ++index)
	{
		auto tuneResponse = mSiChip->tuneFrequencyIndex(index);
		if (tuneResponse.VALID)
		{
			std::cout << "Frequency: " << (double) frequencyList.mFrequencies[index]/1000 << " Mhz (index: " << index << "), RSSI: " << (int) tuneResponse.RSSI << std::endl;

			auto serviceList = mSiChip->getServices();
			std::cout  << "Services: " << std::endl;// <<  serviceList.toString() << std::endl;
			for(const auto& service: serviceList.mServices)
			{
				std::cout << service.ServiceId << ": " << service.Label << " (Component(s): ";
				for (const auto& component: service.Components)
				{
					std::cout << (int) component << " ";
				}
				std::cout << ")" << std::endl;

			}
			std::cout  <<"==========================================================================" << std::endl;
		}
		else
		{
			// std::cout << "Frequency: " << (double) frequencyList.mFrequencies[index]/1000 << " Mhz (index: " << index << "): Empty" << std::endl;
			// std::cout  <<"==========================================================================" << std::endl;
		}
		
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
// std::string commandToString(uint8_t command)
// {
//     switch(command)
//     {
//         case SI468X_RD_REPLY: return "RD_REPLY";
//         case SI468X_POWER_UP: return "POWER_UP";
//         case SI468X_HOST_LOAD: return "HOST_LOAD";
//         case SI468X_LOAD_INIT: return "LOAD_INIT";
//         case SI468X_BOOT: return "BOOT";
//         case SI468X_GET_PART_INFO: return "GET_PART_INFO";
//         case SI468X_GET_SYS_STATE: return "GET_SYS_STATE";
//         case SI468X_SET_PROPERTY: return "SET_PROPERTY";
//         case SI468X_GET_PROPERTY: return "GET_PROPERTY";
//         case SI468X_DAB_TUNE_FREQ: return "DAB_TUNE_FREQ";
//         case SI468X_DAB_DIGRAD_STATUS: return "DAB_DIGRAD_STATUS";
//         case SI468X_DAB_GET_FREQ_LIST: return "DAB_GET_FREQ_LIST";
//         default: return "Unknown";
//     }
// }
}
