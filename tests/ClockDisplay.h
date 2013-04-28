/*
 * ClockDisplay.h
 *
 *  Created on: Apr 28, 2013
 *      Author: koen
 */

#ifndef CLOCKDISPLAY_H_
#define CLOCKDISPLAY_H_
#include "LCDisplay.h"
#include "LightSensor.h"
#include <string>

class ClockDisplay {
public:
	ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress);
	virtual ~ClockDisplay();

	void showClock();
	void hideClock();

	void showVolume(uint8_t vol);
	void hideVolume();

	void showSignal(uint8_t vol);
	void hideSignal();

	void showRDSInfo(std::string rdsInfo);
	void hideRDSInfo();

private:
	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();

	LCDisplay mLCDisplay;
	LightSensor mLightSensor;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    uint8_t mPrevMin;

};

#endif /* CLOCKDISPLAY_H_ */
