/*
 * MainboardControl.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: koen
 */

#include "MainboardControl.h"
namespace
{
const uint8_t RL_U17 = 0;
const uint8_t RL_U16 = 2;
const uint8_t RL_U15 = 4;
const uint8_t RL_U14 = 8;

const uint8_t MUTE      = 0;
const uint8_t RADIO_IN  = 2;
const uint8_t AUX_IN    = 4;
const uint8_t RADIO_RST = 8;
const uint8_t WATCHDOG  = 10;


}

namespace Hardware {

MainboardControl::MainboardControl(I2C &i2c, uint8_t hwrevision, uint8_t address) :
		mIO(i2c, address),
		mHwRevision(hwrevision)
{

}

MainboardControl::~MainboardControl()
{
}

void MainboardControl::keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state)
{
	if (keyboardInfo[KEY_UP].mShortPressed || keyboardInfo[KEY_UP].mLongPress)
	{
		//mIO.writeB()
	}

}
void MainboardControl::mute(bool mute)
{

}
void MainboardControl::resetTuner()
{

}
void MainboardControl::selectInput(InputSelection input)
{

}
void MainboardControl::signalWatchdog()
{

}
} /* namespace Hardware */
