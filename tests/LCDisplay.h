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

class LCDisplay {
public:
	LCDisplay(I2C &i2c, uint8_t address);
	virtual ~LCDisplay();
	void clearDisplay();

	void checkControlBus();

	void toggleBit();

	void init();

private:

	void setDDRamAddress(uint8_t addr);
	void writeData(uint8_t byte);
	void writeControl(uint8_t byte);
	uint8_t readControl();
	uint8_t readData();
	IOExpander mIO;
	uint8_t mPortA;
	std::bitset<8> mControlBus;
	bool test;


};

#endif /* LCDISPLAY_H_ */
