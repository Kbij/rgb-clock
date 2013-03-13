/*
 * RgbLed.h
 *
 *  Created on: Mar 12, 2013
 *      Author: koen
 */

#ifndef RGBLED_H_
#define RGBLED_H_
#include "I2C.h"
#include <stdint.h>


class RgbLed {
public:
	RgbLed(I2C &i2c, uint8_t writeAddress);
	virtual ~RgbLed();

	bool pwrOn();
	bool pwrOff();

	void intensity(uint8_t value);
	void write();
private:
	I2C &mI2C;
	const uint8_t mWriteAddress;
	const uint8_t mReadAddress;
	uint8_t mIntensity;
};

#endif /* RGBLED_H_ */
