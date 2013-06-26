/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"
#include <glog/logging.h>
#include <time.h>

namespace App
{

Light::Light(Hardware::I2C &i2c, uint8_t address) :
		mState(State::PwrOff),
		mRGBLed(i2c, address),
		mLuminance(1000),
		mLastLong(time(nullptr)),
		mDimDown(true)
{
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
	//mRGBLed.luminance(mLuminance);
	//mRGBLed.write();
}

Light::~Light()
{
	mRGBLed.pwrOff();
}
void Light::pwrOn()
{
	mRGBLed.luminance(mLuminance);
	mRGBLed.pwrOn();
	mState = State::PwrOn;
}

void Light::pwrOff()
{
	mRGBLed.pwrOff();
	mState = State::PwrOff;
}

void Light::pwrToggle()
{
	if (mState == State::PwrOn)
	{
		pwrOff();
	}
	else
	{
		pwrOn();
	}
}

void Light::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (keyboardInfo[KEY_CENTRAL].mPressed)
	{
		pwrToggle();
	}

	if (keyboardInfo[KEY_CENTRAL].mReleased)
	{
		mDimDown = !mDimDown;
	}

	if (keyboardInfo[KEY_CENTRAL].mLongPress)
	{
		if (mState == State::PwrOn)
		{
			if (difftime(time(nullptr) , mLastLong) > 5)
			{
				mDimDown = true;
			}
			mLastLong = time(nullptr);

			if (mDimDown)
			{
				if (mLuminance > 9)
				{
					mLuminance -= 10;
				}
			}
			else
			{
				if (mLuminance < 3990 )
				{
					mLuminance += 10;
				}
			}

			mRGBLed.luminance(mLuminance);
			mRGBLed.write();
		}
	}

}
} /* namespace App */
