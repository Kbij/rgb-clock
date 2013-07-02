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
#include "RDSInfo.h"
#include "LightSensor.h"
#include "Radio.h"
#include <string>
#include <time.h>
#include <mutex>

namespace Hardware
{

class ClockDisplay : public RadioObserverIf {
public:
	ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress);
	virtual ~ClockDisplay();

	void showClock();
	void hideClock();

	void showVolume();
	void hideVolume();

	void showSignal();
	void hideSignal();

	void showRDSInfo();
	void hideRDSInfo();

	void showNextAlarm(const struct tm& nextAlarm);
	void hideNextAlarm();

    void radioRdsUpdate(RDSInfo rdsInfo);
    void radioStateUpdate(RadioInfo radioInfo);

private:
    void drawVolume();
    void eraseVolume();
    void drawSignal();
    void eraseSignal();
    void drawRDS();
    void eraseRDS();

	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();

	LCDisplay mLCDisplay;
	LightSensor mLightSensor;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    uint8_t mPrevMin;
    std::recursive_mutex mRadioInfoMutex;
    std::atomic_bool mRDSVisible;
    std::atomic_bool mVolumeVisible;
    std::atomic_bool mSignalVisible;
    std::string mRDSStationName;
    std::string mRDSText;
    int mRDSTextPos;
    int mReceiveLevel;
    int mVolume;
};
}
#endif /* CLOCKDISPLAY_H_ */
