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
#include "KeyboardObserverIf.h"
#include "RDSInfo.h"
#include "LightSensor.h"
#include "Radio.h"
#include <string>
#include <time.h>
#include <mutex>

namespace
{
enum class DisplayState
{
	stNormal,
	stEditAlarms
};

enum class EditState
{
	edUndefined,
	edListAlarms,
	edConfirmDelete,
	edEditAlarm
};

enum class EditPos
{
	posIndex,
	posEnable,
	posUnit,
	posHourT,
	posHourE,
	posMinT,
	posMinE,
	posOneTime,
	posDaySu,
	posDayMo,
	posDayTu,
	posDayWe,
	posDayTh,
	posDayFr,
	posDaySa,
	posVol
};

}
namespace App
{
class AlarmManager;
struct Alarm;
struct UnitConfig;
}
namespace Hardware
{
class Keyboard;

class ClockDisplay : public RadioObserverIf, public Hardware::KeyboardObserverIf {
public:
	ClockDisplay(I2C &i2c, Keyboard& keyboard, App::AlarmManager &alarmManager, const App::UnitConfig& unitConfig);
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

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo, Hardware::KeyboardState state);


private:
	void updateEditDisplay();
	void writeAlarm(int line, const App::Alarm& alarm);

	void drawVolume();
    void eraseVolume();
    void drawSignal();
    void eraseSignal();
    void drawRDS();
    void eraseRDS();

    bool confirmDelete();

	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();


	LCDisplay mLCDisplay;
	LightSensor mLightSensor;
	Keyboard& mKeyboard;
	App::AlarmManager& mAlarmManager;

	std::atomic<DisplayState> mDisplayState;
	EditState mEditState;
	EditPos mEditPos;
	unsigned int mAlarmCount;
    std::thread* mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    std::atomic_bool mForceRefresh;
    std::recursive_mutex mRadioInfoMutex;
    std::atomic_bool mRDSVisible;
    std::atomic_bool mVolumeVisible;
    std::atomic_bool mSignalVisible;
    std::string mRDSStationName;
    std::string mRDSText;
    int mRDSTextPos;
    int mReceiveLevel;
    int mVolume;
    const std::string mUnitName;
    unsigned int mAlarmEditIndex;
    bool mConfirmDelete;
};
}
#endif /* CLOCKDISPLAY_H_ */
