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

	writeControl(0b00000001); // Display Clear

	// Wait 2ms after clear display
	std::chrono::milliseconds dura( 2 );
	std::this_thread::sleep_for( dura );
}

void LCDisplay::writeText(uint8_t pos, std::string text)
{
	writeControl(0b00000110);
	setDDRamAddress(pos); //Fist Char position
	for (int i = 0; i < text.length(); ++i)
	{
		writeData(text[i]);
	}
}

void LCDisplay::checkControlBus()
{
	uint8_t byte;
	mIO.directionB(IOExpander::DataDirection::dirIn);

	mIO.readB(byte);
	LOG(INFO) << "ConstrolBus: 0x" << std::hex << (int) byte;
}

void LCDisplay::init()
{
	LOG(INFO) << "Init Display";

	// Wait 40ms at startup
	std::chrono::milliseconds dura( 40 );
	std::this_thread::sleep_for( dura );

	// 8 bit interface
	writeControl(0b00110000);
//	std::this_thread::sleep_for( dura );
	writeControl(0b00110000);
//	std::this_thread::sleep_for( dura );

	// display on
	writeControl(0b00001100);
//	std::this_thread::sleep_for( dura );

	// home
	writeControl(0b00000010);
//	std::this_thread::sleep_for( dura );

	clearDisplay();

	// display control
	writeControl(0b00000110);
//	std::this_thread::sleep_for( dura );

	// SEt CGRAM Address counter
	//writeControl(0x38);
	setDDRamAddress(0); //Fist Char position

	writeData('H');
	writeData('e');
	writeData('l');
	writeData('l');
	writeData('o');
	writeData(' ');
	writeData('W');
	writeData('o');
	writeData('r');
	writeData('l');
	writeData('d');
	writeData('!');
	writeData('!');

}

void LCDisplay::setDDRamAddress(uint8_t addr)
{
	writeControl(0x80 | addr);
}

void LCDisplay::writeData(uint8_t byte)
{
	//byte = reverse(byte);
	mControlBus[RS] = 1; // Write Data
	mControlBus[E] = 1;
	mControlBus[RW] = 0;
	//E/RW/RS
	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	mIO.writeA(byte);

//	mControlBus[E] = 1;
//	mIO.writeB(mControlBus.to_ulong());

//	std::chrono::milliseconds dura( 10 );
//	std::this_thread::sleep_for( dura );


	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());

//	mControlBus[E] = 1;
//	std::this_thread::sleep_for( dura );
//	mIO.writeB(mControlBus.to_ulong());
//	mIO.writeB(0b00000101);

}

void LCDisplay::writeControl(uint8_t byte)
{
	mControlBus[RS] = 0; // Write Control
	mControlBus[E] = 1;
	mControlBus[RW] = 0;

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());

	mIO.writeA(byte);

//	mControlBus[E] = 1;
//	mIO.writeB(mControlBus.to_ulong());

	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());

//	mControlBus[E] = 1;
//	std::this_thread::sleep_for( dura );
//	mIO.writeB(mControlBus.to_ulong());
}

uint8_t LCDisplay::readControl()
{
	mIO.directionA(IOExpander::DataDirection::dirIn);
	mControlBus[RS] = 0; // Read Control
	mControlBus[E] = 1;
	mControlBus[RW] = 1; // Set read

	// Set as Read
	mIO.writeB(mControlBus.to_ulong());

	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());

	uint8_t byte;

	mIO.readA(byte);

	mControlBus[E] = 1;
	mControlBus[RW] = 0; // Set back to write

	mIO.writeB(mControlBus.to_ulong());

	// and change the direction of the bus back
	mIO.directionA(IOExpander::DataDirection::dirOut);

	LOG(INFO) << "ConstrolBus read: 0x" << std::hex << (int) byte;
	return byte;
}

uint8_t LCDisplay::readData()
{
	mIO.directionA(IOExpander::DataDirection::dirIn);
	mControlBus[RS] = 1; // Read Data
	mControlBus[E] = 1;
	mControlBus[RW] = 1; // Set read

	// Set as Read
	mIO.writeB(mControlBus.to_ulong());

	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());

	uint8_t byte;

	mIO.readA(byte);

	mControlBus[E] = 1;
	mControlBus[RW] = 0; // Set back to write

	mIO.writeB(mControlBus.to_ulong());

	// and change the direction of the bus back
	mIO.directionA(IOExpander::DataDirection::dirOut);

	LOG(INFO) << "DataBus read: 0x" << std::hex << (int) byte;
	return byte;
}


