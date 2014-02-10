/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"
#include <glog/logging.h>
#include <time.h>
#include <pthread.h>

namespace Hardware
{
const int MIN_DIMMER_INTENSITY = 20;

Light::Light(I2C &i2c, uint8_t address) :
		mState(State::PwrOff),
		mRGBLed(i2c, address),
		mLuminance(1000),
		mLastLong(time(0)),
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
	if (mState == State::PwrOff)
	{
		if (mLuminance < 10)
		{
			mLuminance = 10;
		}

		initiateFastUp();
	}
}

void Light::pwrSlowOn()
{
	initiateSlowUp();
}

void Light::pwrOff()
{
	if ((mState == State::PwrOn) ||(mState == State::SlowUp))
	{
		initiateFastDown();
	}
}

void Light::pwrToggle()
{
	if ((mState == State::PwrOn) ||(mState == State::SlowUp))
	{
		initiateFastDown();
	}
	if (mState == State::PwrOff)
	{
	    initiateFastUp();
	}
}

void Light::keyboardPressed(const std::vector<KeyInfo>& keyboardInfo, KeyboardState state)
{
	if (state != KeyboardState::stNormal)
	{
		return;
	}

	if (keyboardInfo[KEY_CENTRAL_L].mShortPressed || keyboardInfo[KEY_CENTRAL_R].mShortPressed)
	{
		pwrToggle();
	}

	if (keyboardInfo[KEY_CENTRAL_L].mReleased || keyboardInfo[KEY_CENTRAL_R].mReleased)
	{
		mDimDown = !mDimDown;
	}

	if (keyboardInfo[KEY_CENTRAL_L].mLongPress || keyboardInfo[KEY_CENTRAL_R].mLongPress)
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

				if (mLuminance <= MIN_DIMMER_INTENSITY)
				{
					mLuminance = MIN_DIMMER_INTENSITY;
				    std::lock_guard<std::mutex> lk_guard(mLedMutex);

					mRGBLed.luminance(mLuminance);
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
		}
		if (mState == State::PwrOff)
		{
			initiateSlowUp();
		}
	}

}

bool Light::isAttached()
{
	std::lock_guard<std::mutex> lk_guard(mLedMutex);

	return mRGBLed.isAttached();
}

void Light::initiateFastUp()
{
	LOG(INFO) << "Init fast, lum=" << mLuminance;
	if (mLuminance < 10)
	{
		mLuminance = 10;
	}
	std::lock_guard<std::mutex> lk_guard(mLedMutex);

	mRGBLed.luminance(0);
	mRGBLed.pwrOn();
	mState = State::FastUp;
	startDimmerThread();
}

void Light::initiateFastDown()
{
	mState = State::FastDown;
	startDimmerThread();
}

void Light::initiateSlowUp()
{
	mLuminance = 0;
    std::lock_guard<std::mutex> lk_guard(mLedMutex);

	mRGBLed.luminance(mLuminance);
	mRGBLed.pwrOn();
	mState = State::SlowUp;
	startDimmerThread();
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
	pthread_setname_np(pthread_self(), "Dimmer");

	int sleepInterval = 1;
	int deltaLuminance = 20;
	int targetLuminance = 0;
	int memorizedLuminance = 0;
	const int slowUpMinutes = 10;

	switch(mState)
	{
	case State::SlowUp:
		targetLuminance = 1500;
   	    sleepInterval = (double (slowUpMinutes * 60.0 )/ double(targetLuminance)) * 1000;
		deltaLuminance = 1;
		mLuminance = 0;
		LOG(INFO) << "RGBLed SlowUp";
		break;
	case State::SlowDown:
		sleepInterval = 100;
		deltaLuminance = -1;
		targetLuminance = 0;
		memorizedLuminance = mLuminance;
		LOG(INFO) << "RGBLed SlowDown";
		break;
	case State::FastUp:
		sleepInterval = 1;
		deltaLuminance = 20;
		targetLuminance = mLuminance;
		mLuminance = 0;
		LOG(INFO) << "RGBLed FastUp";
		break;
	case State::FastDown:
		sleepInterval = 1;
		deltaLuminance = -20;
		targetLuminance = 0;
		memorizedLuminance = mLuminance;
		LOG(INFO) << "RGBLed FastDown";
		break;
	default: break;
	}
	LOG(INFO) << "Thread Start, target: " << targetLuminance;

   while (mDimmerThreadRunning)
   {
	   std::this_thread::sleep_for(std::chrono::milliseconds(sleepInterval));
	   mLuminance += deltaLuminance;

	   if (deltaLuminance > 0)
	   {
		   if (mLuminance >= targetLuminance)
		   {
			   mLuminance = targetLuminance;
			   mDimmerThreadRunning = false;
			   mState = State::PwrOn;
			   LOG(INFO) << "RGBLed PwrOn";
		   }
		   std::lock_guard<std::mutex> lk_guard(mLedMutex);

		   mRGBLed.luminance(mLuminance);
	   }
	   else
	   {
		   if (mLuminance <= 0)
		   {
			   mLuminance = 0;
			   mDimmerThreadRunning = false;
			   mState = State::PwrOff;

			   std::lock_guard<std::mutex> lk_guard(mLedMutex);
			   mRGBLed.pwrOff();

			   LOG(INFO) << "RGBLed PwrOff";
		   }
		   else
		   {
			   std::lock_guard<std::mutex> lk_guard(mLedMutex);

			   mRGBLed.luminance(mLuminance);
		   }
	   }
   }

   if (memorizedLuminance > 0)
   {
	   mLuminance = memorizedLuminance;
   }

   LOG(INFO) << "Thread Exit";

}
} /* namespace Hardware */
