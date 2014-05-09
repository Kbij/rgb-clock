/*
 * AlarmClock.h
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#ifndef ALARMCLOCK_H_
#define ALARMCLOCK_H_
#include "AlarmObserverIf.h"

#include "lib/Keyboard.h"
#include "lib/KeyboardObserverIf.h"
#include "lib/Radio.h"
#include "lib/ClockDisplay.h"
#include "lib/MainboardControl.h"
#include "Config.h"
#include "AlarmSyncListenerIf.h"
#include <stdint.h>
#include <atomic>

namespace Hardware
{
class I2C;
class RTC;
class MainboardControl;
class Light;
}

namespace App
{
class AlarmManager;

class AlarmClock : public Hardware::KeyboardObserverIf, public App::AlarmObserverIf, public App::AlarmSyncListenerIf {
public:
	AlarmClock(Hardware::I2C &i2c, Hardware::RTC &rtc, Hardware::FMReceiver &fmReceiver, const SystemConfig &systemConfig, AlarmManager &alarmManager, Hardware::MainboardControl &mainboardControl, const UnitConfig& unitConfig);
	virtual ~AlarmClock();

	void registerLight(Hardware::Light *light);
	void unRegisterLight(Hardware::Light *light);

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state);

	void alarmNotify(int volume);
	std::string name();

	bool hasRegisteredLight();
	bool isAttached();

	void alarmSnooze();
	void alarmOff();
	// Prevent copy constructor
	AlarmClock(const AlarmClock& source) = delete;

private:
	void startAlarm();

	void startAlarmMaintenanceThread();
	void stopAlarmMaintenanceThread();
	void alarmMaintenanceThread();

	const UnitConfig& mUnitConfig;
	Hardware::Keyboard mKeyboard;
	Hardware::Radio mRadio;
	AlarmManager& mAlarmManager;
    Hardware::MainboardControl &mMainboardControl;
	Hardware::ClockDisplay mDisplay;
	Hardware::Light *mLight;
	std::recursive_mutex mLightMutex;
	std::atomic<ClockState> mClockState;
    std::thread* mAlarmMaintenanceThread;
    std::atomic_bool mAlarmMaintenanceThreadRunning;
    std::atomic_int mAlarmCounter;
    std::atomic_int mAlarmVolume;

};

} /* namespace App */
#endif /* ALARMCLOCK_H_ */
