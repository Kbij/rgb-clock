/*
 * AutoPowerOffTimer.h
 *
 *  Created on: Apr 15, 2015
 *      Author: koen
 */

#ifndef AUTOPOWEROFFTIMER_H_
#define AUTOPOWEROFFTIMER_H_
#include <atomic>
#include <thread>
#include <mutex>


namespace Hardware {
class AutoPowerOffDeviceIf;

class AutoPowerOffTimer {
public:
	AutoPowerOffTimer(AutoPowerOffDeviceIf& device);
	virtual ~AutoPowerOffTimer();

	void startAutoPowerOff(int minutes);
	void cancelAutoPowerOff();
private:
	AutoPowerOffDeviceIf& mDevice;
	std::unique_ptr<std::thread> mAutoOffThread;
	std::mutex mThreadMutex;
    std::atomic_bool mAutoOffThreadRunning;

	void startAutoOffThread(int minutes);
	void stopAutoOffThread();
	void autoOffThread(int minutes);

};

} /* namespace App */
#endif /* AUTOPOWEROFFTIMER_H_ */
