/*
 * RgbLed.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: koen
 */

#include "RgbLed.h"
#include <vector>
#include <glog/logging.h>
#include <iostream>

const uint16_t MAX_RESOLUTION = 4000;

RgbLed::RgbLed(I2C &i2c, uint8_t address) :
			mI2C(i2c),
			mAddress(address),
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
	init();
}

RgbLed::~RgbLed() {
	pwrOff();
}

bool RgbLed::pwrOn()
{
	// Re-initialize after a possible power failure
	init();
	// Write the current RGB values to the led
	write();
	return true;
}

bool RgbLed::pwrOff()
{
	std::vector<uint8_t> initBuffer;
	// Set Mode1
	initBuffer.push_back(0x00); //MODE1 Register
	/*
	 * Bits:
	 * 7:0: Restart disabled
	 * 6:0: Internal Clock
	 * 5:1: Register Auto Increment enabled
	 * 4:1: Sleep Mode
	 * 3:0: Sub1
	 * 2:0: Sub2
	 * 1:0: Sub3
	 * 0:0: Device does not listen to All Call
	 */
	initBuffer.push_back(0b00110000); //MODE1 register Value
	return mI2C.writeDataSync(mAddress, initBuffer);
}

void RgbLed::intensity(uint16_t value)
{
	mIntensity = value;
}

void RgbLed::red(uint16_t value)
{
	mRed = value;
}

void RgbLed::green(uint16_t value)
{
	mGreen = value;
}

void RgbLed::blue(uint16_t value)
{
	mBlue = value;
}

void RgbLed::hue(uint16_t value)
{
	mHue = value;
	hslToRgb();
}

void RgbLed::saturation(uint16_t value)
{
	mSat = value;
	hslToRgb();
}

void RgbLed::luminance(uint16_t value)
{
	mLum = value;
	hslToRgb();
}

void RgbLed::write()
{
	int16_t redOffTime = (4095 / MAX_RESOLUTION) * mRed;
	if (mRed == MAX_RESOLUTION)
	{
		redOffTime = 4095;
	}
	int16_t greenOffTime = (4095 / MAX_RESOLUTION) * mGreen;
	if (mGreen == MAX_RESOLUTION)
	{
		greenOffTime = 4095;
	}
	int16_t blueOffTime = (4095 / MAX_RESOLUTION) * mBlue;
	if (mBlue == MAX_RESOLUTION)
	{
		blueOffTime = 4095;
	}

	std::vector<uint8_t> buffer;
	buffer.push_back(0x06); // Red LedOn, byte0 start Register

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

	mI2C.writeDataSync(mAddress, buffer);
}

bool RgbLed::init()
{
	bool result = true;

	// Set the prescaler
	std::vector<uint8_t> initBuffer;
	initBuffer.push_back(0xFE); // PreScaler register
	/*
     * Prescaler value = (25Mhz/(4096 * OutputFreq)) - 1
	 * OutputFreq = 200Hz
     *  Value = (25000000/(4096*200)) - 1 = 30
	 */
	initBuffer.push_back(30); // Value of Prescaler

	result = result && mI2C.writeDataSync(mAddress, initBuffer);

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
	result = result && mI2C.writeDataSync(mAddress, initBuffer);

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
	return result && mI2C.writeDataSync(mAddress, initBuffer);
}

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes mHue, mSat, and mLum are contained in the set [0, MAX_RESOLUTION] and
 * returns mRed, mGreen, and mBlue in the set [0, MAX_RESOLUTION].
 *
 */
void RgbLed::hslToRgb()
{
	double h = (double) mHue / MAX_RESOLUTION;
	double l = (double) mLum / MAX_RESOLUTION;
	double s = (double) mSat / MAX_RESOLUTION;

	if(mSat == 0) // achromatic
    {
    	mRed = l * MAX_RESOLUTION;
    	mGreen = l * MAX_RESOLUTION;
        mBlue = l * MAX_RESOLUTION;
    }
    else
    {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        double r = hue2rgb(p, q, double(h + 1.0/3.0));
        double g = hue2rgb(p, q, h);
        double b = hue2rgb(p, q, double(h - 1.0/3.0));

        mRed = r * MAX_RESOLUTION;
        mGreen = g * MAX_RESOLUTION;
        mBlue = b * MAX_RESOLUTION;
    }
	VLOG(1) << "HSL Converted to Red:" << (int) mRed << ", Green:"<< (int) mGreen << ", Blue:" << (int)mBlue;
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

    if (t < 1.0/6.0)
    {
    	return p + (q - p) * 6 * t;
    }

    if (t < 0.5)
    {
    	return q;
    }

    if (t < 2.0/3.0)
    {
    	return p + (q - p) * ((2.0/3.0) - t) * 6;
    }

    return p;
}
