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

namespace
{
const uint8_t RS = 0;
const uint8_t RW = 1;
const uint8_t E = 2;

const uint32_t CHANGED_BIT = 0x80000000;
}

namespace Hardware
{

LCDisplay::LCDisplay(I2C &i2c, uint8_t address):
	mIO(i2c, address),
	mPortA(0),
	mControlBus(0),
	mGraphicRam(),
	mFontMap(),
	mDisplayMutex()
{
	i2c.registerAddress(address, "Display");
    //mGraphicRam = new std::array<std::array<MyGraphicWord,10>,32>;
    mFontMap[FontType::Verdana20].mWidth = 32;
    mFontMap[FontType::Verdana20].mSpacing = 22;
    mFontMap[FontType::Verdana20].mHeight = 25;//27;
    mFontMap[FontType::Verdana20].mPointer = &VerdanaBold32x27;

    mFontMap[FontType::Terminal8].mWidth = 6;
    mFontMap[FontType::Terminal8].mSpacing = 6;
    mFontMap[FontType::Terminal8].mHeight = 8;
    mFontMap[FontType::Terminal8].mPointer = &Terminal6x8;

	init();
}

LCDisplay::~LCDisplay()
{
	LOG(INFO) << "LCDisplay destructor exit";
}

void LCDisplay::initStandard()
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);
}

void LCDisplay::clearStandardDisplay()
{
	LOG(INFO) << "Clear Standard Display";
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	writeControl(0b00000001); // Display Clear

	// Wait 2ms after clear display
	std::chrono::milliseconds dura( 2 );
	std::this_thread::sleep_for( dura );
}

void LCDisplay::initGraphic()
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	// Set Extended
	writeControl(0b00110100);

	// Set graphic display on
	writeControl(0b00110110);
}

void LCDisplay::clearGraphicDisplay()
{
	LOG(INFO) << "Clear Graphic Display";
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	for (int vert = 0; vert < 32; ++vert)
	{
		for (int horz = 0; horz < 10; ++horz )
		{
			mGraphicRam[vert][horz].mBits.reset();
			mGraphicRam[vert][horz].mChanged = true;
		}
	}
	refreshDisplay();
}

void LCDisplay::writeText(uint8_t pos, std::string text)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	writeControl(0b00000110);
	setDDRamAddress(pos); //Fist Char position
	for (unsigned int i = 0; i < text.length(); ++i)
	{
		writeData(text[i]);
	}
}

bool LCDisplay::writeGraphicText(uint8_t col, uint8_t row, std::string text, FontType font)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	uint8_t myCol = col;
	for (auto myChar: text)
	{
		rawGraphicChar(myCol, row, myChar, font);
		myCol += mFontMap[font].mSpacing;
	}
	refreshDisplay();
	return true;
}

void LCDisplay::point(uint8_t x, uint8_t y, bool set)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	rawPoint(x, y, set);
	refreshDisplay();
}

void LCDisplay::hLine(uint8_t col1, uint8_t col2, uint8_t row, bool set)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	rawHLine(col1, col2, row, set);
	refreshDisplay();
}

void LCDisplay::vLine(uint8_t col, uint8_t row1, uint8_t row2, bool set)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	rawVLine(col, row1, row2, set);
	refreshDisplay();
}

void LCDisplay::rectangle(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, bool set, bool fill)
{
    std::lock_guard<std::mutex> lk_guard(mDisplayMutex);

	if (fill)
	{
		for (int row = row1; row <= row2; ++row)
		{
			rawHLine(col1, col2, row, set);
		}
	}
	else
	{
		rawHLine(col1, col2, row1, set);
		rawHLine(col1, col2, row2, set);
		rawVLine(col1, row1, row2, set);
		rawVLine(col2, row1, row2, set);
	}
	refreshDisplay();
}

void LCDisplay::drawSignal(int8_t signal)
{
	if (signal >= 75)
	{
		rawHLine(159-3, 159, 0, true);
	}
	else
	{
		rawHLine(159-3, 159, 0, false);
	}

	if (signal >= 50)
	{
		rawHLine(159-2, 159, 1, true);
	}
	else
	{
		rawHLine(159-2, 159, 1, false);
	}
	if (signal >= 25)
	{
		rawHLine(159-1, 159, 2, true);
	}
	else
	{
		rawHLine(159-1, 159, 2, false);
	}

	if (signal >= 0)
	{
		rawPoint(159, 3, true);
	}
	else
	{
		rawPoint(159, 3, false);
	}

	refreshDisplay();
}

