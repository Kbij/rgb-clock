/*
 * MainboardControl.cpp
 *
 *  Created on: Nov 30, 2013
 *      Author: koen
 */

#include "MainboardControl.h"

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

} /* namespace Hardware */
