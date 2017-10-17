/*
 * AlarmManager.h
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#ifndef ALARMMANAGER_H_
#define ALARMMANAGER_H_
#include "lib/WatchdogFeederIf.h"
#include "lib/SystemClockIf.h"

#include <set>
#include <vector>
#include <string>
#include <bitset>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>
#include <sstream>
#include <iomanip>
#include <memory>

namespace Hardware {
class WatchDogIf;
}

namespace App {

class AlarmObserverIf;
class Config;
enum class AlarmType {Daily, OnTimeSmooth, OnTimeLoud};
struct Alarm
{
	Alarm() :
		mEnabled(true),
		mHour(7),
		mMinutes(0),
		mAlarmType(AlarmType::Daily),
		mDays(),
		mUnit(""),
		mVolume(40),
		mSignalled(false){};
	bool mEnabled;
	int mHour;
	int mMinutes;
	AlarmType mAlarmType;
	std::bitset<7> mDays;
	std::string mUnit;
	int mVolume;
	bool mSignalled;
	std::string daysString() const
	{
		std::stringstream result;
		result << "[";
		if (mDays[(int)Hardware::DayOfWeek::Sunday])
		{
			result << "Z";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Monday])
		{
			result << "M";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Thuesday])
		{
			result << "D";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Wednesday])
		{
			result << "W";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Thursday])
		{
			result << "D";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Friday])
		{
			result << "V";
		}
		else
		{
			result << "_";
		}
		if (mDays[(int)Hardware::DayOfWeek::Saturday])
		{
			result << "Z";
		}
		else
		{
			result << "_";
		}
		result << "]";
		return result.str();
	}
	std::string typeString() const
	{
		switch(mAlarmType)
		{
			case AlarmType::Daily: return "D";
			case AlarmType::OnTimeLoud: return "L";
			case AlarmType::OnTimeSmooth: return "E";
		}
		return "";
	}
	std::string to_string_long() const
	{
		std::stringstream result;
		result << "Unit: " << mUnit << ": " << mHour << ":" << mMinutes << ", Type: " << typeString();
		if (mAlarmType == AlarmType::Daily) result << ", Days: " << daysString();
		result << ", Volume: " << mVolume;
		return result.str();
	}

	std::string to_string_short() const
	{
		std::stringstream result;

		result << "'" << mUnit << "' " << mHour << ":" << mMinutes << " " << typeString();
		if (mAlarmType == AlarmType::Daily) result  << daysString() << " ";
		result << mVolume;
		return result.str();
	}

};

using AlarmList = std::vector<Alarm>;
class AlarmManager: public Hardware::WatchdogFeederIf {

public:
	AlarmManager(const std::string& alarmFile, const std::vector<std::string> unitList, Hardware::WatchDogIf& watchdDog, Hardware::SystemClockIf& systemClock);
	virtual ~AlarmManager();

	void registerAlarmObserver(AlarmObserverIf* observer);
	void unRegisterAlarmObserver(AlarmObserverIf* observer);

	AlarmList* editAlarms(std::string unitName);
	void saveAlarms(std::string unitName);
	std::string nextAlarm(std::string unitName);
	std::string nextUnitName(std::string currentUnitName);

	void sendAlarmSnooze();
	void sendAlarmOff();

	virtual std::string feederName() const;
private:
	struct NextAlarm
	{
		NextAlarm() :
			mIntervalMinutes(0),
			mHour(0),
			mMinutes(0){};

		int mIntervalMinutes;
		int mHour;
		int mMinutes;
		std::string to_string()
		{
			std::stringstream result;
			result << std::setw(2) << std::setfill('0');
			result << mHour << ":";
			result << std::setw(2) << std::setfill('0') << mMinutes;

			return result.str();
		}
	};
	void loadAlarms();
	void saveAlarms();

	void startAlarmThread();
	void stopAlarmThread();

	void alarmThread();
	const std::string mAlarmFile;
	std::vector<std::string> mUnitList;
	Hardware::WatchDogIf& mWatchdDog;
	Hardware::SystemClockIf& mSystemClock;
	AlarmList mAlarmList;
	std::set<AlarmObserverIf*> mAlarmObservers;
	std::string mCurrentEditor;
	std::mutex mAlarmsMutex;
	std::mutex mAlarmObserversMutex;
    std::unique_ptr<std::thread> mAlarmThread;
	std::atomic_bool mAlarmThreadRunning;
	std::map<std::string, NextAlarm> mNextAlarmMap;
	std::mutex mNextAlarmMapMutex;
	std::atomic_bool mSendAlarmSnooze;
	std::atomic_bool mSendAlarmOff;
};

} /* namespace App */
#endif /* ALARMMANAGER_H_ */
