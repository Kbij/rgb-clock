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

namespace Hardware
{
class RgbLed {
public:
	RgbLed(I2C &i2c, uint8_t address);
	virtual ~RgbLed();

	bool pwrOn();
	bool pwrOff();

	void intensity(uint16_t value);
	void red(uint16_t value);
	void green(uint16_t value);
	void blue(uint16_t value);

	void hue(uint16_t value);
	void saturation(uint16_t value);
	void luminance(uint16_t value);

	void write();

	bool isAttached();
private:
	bool init();
	void hslToRgb();
	double hue2rgb(double p, double q, double t);

	I2C &mI2C;
	const uint8_t mAddress;
	uint16_t mIntensity;
	uint16_t mRed;
	uint16_t mGreen;
	uint16_t mBlue;

	uint16_t mHue;
	uint16_t mSat;
	uint16_t mLum;
};
}

#endif /* RGBLED_H_ */
