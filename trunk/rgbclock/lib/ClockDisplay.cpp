/*
 * ClockDisplay.cpp
 *
 *  Created on: Apr 28, 2013
 *      Author: koen
 */

#include "ClockDisplay.h"
#include "I2C.h"
#include "RTC.h"
#include "Keyboard.h"
#include "AlarmManager.h"
#include "Config.h"
#include <sstream>
#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include <pthread.h>

namespace {
const int LINESPACING = 8;
const int LINE1 = 0;
const int LINE2 = LINESPACING;
const int LINE3 = 2 * LINESPACING;
const int LINE4 = 3 * LINESPACING;

const int POS_ENABLE = 8;
const int POS_UNITNAME = 16;
const int POS_HOUR_T = 50;
const int POS_HOUR_E = 56;
const int POS_MIN_T = 64;
const int POS_MIN_E = 70;
const int POS_ONETIME = 82;
const int POS_DAYS = 90;
const int POS_DAY_SU = 95;
const int POS_DAY_MO = 102;
const int POS_DAY_TU = 108;
const int POS_DAY_WE = 114;
const int POS_DAY_TH = 120;
const int POS_DAY_FR = 126;
const int POS_DAY_SA = 132;

const int POS_VOLUME = 148;
}

namespace Hardware
{
ClockDisplay::ClockDisplay(I2C &i2c, RTC &rtc, Keyboard& keyboard, App::AlarmManager &alarmManager, uint8_t hwrevision, const App::UnitConfig& unitConfig) :
	mRTC(rtc),
	mLCDisplay(i2c, unitConfig.mLCD),
	mBackLight(i2c, hwrevision, unitConfig.mBackLight, unitConfig.mLightSensor),
	mKeyboard(keyboard),
	mAlarmManager(alarmManager),
	mDisplayState(DisplayState::stNormal),
	mEditState(EditState::edUndefined),
	mEditPos(EditPos::posEnable),
	mRefreshThread(nullptr),
	mRefreshThreadRunning(false),
	mForceRefresh(false),
	mRadioInfoMutex(),
	mRadioOn(false),
	mRDSStationName(),
	mRDSText(),
	mRDSTextPos(0),
	mReceiveLevel(0),
	mVolume(0),
	mUnitName(unitConfig.mName),
	mAlarmEditIndex(0),
	mConfirmDelete(false),
	mClockState(App::ClockState::clkNormal),
	mUpdateEditDisplay(false)
{
	mLCDisplay.initGraphic();
	mLCDisplay.clearGraphicDisplay();
	mKeyboard.registerKeyboardObserver(this);
	startRefreshThread();
}

ClockDisplay::~ClockDisplay()
{
	LOG(INFO) << "ClockDisplay destructor";
	mKeyboard.unRegisterKeyboardObserver(this);
	stopRefreshThread();
	mLCDisplay.clearGraphicDisplay();

	LOG(INFO) << "ClockDisplay destructor exit";
}

void ClockDisplay::signalClockState(App::ClockState state)
{
	mClockState = state;
}

void ClockDisplay::radioRdsUpdate(RDSInfo rdsInfo)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);
	if (mRDSStationName != rdsInfo.mStationName)
	{
		mRDSStationName = rdsInfo.mStationName;
		mRDSStationName = mRDSStationName.substr(0, 7);
		mRDSStationName.append(7 - mRDSStationName.size(), ' ');
	}

	if (mRDSText != rdsInfo.mText)
	{
		mRDSText = rdsInfo.mText;
		mRDSTextPos = 0;
	}

	// Receive Max is 127
	// Normalize to 50
	mReceiveLevel = rdsInfo.mReceiveLevel;
	if (mReceiveLevel > 50)
	{
		mReceiveLevel = 100;
	}
	mReceiveLevel = static_cast<int>(static_cast<double> (mReceiveLevel) / 50 * 100);
}

void ClockDisplay::radioStateUpdate(RadioInfo radioInfo)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);

	mVolume = radioInfo.mVolume;
	mRadioOn = radioInfo.mState == RadioState::PwrOn;
}

