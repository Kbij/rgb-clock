/*
 * AlarmClock.h
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#ifndef ALARMCLOCK_H_
#define ALARMCLOCK_H_

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

class Light;

class AlarmClock : public Hardware::KeyboardObserverIf {
public:
	AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, UnitConfig addresses);
	virtual ~AlarmClock();

	void registerLight(Light *light);
	void unregisterLight(Light *light);

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo);

	bool isAttached();

	// Prevent copy constructor
	AlarmClock(const AlarmClock& source) = delete;

private:
	Hardware::Keyboard mKeyboard;
	Hardware::Radio mRadio;
	Hardware::ClockDisplay mDisplay;
	Light *mLight;
};

} /* namespace App */
#endif /* ALARMCLOCK_H_ */
