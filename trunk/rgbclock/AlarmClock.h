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
#include "Config.h"
#include <stdint.h>

namespace Hardware
{
class I2C;
}

namespace App
{

class AlarmManager;
class Light;

class AlarmClock : public Hardware::KeyboardObserverIf, public App::AlarmObserverIf {
public:
	AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, AlarmManager &alarmManager, UnitConfig unitConfig);
	virtual ~AlarmClock();

	void registerLight(Light *light);
	void unRegisterLight(Light *light);

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo, Hardware::KeyboardState state);

	void alarmNotify();
	std::string name();

	bool hasRegisteredLight();
	bool isAttached();


	// Prevent copy constructor
	AlarmClock(const AlarmClock& source) = delete;

private:
	UnitConfig mUnitConfig;
	Hardware::Keyboard mKeyboard;
	Hardware::Radio mRadio;
	Hardware::ClockDisplay mDisplay;
	Light *mLight;
	AlarmManager& mAlarmManager;
};

} /* namespace App */
#endif /* ALARMCLOCK_H_ */
