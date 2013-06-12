/*
 * Radio.h
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#ifndef RADIO_H_
#define RADIO_H_
#include "RDSInfo.h"
#include <stdint.h>

class FMReceiver;
class I2C;
class RadioObserverIf;

class Radio {
public:
	Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver);
	virtual ~Radio();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

    bool powerOn();
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();
private:
	I2C &mI2C;
	const uint8_t mAplifierAddress;
	FMReceiver &mFMReceiver;
};

#endif /* RADIO_H_ */
