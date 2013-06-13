/*
 * RTC.h
 *
 *  Created on: Jun 13, 2013
 *      Author: koen
 */

#ifndef RTC_H_
#define RTC_H_
#include "I2C.h"

class RTC {
public:
	RTC(I2C &i2c);
	virtual ~RTC();

	void showNTPStatus();
private:
	I2C &mI2C;
};

#endif /* RTC_H_ */