void ClockDisplay::keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, Hardware::KeyboardState state)
{
	mBackLight.signalUserActivity();


	if (keyboardInfo[KEY_LEFT].mLongPress && !(keyboardInfo[KEY_LEFT].mRepeat))
	{
		if (state == KeyboardState::stNormal)
		{
			App::AlarmList* alarms = mAlarmManager.editAlarms(mUnitName);
			if (alarms)
			{
				mKeyboard.keyboardState(KeyboardState::stAlarmEdit);
				mAlarmEditIndex = 0;
				mDisplayState = DisplayState::stEditAlarms;
				mEditState = EditState::edListAlarms;
				mEditPos = EditPos::posEnable;
				mUpdateEditDisplay = true;
			}
		}
		else
			if (state == KeyboardState::stAlarmEdit)
			{
				mKeyboard.keyboardState(KeyboardState::stNormal);
				mAlarmManager.saveAlarms(mUnitName);
				mDisplayState = DisplayState::stNormal;
				mEditState = EditState::edListAlarms;
				mLCDisplay.clearGraphicDisplay();
				mForceRefresh = true;
			}
	}

	if (mDisplayState == DisplayState::stEditAlarms)
	{
		App::AlarmList* alarms = mAlarmManager.editAlarms(mUnitName);
		if (alarms)
		{
			if (mEditState == EditState::edListAlarms)
			{
				if (((keyboardInfo[KEY_CENTRAL_L].mLongPress && !keyboardInfo[KEY_CENTRAL_L].mRepeat) ||  (keyboardInfo[KEY_CENTRAL_R].mLongPress && !keyboardInfo[KEY_CENTRAL_R].mRepeat))
					&& (mAlarmEditIndex < alarms->size()))
				{
					LOG(INFO) << "Confirm delete";
					mLCDisplay.rectangle(0, 0, 163, 63, false, true);
					mEditState = EditState::edConfirmDelete;
					mConfirmDelete = false;

					mUpdateEditDisplay = true;
					return;
				}

				if (keyboardInfo[KEY_CENTRAL_R].mShortPressed)
				{
					if (mAlarmEditIndex == alarms->size()) // add a new alarm
					{
						LOG(INFO) << "Add new alarm";
						App::Alarm alarm;
						alarms->push_back(alarm); // Add a new alarm
					}

					mEditPos = EditPos::posEnable;
					mEditState = EditState::edEditAlarm;

					// going from list alarm to edit individual alarm; clear screen
					mLCDisplay.rectangle(0, 0, 163, 63, false, true);

					mUpdateEditDisplay = true;
					return;
				}

				// Up and down must be interpresed in reverse order (be consistent with the display order in the list)
				if (keyboardInfo[KEY_DOWN].mShortPressed)
				{
					++mAlarmEditIndex;
					if (mAlarmEditIndex > alarms->size()) // last pos: new alarm
					{
						mAlarmEditIndex = alarms->size();
					}

					mUpdateEditDisplay = true;
					return;
				}

				if (keyboardInfo[KEY_UP].mShortPressed)
				{
					if (mAlarmEditIndex > 0)
					{
						--mAlarmEditIndex;
					}

					mUpdateEditDisplay = true;
					return;
				}
			}

			if (mEditState == EditState::edConfirmDelete)
			{
				if (keyboardInfo[KEY_DOWN].mShortPressed || keyboardInfo[KEY_UP].mShortPressed )
				{
					mConfirmDelete = ! mConfirmDelete;
					mUpdateEditDisplay = true;
					return;
				}
				if (keyboardInfo[KEY_CENTRAL_L].mShortPressed || keyboardInfo[KEY_CENTRAL_R].mShortPressed )
				{
					if (mConfirmDelete)
					{
						alarms->erase(alarms->begin() + mAlarmEditIndex);
					}

					mAlarmEditIndex = 0;
					mEditState = EditState::edListAlarms;
					mEditPos = EditPos::posEnable;
					mUpdateEditDisplay = true;

					return;
				}
			}

			if (mEditState == EditState::edEditAlarm)
			{
				if (mAlarmEditIndex >= alarms->size())
				{
					LOG(ERROR) << "mAlarmIndex out of range (index: " << mAlarmEditIndex << ", alarmsize: " << alarms->size();
					return;
				}

				auto& alarm = (*alarms)[mAlarmEditIndex];

				// When you immediately edit a alarm after it was triggered
				alarm.mSignalled = false;
				if (keyboardInfo[KEY_CENTRAL_R].mShortPressed)
				{
					switch(mEditPos)
					{
						case EditPos::posEnable:
						{
							mEditPos = EditPos::posUnit;
							break;
						}
						case EditPos::posUnit:
						{
							mEditPos = EditPos::posHourT;
							break;
						}
						case EditPos::posHourT:
						{
							mEditPos = EditPos::posHourE;
							break;
						}
						case EditPos::posHourE:
						{
							mEditPos = EditPos::posMinT;
							break;
						}
						case EditPos::posMinT:
						{
							mEditPos = EditPos::posMinE;
							break;
						}
						case EditPos::posMinE:
						{
							mEditPos = EditPos::posOneTime;
							break;
						}
						case EditPos::posOneTime:
						{
							mEditPos = alarm.mOneTime ? EditPos::posVol: EditPos::posDaySu;
							break;
						}
						case EditPos::posDaySu:
						{
							mEditPos = EditPos::posDayMo;
							break;
						}
						case EditPos::posDayMo:
						{
							mEditPos = EditPos::posDayTu;
							break;
						}
						case EditPos::posDayTu:
						{
							mEditPos = EditPos::posDayWe;
							break;
						}
						case EditPos::posDayWe:
						{
							mEditPos = EditPos::posDayTh;
							break;
						}
						case EditPos::posDayTh:
						{
							mEditPos = EditPos::posDayFr;
							break;
						}
						case EditPos::posDayFr:
						{
							mEditPos = EditPos::posDaySa;
							break;
						}
						case EditPos::posDaySa:
						{
							mEditPos = EditPos::posVol;
							break;
						}
						case EditPos::posVol:
						{
							mEditPos = EditPos::posEnable;
							mEditState = EditState::edListAlarms;
							mAlarmEditIndex = 0;
							break;
						}
					}

					mUpdateEditDisplay = true;
				}

				if (keyboardInfo[KEY_CENTRAL_L].mShortPressed)
				{
					switch(mEditPos)
					{
						case EditPos::posEnable:
						{
							mEditPos = EditPos::posEnable;
							mEditState = EditState::edListAlarms;
							mAlarmEditIndex = 0;
							break;
						}
						case EditPos::posUnit:
						{
							mEditPos = EditPos::posEnable;
							break;
						}
						case EditPos::posHourT:
						{
							mEditPos = EditPos::posUnit;
							break;
						}
						case EditPos::posHourE:
						{
							mEditPos = EditPos::posHourT;
							break;
						}
						case EditPos::posMinT:
						{
							mEditPos = EditPos::posHourE;
							break;
						}
						case EditPos::posMinE:
						{
							mEditPos = EditPos::posMinT;
							break;
						}
						case EditPos::posOneTime:
						{
							mEditPos = EditPos::posMinE;
							break;
						}
						case EditPos::posDaySu:
						{
							mEditPos = EditPos::posOneTime;
							break;
						}
						case EditPos::posDayMo:
						{
							mEditPos = EditPos::posDaySu;
							break;
						}
						case EditPos::posDayTu:
						{
							mEditPos = EditPos::posDayMo;
							break;
						}
						case EditPos::posDayWe:
						{
							mEditPos = EditPos::posDayTu;
							break;
						}
						case EditPos::posDayTh:
						{
							mEditPos = EditPos::posDayWe;
							break;
						}
						case EditPos::posDayFr:
						{
							mEditPos = EditPos::posDayTh;
							break;
						}
						case EditPos::posDaySa:
						{
							mEditPos = EditPos::posDayFr;
							break;
						}
						case EditPos::posVol:
						{
							mEditPos = alarm.mOneTime ? EditPos::posOneTime: EditPos::posDaySa;
							break;
						}
					}

					mUpdateEditDisplay = true;
				}

				if (keyboardInfo[KEY_DOWN].mShortPressed)
				{
					switch(mEditPos)
					{
						case EditPos::posEnable: alarm.mEnabled = !alarm.mEnabled;
							break;
						case EditPos::posUnit:
						{
							alarm.mUnit = mAlarmManager.nextUnitName(alarm.mUnit);
							break;
						}
						case EditPos::posHourT:
						{
							alarm.mHour -=  10;
							if (alarm.mHour < 0)
							{
								alarm.mHour += 10;
							}
							break;
						}
						case EditPos::posHourE:
						{
							alarm.mHour -= 1;
							if (alarm.mHour < 0)
							{
								alarm.mHour += 1;
							}
							break;
						}
						case EditPos::posMinT:
						{
							alarm.mMinutes -= 10;
							if (alarm.mMinutes < 0)
							{
								alarm.mMinutes += 10;
							}
							break;
						}
						case EditPos::posMinE:
						{
							alarm.mMinutes -= 1;
							if (alarm.mMinutes < 0)
							{
								alarm.mMinutes += 1;
							}
							break;
						}
						case EditPos::posOneTime:
						{
							alarm.mOneTime = !alarm.mOneTime;
							break;
						}
						case EditPos::posDaySu:
						{
							alarm.mDays[App::Sunday] = !alarm.mDays[App::Sunday];
							break;
						}
						case EditPos::posDayMo:;
						{
							alarm.mDays[App::Monday] = !alarm.mDays[App::Monday];
							break;
						}
						case EditPos::posDayTu:;
						{
							alarm.mDays[App::Thusday] = !alarm.mDays[App::Thusday];
							break;
						}
						case EditPos::posDayWe:;
						{
							alarm.mDays[App::Wednesday] = !alarm.mDays[App::Wednesday];
							break;
						}
						case EditPos::posDayTh:;
						{
							alarm.mDays[App::Thursday] = !alarm.mDays[App::Thursday];
							break;
						}
						case EditPos::posDayFr:;
						{
							alarm.mDays[App::Friday] = !alarm.mDays[App::Friday];
							break;
						}
						case EditPos::posDaySa:;
						{
							alarm.mDays[App::Saturday] = !alarm.mDays[App::Saturday];
							break;
						}
						case EditPos::posVol:
						{
							alarm.mVolume -= 1;
							if (alarm.mVolume < 0)
							{
								alarm.mVolume += 1;
							}
							break;
						}
					}

					mUpdateEditDisplay = true;
				}

				if (keyboardInfo[KEY_UP].mShortPressed)
				{
					switch(mEditPos)
					{
						case EditPos::posEnable: alarm.mEnabled = !alarm.mEnabled;
							break;
						case EditPos::posUnit:
						{
							alarm.mUnit = mAlarmManager.nextUnitName(alarm.mUnit);
							break;
						}
						case EditPos::posHourT:
						{
							alarm.mHour += 10;
							if (alarm.mHour > 23)
							{
								alarm.mHour -= 10;
							}
							break;
						}
						case EditPos::posHourE:
						{
							alarm.mHour += 1;
							if (alarm.mHour > 23)
							{
								alarm.mHour -= 1;
							}
							break;
						}
						case EditPos::posMinT:
						{
							alarm.mMinutes += 10;
							if (alarm.mMinutes > 59)
							{
								alarm.mMinutes -= 10;
							}
							break;
						}
						case EditPos::posMinE:
						{
							alarm.mMinutes += 1;
							if (alarm.mMinutes > 59)
							{
								alarm.mMinutes -= 1;
							}
							break;
						}
						case EditPos::posOneTime:
						{
							alarm.mOneTime = !alarm.mOneTime;
							break;
						}
						case EditPos::posDaySu:
						{
							alarm.mDays[App::Sunday] = !alarm.mDays[App::Sunday];
							break;
						}
						case EditPos::posDayMo:;
						{
							alarm.mDays[App::Monday] = !alarm.mDays[App::Monday];
							break;
						}
						case EditPos::posDayTu:;
						{
							alarm.mDays[App::Thusday] = !alarm.mDays[App::Thusday];
							break;
						}
						case EditPos::posDayWe:;
						{
							alarm.mDays[App::Wednesday] = !alarm.mDays[App::Wednesday];
							break;
						}
						case EditPos::posDayTh:;
						{
							alarm.mDays[App::Thursday] = !alarm.mDays[App::Thursday];
							break;
						}
						case EditPos::posDayFr:;
						{
							alarm.mDays[App::Friday] = !alarm.mDays[App::Friday];
							break;
						}
						case EditPos::posDaySa:;
						{
							alarm.mDays[App::Saturday] = !alarm.mDays[App::Saturday];
							break;
						}
						case EditPos::posVol:
						{
							alarm.mVolume += 1;
							if (alarm.mVolume > 99)
							{
								alarm.mVolume -= 1;
							}
							break;
						}
					}

					mUpdateEditDisplay = true;
				}
			}
		}

	}
}

