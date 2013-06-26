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

namespace Hardware
{
class FMReceiver;
class I2C;
class RadioObserverIf;
class ClockDisplay;

class Radio {
public:
	Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver);
	virtual ~Radio();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);
//must be internal
    void setDisplay(ClockDisplay *clockDisplay);
    bool powerOn();
	bool powerOff();
    void volumeUp();
    void volumeDown();
	void readRegisters();
	void writeRegisters();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();

private:

	I2C &mI2C;
	const uint8_t mAplifierAddress;
	FMReceiver &mFMReceiver;
	ClockDisplay *mClockDisplay;
	uint8_t mMaskRegister;
	uint8_t mControlRegister;
	uint8_t mVolume;

};
}

#endif /* RADIO_H_ */
