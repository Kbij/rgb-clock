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

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, AlarmManager &alarmManager, const UnitConfig& unitConfig) :
	mUnitConfig(unitConfig),
	mKeyboard(i2c, unitConfig.mKeyboard),
	mRadio(i2c, unitConfig.mAmplifier, fmReceiver),
	mAlarmManager(alarmManager),
	mDisplay(i2c, mKeyboard, mAlarmManager, unitConfig),
	mLight(nullptr),
	mClockState(ClockState::clkNormal)
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

void AlarmClock::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo, Hardware::KeyboardState state)
{
	if (mClockState == ClockState::clkAlarm)
	{
		if (keyboardInfo[KEY_1].mPressed)
		{
			mClockState = ClockState::clkNormal;
			mRadio.powerOff();
			if (mLight)
			{
				mLight->pwrOff();
			}

			mKeyboard.keyboardState(Hardware::KeyboardState::stNormal);
		}
	}


}

void AlarmClock::alarmNotify(int volume)
{
	LOG(INFO) << "Received alarmNotify";
	mClockState = ClockState::clkAlarm;
	mKeyboard.keyboardState(Hardware::KeyboardState::stAlarmActive);
	mRadio.slowPowerOn(volume);
	if (mLight)
	{
		mLight->pwrSlowOn();
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
