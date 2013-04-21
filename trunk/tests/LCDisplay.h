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
struct MyGraphicWord
{
	std::bitset<16> mBits;
	bool mChanged;
};

class LCDisplay {
public:
	LCDisplay(I2C &i2c, uint8_t address);
	virtual ~LCDisplay();
	void clearDisplay();
	void clearGraphicDisplay();
	void writeText(uint8_t pos, std::string text);
	void point(uint8_t x, uint8_t y, bool set);
	void HLine(uint8_t x1, uint8_t x2, uint8_t y, bool set);
	void VLine(uint8_t y1, uint8_t y2, uint8_t x, bool set);
	void testGraphic();
private:
	void init();
	void checkControlBus();

	void setDDRamAddress(uint8_t addr);
	void setGAddress(uint8_t horizontal, uint8_t vertical);

	void refreshDisplay();
	void writeData(uint8_t byte);
	void writeControl(uint8_t byte);
	uint8_t readControl();
	uint8_t readData();
	IOExpander mIO;
	uint8_t mPortA;
	std::bitset<8> mControlBus;
//	std::array<std::array<uint32_t,10>,32> mGraphicRam;
	//std::array<std::array<uint32_t,100>,100>* mGraphicRam;
	std::array<std::array<MyGraphicWord,10>,32>* mGraphicRam2;

};

#endif /* LCDISPLAY_H_ */
