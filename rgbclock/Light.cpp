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
		mRGBLed(i2c, address),
		mIntensity(0)
{
	mRGBLed.pwrOn();
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
}

Light::~Light() {
	// TODO Auto-generated destructor stub
}

void Light::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (keyboardInfo[2].mLongPress)
	{
		mIntensity += 10;
	}

	if (keyboardInfo[1].mLongPress)
	{
		mIntensity -= 10;
	}
	LOG(INFO) << "Intensity: " << mIntensity;
	mRGBLed.luminance(mIntensity);
	mRGBLed.write();
}
} /* namespace App */
