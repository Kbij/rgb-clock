/*
 * Light.h
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "lib/I2C.h"
#include "lib/RgbLed.h"
#include "lib/KeyboardObserverIf.h"
#include <stdint.h>

namespace Hardware
{
class I2C;
}

namespace App {

enum class State {
	PwrOn,
	PwrOff,
	SlowUp,
	SlowDown,
	FastUp,
	FastDown
};

class Light : public Hardware::KeyboardObserverIf {
public:
	Light(Hardware::I2C &i2c, uint8_t address);
	virtual ~Light();

	void pwrOn();
	void pwrOff();
	void pwrToggle();

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo);
private:
	State mState;
	Hardware::RgbLed mRGBLed;
	int mLuminance;
	time_t mLastLong;
	bool mDimDown;
};

} /* namespace App */
#endif /* LIGHT_H_ */
