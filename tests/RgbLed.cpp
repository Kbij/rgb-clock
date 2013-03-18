/*
 * RgbLed.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: koen
 */

#include "RgbLed.h"
#include <vector>
#include <glog/logging.h>

RgbLed::RgbLed(I2C &i2c, uint8_t writeAddress) :
			mI2C(i2c),
			mWriteAddress(writeAddress),
			mReadAddress(writeAddress | 0x01),
			mIntensity(0),
			mRed(0),
			mGreen(0),
			mBlue(0),
			mHue(0),
			mSat(0),
			mLum(0)
{
// Reset PCA9685
	mI2C.writeByteSync(0x00, 0x06); // General Call Address, Send SWRST data byte 1):

// Set the prescaler
	std::vector<uint8_t> initBuffer;
	initBuffer.push_back(0xFE); // PreScaler register
	/*
	 * Prescaler value = (25Mhz/(4096 * OutputFreq)) - 1
	 * OutputFreq = 200Hz
	 *  Value = (25000000/(4096*200)) - 1 = 30
	 */
	initBuffer.push_back(30); // Value of Prescaler

// Set Mode1
	initBuffer.clear();
	initBuffer.push_back(0x00); //MODE1 Register
	/*
	 * Bits:
	 * 7:0: Restart disabled
	 * 6:0: Internal Clock
	 * 5:1: Register Auto Increment enabled
	 * 4:0: Normal Mode
	 * 3:0: Sub1
	 * 2:0: Sub2
	 * 1:0: Sub3
	 * 0:0: Device does not listen to All Call
	 */
	initBuffer.push_back(0b00100000); //MODE1 register Value
	mI2C.writeDataSync(mWriteAddress, initBuffer);

// Set Mode2
	initBuffer.clear();
	initBuffer.push_back(0x01); //MODE2 Register
	/*
	 * Bits:
	 * 7:0: Reserved
	 * 6:0: Reserved
	 * 5:0: Reserved
	 * 4:0: Inv
	 * 3:0: OCH
	 * 2:0: OutDrv
	 * 1:0: OutNe1
	 * 0:0: OutNe0
	*/
	initBuffer.push_back(0b00000000); //MODE2 register Value
	mI2C.writeDataSync(mWriteAddress, initBuffer);
}

RgbLed::~RgbLed() {
	pwrOff();
}

bool RgbLed::pwrOn()
{
	return true;
}

bool RgbLed::pwrOff()
{
	return true;
}
void RgbLed::intensity(uint8_t value)
{
	mIntensity = value;
}
void RgbLed::red(uint8_t value)
{
	mRed = value;
}

void RgbLed::green(uint8_t value)
{
	mGreen = value;
}

void RgbLed::blue(uint8_t value)
{
	mBlue = value;
}
void RgbLed::hue(uint8_t value)
{
	mHue = value;
	hslToRgb();
}

void RgbLed::saturation(uint8_t value)
{
	mSat = value;
	hslToRgb();
}

void RgbLed::luminance(uint8_t value)
{
	mLum = value;
	hslToRgb();
}


void RgbLed::write()
{
/*
	int32_t offTime = (4095 / 100) * mIntensity;
	if (mIntensity == 100)
	{
		offTime = 4095;
	}

	LOG(INFO) << "Calculcated offTime = " << offTime;
	std::vector<uint8_t> buffer;
	buffer.push_back(0xFA); // All LedOn, byte0 = start Register
	buffer.push_back(0x00); // LedOn, byte 0 value
	buffer.push_back(0x00); // LedOn, byte 1 value
	buffer.push_back(offTime & 0xFF); // LedOff, byte 0 value

	buffer.push_back((offTime >> 8) & 0x0F); // LedOff, byte 1 value
*/

	int16_t redOffTime = (4095 / 1000) * mRed;
	if (mRed == 1000)
	{
		redOffTime = 4095;
	}
	int16_t greenOffTime = (4095 / 1000) * mGreen;
	if (mGreen == 1000)
	{
		greenOffTime = 4095;
	}
	int16_t blueOffTime = (4095 / 1000) * mBlue;
	if (mBlue == 1000)
	{
		blueOffTime = 4095;
	}

	std::vector<uint8_t> buffer;
	buffer.push_back(0x06); // All LedOn, byte0 = start Register

	buffer.push_back(0x00); // Red LedOn, byte 0 value
	buffer.push_back(0x00); // Red LedOn, byte 1 value
	buffer.push_back(redOffTime & 0xFF); // Red LedOff, byte 0 value
	buffer.push_back((redOffTime >> 8) & 0x0F); // Red LedOff, byte 1 value

	buffer.push_back(0x00); // Green LedOn, byte 0 value
	buffer.push_back(0x00); // Green LedOn, byte 1 value
	buffer.push_back(greenOffTime & 0xFF); // Green LedOff, byte 0 value
	buffer.push_back((greenOffTime >> 8) & 0x0F); // Green LedOff, byte 1 value

	buffer.push_back(0x00); // Blue LedOn, byte 0 value
	buffer.push_back(0x00); // Blue LedOn, byte 1 value
	buffer.push_back(blueOffTime & 0xFF); // Blue LedOff, byte 0 value
	buffer.push_back((blueOffTime >> 8) & 0x0F); // Blue LedOff, byte 1 value


	mI2C.writeDataSync(mWriteAddress, buffer);
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 255] and
 * returns r, g, and b in the set [0, 255].
 *
 */
void RgbLed::hslToRgb()
{
	double h = (double) mHue / 1000;
	double l = (double) mLum / 1000;
	double s = (double) mSat / 1000;

	if(mSat == 0) // achromatic
    {
    	mRed = l * 1000;
    	mGreen = l * 1000;
        mBlue = l * 1000;
    }
    else
    {


        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        double r = hue2rgb(p, q, double(h + (0.33333)));
        double g = hue2rgb(p, q, h);
        double b = hue2rgb(p, q, double(h - (0.33333)));

        mRed = r * 1000;
        mGreen = g * 1000;
        mBlue = b * 1000;
    }
	LOG(INFO) << "HSL Converted to Red:" << (int) mRed << ", Green:"<< (int) mGreen << ", Blue:" << (int)mBlue;

}

double RgbLed::hue2rgb(double p, double q, double t)
{
    if (t < 0)
    {
    	t += 1;
    }

    if (t > 1)
    {
    	t -= 1;
    }

    if (t < 0.16666)
    {
    	return p + (q - p) * 6 * t;
    }

    if (t < 0.5)
    {
    	return q;
    }

    if (t < 0.66666)
    {
    	return p + (q - p) * ((0.66666) - t) * 6;
    }

    return p;
}