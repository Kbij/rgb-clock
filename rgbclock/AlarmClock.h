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
#include <stdint.h>

namespace Hardware
{
class I2C;
}

namespace App
{
class Light;

struct Addresses
{
	uint8_t mLight;
	uint8_t mKeyboard;
	uint8_t mAmplifier;
	uint8_t mLightSensor;
	uint8_t mLCD;
};

class AlarmClock : public Hardware::KeyboardObserverIf {
public:
	AlarmClock(Hardware::I2C &i2c, Hardware::FMReceiver & fmReceiver, Addresses addresses);
	virtual ~AlarmClock();

	void registerLight(Light *light);
	void unregisterLight();

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo);
private:
	Addresses mAddresses;
	Hardware::Keyboard mKeyboard;
	Hardware::Radio mRadio;
	Hardware::ClockDisplay mDisplay;
	Light *mLight;
	int mRadioVolume;
};

} /* namespace App */
#endif /* ALARMCLOCK_H_ */
