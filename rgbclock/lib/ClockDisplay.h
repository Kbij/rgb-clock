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
#include "LCDBacklight.h"
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

enum class ClockState
{
	clkNormal,
	clkAlarm,
	clkSnooze
};
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

	void signalClockState(App::ClockState state);

    void radioRdsUpdate(RDSInfo rdsInfo);
    void radioStateUpdate(RadioInfo radioInfo);

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state);


private:
	void updateEditDisplay();
	void writeAlarm(int line, const App::Alarm& alarm);

	void drawVolume();
    void eraseVolume();
    void drawSignal();
    void eraseSignal();
    void drawRDS();
    void eraseRDS();

    void updateAlarmInfo();

	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();


	LCDisplay mLCDisplay;
	LCDBacklight mBackLight;
	Keyboard& mKeyboard;
	App::AlarmManager& mAlarmManager;

	std::atomic<DisplayState> mDisplayState;
	EditState mEditState;
	EditPos mEditPos;
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
    App::ClockState mClockState;
};
}
#endif /* CLOCKDISPLAY_H_ */