void ClockDisplay::updateEditDisplay()
{
	App::AlarmList* alarms = mAlarmManager.editAlarms(mUnitName);
	if (alarms)
	{
		if (mEditState == EditState::edListAlarms)
		{
			mLCDisplay.rectangle(0, 0, 163, 63, false, true);
			unsigned int alarmIndex = mAlarmEditIndex;
			int line = 0;
			mLCDisplay.writeGraphicText(0, 0, ">", FontType::Terminal8);
			while (alarmIndex < alarms->size() && (line < 3))
			{
				auto alarm = (*alarms)[alarmIndex];
				writeAlarm(line,alarm);
				++line;
				++alarmIndex;
			}
			mLCDisplay.writeGraphicText(POS_UNITNAME, line * LINESPACING, "Nieuw alarm                   ", FontType::Terminal8);

		}
		if (mEditState == EditState::edConfirmDelete)
		{
			int carretPos = mConfirmDelete ? 34: 86;
			mLCDisplay.writeGraphicText(20, 8, "Alarm verwijderen ?", FontType::Terminal8);
			mLCDisplay.writeGraphicText(20, 16, "  Ja      Nee", FontType::Terminal8);
			mLCDisplay.writeGraphicText(0, 24, "                                       ", FontType::Terminal8);
			mLCDisplay.writeGraphicText(carretPos, 24, "^", FontType::Terminal8);

		}
		if (mEditState == EditState::edEditAlarm)
		{
			auto alarm = (*alarms)[mAlarmEditIndex];
			writeAlarm(0,alarm);
			mLCDisplay.rectangle(0, 8, 163, 63, false, true);

			int carretPos = 0;
			switch (mEditPos)
			{
				case EditPos::posEnable: carretPos = POS_ENABLE;
					break;
				case EditPos::posUnit: carretPos = POS_UNITNAME;
					break;
				case EditPos::posHourT: carretPos = POS_HOUR_T;
					break;
				case EditPos::posHourE: carretPos = POS_HOUR_E;
					break;
				case EditPos::posMinT: carretPos = POS_MIN_T;
					break;
				case EditPos::posMinE: carretPos = POS_MIN_E;
					break;
				case EditPos::posOneTime: carretPos = POS_ONETIME;
					break;
				case EditPos::posDaySu: carretPos = POS_DAY_SU;
					break;
				case EditPos::posDayMo: carretPos = POS_DAY_MO;
					break;
				case EditPos::posDayTu: carretPos = POS_DAY_TU;
					break;
				case EditPos::posDayWe: carretPos = POS_DAY_WE;
					break;
				case EditPos::posDayTh: carretPos = POS_DAY_TH;
					break;
				case EditPos::posDayFr: carretPos = POS_DAY_FR;
					break;
				case EditPos::posDaySa: carretPos = POS_DAY_SA;
					break;
				case EditPos::posVol: carretPos = POS_VOLUME;
					break;
			}
			mLCDisplay.writeGraphicText(carretPos, LINESPACING, "^", FontType::Terminal8);
		}
	}
}

