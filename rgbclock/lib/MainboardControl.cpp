/*
 * MainboardControl.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: koen
 */

#include "MainboardControl.h"
namespace
{
const uint8_t RL_U17 = 0;
const uint8_t RL_U16 = 2;
const uint8_t RL_U15 = 4;
const uint8_t RL_U14 = 8;

const uint8_t MUTE      = 0;
const uint8_t RADIO_IN  = 2;
const uint8_t AUX_IN    = 4;
const uint8_t RADIO_RST = 8;
const uint8_t WATCHDOG  = 10;


}

namespace Hardware {

MainboardControl::MainboardControl(I2C &i2c, uint8_t hwrevision, uint8_t address, bool enableWatchdog) :
		mIO(i2c, address),
		mHwRevision(hwrevision),
		mWatchdogEnabled(enableWatchdog),
		mRelaisBus(0),
		mMainBus(0),
		mBusMutex(),
		mWatchdogHandle(0),
		mWatchdogThread(nullptr),
		mWatchdogThreadRunning(false)
{
	init();
	if (mWatchdogEnabled)
	{
		startWatchdogThread();
	}
}

MainboardControl::~MainboardControl()
{
	stopWatchdogThread();
}

void MainboardControl::keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state)
{
	if (mHwRevision == 1)
	{
		return;
	}

	if (keyboardInfo[KEY_UP].mShortPressed || keyboardInfo[KEY_UP].mLongPress)
	{
		//mIO.writeB()
	}

}

void MainboardControl::promiseWatchdog(WatchdogFeederIf *watchdogFeeder, int timeoutMiliseconds)
{
	if (mWatchdogEnabled)
	{
		startWatchdogThread();
	}
}

void MainboardControl::removePromise(WatchdogFeederIf *watchdogFeeder)
{
	if (mWatchdogEnabled)
	{
		startWatchdogThread();
	}
}

void MainboardControl::mute(bool mute)
{
	if (mHwRevision == 1)
	{
		return;
	}

	LOG(INFO) << "Mute audio:" << mute;
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

	mMainBus.set(MUTE) = !mute;
	mIO.writeB(mMainBus.to_ulong());
}

void MainboardControl::resetTuner()
{
	if (mHwRevision == 1)
	{
		return;
	}

	LOG(INFO) << "Reset Tuner";
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

	mMainBus[RADIO_RST] = 0;
	mIO.writeB(mMainBus.to_ulong());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    mMainBus[RADIO_RST] = 1;
	mIO.writeB(mMainBus.to_ulong());
}

void MainboardControl::selectInput(InputSelection input)
{
	if (mHwRevision == 1)
	{
		return;
	}

	LOG(INFO) << "Select audio input";
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

	// Disable all first
	mMainBus[RADIO_IN] = 1;
	mMainBus[AUX_IN] = 1;
	mIO.writeB(mMainBus.to_ulong());

	if (input == InputSelection::RadioIn)
	{
		mMainBus[RADIO_IN] = 0;
		mMainBus[AUX_IN] = 1;
	}
	else
	{
		mMainBus[RADIO_IN] = 1;
		mMainBus[AUX_IN] = 0;
	}

	mIO.writeB(mMainBus.to_ulong());
}

void MainboardControl::signalWatchdog()
{
	if (mHwRevision == 1)
	{
		return;
	}

/*
         int deviceHandle;
        int disableWatchdog = 1;

        // test watchdog reset via t-param
        if (argc > 1) {
                if (!strncasecmp(argv[1], "-t", 2)) {
                        disableWatchdog = 0;
                }
        }

        printf("Disabling watchdog before closing device: %d\n", disableWatchdog);

        // open watchdog device on /dev/watchdog
        if ((deviceHandle = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) {
                printf("Error: Couldn't open watchdog device! %d\n", deviceHandle);
                return 1;
        }

        // get timeout info of watchdog (try to set it to 15s before)
        int timeout = 15;
        ioctl(deviceHandle, WDIOC_SETTIMEOUT, &timeout);
        ioctl(deviceHandle, WDIOC_GETTIMEOUT, &timeout);
        printf("The watchdog timeout is %d seconds.\n\n", timeout);

        // feed watchdog 3 times with heartbeats
        int i;
        for (i = 0; i < 3; i++) {
                printf("Feeding the dog with a heartbeat.\n");
                ioctl(deviceHandle, WDIOC_KEEPALIVE, 0);
                sleep(10);
        }

        if (disableWatchdog)
        {
                printf("Disable watchdog.\n");
                write(deviceHandle, "V", 1);
        }

        // close connection and return
        close(deviceHandle);
        return 0;
 */
}

void MainboardControl::stopWatchdog()
{
	if (mWatchdogHandle > 0)
	{
		LOG(INFO) << "Disable watchdog";
	    write(mWatchdogHandle, "V", 1);
	}
}

void MainboardControl::init()
{
	mIO.directionA(IOExpander::DataDirection::dirOut);
	mIO.directionB(IOExpander::DataDirection::dirOut);

	mIO.writeA(mRelaisBus.to_ulong());

	mMainBus[MUTE] = 0;
	mMainBus[RADIO_IN] = 0;
	mMainBus[AUX_IN] = 0;
	mMainBus[RADIO_RST] = 0;
	mIO.writeB(mMainBus.to_ulong());
}

void MainboardControl::startWatchdogThread()
{
	// Delete any previous running thread
	stopWatchdogThread();

	mWatchdogThreadRunning = true;

	mWatchdogThread = new std::thread(&MainboardControl::watchdogThread, this);
}

void MainboardControl::stopWatchdogThread()
{
	mWatchdogThreadRunning = false;

    if (mWatchdogThread)
    {
    	mWatchdogThread->join();

        delete mWatchdogThread;
        mWatchdogThread = nullptr;
    }
}

void MainboardControl::watchdogThread()
{
	   while (mWatchdogThreadRunning)
	   {
		   std::this_thread::sleep_for(std::chrono::milliseconds(100));


	   }
}

} /* namespace Hardware */
