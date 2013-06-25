/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"
#include <glog/logging.h>

namespace App
{

Light::Light(Hardware::I2C &i2c, uint8_t address) :
		mOn(false),
		mRGBLed(i2c, address),
		mIntensity(0)
{
	//mRGBLed.pwrOn();
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
	mIntensity = 100;
	mRGBLed.luminance(mIntensity);
}

Light::~Light() {
	// TODO Auto-generated destructor stub
}

void Light::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (keyboardInfo[5].mPressed)
	{
		if (mOn)
		{
			mOn = false;
			mRGBLed.pwrOff();
		}
		else
		{
			mOn = true;
			mRGBLed.pwrOn();
		}
	}
	if (keyboardInfo[5].mLongPress)
	{
		mIntensity += 10;
		mRGBLed.luminance(mIntensity);
		mRGBLed.write();
	}

//	LOG(INFO) << "Intensity: " << mIntensity;
}
} /* namespace App */
