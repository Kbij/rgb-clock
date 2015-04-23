/*
 * UpDownTimer.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: koen
 */

#include "UpDownTimer.h"
#include "UpDownDeviceIf.h"
#include <glog/logging.h>
#include <cmath>

namespace Hardware {

UpDownTimer::UpDownTimer(UpDownDeviceIf& device):
		mDevice(device),
		mThread(),
		mThreadMutex(),
	    mThreadRunning(false),
	    mStep(0),
	    mTotalSteps(0),
	    mThreadIntervalms(0),
	    mDown(false)
{

}

UpDownTimer::~UpDownTimer()
{
	stopDimmerThread();
}

void UpDownTimer::initiateUp(int target, int timeSeconds)
{
	LOG(INFO) << __PRETTY_FUNCTION__ << "1";
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);
	LOG(INFO) << __PRETTY_FUNCTION__ << "2";

	calculate(target, timeSeconds);
	mDown = false;
	startDimmerThread();
	LOG(INFO) << __PRETTY_FUNCTION__ << "3";
}

void UpDownTimer::initiateDown(int current, int timeSeconds)
{
	LOG(INFO) << __PRETTY_FUNCTION__ << "1";
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);
	LOG(INFO) << __PRETTY_FUNCTION__ << "2";

	calculate(current, timeSeconds);
	mDown = true;
	startDimmerThread();
	LOG(INFO) << __PRETTY_FUNCTION__ << "3";
}

void UpDownTimer::cancelDimmer()
{
	mThreadRunning = false;
}

void UpDownTimer::calculate(int steps, int timeSeconds)
{
	LOG(INFO) << "Calculated, steps=" << steps<< ", seconds=" << timeSeconds;
	mTotalSteps = steps;
	if (steps > (timeSeconds * 1000))
	{
		mStep = (int) ceil(((double)steps / ((double)timeSeconds * 1000)));
		mThreadIntervalms = 1;
	}
	else
	{
		mStep = 1;
		mThreadIntervalms = (int) (((double) timeSeconds * 1000) / (double) steps);
	}
	LOG(INFO) << "Calculated, step=" << mStep << ", interval=" << mThreadIntervalms;
}

void UpDownTimer::startDimmerThread()
{
	stopDimmerThread();

	mThreadRunning = true;
	mThread.reset(new std::thread(&UpDownTimer::dimmerThread, this));
}

void UpDownTimer::stopDimmerThread()
{
	mThreadRunning = false;

    if (mThread)
    {
    	mThread->join();
        mThread.reset();
    }
}

void UpDownTimer::dimmerThread()
{
	pthread_setname_np(pthread_self(), "UpDown Timer Thread");
	int totalSteps = mTotalSteps;
	int step = mStep;
	LOG(INFO) << "UpDownTimer thread started";

	while (mThreadRunning && (totalSteps > 0))
	{
	   std::this_thread::sleep_for(std::chrono::milliseconds(mThreadIntervalms));
	   if (mThreadRunning)
	   { // If still running
		   if (mDown)
		   {
			   // Down is forever, until stopped
			   mDevice.down(step);
		   }
		   else
		   {
			   totalSteps -= step;
			   mDevice.up(step);
		   }
	   }
	}

	LOG(INFO) << "UpDownTimer thread stopped";
}

} /* namespace Hardware */
