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

enum class PowerState
{
	POWERON,
	POWEROFF,
	UNKNOWN
};

class FMReceiver {
public:
	FMReceiver(I2C &i2c, uint8_t address);
	virtual ~FMReceiver();

	bool powerOn();
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
private:
	bool init();

	void debugTuningStatus();
	bool waitForCTS();
	bool waitForSTC();
	bool readCTS();
	bool readSTC();

	I2C &mI2C;
	const uint8_t mAddress;
	PowerState mPowerState;
};

#endif /* FMRECEIVER_H_ */
