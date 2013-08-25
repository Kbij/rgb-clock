/*
 * LCDisplay.h
 *
 *  Created on: Apr 11, 2013
 *      Author: koen
 */

#ifndef LCDISPLAY_H_
#define LCDISPLAY_H_
#include "I2C.h"
#include "IOExpander.h"
#include <stdint.h>
#include <bitset>
#include <string>
#include <array>
#include <map>

namespace Hardware
{
struct MyGraphicWord
{
	std::bitset<16> mBits;
	bool mChanged;
};
enum class FontType
{
	Verdana20,
	Courier15,
	Large9,
	Volter,
	Terminal8
};

struct FontInfo
{
	uint8_t mWidth;
	uint8_t mHeight;
	uint8_t mSpacing;
	const std::vector<uint8_t>* mPointer;
};

class LCDisplay {
public:
	LCDisplay(I2C &i2c, uint8_t address);
	virtual ~LCDisplay();
	void initStandard();
	void clearStandardDisplay();

	void initGraphic();
	void clearGraphicDisplay();

	void writeText(uint8_t pos, std::string text);
	void writeGraphicText(uint8_t col, uint8_t row, std::string text, FontType font);

	void point(uint8_t x, uint8_t y, bool set);
	void hLine(uint8_t col1, uint8_t col2, uint8_t row, bool set);
	void vLine(uint8_t col, uint8_t row1, uint8_t row2, bool set);
	void rectangle(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, bool set, bool fill);

private:
	void init();

	void rawPoint(uint8_t col, uint8_t row, bool set);
	void rawVertByte(uint8_t col, uint8_t& row, uint8_t maxRemainingRows, uint8_t byte);
	void rawHLine(uint8_t col1, uint8_t col2, uint8_t row, bool set);
	void rawVLine(uint8_t col, uint8_t row1, uint8_t row2, bool set);
	void rawGraphicChar(uint8_t col, uint8_t row, uint8_t character, FontType font);

	void setDDRamAddress(uint8_t addr);
	void setGAddress(uint8_t horizontal, uint8_t vertical);

	void refreshDisplay();
	void writeData(uint8_t byte);
	void writeControl(uint8_t byte);
	uint8_t readControl();
	uint8_t readData();

	void setRSRWBits(bool wantedRS, bool wantedRW);

	IOExpander mIO;
	uint8_t mPortA;
	std::bitset<8> mControlBus;
	std::array<std::array<MyGraphicWord,10>,32>* mGraphicRam;
	std::map<FontType, FontInfo> mFontMap;
	std::mutex mDisplayMutex;
};
}

#endif /* LCDISPLAY_H_ */
