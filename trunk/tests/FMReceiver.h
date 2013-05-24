/*
 * FMReceiver.h
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#ifndef FMRECEIVER_H_
#define FMRECEIVER_H_

#include "I2C.h"
#include <stdint.h>

class FMReceiver {
public:
	FMReceiver(I2C &i2c, uint8_t address);
	virtual ~FMReceiver();
private:
	bool init();
	bool waitForCTS();
	bool readCTS();
	bool readSTC();
	I2C &mI2C;
	const uint8_t mAddress;
};

#endif /* FMRECEIVER_H_ */
