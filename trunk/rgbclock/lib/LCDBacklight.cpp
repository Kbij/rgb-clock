/*
 * LCDBacklight.cpp
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#include "LCDBacklight.h"
#include "I2C.h"
#include <glog/logging.h>

namespace
{
	const uint16_t USERACTIVITY_TIMER = 30;
}
namespace Hardware {

LCDBacklight::LCDBacklight(I2C &i2c, uint8_t pwmAddress, uint8_t lightSensorAddress):
		mPwmDriver(i2c, pwmAddress),
		mLightSensor(i2c, lightSensorAddress),
		mUserActivityTimer(0),
		mBackLightThread(),
		mBackLightThreadRunning(false)

{
	startBackLightThread();
}

LCDBacklight::~LCDBacklight()
{
	stopBackLightThread();
}

void LCDBacklight::signalUserActivity()
{
	mUserActivityTimer = USERACTIVITY_TIMER;
}

void LCDBacklight::startBackLightThread()
{
	mBackLightThreadRunning = true;

	mBackLightThread = new std::thread(&LCDBacklight::backLightThread, this);
}

void LCDBacklight::stopBackLightThread()
{
	mBackLightThreadRunning = false;

    if (mBackLightThread)
    {
    	mBackLightThread->join();

        delete mBackLightThread;
        mBackLightThread = nullptr;
    }
}

void LCDBacklight::backLightThread()
{
    while (mBackLightThreadRunning)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        auto lux = mLightSensor.lux();

        LOG(INFO) << "Measured Lux: " << lux;
        if (mUserActivityTimer > 0)
        {
        	--mUserActivityTimer;
        	mPwmDriver.pwmValue(PwmLedDriver::PwmChannel::Channel1, 2000);
        }
        else
        {
            mPwmDriver.pwmValue(PwmLedDriver::PwmChannel::Channel1, lux);
        }
    }
}
} /* namespace Hardware */
