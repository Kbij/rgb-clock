/*
 * AlarmClock.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "AlarmClock.h"
#include "Light.h"
#include "AlarmManager.h"
#include "lib/FMReceiver.h"

#include <glog/logging.h>

namespace App {

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, AlarmManager &alarmManager, UnitConfig unitConfig) :
	mUnitConfig(unitConfig),
	mKeyboard(i2c, unitConfig.mKeyboard),
	mRadio(i2c, unitConfig.mAmplifier, fmReceiver),
	mDisplay(i2c, unitConfig.mLCD, unitConfig.mLightSensor),
	mLight(nullptr),
	mAlarmManager(alarmManager)
{
	mKeyboard.registerKeyboardObserver(this);
	mKeyboard.registerKeyboardObserver(&mRadio);
	mRadio.registerRadioObserver(&mDisplay);
	mAlarmManager.registerAlarmObserver(this);
}

AlarmClock::~AlarmClock()
{
	mAlarmManager.unRegisterAlarmObserver(this);

	mKeyboard.unRegisterKeyboardObserver(this);
	mKeyboard.registerKeyboardObserver(&mRadio);
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

void AlarmClock::alarmNotify()
{
	LOG(INFO) << "Received alarmNotify";
	mRadio.alarmNotify();
	if (mLight)
	{
		mLight->alarmNotify();
	}
}

std::string AlarmClock::name()
{
	return mUnitConfig.mName;
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
