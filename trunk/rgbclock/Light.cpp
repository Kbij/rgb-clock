/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"
#include <glog/logging.h>
#include <time.h>

namespace App
{

Light::Light(Hardware::I2C &i2c, uint8_t address) :
		mState(State::PwrOff),
		mRGBLed(i2c, address),
		mLuminance(1000),
		mLastLong(time(nullptr)),
		mDimDown(true),
		mLedMutex(),
	    mDimmerThread(nullptr),
	    mDimmerThreadRunning(false)
{
	pwrOff();
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
}

Light::~Light()
{
	mRGBLed.pwrOff();
	stopDimmerThread();
}
void Light::pwrOn()
{
    std::lock_guard<std::mutex> lk_guard(mLedMutex);

	mRGBLed.luminance(mLuminance);
	mRGBLed.pwrOn();
	mState = State::PwrOn;
}

void Light::pwrOff()
{
    std::lock_guard<std::mutex> lk_guard(mLedMutex);

    mRGBLed.pwrOff();
	mState = State::PwrOff;
}

void Light::pwrToggle()
{
	if ((mState == State::PwrOn) ||(mState == State::SlowUp))
	{
		mState = State::FastDown;
		startDimmerThread();
	}
	if (mState == State::PwrOff)
	{
	    std::lock_guard<std::mutex> lk_guard(mLedMutex);

		mLuminance = 0;
		mRGBLed.luminance(mLuminance);
		mRGBLed.pwrOn();
		mState = State::FastUp;
		startDimmerThread();
	}
}

void Light::keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo)
{
	if (keyboardInfo[KEY_CENTRAL].mPressed)
	{
		pwrToggle();
	}

	if (keyboardInfo[KEY_CENTRAL].mReleased)
	{
		mDimDown = !mDimDown;
	}

	if (keyboardInfo[KEY_CENTRAL].mLongPress)
	{
		if (mState == State::PwrOn)
		{
			if (difftime(time(nullptr) , mLastLong) > 5)
			{
				mDimDown = true;
			}
			mLastLong = time(nullptr);

			if (mDimDown)
			{
				mLuminance -= 20;
				if (mLuminance < 0)
				{
					mLuminance = 0;
					pwrOff();
				}
			}
			else
			{
				mLuminance += 20;
				if (mLuminance > 4000)
				{
					mLuminance = 4000;
				}
			}
		    std::lock_guard<std::mutex> lk_guard(mLedMutex);

			mRGBLed.luminance(mLuminance);
			mRGBLed.write();
		}
		if (mState == State::PwrOff)
		{
		    std::lock_guard<std::mutex> lk_guard(mLedMutex);

			mLuminance = 0;
			mRGBLed.luminance(mLuminance);
			mRGBLed.pwrOn();
			mState = State::SlowUp;
			startDimmerThread();
		}
	}

}
void Light::startDimmerThread()
{
	stopDimmerThread();

	mDimmerThreadRunning = true;

	mDimmerThread = new std::thread(&Light::dimmerThread, this);
}
void Light::stopDimmerThread()
{
	mDimmerThreadRunning = false;

    if (mDimmerThread)
    {
    	mDimmerThread->join();

        delete mDimmerThread;
        mDimmerThread = nullptr;
    }
}
void Light::dimmerThread()
{
	int sleepInterval = 1;
	int deltaLuminance = 20;
	int luminance = mLuminance;
	switch(mState)
	{
	case State::SlowUp:
		sleepInterval = 100;
		deltaLuminance = 1;
		LOG(INFO) << "RGBLed SlowUp";
		break;
	case State::SlowDown:
		sleepInterval = 100;
		deltaLuminance = -1;
		LOG(INFO) << "RGBLed SlowDown";
		break;
	case State::FastUp:
		sleepInterval = 1;
		deltaLuminance = 20;
		LOG(INFO) << "RGBLed FastUp";
		break;
	case State::FastDown:
		sleepInterval = 1;
		deltaLuminance = -20;
		LOG(INFO) << "RGBLed FastDown";
		break;
	default: break;
	}

   while (mDimmerThreadRunning)
   {
	   std::this_thread::sleep_for(std::chrono::milliseconds(sleepInterval));
	   luminance += deltaLuminance;

	   if (deltaLuminance > 0)
	   {
		   if (luminance >= mLuminance)
		   {
			   luminance = mLuminance;
			   mDimmerThreadRunning = false;
			   mState = State::PwrOn;
			   LOG(INFO) << "RGBLed PwrOn";
		   }
		   std::lock_guard<std::mutex> lk_guard(mLedMutex);

		   mRGBLed.luminance(luminance);
		   mRGBLed.write();
	   }
	   else
	   {
		   if (luminance <= 0)
		   {
			   luminance = 0;
			   mDimmerThreadRunning = false;
			   pwrOff();
			   LOG(INFO) << "RGBLed PwrOff";
		   }
		   std::lock_guard<std::mutex> lk_guard(mLedMutex);

		   mRGBLed.luminance(mLuminance);
		   mRGBLed.write();
	   }
   }
	mLuminance = luminance;
}
} /* namespace App */
