/*
 * ClockDisplay.h
 *
 *  Created on: Apr 28, 2013
 *      Author: koen
 */

#ifndef CLOCKDISPLAY_H_
#define CLOCKDISPLAY_H_
#include "LCDisplay.h"
#include "RadioObserverIf.h"
#include "LightSensor.h"
#include "FMReceiver.h"
#include <string>
#include <time.h>
#include <mutex>

class ClockDisplay : public RadioObserverIf {
public:
	ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress, FMReceiver& receiver);
	virtual ~ClockDisplay();

	void showClock();
	void hideClock();

	void showVolume(uint8_t vol);
	void hideVolume();

	void showSignal(uint8_t signal);
	void hideSignal();

	void showRDSInfo();
	void hideRDSInfo();

	void showNextAlarm(const struct tm& nextAlarm);
	void hideNextAlarm();

	void infoAvailable(InfoType type);
private:
	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();

	LCDisplay mLCDisplay;
	LightSensor mLightSensor;
	FMReceiver& mFMReceiver;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    uint8_t mPrevMin;
    std::recursive_mutex mRDSInfoMutex;
    bool mRDSVisible;
    std::string mRDSStationName;
    std::string mRDSText;
    int mReceiveLevel;

};

#endif /* CLOCKDISPLAY_H_ */
