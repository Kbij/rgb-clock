/*
 * AlarmClock.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "AlarmClock.h"
#include "lib/Light.h"
#include "lib/FMReceiver.h"
#include "lib/MainboardControl.h"
#include "AlarmManager.h"
#include <glog/logging.h>
#include <pthread.h>

namespace {
const int SNOOZE_TIME = 9 * 60; // Snooze interval: 9min
const int ALARM_TIME = 30 * 60; // Alarm time: 30 min
}

namespace App {

AlarmClock::AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, const SystemConfig &systemConfig, AlarmManager &alarmManager, Hardware::MainboardControl &mainboardControl, const UnitConfig& unitConfig) :
	mUnitConfig(unitConfig),
	mKeyboard(i2c, unitConfig.mKeyboard, mainboardControl),
	mRadio(i2c, unitConfig.mAmplifier, fmReceiver, systemConfig.mFrequency),
	mAlarmManager(alarmManager),
	mMainboardControl(mainboardControl),
	mDisplay(i2c, mKeyboard, mAlarmManager, systemConfig.mHardwareRevision, unitConfig),
	mLight(nullptr),
	mLightMutex(),
	mClockState(ClockState::clkNormal),
	mAlarmMaintenanceThread(nullptr),
	mAlarmMaintenanceThreadRunning(false),
	mAlarmCounter(0),
	mAlarmVolume(0)
{
	mDisplay.signalClockState(mClockState);
	mKeyboard.registerKeyboardObserver(this);
	mKeyboard.registerKeyboardObserver(&mRadio);
	mKeyboard.registerKeyboardObserver(&mMainboardControl);
	mRadio.registerRadioObserver(&mDisplay);
	mAlarmManager.registerAlarmObserver(this);
}

AlarmClock::~AlarmClock()
{
	mAlarmManager.unRegisterAlarmObserver(this);

	mKeyboard.unRegisterKeyboardObserver(&mMainboardControl);
	mKeyboard.unRegisterKeyboardObserver(&mRadio);
	mKeyboard.unRegisterKeyboardObserver(this);
	mRadio.unRegisterRadioObserver(&mDisplay);

	std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);
	if (mLight)
	{
		unRegisterLight(mLight);
	}
}

void AlarmClock::registerLight(Hardware::Light *light)
{
	std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

	if (light)
	{
		mLight = light;
		mKeyboard.registerKeyboardObserver(light);
	}
}

void AlarmClock::unRegisterLight(Hardware::Light *light)
{
	std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

	if (light)
	{
		mKeyboard.unRegisterKeyboardObserver(light);
		mLight = nullptr;
	}
}

void AlarmClock::keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state)
{
	if ((mClockState == ClockState::clkAlarm) || (mClockState == ClockState::clkSnooze))
	{
		if (keyboardInfo[KEY_LEFT].mShortPressed)
		{
			mClockState = ClockState::clkNormal;
			mDisplay.signalClockState(mClockState);

			stopAlarmMaintenanceThread();
			mRadio.powerOff();

			std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

			if (mLight)
			{
				mLight->pwrOff();
			}

			mKeyboard.keyboardState(Hardware::KeyboardState::stNormal);
		}

		if (keyboardInfo[KEY_CENTRAL_L].mShortPressed || keyboardInfo[KEY_CENTRAL_R].mShortPressed)
		{
			mClockState = ClockState::clkSnooze;
			mDisplay.signalClockState(mClockState);
			mRadio.powerOff();

			std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

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
	std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

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

	std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

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
	pthread_setname_np(pthread_self(), "AlarmClock");

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

						std::lock_guard<std::recursive_mutex> lk_guard(mLightMutex);

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
