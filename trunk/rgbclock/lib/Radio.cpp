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
	mVolume(20),
	mRadioObservers(),
	mRadioObserversMutex(),
	mState(RadioState::PwrOff)

{
	mI2C.registerAddress(mAplifierAddress, "Amplifier");
	// Use BTL
	mControlRegister = 0b00010000;
	writeRegisters();
}

Radio::~Radio()
{
	LOG(INFO) << "Radio destructor";
}

void Radio::registerRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.insert(observer);
    }
}
void Radio::unRegisterRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.erase(observer);
    }
}

void Radio::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (mState == RadioState::PwrOn)
	{
		if (keyboardInfo[KEY_UP].mPressed || keyboardInfo[KEY_UP].mLongPress)
		{
			volumeUp();
		}
		if (keyboardInfo[KEY_DOWN].mPressed || keyboardInfo[KEY_DOWN].mLongPress)
		{
			volumeDown();
		}
	}

	if (keyboardInfo[KEY_1].mPressed)
	{
		switch (mState)
		{
			case RadioState::PwrOff: powerOn();
				break;
			case RadioState::PwrOn : powerOff();
				break;
			default: ;
		}

	}
}

void Radio::alarmNotify()
{

}

std::string Radio::name()
{
	return "";
}

bool Radio::powerOn()
{
	LOG(INFO) << "Radio On";
	if (mFMReceiver.powerOn())
	{
		mFMReceiver.tuneFrequency(94.5);
	}

	mState = RadioState::PwrOn;
	mControlRegister = 0b00010000; // PowerUp
	writeRegisters();
	registerFMReceiver();
	return true;
}

bool Radio::powerOff()
{
	LOG(INFO) << "Radio Off";

	mState = RadioState::PwrOff;
	mControlRegister = 0b00010001; // Shutdown
	writeRegisters();
	mFMReceiver.powerOff();

	registerFMReceiver();

	return true;
}

void Radio::volume(int volume)
{
	mVolume = volume;
	writeRegisters();
}

void Radio::volumeUp()
{
	if (mVolume < 99)
	{
		mVolume += 1;
		//LOG(INFO) << "Volume: " << (int) mVolume;
		writeRegisters();
	}

}

void Radio::volumeDown()
{
	if (mVolume > 1)
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
	notifyObservers();
}
void Radio::notifyObservers()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	RadioInfo info;
    	info.mVolume = mVolume;
    	info.mState = mState;
        observer->radioStateUpdate(info);
    }
}

void Radio::registerFMReceiver()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	mFMReceiver.registerRadioObserver(observer);
    }
}

void Radio::unRegisterFMReceiver()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
    	mFMReceiver.unRegisterRadioObserver(observer);
    }
}

}
