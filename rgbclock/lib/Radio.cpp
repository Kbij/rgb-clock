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
const int STARTUP_VOLUME = 30;
const int SLOWUP_MINUTES = 5;

Radio::Radio(I2C &i2c, uint8_t amplifierAddress, FMReceiver &fmReceiver, double frequency):
	mI2C(i2c),
	mAplifierAddress(amplifierAddress),
	mFMReceiver(fmReceiver),
	mFrequency(frequency),
	mMaskRegister(0),
	mControlRegister(0),
	mCurrentVolume(STARTUP_VOLUME),
	mStoredVolume(0),
	mRadioObservers(),
	mRadioObserversMutex(),
	mRadioMutex(),
	mState(RadioState::PwrOff),
    mUpDownTimer(*this),
    mAutoPowerOffTimer(*this)
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
	if (mState == RadioState::PwrOn)
	{
		pwrOff();
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
				up(1);
				//if slowvolume up would be running -> stop
			    mUpDownTimer.cancelDimmer();
			}
			if (keyboardInfo.mKeyInfo[KEY_DOWN].mShortPressed || keyboardInfo.mKeyInfo[KEY_DOWN].mLongPress)
			{
				down(1);
				//if slowvolume up would be running -> stop
			    mUpDownTimer.cancelDimmer();
			}
		}
	}

	if (keyboardInfo.mState == KeyboardState::stNormal)
	{
		if (keyboardInfo.mKeyInfo[KEY_LEFT].mShortPressed)
		{
			switch (mState)
			{
				case RadioState::PwrOff: pwrOn();
					break;
				case RadioState::PwrOn : pwrOff();
					break;
				default: break;
			}

		}
	}
}

void Radio::pwrOn(int volume)
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
	mCurrentVolume = 0;
	writeRegisters();

	if (volume > 0)
	{
		mUpDownTimer.initiateUp(volume, SLOWUP_MINUTES * 60);
	}
	else
	{
		mCurrentVolume = mStoredVolume;
		writeRegisters();
	}

	mAutoPowerOffTimer.startAutoPowerOff(AUTO_OFF_MINUTES);
}

void Radio::pwrOff()
{
	LOG(INFO) << "Radio Off";
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);
    mAutoPowerOffTimer.cancelAutoPowerOff();
    mUpDownTimer.cancelDimmer();


    mStoredVolume = mCurrentVolume;

	mState = RadioState::PwrOff;
	mControlRegister = 0b00010001; // Shutdown
	writeRegisters();
	mFMReceiver.powerOff();

	unRegisterFMReceiver();
}

void Radio::up(int step)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mCurrentVolume < 99)
	{
		mCurrentVolume += 1;
		//LOG(INFO) << "Volume: " << (int) mVolume;
		writeRegisters();
	}
}

void Radio::down(int step)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mCurrentVolume > 1)
	{
		mCurrentVolume -= 1;
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
	double attenuation = 64 - static_cast<double>(mCurrentVolume)/100 * 64.0;
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
    	info.mVolume = mCurrentVolume;
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
}
