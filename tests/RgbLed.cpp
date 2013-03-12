/*
 * RgbLed.cpp
 *
 *  Created on: Mar 12, 2013
 *      Author: koen
 */

#include "RgbLed.h"
#include <vector>


RgbLed::RgbLed(I2C &i2c, uint8_t writeAddress) :
			mI2C(i2c),
			mWriteAddress(writeAddress),
			mReadAddress(writeAddress | 0x01)
{
// Reset PCA9685
	mI2C.writeByteSync(0x00, 0x06); // General Call Adress, Send SWRST data byte 1):

	std::vector<uint8_t> initBuffer;

// Set the prescaler
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