void ClockDisplay::writeAlarm(int line, const App::Alarm& alarm)
{
	mLCDisplay.writeGraphicText(POS_ENABLE, line * LINESPACING, alarm.mEnabled ? "1":"0", FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_UNITNAME, line * LINESPACING, alarm.mUnit == "" ? "     ": alarm.mUnit, FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_HOUR_T, line * LINESPACING, std::to_string((int) alarm.mHour / 10), FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_HOUR_E, line * LINESPACING, std::to_string(alarm.mHour % 10), FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_MIN_T, line * LINESPACING, std::to_string((int) alarm.mMinutes / 10 ), FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_MIN_E, line * LINESPACING, std::to_string(alarm.mMinutes % 10), FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_ONETIME, line * LINESPACING, alarm.mOneTime ? "O": "D", FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_DAYS, line * LINESPACING, alarm.mOneTime ? "[       ]":alarm.daysString(), FontType::Terminal8);
	mLCDisplay.writeGraphicText(POS_VOLUME, line * LINESPACING, std::to_string(alarm.mVolume) + " ", FontType::Terminal8);

}

void ClockDisplay::drawVolume()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);

	if (mVolume > 100)
	{
		mVolume = 100;
	}
	const uint8_t top = 10;
	const uint8_t bottom = 31;

	const double step = ((double)bottom - (double)top) / 100.0;

	uint8_t length = mVolume * step;

	// top part: clear
	mLCDisplay.rectangle(158, top, 159, 31-length - 1, false, false);
	// bottom part: set
	mLCDisplay.rectangle(158, 31-length, 159, 31, true, false);
}

