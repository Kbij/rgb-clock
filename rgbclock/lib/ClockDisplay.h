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
#include <memory>

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
class RTC;

class ClockDisplay : public RadioObserverIf, public Hardware::KeyboardObserverIf {
public:
	ClockDisplay(I2C &i2c, RTC &rtc, Keyboard& keyboard, App::AlarmManager &alarmManager, uint8_t hwrevision, const App::UnitConfig& unitConfig);
	virtual ~ClockDisplay();

	void signalClockState(App::ClockState state);

    void radioRdsUpdate(RDSInfo rdsInfo);
    void radioStateUpdate(RadioInfo radioInfo);

	void keyboardPressed(const KeyboardInfo& keyboardInfo);
private:

	void updateEditDisplay();
	void writeAlarm(int line, const App::Alarm& alarm);

	void drawVolume();
    void eraseVolume();
    void drawSignal();
    void eraseSignal();
    void drawRDS();
    void eraseRDS();
	void drawNTPState();


    void updateAlarmInfo();

	void startRefreshThread();
	void stopRefreshThread();

	void refreshThread();


	RTC& mRTC;
	LCDisplay mLCDisplay;
	LCDBacklight mBackLight;
	Keyboard& mKeyboard;
	App::AlarmManager& mAlarmManager;

	std::atomic<DisplayState> mDisplayState;
	EditState mEditState;
	EditPos mEditPos;
    std::unique_ptr<std::thread> mRefreshThread;
    std::atomic_bool mRefreshThreadRunning;
    std::atomic_bool mForceRefresh;
    std::recursive_mutex mRadioInfoMutex;
    std::atomic_bool mRadioOn;
    std::string mRDSStationName;
    std::string mRDSText;
    int mRDSTextPos;
    int mReceiveLevel;
    int mVolume;
    const std::string mUnitName;
    unsigned int mAlarmEditIndex;
    bool mConfirmDelete;
    App::ClockState mClockState;
    std::atomic_bool mUpdateEditDisplay;

};
}
#endif /* CLOCKDISPLAY_H_ */
