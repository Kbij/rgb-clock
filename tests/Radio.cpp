/*
 * Radio.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#include "Radio.h"
#include "FMReceiver.h"


Radio::Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver):
	mI2C(i2c),
	mAplifierAddress(apmlifierAddress),
	mFMReceiver(fmReceiver)
{

}

Radio::~Radio()
{

}

void Radio::registerRadioObserver(RadioObserverIf *observer)
{
	mFMReceiver.registerRadioObserver(observer);
}
void Radio::unRegisterRadioObserver(RadioObserverIf *observer)
{
	mFMReceiver.unRegisterRadioObserver(observer);
}

bool Radio::powerOn()
{
	return mFMReceiver.powerOn();
}

bool Radio::powerOff()
{
	return mFMReceiver.powerOff();
}

bool Radio::seekUp(int timeout)
{
	return mFMReceiver.seekUp(timeout);
}

bool Radio::tuneFrequency(double frequency)
{
	return mFMReceiver.tuneFrequency(frequency);
}

RDSInfo Radio::getRDSInfo()
{
	return mFMReceiver.getRDSInfo();
}
