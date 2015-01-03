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
const int MIN_DIMMER_INTENSITY = 40;
const int MAX_DIMMER_INTENSITY = 4000;
const int DIMMER_STEP = 20;
const int AUTO_OFF_MINUTES = 60;

Light::Light(I2C &i2c, uint8_t address) :
		mState(State::PwrOff),
		mRGBLed(i2c, address),
		mLuminance(1000),
		mLastLong(time(0)),
		mDimDown(true),
		mLedMutex(),
		mDimmerMutex(),
		mThreadMutex(),
	    mDimmerThread(nullptr),
	    mDimmerThreadRunning(false),
	    mAutoOffThread(nullptr),
	    mAutoOffThreadRunning(false)
{
	pwrOff();
	mRGBLed.hue(200);
	mRGBLed.saturation(4000);
}

Light::~Light()
{
	mRGBLed.pwrOff();
	stopDimmerThread();
	stopAutoOffThread();
}

void Light::pwrOn()
{
    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

	if (mState == State::PwrOff)
	{
		if (mLuminance < MIN_DIMMER_INTENSITY)
		{
			mLuminance = MIN_DIMMER_INTENSITY;
		}
		initiateFastUp();
		startAutoOffThread();
	}
}

void Light::pwrSlowOn()
{
    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

	pwrSlowOn(0);
}

void Light::pwrSlowOn(int startLum)
{
	initiateSlowUp(startLum);
	startAutoOffThread();
}

void Light::pwrOff()
{
    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

    pwrOff(false);
}

void Light::pwrOff(bool autoPowerOff)
{

    if ((mState == State::PwrOn) ||(mState == State::SlowUp))
	{
		initiateFastDown();
		if (!autoPowerOff)
		{
			stopAutoOffThread();
		}
	}
}

void Light::pwrToggle()
{
	if ((mState == State::PwrOn) ||(mState == State::SlowUp))
	{
		stopAutoOffThread();
		initiateFastDown();
	}
	if (mState == State::PwrOff)
	{
	    initiateFastUp();
	    startAutoOffThread();
	}
}

void Light::keyboardPressed(const std::vector<KeyInfo>& keyboardInfo, KeyboardState state)
{
    std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

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
				mLuminance -= DIMMER_STEP;

				if (mLuminance <= MIN_DIMMER_INTENSITY)
				{
					mLuminance = MIN_DIMMER_INTENSITY;
				    std::lock_guard<std::mutex> lk_guard(mLedMutex);

					mRGBLed.luminance(mLuminance);
				}
			}
			else
			{
				mLuminance += DIMMER_STEP;
				if (mLuminance > MAX_DIMMER_INTENSITY)
				{
					mLuminance = MAX_DIMMER_INTENSITY;
				}
			}
		    std::lock_guard<std::mutex> lk_guard(mLedMutex);

			mRGBLed.luminance(mLuminance);
		}
		if (mState == State::PwrOff)
		{
			pwrSlowOn(MIN_DIMMER_INTENSITY);
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
	if (mLuminance < MIN_DIMMER_INTENSITY)
	{
		mLuminance = MIN_DIMMER_INTENSITY;
	}
	std::lock_guard<std::mutex> lk_guard(mLedMutex);

	mRGBLed.luminance(mLuminance);
	mRGBLed.pwrOn();
	mState = State::FastUp;
	startDimmerThread();
}

void Light::initiateFastDown()
{
	mState = State::FastDown;
	startDimmerThread();
}

void Light::initiateSlowUp(int start)
{
	LOG(INFO) << "Init start, lum=" << start;

	mLuminance = start;
    std::lock_guard<std::mutex> lk_guard(mLedMutex);

	mRGBLed.luminance(mLuminance);
	mRGBLed.pwrOn();
	mState = State::SlowUp;
	startDimmerThread();
}

void Light::startDimmerThread()
{
	stopDimmerThread();
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mDimmerThreadRunning = true;
	mDimmerThread.reset(new std::thread(&Light::dimmerThread, this));
}

void Light::stopDimmerThread()
{
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mDimmerThreadRunning = false;

    if (mDimmerThread)
    {
    	mDimmerThread->join();
        mDimmerThread.reset();
    }
}

void Light::startAutoOffThread()
{
	stopAutoOffThread();
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mAutoOffThreadRunning = true;
	mAutoOffThread.reset(new std::thread(&Light::autoOffThread, this));
}

void Light::stopAutoOffThread()
{
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mAutoOffThreadRunning = false;

    if (mAutoOffThread)
    {
    	mAutoOffThread->join();
    	mAutoOffThread.reset();
    }
}

void Light::dimmerThread()
{
	pthread_setname_np(pthread_self(), "Dimmer");

	int sleepInterval = 1;
	int deltaLuminance = DIMMER_STEP;
	int targetLuminance = 0;
	int memorizedLuminance = 0;
	const int slowUpMinutes = 15;

	switch(mState)
	{
	case State::SlowUp:
		targetLuminance = 1500;
   	    sleepInterval = (double (slowUpMinutes * 60.0 )/ double(targetLuminance)) * 1000;
		deltaLuminance = 1;
		//mLuminance = 0;
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
		deltaLuminance = DIMMER_STEP;
		targetLuminance = mLuminance;
		mLuminance = 0;
		LOG(INFO) << "RGBLed FastUp";
		break;
	case State::FastDown:
		sleepInterval = 1;
		deltaLuminance = -DIMMER_STEP;
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
	   std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

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
   std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

   if (memorizedLuminance > 0)
   {
	   mLuminance = memorizedLuminance;
   }

   LOG(INFO) << "Thread Exit";

}

void Light::autoOffThread()
{
	pthread_setname_np(pthread_self(), "Light AutoOff");

	int countDownSeconds = AUTO_OFF_MINUTES * 60;
	while (mAutoOffThreadRunning)
	{
	   std::this_thread::sleep_for(std::chrono::seconds(1));
	   --countDownSeconds;
	   if (countDownSeconds == 0)
	   {

		   mAutoOffThreadRunning = false;
		   std::lock_guard<std::mutex> lk_guard(mDimmerMutex);

		   LOG(INFO) << "Light auto off";
		   pwrOff(true);
	   }
	}
}
} /* namespace Hardware */
