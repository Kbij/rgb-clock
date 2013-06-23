/*
 * Radio.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#include "Radio.h"
#include "FMReceiver.h"
#include "ClockDisplay.h"
#include <glog/logging.h>
#include <iostream>


Radio::Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver):
	mI2C(i2c),
	mAplifierAddress(apmlifierAddress),
	mFMReceiver(fmReceiver),
	mClockDisplay(nullptr),
	mMaskRegister(0),
	mControlRegister(0),
	mVolume(30)

{
	mI2C.registerAddress(mAplifierAddress, "Amplifier");
	// Use BTL
	mControlRegister = 0b00010000;
	writeRegisters();
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

void Radio::setDisplay(ClockDisplay *clockDisplay)
{
	mClockDisplay = clockDisplay;
	mClockDisplay->showVolume(mVolume);
}

bool Radio::powerOn()
{
	return mFMReceiver.powerOn();
}

bool Radio::powerOff()
{
	return mFMReceiver.powerOff();
}
void Radio::volumeUp()
{
	//--mVolume;
	LOG(INFO) << "Volume: " << (int) mVolume;
	writeRegisters();
}

void Radio::volumeDown()
{

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

void Radio::keyboardPressed(uint16_t value)
{
	if (value & 0b10000000)
	{
		if (mVolume <= 95)
		{
			mVolume += 5;
		}
		if (mClockDisplay)
		{
			mClockDisplay->showVolume(mVolume);
		}

	}

	if (value & 0b01000000)
	{
		if (mVolume >= 5)
		{
			mVolume -= 5;
		}

		if (mClockDisplay)
		{
			mClockDisplay->showVolume(mVolume);
		}

	}
	writeRegisters();

}

void Radio::readRegisters()
{
	std::vector<uint8_t> response(6);
	mI2C.writeReadDataSync(mAplifierAddress, std::vector<uint8_t>({}), response);
	LOG(INFO) << "read Registers";
	for (auto reg: response)
	{
		LOG(INFO) << std::hex << "0x" << (int) reg;
	}

}

void Radio::writeRegisters()
{
	double attenuation = 64 - static_cast<double>(mVolume)/100 * 64.0;
	//LOG(INFO) << "mVolume: " << (int) mVolume << ", Amp vol:" << attenuation;

	std::vector<uint8_t> registers;
	registers.push_back(static_cast<uint8_t>(attenuation));
	registers.push_back(static_cast<uint8_t>(attenuation));
	registers.push_back(static_cast<uint8_t>(attenuation));
	registers.push_back(static_cast<uint8_t>(attenuation));
	registers.push_back(mMaskRegister);
	registers.push_back(mControlRegister);

	mI2C.writeDataSync(mAplifierAddress, registers);
}
