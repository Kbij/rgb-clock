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
const int SNOOZE_TIME = 1 * 60; // Snooze interval: 9min
const int ALARM_TIME = 2 * 60; // Alarm time: 30 min

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, AlarmManager &alarmManager, const UnitConfig& unitConfig) :
	mUnitConfig(unitConfig),
	mKeyboard(i2c, unitConfig.mKeyboard),
	mRadio(i2c, unitConfig.mAmplifier, fmReceiver),
	mAlarmManager(alarmManager),
	mDisplay(i2c, mKeyboard, mAlarmManager, unitConfig),
	mLight(nullptr),
	mClockState(ClockState::clkNormal),
	mAlarmMaintenanceThread(nullptr),
	mAlarmMaintenanceThreadRunning(false),
	mAlarmCounter(0),
	mAlarmVolume(0)
{
	mDisplay.signalClockState(mClockState);
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
	if ((mClockState == ClockState::clkAlarm) || (mClockState == ClockState::clkSnooze))
	{
		if (keyboardInfo[KEY_1].mPressed)
		{
			mClockState = ClockState::clkNormal;
			mDisplay.signalClockState(mClockState);

			stopAlarmMaintenanceThread();
			mRadio.powerOff();
			if (mLight)
			{
				mLight->pwrOff();
			}

			mKeyboard.keyboardState(Hardware::KeyboardState::stNormal);
		}

		if (keyboardInfo[KEY_CENTRAL_L].mPressed || keyboardInfo[KEY_CENTRAL_R].mPressed)
		{
			mClockState = ClockState::clkSnooze;
			mDisplay.signalClockState(mClockState);
			mRadio.powerOff();
			if (mLight)
			{
				mLight->pwrOff();
			}

			mAlarmCounter = 0;
		}
	}


}

void AlarmClock::alarmNotify(int volume)
{
	LOG(INFO) << "Received alarmNotify";
	mAlarmVolume = volume;
	startAlarm();

	startAlarmMaintenanceThread();
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

void AlarmClock::startAlarm()
{
	mClockState = ClockState::clkAlarm;
	mDisplay.signalClockState(mClockState);

	mAlarmCounter = 0;

	mKeyboard.keyboardState(Hardware::KeyboardState::stAlarmActive);
	mRadio.slowPowerOn(mAlarmVolume);
	if (mLight)
	{
		mLight->pwrSlowOn();
	}

}

void AlarmClock::startAlarmMaintenanceThread()
{
	// Delete any previous running thread
	stopAlarmMaintenanceThread();

	mAlarmMaintenanceThreadRunning = true;

	mAlarmMaintenanceThread = new std::thread(&AlarmClock::alarmMaintenanceThread, this);
}

void AlarmClock::stopAlarmMaintenanceThread()
{
	mAlarmMaintenanceThreadRunning = false;

    if (mAlarmMaintenanceThread)
    {
    	mAlarmMaintenanceThread->join();

        delete mAlarmMaintenanceThread;
        mAlarmMaintenanceThread = nullptr;
    }
}

void AlarmClock::alarmMaintenanceThread()
{
	   while (mAlarmMaintenanceThreadRunning)
	   {
		   std::this_thread::sleep_for(std::chrono::seconds(1));
		   mAlarmCounter++;

		   switch(mClockState)
		   {
		   case ClockState::clkNormal:
			   break;
		   case ClockState::clkAlarm:
		   {
			   if (mAlarmCounter >= ALARM_TIME)
			   {
					mClockState = ClockState::clkNormal;
					mDisplay.signalClockState(mClockState);

					mKeyboard.keyboardState(Hardware::KeyboardState::stNormal);
					mRadio.powerOff();
					if (mLight)
					{
						mLight->pwrOff();
					}

				   mAlarmMaintenanceThreadRunning = false;
			   }
			   break;
		   }
		   case ClockState::clkSnooze:
		   {
			   if (mAlarmCounter >= SNOOZE_TIME)
			   {
				   startAlarm();
			   }

			   break;
		   }

		   }
	   }

}
} /* namespace App */
