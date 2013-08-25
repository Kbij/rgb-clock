/*
 * RTC.h
 *
 *  Created on: Jun 13, 2013
 *      Author: koen
 */

#ifndef RTC_H_
#define RTC_H_
#include "I2C.h"
namespace Hardware
{
class RTC {
public:
	RTC(I2C &i2c);
	virtual ~RTC();

	bool ntpSynchronized();
	void showNTPStatus();

private:
	// Is the RTC clock set to a valid date/time ?
	bool rtcValidDateTime();
	I2C &mI2C;
};
}
#endif /* RTC_H_ */
