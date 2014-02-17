/*
 * AlarmManager.h
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#ifndef ALARMMANAGER_H_
#define ALARMMANAGER_H_
#include "lib/WatchdogFeederIf.h"

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

namespace Hardware {
class MainboardControl;
}

namespace App {

class AlarmObserverIf;
class Config;

enum Day
{
	// Keep these enum values, they correspond with tm struct tm_wday enum values
	Sunday    = 0,
	Monday    = 1,
	Thusday   = 2,
	Wednesday = 3,
	Thursday  = 4,
	Friday    = 5,
	Saturday  = 6
};

struct Alarm
{
	Alarm() :
		mEnabled(true),
		mHour(7),
		mMinutes(0),
		mOneTime(false),
		mDays(),
		mUnit(""),
		mVolume(40),
		mSignalled(false){};
	bool mEnabled;
	int mHour;
	int mMinutes;
	bool mOneTime;
	std::bitset<7> mDays;
	std::string mUnit;
	int mVolume;
	bool mSignalled;
	std::string daysString() const
	{
		std::string result = "[";
		if (mDays[Sunday])
		{
			result = result + "Z";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Monday])
		{
			result = result + "M";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Thusday])
		{
			result = result + "D";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Wednesday])
		{
			result = result + "W";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Thursday])
		{
			result = result + "D";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Friday])
		{
			result = result + "V";
		}
		else
		{
			result = result + "_";
		}
		if (mDays[Saturday])
		{
			result = result + "Z";
		}
		else
		{
			result = result + "_";
		}

		return result + "]";
	}

	std::string to_string_long() const
	{
		std::string result;
		result = "Unit: " + mUnit + ": " + std::to_string(mHour) + ":" + std::to_string(mMinutes) + ", OneTime: " + std::to_string(mOneTime) +  ", Days: " + daysString() + ", Volume: " + std::to_string(mVolume);
		return result;
	}

	std::string to_string_short() const
	{
		std::string result;
		std::string oneTime = "";
		std::string daysStr = "";
		if (mOneTime)
		{
			oneTime = "O";
		}
		else
		{
			oneTime = "D";
			daysStr = daysString();
		}

		result = "'" + mUnit + "' " + std::to_string(mHour) + ":" + std::to_string(mMinutes) + " " + oneTime + daysStr + " " + std::to_string(mVolume);
		return result;
	}

};

using AlarmList = std::vector<Alarm>;
class AlarmManager: public Hardware::WatchdogFeederIf {

public:
	AlarmManager(const Config& config, Hardware::MainboardControl &mainboardControl);
	virtual ~AlarmManager();

	void registerAlarmObserver(AlarmObserverIf* observer);
	void unRegisterAlarmObserver(AlarmObserverIf* observer);

	AlarmList* editAlarms(std::string unitName);
	void saveAlarms(std::string unitName);

	std::string nextAlarm(std::string unitName);

	std::string nextUnitName(std::string currentUnitName);

	virtual std::string feederName() const;
private:
	struct NextAlarm
	{
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

    Hardware::MainboardControl &mMainboardControl;
	AlarmList mAlarmList;
	std::set<AlarmObserverIf*> mAlarmObservers;
	std::string mCurrentEditor;
	std::mutex mAlarmsMutex;
	std::mutex mAlarmObserversMutex;
    std::thread* mAlarmThread;
	std::atomic_bool mAlarmThreadRunning;
	std::map<std::string, NextAlarm> mNextAlarmMap;
	std::mutex mNextAlarmMapMutex;

	const Config& mConfig;

};

} /* namespace App */
#endif /* ALARMMANAGER_H_ */