/*
 * SystemClock.cpp
 *
 *  Created on: Aug 12, 2017
 *      Author: koen
 */

#include <lib/SystemClock.h>
#include <ctime>


namespace Hardware {

SystemClock::SystemClock()
{
}

SystemClock::~SystemClock()
{
}

LocalTime SystemClock::localTime()
{
	LocalTime result;
	time_t rawTime;
	struct tm* timeInfo;
	time(&rawTime);
	timeInfo = localtime(&rawTime);
	result.mDay = (DayOfWeek)timeInfo->tm_wday;
	result.mHour = timeInfo->tm_hour;
	result.mMin = timeInfo->tm_min;

	return result;
}
} /* namespace Hardware */
