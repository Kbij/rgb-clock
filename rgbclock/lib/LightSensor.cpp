/*
 * LightSensor.cpp
 *
 *  Created on: Apr 17, 2013
 *      Author: koen
 */

#include "LightSensor.h"
#include <glog/logging.h>
#include <cmath>

namespace Hardware
{
LightSensor::LightSensor(I2C &i2c, uint8_t address) :
		mI2C(i2c),
		mAddress(address),
		mLux(),
		mIntensityMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false)
{
	mI2C.registerAddress(address, "Light Sensor");
	init();
}

LightSensor::~LightSensor()
{
	stopReadThread();
}

double LightSensor::lux()
{
    std::lock_guard<std::mutex> lk_guard(mIntensityMutex);
	return mLux;
}

bool LightSensor::init()
{
	// Startup the device
	mI2C.writeByteSync(mAddress, 0x03);

	// Thread will wait for 500ms before the first read
	startReadThread();
	return true;
}

void LightSensor::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = new std::thread(&LightSensor::readThread, this);
}

void LightSensor::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();

        delete mReadThread;
        mReadThread = nullptr;
    }
}

void LightSensor::readThread()
{
    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //LOG(INFO) << "Measuring light intensity";
        uint16_t channel0;
        uint16_t channel1;

        mI2C.readWordSync(mAddress, 0xAC, channel0);
        mI2C.readWordSync(mAddress, 0xAE, channel1);
        VLOG(2) << "Channel 0: " << (int) channel0;
        VLOG(2) << "Channel 1: " << (int) channel1;
        calculateLux(channel0, channel1);
    }
}

void LightSensor::calculateLux(uint16_t channel0, uint16_t channel1)
{
/* Calculation from datasheet:
 * For 0 < CH1/CH0 <= 0.50:    Lux = 0.0304 x CH0 − 0.062 x CH0 x ((CH1/CH0)1.4)
 * For 0.50 < CH1/CH0 <= 0.61: Lux = 0.0224 x CH0 − 0.031 x CH1
 * For 0.61 < CH1/CH0 <= 0.80: Lux = 0.0128 x CH0 − 0.0153 x CH1
 * For 0.80 < CH1/CH0 <= 1.30: Lux = 0.00146 x CH0 − 0.00112 x CH1
 * For CH1/CH0 > 1.30          Lux = 0
 */
	double ch0 = channel0;
	double ch1 = channel1;
	double lux = 0;

	if ((0 < (ch1/ch0)) && (ch1/ch0 <= 0.50))
	{
		lux=(0.0304 * ch0) - (0.062 * ch0 * (pow((ch1/ch0),1.4)));
	}
	if ((0.50 < (ch1/ch0)) && (ch1/ch0 <= 0.61))
	{
		lux = (0.0224 * ch0) - (0.031 * ch1);
	}
	if ((0.61 < (ch1/ch0)) && (ch1/ch0 <= 0.80))
	{
		lux = (0.0128 * ch0) - (0.0153 * ch1);
	}
	if ((0.80 < (ch1/ch0)) && (ch1/ch0 <= 1.30))
	{
		lux = (0.00146 * ch0) - (0.00112 * ch1);
	}
	if (ch1/ch0 > 1.3)
	{
		lux = 0;
	}
    //LOG(INFO) << "lux: " << lux;

    std::lock_guard<std::mutex> lk_guard(mIntensityMutex);

    mLux = lux;
}
}
