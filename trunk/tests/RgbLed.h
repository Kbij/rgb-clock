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
	void red(uint8_t value);
	void green(uint8_t value);
	void blue(uint8_t value);

	void hue(uint8_t value);
	void saturation(uint8_t value);
	void luminance(uint8_t value);

	void write();
private:
	void hslToRgb();
	double hue2rgb(double p, double q, double t);

	I2C &mI2C;
	const uint8_t mWriteAddress;
	const uint8_t mReadAddress;
	uint8_t mIntensity;
	uint8_t mRed;
	uint8_t mGreen;
	uint8_t mBlue;

	uint8_t mHue;
	uint8_t mSat;
	uint8_t mLum;
};

#endif /* RGBLED_H_ */
