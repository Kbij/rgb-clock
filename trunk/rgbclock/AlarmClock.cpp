/*
 * AlarmClock.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: koen
 */

#include "AlarmClock.h"
#include "Light.h"

namespace App {

AlarmClock::AlarmClock(Hardware::I2C &i2c, Addresses addresses) :
	mAddresses(addresses),
	mKeyboard(i2c, addresses.mKeyboard),
	mLight(nullptr)
{
	// TODO Auto-generated constructor stub

}

AlarmClock::~AlarmClock()
{
	// TODO Auto-generated destructor stub
}

void AlarmClock::registerLight(Light *light)
{
	mLight = light;
	mKeyboard.registerKeyboardObserver(light);
}

void AlarmClock::unregisterLight()
{
	mLight = nullptr;
}

} /* namespace App */
