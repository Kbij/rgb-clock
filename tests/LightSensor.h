/*
 * LichtSensor.h
 *
 *  Created on: Apr 17, 2013
 *      Author: koen
 */

#ifndef LIGHTSENSOR_H_
#define LIGHTSENSOR_H_
#include "I2C.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>

class LightSensor {
public:
	LightSensor(I2C &i2c, uint8_t address);
	virtual ~LightSensor();

	uint16_t intensity();

private:
	bool init();
	void startReadThread();
	void stopReadThread();
	void readThread();
	uint32_t calculateLux(uint16_t ch0, uint16_t ch1);
	I2C &mI2C;
	const uint8_t mAddress;

	uint16_t mIntensity;
	std::mutex mIntensityMutex;
    std::thread* mReadThread;
    std::atomic_bool mReadThreadRunning;


};

#endif /* LIGHTSENSOR_H_ */
