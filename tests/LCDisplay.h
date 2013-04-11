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

	void toggleBit();

private:
	void init();
	void writeData(uint8_t byte);
	void writeControl(uint8_t byte);
	IOExpander mIO;
	std::bitset<8> mControlBus;

};

#endif /* LCDISPLAY_H_ */
