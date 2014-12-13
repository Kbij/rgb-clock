/*
 * LCDBacklight.cpp
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#include "LCDBacklight.h"
#include "I2C.h"
#include <glog/logging.h>
#include <pthread.h>

namespace
{
	const uint16_t USERACTIVITY_TIMER = 10;
}
namespace Hardware {

LCDBacklight::LCDBacklight(I2C &i2c, uint8_t hwrevision, uint8_t pwmAddress, uint8_t lightSensorAddress):
		mPwmDriver(nullptr),
		mLightSensor(i2c, lightSensorAddress),
		mUserActivityTimer(0),
		mBackLightThread(),
		mBackLightThreadRunning(false)

{
	if (hwrevision >1)
	{
		mPwmDriver = std::move(std::unique_ptr<PwmLedDriver>(new PwmLedDriver(i2c, pwmAddress)));
	}
	startBackLightThread();
}

LCDBacklight::~LCDBacklight()
{
	LOG(INFO) << "Backlight destructor";
	stopBackLightThread();
}

void LCDBacklight::signalUserActivity()
{
	mUserActivityTimer = USERACTIVITY_TIMER;
}

void LCDBacklight::startBackLightThread()
{
	mBackLightThreadRunning = true;

	mBackLightThread = std::unique_ptr<std::thread>(new std::thread(&LCDBacklight::backLightThread, this));
}

void LCDBacklight::stopBackLightThread()
{
	mBackLightThreadRunning = false;

    if (mBackLightThread)
    {
    	mBackLightThread->join();
        mBackLightThread.reset();
    }
}

void LCDBacklight::backLightThread()
{
	pthread_setname_np(pthread_self(), "Backlight");

    while (mBackLightThreadRunning)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        auto lux = mLightSensor.lux();
        auto pwmValue = lux;

        if (mUserActivityTimer > 0)
        {
        	--mUserActivityTimer;
            pwmValue = 2500;
        }
        else
        {
            pwmValue = 200+(lux*20);
        }

    	if (mPwmDriver)
    	{
        	mPwmDriver->pwmSingle(pwmValue);
    	}
    }
}
} /* namespace Hardware */
