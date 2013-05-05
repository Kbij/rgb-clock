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
#include <time.h>

class ClockDisplay {
public:
	ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress);
	virtual ~ClockDisplay();

	void showClock();
	void hideClock();

	void showVolume(uint8_t vol);
	void hideVolume();

	void showSignal(uint8_t signal);
	void hideSignal();

	void showRDSInfo(std::string rdsInfo);
	void hideRDSInfo();

	void showNextAlarm(const struct tm& nextAlarm);
	void hideNextAlarm();
private:
	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();

	LCDisplay mLCDisplay;
	//LightSensor mLightSensor;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    uint8_t mPrevMin;

};

#endif /* CLOCKDISPLAY_H_ */
