/*
 * UpDownTimer.h
 *
 *  Created on: Apr 16, 2015
 *      Author: koen
 */

#ifndef UPDOWNTIMER_H_
#define UPDOWNTIMER_H_
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>

namespace Hardware {
class UpDownDeviceIf;

class UpDownTimer {
public:
	UpDownTimer(UpDownDeviceIf& device);
	virtual ~UpDownTimer();

	void initiateUp(int target, int timeSeconds);
	void initiateDown(int current, int timeSeconds);
	void cancelDimmer();

private:
	UpDownDeviceIf& mDevice;
	std::unique_ptr<std::thread> mThread;
	std::mutex mThreadMutex;
    std::atomic_bool mThreadRunning;
    int mStep;
    int mTotalSteps;
    int mThreadIntervalms;
    bool mDown;

    void calculate(int steps, int timeSeconds);
	void startDimmerThread();
	void stopDimmerThread();
	void dimmerThread();

};

} /* namespace Hardware */
#endif /* UPDOWNTIMER_H_ */
