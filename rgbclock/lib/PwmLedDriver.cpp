/*
 * PwmLedDriver.cpp
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#include "PwmLedDriver.h"
#include "I2C.h"
#include <vector>
#include <glog/logging.h>
#include <iostream>

const uint16_t MAX_RESOLUTION = 4000;
const uint8_t PRESCALER_REGISTER = 0xFE;
/*
 * Prescaler value = (25Mhz/(4096 * OutputFreq)) - 1
 * OutputFreq = 2Khz
 *  Value = (25000000/(4096*2000)) - 1 = 2
 */
const uint8_t PRESCALER_VALUE = 2;
namespace Hardware {

PwmLedDriver::PwmLedDriver(I2C &i2c, uint8_t address):
		mI2C(i2c),
		mAddress(address)
{
	mI2C.registerAddress(address, "PWM");

	init();
}

PwmLedDriver::~PwmLedDriver()
{
	chipSleep();
}

void PwmLedDriver::powerOn(bool powerOn)
{
	if (powerOn)
	{
		init();
	}
	else
	{
		chipSleep();
	}
}

void PwmLedDriver::pwmValue(PwmChannel channel, uint16_t value)
{
	int16_t offTime = (4095 / MAX_RESOLUTION) * value;
	if (value == MAX_RESOLUTION)
	{
		offTime = 4095;
	}
	std::vector<uint8_t> buffer;

	// Write the start address of the registers
	switch (channel)
	{
	case PwmChannel::Channel1: buffer.push_back(0x06);
		break;
	case PwmChannel::Channel2: buffer.push_back(0x0A);
		break;
	case PwmChannel::Channel3: buffer.push_back(0x0E);
		break;
	}

	buffer.push_back(0x00); // Channel LedOn, byte 0 value
	buffer.push_back(0x00); // Channel LedOn, byte 1 value
	buffer.push_back(offTime & 0xFF); // Channel LedOff, byte 0 value
	buffer.push_back((offTime >> 8) & 0x0F); // Channel LedOff, byte 1 value
	mI2C.writeDataSync(mAddress, buffer);
}

bool PwmLedDriver::isAttached()
{
	uint8_t prescaler;
	mI2C.readByteSync(mAddress, PRESCALER_REGISTER, prescaler);

	return (prescaler >= (PRESCALER_VALUE -1)) && (prescaler <= (PRESCALER_VALUE + 1));
}

void PwmLedDriver::init()
{
	// Set the prescaler
	std::vector<uint8_t> initBuffer;
	initBuffer.push_back(PRESCALER_REGISTER); // PreScaler register

	initBuffer.push_back(PRESCALER_VALUE); // Value of Prescaler

	mI2C.writeDataSync(mAddress, initBuffer);

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
	mI2C.writeDataSync(mAddress, initBuffer);

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

	mI2C.writeDataSync(mAddress, initBuffer);
}

void PwmLedDriver::chipSleep()
{
	std::vector<uint8_t> buffer;
	// Set Mode1
	buffer.push_back(0x00); //MODE1 Register
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
	buffer.push_back(0b00110000); //MODE1 register Value
	mI2C.writeDataSync(mAddress, buffer);
}

} /* namespace Hardware */
