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

	void showNTPStatus();
	bool isNTPSync();

private:
	// Is the RTC clock set to a valid date/time ?
	bool ntpSynchronized();
	bool rtcValidDateTime(struct std::tm utcTime);
	std::string runCmd(const std::string& cmd, bool log);
	struct std::tm readRTCTime();
	void writeRTCTime();
	void setSystemTime(struct std::tm utcTime);

	void log(const std::string& message);

	void startRTCUpdateThread();
	void stopRTCUpdateThread();
	void rtcThread();

	I2C &mI2C;
	const uint8_t mAddress;
    std::thread* mRTCThread;
    std::atomic_bool mRTCThreadRunning;
    std::ofstream mRTCStartupLog;
    std::atomic_bool mNTPSync;

};
}
#endif /* RTC_H_ */
