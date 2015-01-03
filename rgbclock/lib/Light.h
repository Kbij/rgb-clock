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
#include <memory>

namespace Hardware
{
class I2C;

enum class State {
	PwrOn,
	PwrOff,
	SlowUp,
	SlowDown,
	FastUp,
	FastDown
};

class Light : public KeyboardObserverIf {
public:
	Light(I2C &i2c, uint8_t address);
	virtual ~Light();

	void pwrOn();
	void pwrSlowOn();
	void pwrOff();

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state);

	bool isAttached();

	// Prevent copy constructor
	Light(const Light& source) = delete;

private:
	void pwrSlowOn(int startLum);
	void pwrOff(bool autoPowerOff);
	void pwrToggle();
	void initiateFastUp();
	void initiateFastDown();
	void initiateSlowUp(int start);

	void startDimmerThread();
	void stopDimmerThread();
	void startAutoOffThread();
	void stopAutoOffThread();
	void dimmerThread();
	void autoOffThread();
	std::atomic<State> mState;
	RgbLed mRGBLed;
	std::atomic_int mLuminance;
	time_t mLastLong;
	bool mDimDown;
	std::mutex mLedMutex;
	std::mutex mDimmerMutex;
	std::mutex mThreadMutex;
    std::unique_ptr<std::thread> mDimmerThread;
    std::atomic_bool mDimmerThreadRunning;

    std::unique_ptr<std::thread> mAutoOffThread;
    std::atomic_bool mAutoOffThreadRunning;
};

} /* namespace Hardware */
#endif /* LIGHT_H_ */
