/*
 * LCDisplay.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: koen
 */

#include "LCDisplay.h"
#include <chrono>
#include <thread>
#include <glog/logging.h>


const uint8_t RS = 0;
const uint8_t RW = 1;
const uint8_t E = 2;

LCDisplay::LCDisplay(I2C &i2c, uint8_t address):
	mIO(i2c, address),
	mPortA(0),
	mControlBus(0)
{
	init();
}

LCDisplay::~LCDisplay() {

}
void LCDisplay::clearDisplay()
{
	LOG(INFO) << "Clear Display";

	writeControl(0x01); // Display Clear

	// Wait 2ms after clear display
	std::chrono::milliseconds dura( 100 );
	std::this_thread::sleep_for( dura );
}

void LCDisplay::toggleBit()
{
	return;
//	mPortA == 0 ? mPortA = 0xFF: mPortA = 0;
//	writeData(mPortA);
	if (test)
	{
		mControlBus[E] = 0;
		test = false;
	}
	else
	{
		mControlBus[E] = 1;
		test = true;
	}
	//mIO.writeB(mControlBus.to_ulong());
	mIO.writeA(mPortA++);
}

void LCDisplay::init()
{
	LOG(INFO) << "Init Display";

	// Wait 40ms at startup
	std::chrono::milliseconds dura( 100 );
	std::this_thread::sleep_for( dura );

	// 8 bit interface
//	writeControl(0b00110000);
	writeControl(0x38);
	std::this_thread::sleep_for( dura );
//	writeControl(0b00110000);
	writeControl(0x38);
	std::this_thread::sleep_for( dura );

	// display control
	writeControl(0b00000110);
	std::this_thread::sleep_for( dura );

	// display on
	writeControl(0b00001100);
	std::this_thread::sleep_for( dura );

	// home
	writeControl(0b00000010);
	std::this_thread::sleep_for( dura );


	clearDisplay();

	// SEt CGRAM Address counter
	writeControl(0x38);
	//setDDRamAddress(0x80); //Fist Char position
	std::this_thread::sleep_for( dura );

	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('H');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('e');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('l');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('l');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('0');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData(' ');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('W');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('o');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('r');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('l');
	std::this_thread::sleep_for( dura );
	writeData(0x0);
	std::this_thread::sleep_for( dura );
	writeData('d');

}

void LCDisplay::setDDRamAddress(uint8_t addr)
{
	writeControl(addr);
}

void LCDisplay::writeData(uint8_t byte)
{
	mControlBus[RS] = 1; // Write Data
	mControlBus[E] = 1;
	mControlBus[RW] = 0;

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	mIO.writeA(byte);

	std::chrono::milliseconds dura( 10 );
	std::this_thread::sleep_for( dura );


	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());
	mControlBus[E] = 1;
	std::this_thread::sleep_for( dura );
	mIO.writeB(mControlBus.to_ulong());

}

void LCDisplay::writeControl(uint8_t byte)
{
	mControlBus[RS] = 0; // Write Control
	mControlBus[E] = 1;
	mControlBus[RW] = 0;

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	mIO.writeA(byte);

	std::chrono::milliseconds dura( 10 );
	std::this_thread::sleep_for( dura );


	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());
	mControlBus[E] = 1;
	std::this_thread::sleep_for( dura );
	mIO.writeB(mControlBus.to_ulong());

}

uint8_t LCDisplay::readControl()
{
	return 0;
}


