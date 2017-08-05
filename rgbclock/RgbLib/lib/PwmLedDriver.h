/*
 * PwmLedDriver.h
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#ifndef PWMLEDDRIVER_H_
#define PWMLEDDRIVER_H_
#include <stdint.h>

namespace Hardware {
class I2C;

class PwmLedDriver {

public:
	PwmLedDriver(I2C &i2c, uint8_t address);
	virtual ~PwmLedDriver();

	void powerOn(bool powerOn);
	void pwmSingle(uint16_t value);
	void pwmRGB(uint16_t red, uint16_t green, uint16_t blue);

	bool isAttached();
private:
	void init();
	void chipSleep();

	I2C &mI2C;
	const uint8_t mAddress;
};

} /* namespace Hardware */
#endif /* PWMLEDDRIVER_H_ */
