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
#include "Fonts.h"


const uint8_t RS = 0;
const uint8_t RW = 1;
const uint8_t E = 2;
const uint32_t CHANGED_BIT = 0x80000000;

LCDisplay::LCDisplay(I2C &i2c, uint8_t address):
	mIO(i2c, address),
	mPortA(0),
	mControlBus(0),
	mGraphicRam(nullptr),
	mFontMap()

{
    mGraphicRam = new std::array<std::array<MyGraphicWord,10>,32>;
    mFontMap[FontType::Verdana20].mWidth = 32;
    mFontMap[FontType::Verdana20].mHeight = 27;
    mFontMap[FontType::Verdana20].mPointer = &VerdanaBold32x27;

	init();
}

LCDisplay::~LCDisplay()
{
	delete mGraphicRam;
	mGraphicRam = nullptr;
}

void LCDisplay::initStandard()
{

}

void LCDisplay::clearStandardDisplay()
{
	LOG(INFO) << "Clear Standard Display";

	writeControl(0b00000001); // Display Clear

	// Wait 2ms after clear display
	std::chrono::milliseconds dura( 2 );
	std::this_thread::sleep_for( dura );
}

void LCDisplay::initGraphic()
{
	// Set Extended
	writeControl(0b00110100);

	// Set graphic display on
	writeControl(0b00110110);
}

void LCDisplay::clearGraphicDisplay()
{
	LOG(INFO) << "Clear Graphic Display";

	for (int vert = 0; vert < 32; ++vert)
	{
		for (int horz = 0; horz < 10; ++horz )
		{
			(*mGraphicRam)[vert][horz].mBits.reset();
			(*mGraphicRam)[vert][horz].mChanged = true;
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
void LCDisplay::writegGraphicChar(uint8_t x, uint8_t y, uint8_t character)
{
	LOG(INFO) << "Write Char, x:" << (int)x << ", y: " << (int)y << ", char:" << character;
	uint16_t arrayOffset = ((character - (int)' ') * (1 + 32 * 4)) + 1;

	LOG(INFO) << "Position in Array:" << (int)arrayOffset;
	div_t divresultW = div(mFontMap[FontType::Verdana20].mWidth, 8);
	uint8_t bytesPerCharLine = divresultW.quot;
	if (divresultW.rem > 0)
	{
		++bytesPerCharLine;
	}
	div_t divresultH = div(mFontMap[FontType::Verdana20].mHeight, 8);
	uint8_t lineBlocks = divresultH.quot;
	if (divresultH.rem > 0)
	{
		++lineBlocks;
	}

	LOG(INFO) << "bytesPerCharLine: " << (int)bytesPerCharLine;
	LOG(INFO) << "lineBlocks: " << (int)lineBlocks;

	for (uint8_t arrayY = 0; arrayY < lineBlocks; ++arrayY)
	{
		for (uint8_t blockLine = 0; blockLine < 8; blockLine++)
		{
			for (uint8_t arrayX = 0; arrayX < bytesPerCharLine; ++arrayX)
			{
				LOG(INFO) << "arrayY: " << (int)arrayY << ", arrayX: " << (int)arrayX;

				uint16_t arrayPos = arrayOffset;

				arrayPos += ((arrayY+1) * 8) - blockLine;

				arrayPos += arrayX;

				LOG(INFO) << "arrayPos: " << (int)arrayPos;

	//			arrayPos = 0;
				uint8_t byte = (*(mFontMap[FontType::Verdana20].mPointer))[arrayPos];
				//LOG(INFO) << "byte: " << (int)byte;


				uint8_t bitMask = 0x80;
				// for each bit
				for (uint8_t i = 0; i < 8; ++i)
				{
					uint8_t pointX = x + (arrayX * 8) +  i;
					uint8_t pointY = y + (arrayY * 8) + blockLine;
					LOG(INFO) << "point x: " << (int) pointX <<  "point y:" << (int)pointY ;
					if (byte & bitMask)
					{
						rawPoint(pointY, pointX, true);
//						rawPoint(y + arrayY, x + (arrayX * 8) +  i,  true);

					}
					else
					{
						rawPoint(pointY, pointX, false);
	//					rawPoint(y + arrayY, x + (arrayX * 8) +  i,  false);
					}
					bitMask = bitMask >> 1;
				}
		}

		}
		//uint8_t arrayPos = 0;
		//const uint8_t byte = (*(mFontMap[FontType::Verdana20].mPointer))[arrayPos];

	}
	refreshDisplay();

}

void LCDisplay::point(uint8_t x, uint8_t y, bool set)
{
	rawPoint(x, y, set);
	refreshDisplay();
}

void LCDisplay::hLine(uint8_t x1, uint8_t x2, uint8_t y, bool set)
{
	rawHLine(x1, x2, y, set);
	refreshDisplay();
}

void LCDisplay::vLine(uint8_t x, uint8_t y1, uint8_t y2, bool set)
{
	rawVLine(x, y1, y2, set);
	refreshDisplay();
}

void LCDisplay::rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool set, bool fill)
{
	if (fill)
	{
		for (int i = y1; i <= y2; ++i)
		{
			rawHLine(x1, x2, i, set);
		}
	}
	else
	{
		rawHLine(x1, x2, y1, set);
		rawHLine(x1, x2, y2, set);
		rawVLine(x1, y1, y2, set);
		rawVLine(x2, y1, y2, set);
	}
	refreshDisplay();
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

	clearStandardDisplay();

	// display control
	writeControl(0b00000110);
}

void LCDisplay::rawPoint(uint8_t x, uint8_t y, bool set)
{
	if ((x > 159) || (y > 31))
	{
		return;
	}
	div_t divresult;
	divresult = div(x, 16);
	uint myX= 0;
	myX = 15 - divresult.rem;
	(*mGraphicRam)[y][divresult.quot].mBits.set(myX, set);
	(*mGraphicRam)[y][divresult.quot].mChanged = true;
}

void LCDisplay::rawHLine(uint8_t x1, uint8_t x2, uint8_t y, bool set)
{
	for (int i = x1; i <=x2; ++i)
	{
		rawPoint(i,y,set);
	}
}

void LCDisplay::rawVLine(uint8_t x, uint8_t y1, uint8_t y2, bool set)
{
	for (int i = y1; i <=y2; ++i)
	{
		rawPoint(x,i,set);
	}
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
		int8_t prevHorz = -2;

		for (int horz = 0; horz < 10; ++horz )
		{
			if ((*mGraphicRam)[vert][horz].mChanged)
			{
				if (!(prevHorz == (horz -1)))
				{
					setGAddress(vert, horz);
				}
				writeData(((*mGraphicRam)[vert][horz].mBits.to_ulong() >> 8) & 0xFF);
				writeData((*mGraphicRam)[vert][horz].mBits.to_ulong() & 0xFF);

				// Clear Changed Bit
				(*mGraphicRam)[vert][horz].mChanged = false;
				prevHorz = horz;
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

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	// Write databus
	mIO.writeA(byte);

	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());
}

void LCDisplay::writeControl(uint8_t byte)
{
	mControlBus[RS] = 0; // Write Control
	mControlBus[E] = 1;
	mControlBus[RW] = 0;

	// Set the control Bus to the initial state
	mIO.writeB(mControlBus.to_ulong());
	// Write databus
	mIO.writeA(byte);

	// Write data to display
	mControlBus[E] = 0;
	mIO.writeB(mControlBus.to_ulong());
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


