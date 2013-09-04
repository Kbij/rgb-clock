/*
 * AlarmClock.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "AlarmClock.h"
#include "Light.h"
#include "lib/FMReceiver.h"

#include <glog/logging.h>

namespace App {

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, UnitConfig addresses) :
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
	LOG(INFO) << "Unregistering ourself from the keyboard";
	mKeyboard.unRegisterKeyboardObserver(this);

	LOG(INFO) << "Unregistering the radio from the keyboard";
	mKeyboard.registerKeyboardObserver(&mRadio);

	LOG(INFO) << "Unregistering the radio observer";
	mRadio.unRegisterRadioObserver(&mDisplay);
}

void AlarmClock::registerLight(Light *light)
{
	mLight = light;
	mKeyboard.registerKeyboardObserver(light);
}

void AlarmClock::unRegisterLight(Light *light)
{
	mKeyboard.unRegisterKeyboardObserver(light);
	mLight = nullptr;
}

void AlarmClock::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{


}

bool AlarmClock::hasRegisteredLight()
{
	return mLight != nullptr;
}

bool AlarmClock::isAttached()
{
	return mKeyboard.isAttached();
}

} /* namespace App */
