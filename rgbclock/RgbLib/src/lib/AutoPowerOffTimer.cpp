/*
 * AutoPowerOffTimer.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: koen
 */

#include "AutoPowerOffTimer.h"
#include "AutoPowerOffDeviceIf.h"
#include "glog/logging.h"
#include <time.h>
#include <pthread.h>

namespace Hardware {

AutoPowerOffTimer::AutoPowerOffTimer(AutoPowerOffDeviceIf& device):
		mDevice(device),
		mAutoOffThread(nullptr),
		mThreadMutex(),
		mAutoOffThreadRunning(false)
{

}

AutoPowerOffTimer::~AutoPowerOffTimer()
{
	stopAutoOffThread();
}

void AutoPowerOffTimer::startAutoPowerOff(int minutes)
{
	VLOG(1) << "Start PowerOff";
	startAutoOffThread(minutes);
}

void AutoPowerOffTimer::cancelAutoPowerOff()
{
	mAutoOffThreadRunning = false;
}

void AutoPowerOffTimer::startAutoOffThread(int minutes)
{
	stopAutoOffThread();
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mAutoOffThreadRunning = true;
	mAutoOffThread.reset(new std::thread(&AutoPowerOffTimer::autoOffThread, this, minutes));
}

void AutoPowerOffTimer::stopAutoOffThread()
{
    std::lock_guard<std::mutex> lk_guard(mThreadMutex);

	mAutoOffThreadRunning = false;

    if (mAutoOffThread)
    {
    	mAutoOffThread->join();
    	mAutoOffThread.reset();
    }
}

void AutoPowerOffTimer::autoOffThread(int minutes)
{
	pthread_setname_np(pthread_self(), "Auto PowerOff");
	VLOG(1) << "AutoPowerOff thread started";

	int countDownSeconds = minutes * 60;
	while (mAutoOffThreadRunning)
	{
	   std::this_thread::sleep_for(std::chrono::seconds(1));
	   --countDownSeconds;
	   if (countDownSeconds == 0)
	   {
		   if (mAutoOffThreadRunning)
		   { // If still running
			   mAutoOffThreadRunning = false;
			   VLOG(1) << "Auto PowerOff";
			   mDevice.pwrOff();
		   }
	   }
	}

	VLOG(1) << "AutoPowerOff thread stopped";
}

} /* namespace App */
