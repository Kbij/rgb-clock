/*
 * Radio.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#include "Radio.h"
#include "FMReceiver.h"
#include "I2C.h"
#include <glog/logging.h>
#include <iostream>
#include <cmath>
#include <pthread.h>

namespace Hardware
{
const int AUTO_OFF_MINUTES = 60;

Radio::Radio(I2C &i2c, uint8_t amplifierAddress, FMReceiver &fmReceiver, double frequency):
	mI2C(i2c),
	mAplifierAddress(amplifierAddress),
	mFMReceiver(fmReceiver),
	mFrequency(frequency),
	mMaskRegister(0),
	mControlRegister(0),
	mVolume(20),
	mRadioObservers(),
	mRadioObserversMutex(),
	mRadioMutex(),
	mState(RadioState::PwrOff),
	mMaintenanceThread(nullptr),
	mMaintenanceThreadRunning(false),
	mTargetVolume(0),
    mAutoOffThread(nullptr),
    mAutoOffThreadRunning(false)
{
	mI2C.registerAddress(mAplifierAddress, "Amplifier");
	// Use BTL + Shutdown
	mControlRegister = 0b00010001;
	writeRegisters();
	mControlRegister = 0b00010001; // Shutdown
	writeRegisters();
}

Radio::~Radio()
{
	LOG(INFO) << "Radio destructor";
	stopAutoOffThread();
	if (mState == RadioState::PwrOn)
	{
		powerOff();
	}
	LOG(INFO) << "Radio destructor exit";
}

void Radio::registerRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.insert(observer);
    }
}
void Radio::unRegisterRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.erase(observer);
    }
}

void Radio::keyboardPressed(const KeyboardInfo& keyboardInfo)
{
	if ((keyboardInfo.mState == KeyboardState::stNormal) || (keyboardInfo.mState == KeyboardState::stAlarmActive))
	{
		if (mState == RadioState::PwrOn)
		{
			if (keyboardInfo.mKeyInfo[KEY_UP].mShortPressed || keyboardInfo.mKeyInfo[KEY_UP].mLongPress)
			{
				volumeUp();
				//if slowvolume up would be running -> stop
				mMaintenanceThreadRunning = false;
			}
			if (keyboardInfo.mKeyInfo[KEY_DOWN].mShortPressed || keyboardInfo.mKeyInfo[KEY_DOWN].mLongPress)
			{
				volumeDown();
				//if slowvolume up would be running -> stop
				mMaintenanceThreadRunning = false;
			}
		}
	}

	if (keyboardInfo.mState == KeyboardState::stNormal)
	{
		if (keyboardInfo.mKeyInfo[KEY_LEFT].mShortPressed)
		{
			switch (mState)
			{
				case RadioState::PwrOff: powerOn();
					break;
				case RadioState::PwrOn : powerOff();
					break;
				default: break;
			}

		}
	}
}
bool Radio::powerOn()
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	LOG(INFO) << "Radio On";
	registerFMReceiver();
	if (mFMReceiver.powerOn())
	{
		mFMReceiver.tuneFrequency(mFrequency);
	}

	mState = RadioState::PwrOn;
	mControlRegister = 0b00010000; // PowerUp
	writeRegisters();
	startAutoOffThread();
	return true;
}

bool Radio::slowPowerOn(int volume)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);
	LOG(INFO) << "Radio slow power on";

    mTargetVolume = volume;
	mVolume = 0;
	// Write volume first
	writeRegisters();
	powerOn();
	// at last: volume up thread
	startMaintenanceThread();
	return true;
}

bool Radio::powerOff()
{
	return powerOff(false);
}

bool Radio::powerOff(bool autoPowerOff)
{
	LOG(INFO) << "Radio Off";
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);
	if (!autoPowerOff)
	{
		stopAutoOffThread();
	}

	mState = RadioState::PwrOff;
	mControlRegister = 0b00010001; // Shutdown
	writeRegisters();
	mFMReceiver.powerOff();

	unRegisterFMReceiver();
	stopMaintenanceThread();
	return true;
}

void Radio::volumeUp()
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mVolume < 99)
	{
		mVolume += 1;
		//LOG(INFO) << "Volume: " << (int) mVolume;
		writeRegisters();
	}

}

void Radio::volumeDown()
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mVolume > 1)
	{
		mVolume -= 1;
		writeRegisters();
	}
}


bool Radio::seekUp(int timeout)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	return mFMReceiver.seekUp(timeout);
}

bool Radio::tuneFrequency(double frequency)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	return mFMReceiver.tuneFrequency(frequency);
}

RDSInfo Radio::getRDSInfo()
{
	return mFMReceiver.getRDSInfo();
}

void Radio::writeRegisters()
{
	double attenuation = 64 - static_cast<double>(mVolume)/100 * 64.0;
	uint8_t att =  static_cast<uint8_t>(ceil(attenuation));

	std::vector<uint8_t> registers;
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(mMaskRegister);
	registers.push_back(mControlRegister);

	mI2C.writeData(mAplifierAddress, registers);
	notifyObservers();
}

void Radio::notifyObservers()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	RadioInfo info;
    	info.mVolume = mVolume;
    	info.mState = mState;
        observer->radioStateUpdate(info);
    }
}

void Radio::registerFMReceiver()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	mFMReceiver.registerRadioObserver(observer);
    }
}

void Radio::unRegisterFMReceiver()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	mFMReceiver.unRegisterRadioObserver(observer);
    }
}

void Radio::startMaintenanceThread()
{
	stopMaintenanceThread();

	mMaintenanceThreadRunning = true;

	mMaintenanceThread = std::unique_ptr<std::thread>(new std::thread(&Radio::maintenanceThread, this));
}

void Radio::stopMaintenanceThread()
{
	mMaintenanceThreadRunning = false;

    if (mMaintenanceThread)
    {
    	mMaintenanceThread->join();
        mMaintenanceThread.reset();
    }
}

void Radio::startAutoOffThread()
{
	stopAutoOffThread();

	mAutoOffThreadRunning = true;

	mAutoOffThread.reset(new std::thread(&Radio::autoOffThread, this));
}

void Radio::stopAutoOffThread()
{
	mAutoOffThreadRunning = false;

    if (mAutoOffThread)
    {
    	mAutoOffThread->join();

    	mAutoOffThread.reset();
    }
}

void Radio::maintenanceThread()
{
	pthread_setname_np(pthread_self(), "Radio");

	   while (mMaintenanceThreadRunning)
	   {
		   std::this_thread::sleep_for(std::chrono::seconds(2));

		   if (mVolume < mTargetVolume)
		   {
			   mVolume++;
			   writeRegisters();
		   }
		   else
		   {
			   mMaintenanceThreadRunning = false;
		   }
	   }
}

void Radio::autoOffThread()
{
	pthread_setname_np(pthread_self(), "Radio AutoOff");

	int countDownSeconds = AUTO_OFF_MINUTES * 60;
	while (mAutoOffThreadRunning)
	{
	   std::this_thread::sleep_for(std::chrono::seconds(1));
	   --countDownSeconds;
	   if (countDownSeconds == 0)
	   {
		   mAutoOffThreadRunning = false;
		   if (mState == RadioState::PwrOn)
		   {
			   LOG(INFO) << "Radio auto off";
			   powerOff(true);
		   }
	   }
	}
}
}
