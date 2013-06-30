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

	void showVolume(uint8_t vol);
	void hideVolume();

	void showSignal(uint8_t signal);
	void hideSignal();

	void showRDSInfo();
	void hideRDSInfo();

	void showNextAlarm(const struct tm& nextAlarm);
	void hideNextAlarm();

	void infoAvailable(RDSInfo rdsInfo);
    void volumeChange(int volume);
private:
	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();

	LCDisplay mLCDisplay;
	LightSensor mLightSensor;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    uint8_t mPrevMin;
    std::recursive_mutex mRDSInfoMutex;
    bool mRDSVisible;
    std::atomic_bool mNewRDSAvailable;
    std::string mRDSStationName;
    std::string mRDSText;
    int mRDSTextPos;
    int mReceiveLevel;

};
}
#endif /* CLOCKDISPLAY_H_ */
