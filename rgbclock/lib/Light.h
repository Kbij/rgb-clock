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
#include "AutoPowerOffTimer.h"
#include "AutoPowerOffDeviceIf.h"
#include "UpDownTimer.h"
#include "UpDownDeviceIf.h"

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>

namespace Hardware
{
class I2C;

class Light : public KeyboardObserverIf, public AutoPowerOffDeviceIf, public UpDownDeviceIf {
public:
	Light(I2C &i2c, uint8_t address);
	virtual ~Light();

	void pwrOn(bool slow = false);
	void pwrOff();
	void pwrToggle();

	void up(int step);
	void down(int step);

	void keyboardPressed(const KeyboardInfo& keyboardInfo);

	bool isAttached();

	// Prevent copy constructor
	Light(const Light& source) = delete;

private:
	void initiateFastUp();
	void initiateFastDown();
	void initiateSlowUp();

	void internalPwrOn();
	void internalPwrOff();
	std::atomic_bool mPowerOn;
	bool mPowerDownInitiated;
	RgbLed mRGBLed;
	int mCurrentLuminance;
	int mStoredLuminance;
	time_t mLastLong;
	bool mDimDown;
	std::mutex mDimmerMutex;

    UpDownTimer mUpDownTimer;
    AutoPowerOffTimer mAutoPowerOffTimer;
};

} /* namespace Hardware */
#endif /* LIGHT_H_ */
