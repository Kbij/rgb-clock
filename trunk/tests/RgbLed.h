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
private:
	I2C &mI2C;
	const uint8_t mWriteAddress;
	const uint8_t mReadAddress;
};

#endif /* RGBLED_H_ */