void LCDisplay::drawNTPState(bool ntpSync)
{
	if (ntpSync)
	{
		rawPoint(150, 2, true);
		rawPoint(151, 3, true);
		rawPoint(152, 2, true);
		rawPoint(153, 1, true);
		rawPoint(154, 0, true);

		refreshDisplay();
	}
	else
	{
		// Clear; no refresh needed
		rectangle(150, 0, 155, 3, false, true);
	}
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

void LCDisplay::rawPoint(uint8_t col, uint8_t row, bool set)
{
	if ((col > 159) || (row > 31))
	{
		return;
	}
	div_t divresult;
	divresult = div(col, 16);
	uint myCol= 0;
	myCol = 15 - divresult.rem;

	// If there is a difference
	if (mGraphicRam[row][divresult.quot].mBits.test(myCol) != set)
	{
		mGraphicRam[row][divresult.quot].mBits.set(myCol, set);
		mGraphicRam[row][divresult.quot].mChanged = true;
	}
}

void LCDisplay::rawVertByte(uint8_t col, uint8_t& row, uint8_t maxRemainingRows, uint8_t byte)
{
	uint8_t bitMask = 0x01;
	// for each bit
	for (uint8_t i = 0; (i < 8) && (i < maxRemainingRows); ++i)
	{
		rawPoint(col, row, byte & bitMask);
		bitMask = bitMask << 1;
		row++;
	}
}

void LCDisplay::rawHLine(uint8_t col1, uint8_t col2, uint8_t row, bool set)
{
	for (int i = col1; i <=col2; ++i)
	{
		rawPoint(i,row,set);
	}
}

void LCDisplay::rawVLine(uint8_t col, uint8_t row1, uint8_t row2, bool set)
{
	for (int i = row1; i <= row2; ++i)
	{
		rawPoint(col,i,set);
	}
}
void LCDisplay::rawGraphicChar(uint8_t col, uint8_t row, uint8_t character, FontType font)
{
	// (1 +32 *4) = number of bytes per char
	// +1 : fist byte
	div_t divresultW = div(mFontMap[font].mHeight, 8);
	int bytesPerCharCol = divresultW.quot;
	if (divresultW.rem > 0)
	{
		++bytesPerCharCol;
	}
	int characterNumber = character - (int)' ';
	int arrayOffset = ((mFontMap[font].mWidth * bytesPerCharCol) + 1) * characterNumber;
	arrayOffset++; // Ignore first byte

	for (uint8_t charCol = 0; charCol < mFontMap[font].mWidth; ++charCol)
	{
		uint8_t myRow = row;

		for (uint8_t byteNr = 0; byteNr < bytesPerCharCol; ++byteNr)
		{

			uint8_t myCol = col + charCol;
			uint16_t arrayPos = arrayOffset;
			arrayPos += (charCol * bytesPerCharCol) + byteNr;
			uint8_t byte = (*(mFontMap[font].mPointer))[arrayPos];

			uint8_t maxRemainingRows = mFontMap[font].mHeight - (myRow - row) ;
			rawVertByte(myCol, myRow, maxRemainingRows, byte);
		}

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
	VLOG(2) << "Refresh Graphic Display";

	for (int vert = 0; vert < 32; ++vert)
	{
		int8_t prevHorz = -2;

		for (int horz = 0; horz < 10; ++horz )
		{
			if (mGraphicRam[vert][horz].mChanged)
			{
				if (!(prevHorz == (horz -1)))
				{
					setGAddress(vert, horz);
				}
				writeData((mGraphicRam[vert][horz].mBits.to_ulong() >> 8) & 0xFF);
				writeData(mGraphicRam[vert][horz].mBits.to_ulong() & 0xFF);

				// Clear Changed Bit
				mGraphicRam[vert][horz].mChanged = false;
				prevHorz = horz;
			}
		}
	}
}

void LCDisplay::writeData(uint8_t byte)
{
	setRSRWBits(1, 0); // RS=1, RW = 0

	// Write databus
	mIO.writeA(byte);

	// Write data to display
	mControlBus.flip(E);
	mIO.writeB(mControlBus.to_ulong());
}

void LCDisplay::writeControl(uint8_t byte)
{
	setRSRWBits(0, 0); // RS=0, RW = 0

	// Write databus
	mIO.writeA(byte);

	// Write data to display
	mControlBus.flip(E);

	mIO.writeB(mControlBus.to_ulong());
}

uint8_t LCDisplay::readControl()
{
	mIO.directionA(IOExpander::DataDirection::dirIn);

	setRSRWBits(0, 1); // RS=0, RW = 1

	mControlBus.flip(E);
	mIO.writeB(mControlBus.to_ulong());

	uint8_t byte;

	mIO.readA(byte);

	setRSRWBits(0, 0); // RS=0, RW = 1

	// and change the direction of the bus back
	mIO.directionA(IOExpander::DataDirection::dirOut);

	LOG(INFO) << "ConstrolBus read: 0x" << std::hex << (int) byte;
	return byte;
}

uint8_t LCDisplay::readData()
{
	mIO.directionA(IOExpander::DataDirection::dirIn);

	setRSRWBits(1, 1); // RS=0, RW = 1


	mControlBus.flip(E);
	mIO.writeB(mControlBus.to_ulong());

	uint8_t byte;

	mIO.readA(byte);

	setRSRWBits(1, 0); // RS=0, RW = 0
	mIO.writeB(mControlBus.to_ulong());

	// and change the direction of the bus back
	mIO.directionA(IOExpander::DataDirection::dirOut);

	LOG(INFO) << "DataBus read: 0x" << std::hex << (int) byte;
	return byte;
}

void LCDisplay::setRSRWBits(bool wantedRS, bool wantedRW)
{
	if ((mControlBus[RS] != wantedRS) || (mControlBus[RW] != wantedRW))
	{
		mControlBus[RS] = wantedRS; // Write Data
		mControlBus[RW] = wantedRW;

		// Set the control Bus to the initial state
		mIO.writeB(mControlBus.to_ulong());
	}
}

}

