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
struct LocalTime
{
	uint8_t mWDay;
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
