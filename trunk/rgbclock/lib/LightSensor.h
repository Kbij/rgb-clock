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

namespace Hardware
{
class LightSensor {
public:
	LightSensor(I2C &i2c, uint8_t address);
	virtual ~LightSensor();

	double lux();

private:
	bool init();
	double calculateLux(uint16_t ch0, uint16_t ch1);
	I2C &mI2C;
	const uint8_t mAddress;
};
}
#endif /* LIGHTSENSOR_H_ */
