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
	mDisplay(i2c, addresses.mLCD, addresses.mLightSensor),
	mLight(nullptr)
{
	mKeyboard.registerKeyboardObserver(this);
	mKeyboard.registerKeyboardObserver(&mRadio);
	mRadio.registerRadioObserver(&mDisplay);

}

AlarmClock::~AlarmClock()
{
	mKeyboard.unRegisterKeyboardObserver(this);
	mKeyboard.registerKeyboardObserver(&mRadio);
	mRadio.unRegisterRadioObserver(&mDisplay);
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


}


} /* namespace App */
