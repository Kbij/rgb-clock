/*
 * AlarmClock.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "AlarmClock.h"
#include "Light.h"
#include "lib/FMReceiver.h"

namespace App {

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, Addresses addresses) :
	mAddresses(addresses),
	mKeyboard(i2c, addresses.mKeyboard),
	mRadio(i2c, addresses.mAmplifier, fmReceiver),
	mLight(nullptr)
{
	mKeyboard.registerKeyboardObserver(this);
}

AlarmClock::~AlarmClock()
{
	mKeyboard.unRegisterKeyboardObserver(this);
}

void AlarmClock::registerLight(Light *light)
{
	mLight = light;
	mKeyboard.registerKeyboardObserver(light);
}

void AlarmClock::unregisterLight()
{
	mLight = nullptr;
}

void AlarmClock::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (keyboardInfo[KEY_UP].mPressed || keyboardInfo[KEY_UP].mLongPress)
	{
		mRadio.volumeUp();
	}
	if (keyboardInfo[KEY_DOWN].mPressed || keyboardInfo[KEY_DOWN].mLongPress)
	{
		mRadio.volumeDown();
	}

}


} /* namespace App */