void ClockDisplay::eraseVolume()
{
	mLCDisplay.rectangle(158, 10, 159, 31, false, false);
}

void ClockDisplay::drawSignal()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);

	mLCDisplay.drawSignal(mReceiveLevel);
}

void ClockDisplay::eraseSignal()
{
	mLCDisplay.rectangle(156, 0, 159, 3, false, true);
}

void ClockDisplay::drawRDS()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);


	mLCDisplay.writeGraphicText(0, 14, mRDSStationName, FontType::Terminal8);
	if (mRDSText.size() > 0)
	{
		std::string localRDSText = mRDSText.substr(mRDSTextPos, std::string::npos);
		if (localRDSText.size()  > 26)
		{
			localRDSText = localRDSText.substr(0, 26);
			mRDSTextPos +=2;
		}
		else
		{
			localRDSText.append(26 - localRDSText.size(), ' ');
			mRDSTextPos = 0;
		}
		mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
	}
	else
	{
		std::string localRDSText(26, ' ');
		mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
	}
}

void ClockDisplay::eraseRDS()
{
	std::string stationName(7, ' ');
	mLCDisplay.writeGraphicText(0, 14, stationName, FontType::Terminal8);

	std::string localRDSText(26, ' ');
	mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
}

void ClockDisplay::drawNTPState()
{
	mLCDisplay.drawNTPState(mRTC.isNTPSync());
}

