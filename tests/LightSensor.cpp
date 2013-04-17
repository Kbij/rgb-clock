/*
 * LightSensor.cpp
 *
 *  Created on: Apr 17, 2013
 *      Author: koen
 */

#include "LightSensor.h"
#include <glog/logging.h>

LightSensor::LightSensor(I2C &i2c, uint8_t address) :
		mI2C(i2c),
		mAddress(address),
		mIntensityMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false)
{
	init();
}

LightSensor::~LightSensor()
{
	stopReadThread();
}

uint16_t LightSensor::intensity()
{
    std::lock_guard<std::mutex> lk_guard(mIntensityMutex);
	return mIntensity;
}

bool LightSensor::init()
{
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        LOG(INFO) << "Measuring light intensity";
    }

}
uint32_t LightSensor::calculateLux(uint16_t ch0, uint16_t ch1)
{
/*
 * For 0 < CH1/CH0 <= 0.50:    Lux = 0.0304 x CH0 − 0.062 x CH0 x ((CH1/CH0)1.4)
 * For 0.50 < CH1/CH0 <= 0.61: Lux = 0.0224 x CH0 − 0.031 x CH1
 * For 0.61 < CH1/CH0 <= 0.80: Lux = 0.0128 x CH0 − 0.0153 x CH1
 * For 0.80 < CH1/CH0 <= 1.30: Lux = 0.00146 x CH0 − 0.00112 x CH1
 * For CH1/CH0 > 1.30          Lux = 0
 */


}

