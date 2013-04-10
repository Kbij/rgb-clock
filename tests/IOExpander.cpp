/*
 * IOExpander.cpp
 *
 *  Created on: Apr 10, 2013
 *      Author: koen
 */

#include "IOExpander.h"

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
	bool result = true;

	std::vector<uint8_t> buffer;
	buffer.push_back(0x12); // Register: GPIOA (BANK = 0)
	buffer.push_back(byte);

	return mI2C.writeDataSync(mAddress, buffer);
}

bool IOExpander::writeB(uint8_t byte)
{
	bool result = true;

	std::vector<uint8_t> buffer;
	buffer.push_back(0x13); // Register: GPIOB (BANK = 0)
	buffer.push_back(byte);

	return mI2C.writeDataSync(mAddress, buffer);
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


}
