/*
 * IOExpander.h
 *
 *  Created on: Apr 10, 2013
 *      Author: koen
 */

#ifndef IOEXPANDER_H_
#define IOEXPANDER_H_
#include "I2C.h"
#include <stdint.h>


class IOExpander {

public:
enum class DataDirection
{
	dirIn,
	dirOut
};

public:
	IOExpander(I2C &i2c, uint8_t address);
	virtual ~IOExpander();

	bool writeA(uint8_t byte);
	bool writeB(uint8_t byte);

	bool directionA(DataDirection direction);
	bool directionB(DataDirection direction);

	bool readA(uint8_t& byte);
	bool readB(uint8_t& byte);
private:
	bool init();
	I2C &mI2C;
	const uint8_t mAddress;
};

#endif /* IOEXPANDER_H_ */
