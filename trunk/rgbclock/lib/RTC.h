/*
 * RTC.h
 *
 *  Created on: Jun 13, 2013
 *      Author: koen
 */

#ifndef RTC_H_
#define RTC_H_
#include "I2C.h"
#include <atomic>
#include <thread>
#include <fstream>
#include <ctime>

namespace Hardware
{
class RTC {
public:
	RTC(I2C &i2c, uint8_t address);
	virtual ~RTC();

	bool ntpSynchronized();
	void showNTPStatus();

private:
	// Is the RTC clock set to a valid date/time ?
	bool rtcValidDateTime();
	std::string runCmd(const std::string& cmd, bool log);
	struct std::tm readRTCTime();
	void writeRTCTime();

	void startRTCUpdateThread();
	void stopRTCUpdateThread();
	void rtcThread();

	I2C &mI2C;
	const uint8_t mAddress;
    std::thread* mRTCThread;
    std::atomic_bool mRTCThreadRunning;
    std::ofstream mRTCStartupLog;

};
}
#endif /* RTC_H_ */
