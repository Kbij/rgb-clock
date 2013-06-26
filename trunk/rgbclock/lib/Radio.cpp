/*
 * Radio.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#include "Radio.h"
#include "FMReceiver.h"
#include <glog/logging.h>
#include <iostream>
#include <cmath>

namespace Hardware
{

Radio::Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver):
	mI2C(i2c),
	mAplifierAddress(apmlifierAddress),
	mFMReceiver(fmReceiver),
	mMaskRegister(0),
	mControlRegister(0),
	mVolume(30)

{
	mI2C.registerAddress(mAplifierAddress, "Amplifier");
	if (mFMReceiver.powerOn())
	{
		mFMReceiver.tuneFrequency(94.5);
	}
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
	if (mVolume < 95)
	{
		mVolume += 1;
		//LOG(INFO) << "Volume: " << (int) mVolume;
		writeRegisters();
	}
}

void Radio::volumeDown()
{
	if (mVolume > 5)
	{
		mVolume -= 1;
		//LOG(INFO) << "Volume: " << (int) mVolume;
		writeRegisters();
	}
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
	uint8_t att =  static_cast<uint8_t>(ceil(attenuation));

	//LOG(INFO) << "mVolume: " << (int) mVolume << ", Amp vol:" << attenuation << ", att: " << (int) att;
	std::vector<uint8_t> registers;
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(att);
	registers.push_back(mMaskRegister);
	registers.push_back(mControlRegister);

	mI2C.writeDataSync(mAplifierAddress, registers);
}
}
