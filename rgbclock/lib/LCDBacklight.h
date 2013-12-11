/*
 * LCDBacklight.h
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#ifndef LCDBACKLIGHT_H_
#define LCDBACKLIGHT_H_
#include "LightSensor.h"
#include "PwmLedDriver.h"

#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>

namespace Hardware {
class I2C;

class LCDBacklight {
public:
	LCDBacklight(I2C &i2c, uint8_t pwmAddress, uint8_t lightSensorAddress);
	virtual ~LCDBacklight();

	void signalUserActivity();
private:
	void startBackLightThread();
	void stopBackLightThread();
	void backLightThread();

	PwmLedDriver mPwmDriver;
	LightSensor mLightSensor;
	std::atomic_int mUserActivityTimer;

    std::thread* mBackLightThread;
    std::atomic_bool mBackLightThreadRunning;
};

} /* namespace Hardware */
#endif /* LCDBACKLIGHT_H_ */
