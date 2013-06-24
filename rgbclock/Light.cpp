/*
 * Light.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "Light.h"

namespace App
{

Light::Light(Hardware::I2C &i2c, uint8_t address) :
		mRGBLed(i2c, address)
{
	// TODO Auto-generated constructor stub

}

Light::~Light() {
	// TODO Auto-generated destructor stub
}

void Light::keyboardPressed(uint16_t value)
{

}
} /* namespace App */
