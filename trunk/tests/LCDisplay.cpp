/*
 * LCDisplay.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: koen
 */

#include "LCDisplay.h"
const uint8_t RS = 0;
const uint8_t RW = 1;
const uint8_t E = 2;

LCDisplay::LCDisplay(I2C &i2c, uint8_t address):
	mIO(i2c, address)
{


}

LCDisplay::~LCDisplay() {

}

void LCDisplay::toggleBit()
{
	mControlBus[RS] ? mControlBus[RS] = 0: mControlBus[RS] = 1;
	mIO.writeA(mControlBus.to_ulong());
}
void LCDisplay::init()
{
	mControlBus.reset();
	mIO.writeB(mControlBus.to_ulong());
}
void LCDisplay::writeData(uint8_t byte)
{
	mControlBus[RS] = 1;
	mControlBus[E] = 1;
	mControlBus[RW] = 0;

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	mIO.writeA(byte);

	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());
}

void LCDisplay::writeControl(uint8_t byte)
{

}

