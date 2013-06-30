/*
 * IOExpander.cpp
 *
 *  Created on: Apr 10, 2013
 *      Author: koen
 */

#include "IOExpander.h"
const uint8_t DIRA  = 0x00;
const uint8_t DIRB  = 0x01;

const uint8_t GPIOA = 0x12;
const uint8_t GPIOB = 0x13;

const uint8_t GPPUA = 0x0C;
const uint8_t GPPUB = 0x0D;

namespace Hardware
{
IOExpander::IOExpander(I2C &i2c, uint8_t address) :
			mI2C(i2c),
			mAddress(address)
{
	init();
}

IOExpander::~IOExpander() {

}
bool IOExpander::writeA(uint8_t byte)
{
	std::vector<uint8_t> buffer;
	buffer.push_back(GPIOA); // Register: GPIOA (BANK = 0)
	buffer.push_back(byte);

	return mI2C.writeDataSync(mAddress, buffer);
}

bool IOExpander::writeB(uint8_t byte)
{
	std::vector<uint8_t> buffer;
	buffer.push_back(GPIOB); // Register: GPIOB (BANK = 0)
	buffer.push_back(byte);

	return mI2C.writeDataSync(mAddress, buffer);
}

bool IOExpander::directionA(DataDirection direction)
{
	std::vector<uint8_t> initBuffer;
	if (direction == DataDirection::dirOut)
	{
		initBuffer.push_back(DIRA); // Register DirA
		initBuffer.push_back(0x00); // All Output
		mI2C.writeDataSync(mAddress, initBuffer);

		initBuffer.clear();
		initBuffer.push_back(GPPUA); // PullUp PortA
		initBuffer.push_back(0x00); // No Pullup
		mI2C.writeDataSync(mAddress, initBuffer);

	}
	else
	{
		initBuffer.push_back(DIRA); // Register DirA
		initBuffer.push_back(0xFF); // All Input
		mI2C.writeDataSync(mAddress, initBuffer);

		initBuffer.clear();
		initBuffer.push_back(GPPUA); // PullUp PortA
		initBuffer.push_back(0xFF); // All Pullup
		mI2C.writeDataSync(mAddress, initBuffer);
	}
	return true;
}

bool IOExpander::directionB(DataDirection direction)
{
	std::vector<uint8_t> initBuffer;
	if (direction == DataDirection::dirOut)
	{
		initBuffer.push_back(DIRB); // Register DirB
		initBuffer.push_back(0x00); // All Output
		mI2C.writeDataSync(mAddress, initBuffer);

		initBuffer.clear();
		initBuffer.push_back(GPPUB); // PullUp PortB
		initBuffer.push_back(0x00); // No Pullup
		mI2C.writeDataSync(mAddress, initBuffer);

	}
	else
	{
		initBuffer.push_back(DIRB); // Register DirB
		initBuffer.push_back(0xFF); // All Input
		mI2C.writeDataSync(mAddress, initBuffer);

		initBuffer.clear();
		initBuffer.push_back(GPPUB); // PullUp PortB
		initBuffer.push_back(0xFF); // All Pullup
		mI2C.writeDataSync(mAddress, initBuffer);
	}
	return true;
}

bool IOExpander::readA(uint8_t& byte)
{
	return mI2C.readByteSync(mAddress, GPIOA, byte);
}

bool IOExpander::readB(uint8_t& byte)
{
	return mI2C.readByteSync(mAddress, GPIOB, byte);
}

bool IOExpander::init()
{
	bool result = true;
	std::vector<uint8_t> initBuffer;
	initBuffer.push_back(0x0A); // Register: IOCON (After POR BANK = 0)
	/*
	 * Bits:
	 * 7:0: Bank
	 * 6:0: Mirror
	 * 5:1: SeqOp
	 * 4:0: DisSlw
	 * 3:0: Haen
	 * 2:0: Odr
	 * 1:0: IntPol
	 * 0:0: Unimplemented
	 */
	initBuffer.push_back(0b00100000);

	result = result && mI2C.writeDataSync(mAddress, initBuffer);

	result = result & directionA(DataDirection::dirOut);
	result = result & directionB(DataDirection::dirOut);

	return result;
}
}
