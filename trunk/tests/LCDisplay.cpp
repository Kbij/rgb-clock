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
#include <stdlib.h>


const uint8_t RS = 0;
const uint8_t RW = 1;
const uint8_t E = 2;
const uint32_t CHANGED_BIT = 0x80000000;

LCDisplay::LCDisplay(I2C &i2c, uint8_t address):
	mIO(i2c, address),
	mPortA(0),
	mControlBus(0),
	//mGraphicRam(nullptr),
	mGraphicRam2(nullptr)

{
	//std::array<std::array<uint16_t,100>,100>* mGraphicRam;
   // mGraphicRam = new std::array<std::array<uint32_t,100>,100>;
    mGraphicRam2 = new std::array<std::array<MyGraphicWord,10>,32>;

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
void LCDisplay::clearGraphicDisplay()
{
	LOG(INFO) << "Clear Graphic Display";

	for (int vert = 0; vert < 32; ++vert)
	{
		for (int horz = 0; horz < 10; ++horz )
		{
			(*mGraphicRam2)[vert][horz].mBits.reset();
			(*mGraphicRam2)[vert][horz].mChanged = true;
		}
	}
	refreshDisplay();
}

void LCDisplay::writeText(uint8_t pos, std::string text)
{
	writeControl(0b00000110);
	setDDRamAddress(pos); //Fist Char position
	for (unsigned int i = 0; i < text.length(); ++i)
	{
		writeData(text[i]);
	}
}

void LCDisplay::point(uint8_t x, uint8_t y, bool set)
{
	div_t divresult;
	divresult = div(x, 16);
	uint myX= 0;
	LOG(INFO) << "Write Point: Quotient: " << divresult.quot << " Remainder: " << divresult.rem;
	myX = 15 - divresult.rem;
	LOG(INFO) << "myX: " << myX;
	(*mGraphicRam2)[x][divresult.quot].mBits.set(myX, set);
	(*mGraphicRam2)[x][divresult.quot].mChanged = true;
//	refreshDisplay();
}

void LCDisplay::HLine(uint8_t x1, uint8_t x2, uint8_t y, bool set)
{
	for (int i = x1; i <=x2; ++i)
	{
		point(i,y,set);
	}

	refreshDisplay();
}

void LCDisplay::VLine(uint8_t y1, uint8_t y2, uint8_t x, bool set)
{
	for (int i = y1; i <=y2; ++i)
	{
		point(x,i,set);
	}

	refreshDisplay();
}

void LCDisplay::testGraphic()
{
	std::this_thread::sleep_for( std::chrono::milliseconds ( 40) );

	// Set extended instruction
	writeControl(0b00110100);
	std::this_thread::sleep_for( std::chrono::milliseconds ( 40) );

	// Set graphic display on
	writeControl(0b00110110);
	clearGraphicDisplay();

	(*mGraphicRam2)[0][0].mBits.set();
	(*mGraphicRam2)[0][0].mChanged = true;
	(*mGraphicRam2)[31][9].mBits.set();
	(*mGraphicRam2)[31][9].mChanged = true;
	refreshDisplay();

	HLine(0,159,0, true);
	HLine(0,159,15, true);


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
	std::this_thread::sleep_for( std::chrono::milliseconds (40) );

	// 8 bit interface
	writeControl(0b00110000);
	writeControl(0b00110000);

	// display on
	writeControl(0b00001100);

	// home
	writeControl(0b00000010);

	clearDisplay();

	// display control
	writeControl(0b00000110);
}

void LCDisplay::setDDRamAddress(uint8_t addr)
{
	writeControl(0x80 | addr);
}
void LCDisplay::setGAddress(uint8_t vertical, uint8_t horizontal)
{

	writeControl(0b10000000 | vertical);
	writeControl(0b10000000 | horizontal);

}

void LCDisplay::refreshDisplay()
{
	LOG(INFO) << "Refresh Graphic Display";

	for (int vert = 0; vert < 32; ++vert)
	{
		for (int horz = 0; horz < 10; ++horz )
		{
			if ((*mGraphicRam2)[vert][horz].mChanged)
			{
				setGAddress(vert, horz);
				writeData(((*mGraphicRam2)[vert][horz].mBits.to_ulong() >> 8) & 0xFF);
				writeData((*mGraphicRam2)[vert][horz].mBits.to_ulong() & 0xFF);

				// Clear Changed Bit
				(*mGraphicRam2)[vert][horz].mChanged = false;
			}
		}
	}
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