void ClockDisplay::updateAlarmInfo()
{
	switch(mClockState)
	{
		case App::ClockState::clkNormal:
		{
			std::string nextAlarm = mAlarmManager.nextAlarm(mUnitName);
			if (nextAlarm != "")
			{
				mLCDisplay.writeGraphicText(0, 0, nextAlarm + " ", FontType::Terminal8);
			}
			else
			{
				// Erase
				mLCDisplay.writeGraphicText(0, 0, "      ", FontType::Terminal8);
			}

			break;
		}
		case App::ClockState::clkAlarm:
		{
			mLCDisplay.writeGraphicText(0, 0, "Alarm  ", FontType::Terminal8);
			break;
		}
		case App::ClockState::clkSnooze:
		{
			mLCDisplay.writeGraphicText(0, 0, "Snooze ", FontType::Terminal8);
			break;
		}

	}
}

void ClockDisplay::startRefreshThread()
{
	mRefreshThreadRunning = true;

    // create refresh thread object and start read thread
	mRefreshThread = std::unique_ptr<std::thread>(new std::thread(&ClockDisplay::refreshThread, this));
}

void ClockDisplay::stopRefreshThread()
{
	mRefreshThreadRunning = false;

    if (mRefreshThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mRefreshThread->join();
        mRefreshThread.reset();
    }
}

void ClockDisplay::refreshThread()
{
	int prevMin = -1;
	int prevSec = -1;
	pthread_setname_np(pthread_self(), "ClockDisplay");

    while (mRefreshThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        if (mDisplayState == DisplayState::stNormal)
        {
    		time_t rawTime;
    		struct tm* timeInfo;

    		time(&rawTime);
    		timeInfo = localtime(&rawTime);
    		if ((timeInfo->tm_min != prevMin) || mForceRefresh)
    		{
    			mForceRefresh = false;
    			std::stringstream hourStream;
    			hourStream.width(2);
    			hourStream.fill('0');
    			hourStream << timeInfo->tm_hour;
    			mLCDisplay.writeGraphicText(44, 0, hourStream.str(), FontType::Verdana20);
    			mLCDisplay.writeGraphicText(90, 0, ":", FontType::Verdana20);

    			std::stringstream minStream;
    			minStream.width(2);
    			minStream.fill('0');
    			minStream << timeInfo->tm_min;
    			mLCDisplay.writeGraphicText(104,0, minStream.str(), FontType::Verdana20);

    		}

    		if (mRadioOn)
    		{
    	    	if (timeInfo->tm_sec != prevSec)
    	    	{
        	    	drawRDS();
    	    	}
    	    	drawVolume();
    	    	drawSignal();
    		}
    		else
    		{
    			eraseRDS();
    			eraseVolume();
    			eraseSignal();
    		}

    		prevMin = timeInfo->tm_min;
    		prevSec = timeInfo->tm_sec;

    		drawNTPState();
			// update Alarm info
    	    updateAlarmInfo();
        }
        else
        {
        	if (mUpdateEditDisplay)
        	{
        		updateEditDisplay();
        		mUpdateEditDisplay = false;
        	}
        }
    }
}
}
