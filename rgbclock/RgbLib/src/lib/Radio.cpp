/*
 * Radio.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#include "Radio.h"
#include "DABReceiver.h"
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
const int SLOW_VOLUME_START_VOLUME = 5;

Radio::Radio(I2C &i2c, uint8_t amplifierAddress, DABReceiver &dabReceiver):
	mI2C(i2c),
	mAplifierAddress(amplifierAddress),
	mDabReceiver(dabReceiver),
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

void Radio::pwrOn(bool smooth, int volume)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	LOG(INFO) << "Radio On";
	registerFMReceiver();

	mDabReceiver.powerOn();

	mState = RadioState::PwrOn;
	mControlRegister = 0b00010000; // PowerUp
	mCurrentVolume = SLOW_VOLUME_START_VOLUME;
	writeRegisters();

	if (volume > 0)
	{
		if (smooth)
		{
			mUpDownTimer.initiateUp(volume, SLOWUP_MINUTES * 60);
		}
		else
		{
			mCurrentVolume = volume;
			writeRegisters();
		}
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
	mDabReceiver.powerOff();

	unRegisterFMReceiver();
}

void Radio::up(int step)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mCurrentVolume < 99)
	{
		mCurrentVolume += 1;
		VLOG(1) << "Volume: " << (int) mCurrentVolume;
		writeRegisters();
	}
}

void Radio::down(int step)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioMutex);

	if (mCurrentVolume > 1)
	{
		mCurrentVolume -= 1;
		VLOG(1) << "Volume: " << (int) mCurrentVolume;
		writeRegisters();
	}
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
    	mDabReceiver.registerRadioObserver(observer);
    }
}

void Radio::unRegisterFMReceiver()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	mDabReceiver.unRegisterRadioObserver(observer);
    }
}
}
