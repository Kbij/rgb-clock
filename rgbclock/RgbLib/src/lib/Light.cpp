/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"
#include <glog/logging.h>
#include <time.h>
#include <pthread.h>

namespace Hardware
{
const int MIN_DIMMER_INTENSITY = 40;
const int SUNRISE_DIMMER_INTENSITY = 1500;
const int MAX_DIMMER_INTENSITY = 4000;
const int DIMMER_STEP = 20;
const int AUTO_OFF_MINUTES = 60;
const int SLOWUP_MINUTES = 15;
const int FAST_UPDOWN_SECONDS = 2;

Light::Light(I2C &i2c, uint8_t address) :
		mPowerOn(false),
		mPowerDownInitiated(false),
		mRGBLed(i2c, address),
		mCurrentLuminance(0),
		mStoredLuminance(SUNRISE_DIMMER_INTENSITY),
		mLastLong(time(0)),
		mDimDown(true),
		mDimmerMutex(),
		mUpDownTimer(*this),
	    mAutoPowerOffTimer(*this)
{
	internalPwrOff();
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
}

Light::~Light()
{
	mRGBLed.pwrOff();
}

void Light::pwrOn(bool slow)
{
    mAutoPowerOffTimer.cancelAutoPowerOff();
    mUpDownTimer.cancelDimmer();
	bool startTimers = false;
    {
		std::lock_guard<std::mutex> lk_guard(mDimmerMutex);
		if (!mPowerOn)
		{
			mPowerDownInitiated = false;
			internalPwrOn();
			startTimers = true;
		}
    }

    if (startTimers)
    {
		slow ? initiateSlowUp() : initiateFastUp();
		mAutoPowerOffTimer.startAutoPowerOff(AUTO_OFF_MINUTES);
    }
}

void Light::pwrOff()
{
    mAutoPowerOffTimer.cancelAutoPowerOff();
	bool startTimers = false;
	{
	    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);
		if (mPowerOn && !mPowerDownInitiated)
		{
			mUpDownTimer.cancelDimmer();
			mStoredLuminance = mCurrentLuminance;
			mPowerDownInitiated = true;
			startTimers = true;
		}
	}
	if (startTimers)
	{
		initiateFastDown();
	}
}

void Light::pwrToggle()
{
	mPowerOn ? pwrOff() : pwrOn();
}

void Light::up(int step)
{
    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);
    if (mPowerOn)
    {
		mCurrentLuminance += step;
		if (mCurrentLuminance > MAX_DIMMER_INTENSITY)
		{
			mCurrentLuminance = MAX_DIMMER_INTENSITY;
		}

		mRGBLed.luminance(mCurrentLuminance);
    }
}

void Light::down(int step)
{

    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);
    if (mPowerOn)
    {
        mCurrentLuminance -= step;

    	if (mCurrentLuminance <= MIN_DIMMER_INTENSITY)
    	{
    		mCurrentLuminance = MIN_DIMMER_INTENSITY;
    	}

    	mRGBLed.luminance(mCurrentLuminance);
    	if ((mCurrentLuminance <= MIN_DIMMER_INTENSITY) && mPowerDownInitiated)
    	{
    		mUpDownTimer.cancelDimmer();
    		internalPwrOff();
    	}
    }
}

void Light::keyboardPressed(const KeyboardInfo& keyboardInfo)
{
	// keyboard actions only invoke public methods that are protected by the DimmerMutex
	if (keyboardInfo.mState != KeyboardState::stNormal)
	{
		return;
	}

	if (keyboardInfo.mKeyInfo[KEY_CENTRAL_L].mShortPressed || keyboardInfo.mKeyInfo[KEY_CENTRAL_R].mShortPressed)
	{
		pwrToggle();
	}

	if (keyboardInfo.mKeyInfo[KEY_CENTRAL_L].mReleased || keyboardInfo.mKeyInfo[KEY_CENTRAL_R].mReleased)
	{
		mDimDown = !mDimDown;
	}

	if (keyboardInfo.mKeyInfo[KEY_CENTRAL_L].mLongPress || keyboardInfo.mKeyInfo[KEY_CENTRAL_R].mLongPress)
	{

		if (mPowerOn)
		{
			if (difftime(time(nullptr) , mLastLong) > 5)
			{
				mDimDown = true;
			}
			mLastLong = time(nullptr);

			mDimDown ? down(DIMMER_STEP) : up(DIMMER_STEP);
		}
		else
		{
			pwrOn(true);
		}
	}
}

bool Light::isAttached()
{
	std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

	return mRGBLed.isAttached();
}

void Light::initiateFastUp()
{
	if (mStoredLuminance < MIN_DIMMER_INTENSITY)
	{
		mStoredLuminance = MIN_DIMMER_INTENSITY;
	}
	if (mStoredLuminance > MAX_DIMMER_INTENSITY)
	{
		mStoredLuminance = MAX_DIMMER_INTENSITY;
	}

	// Initial powerup always starts from mStoredLuminance
	LOG(INFO) << "Init FastUp, StoredLum=" << mStoredLuminance;
	mUpDownTimer.initiateUp(mStoredLuminance - MIN_DIMMER_INTENSITY, 1);
}

void Light::initiateSlowUp()
{
	LOG(INFO) << "Init SlowUp";
	// Initial powerup always starts from mStoredLuminance
	mUpDownTimer.initiateUp(SUNRISE_DIMMER_INTENSITY - MIN_DIMMER_INTENSITY, SLOWUP_MINUTES * 60);
}

void Light::initiateFastDown()
{
	LOG(INFO) << "Init FastDown";
	mUpDownTimer.initiateDown(mCurrentLuminance, 1);
}

void Light::internalPwrOn()
{
	LOG(INFO) << "Internal PwrOn";

	mCurrentLuminance = MIN_DIMMER_INTENSITY;
	mRGBLed.luminance(mCurrentLuminance);
	mRGBLed.pwrOn();
	mPowerOn = true;
}

void Light::internalPwrOff()
{
	LOG(INFO) << "Internal PwrOff";

	mCurrentLuminance = 0;
	mRGBLed.luminance(mCurrentLuminance);
	mRGBLed.pwrOff();
	mPowerOn = false;
}
} /* namespace Hardware */
