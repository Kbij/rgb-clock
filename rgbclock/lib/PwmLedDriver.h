/*
 * PwmLedDriver.h
 *
 *  Created on: Dec 11, 2013
 *      Author: koen
 */

#ifndef PWMLEDDRIVER_H_
#define PWMLEDDRIVER_H_
#include <stdint.h>

namespace Hardware {
class I2C;

class PwmLedDriver {

public:
	enum class PwmChannel
	{
		Channel1,
		Channel2,
		Channel3
	};
	PwmLedDriver(I2C &i2c, uint8_t address);
	virtual ~PwmLedDriver();

	void powerOn(bool powerOn);
	void pwmValue(PwmChannel channel, uint16_t value);

	bool isAttached();
private:
	void init();
	void chipSleep();

	I2C &mI2C;
	const uint8_t mAddress;
};

} /* namespace Hardware */
#endif /* PWMLEDDRIVER_H_ */
