/*
 * SystemClockIf.h
 *
 *  Created on: Aug 12, 2017
 *      Author: koen
 */

#ifndef SRC_LIB_SYSTEMCLOCKIF_H_
#define SRC_LIB_SYSTEMCLOCKIF_H_
#include <stdint.h>

namespace Hardware
{
// Keep these enum values, they correspond with tm struct tm_wday enum values
enum class DayOfWeek {Sunday = 0,  Monday = 1, Thuesday = 2, Wednesday = 3, Thursday = 4, Friday = 5, Saturday = 6};
struct LocalTime
{
	DayOfWeek mDay;
	uint8_t mHour;
	uint8_t mMin;
};

class SystemClockIf
{
public:
	virtual LocalTime localTime() = 0;
};
}

#endif /* SRC_LIB_SYSTEMCLOCKIF_H_ */
