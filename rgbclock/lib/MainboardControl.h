/*
 * MainboardControl.h
 *
 *  Created on: Nov 30, 2013
 *      Author: koen
 */

#ifndef MAINBOARDCONTROL_H_
#define MAINBOARDCONTROL_H_

#include "I2C.h"
#include "IOExpander.h"
#include "KeyboardObserverIf.h"
#include <glog/logging.h>

#include <atomic>
#include <bitset>
#include <mutex>
#include <thread>

namespace Hardware {
class WatchdogFeederIf;

enum class InputSelection
{
	RadioIn,
	Auxin
};

struct FeederInfo
{
	int mPromiseTimeout;
	int mCurrentTimeout;
};

class MainboardControl:  public Hardware::KeyboardObserverIf {
public:
	MainboardControl(I2C &i2c, uint8_t hwrevision, uint8_t address, bool enableWatchdog);
	virtual ~MainboardControl();

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state);

	void promiseWatchdog(WatchdogFeederIf *watchdogFeeder, int timeoutMiliseconds);
	void removePromise(WatchdogFeederIf *watchdogFeeder);

	void mute(bool mute);
	void resetTuner();
	void selectInput(InputSelection input);
	void signalWatchdog(WatchdogFeederIf *watchdogFeeder);

private:
	void init();
	void stopWatchdog();
	void startWatchdogThread();
	void stopWatchdogThread();
	void watchdogThread();

	IOExpander mIO;
	const uint8_t mHwRevision;
	const bool mWatchdogEnabled;
	std::bitset<8> mRelaisBus;
	std::bitset<8> mMainBus;
	std::mutex mBusMutex;
	std::map<WatchdogFeederIf*, FeederInfo> mWatchdogFeeders;
    std::mutex mFeederMutex;
	int mWatchdogHandle;
    std::thread* mWatchdogThread;
    std::atomic_bool mWatchdogThreadRunning;

};

} /* namespace Hardware */
#endif /* MAINBOARDCONTROL_H_ */
