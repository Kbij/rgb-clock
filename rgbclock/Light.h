/*
 * Light.h
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include "lib/I2C.h"
#include "lib/RgbLed.h"
#include "lib/KeyboardObserverIf.h"
#include <stdint.h>

namespace Hardware
{
class I2C;
}

namespace App {

class Light : public Hardware::KeyboardObserverIf {
public:
	Light(Hardware::I2C &i2c, uint8_t address);
	virtual ~Light();

	void keyboardPressed(uint16_t value);
private:
	Hardware::RgbLed mRGBLed;
};

} /* namespace App */
#endif /* LIGHT_H_ */
