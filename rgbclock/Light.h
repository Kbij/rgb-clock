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
#include <mutex>
#include <atomic>
#include <thread>

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
	void pwrSlowOn();
	void pwrOff();
	void pwrToggle();

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state);

	bool isAttached();

	// Prevent copy constructor
	Light(const Light& source) = delete;

private:
	void initiateFastUp();
	void initiateFastDown();
	void initiateSlowUp();

	void startDimmerThread();
	void stopDimmerThread();
	void dimmerThread();
	std::atomic<State> mState;
	Hardware::RgbLed mRGBLed;
	std::atomic_int mLuminance;
	time_t mLastLong;
	bool mDimDown;
	std::mutex mLedMutex;
    std::thread* mDimmerThread;
    std::atomic_bool mDimmerThreadRunning;
};

} /* namespace App */
#endif /* LIGHT_H_ */