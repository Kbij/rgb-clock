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
const uint8_t PRESCALER_REGISTER = 0xFE;
const uint8_t PRESCALER_VALUE = 30;

namespace Hardware
{
RgbLed::RgbLed(I2C &i2c, uint8_t address) :
			mPwmLedDriver(i2c, address),
			mRed(0),
			mGreen(0),
			mBlue(0),
			mHue(0),
			mSat(0),
			mLum(0)
{
}

RgbLed::~RgbLed() {
	mPwmLedDriver.powerOn(false);

	LOG(INFO) << "RgbLed destructor exit";
}

bool RgbLed::pwrOn()
{
	mPwmLedDriver.powerOn(true);

	return true;
}

bool RgbLed::pwrOff()
{
	mPwmLedDriver.powerOn(false);

	return true;
}

void RgbLed::hue(uint16_t value)
{
	mHue = value;
	hslToRgb();

	write();
}

void RgbLed::saturation(uint16_t value)
{
	mSat = value;
	hslToRgb();

	write();
}

void RgbLed::luminance(uint16_t value)
{
	if (value > MAX_RESOLUTION)
	{
		mLum = MAX_RESOLUTION;
	}
	else
	{
		mLum = value;
	}

	switch (mLum)
	{
	  case 759: mLum = 758;
	  break;
	  case 1175: mLum = 1174;
	  break;
	  case 1176: mLum = 1174;
	  break;
	}

	hslToRgb();

	write();
}

void RgbLed::write()
{
	mPwmLedDriver.pwmRGB(mRed, mGreen, mBlue);
}

bool RgbLed::isAttached()
{
	return mPwmLedDriver.isAttached();
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
//	LOG(INFO) << "Lum: " << mLum << ", mHue: " << mHue << ", mSat: " << mSat << ", R:" << (int) mRed << ", G:"<< (int) mGreen << ", B:" << (int)mBlue;
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
}
