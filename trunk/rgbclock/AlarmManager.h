/*
 * AlarmManager.h
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#ifndef ALARMMANAGER_H_
#define ALARMMANAGER_H_

#include <set>
#include <vector>
#include <string>
#include <bitset>
#include <mutex>
#include <atomic>
#include <thread>

namespace App {

class AlarmObserverIf;

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
	std::string to_string_long()
	{
		std::string result;
		result = "Unit: " + mUnit + ": " + std::to_string(mHour) + ":" + std::to_string(mMinutes) + ", OneTime: " + std::to_string(mOneTime) +  ", Days: " + daysString() + ", Volume: " + std::to_string(mVolume);
		return result;
	}
	std::string to_string_short()
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

class AlarmManager {
public:
	AlarmManager();
	virtual ~AlarmManager();

	void registerAlarmObserver(AlarmObserverIf* observer);
	void unRegisterAlarmObserver(AlarmObserverIf* observer);

	AlarmList* editAlarms(std::string unitName);
	void saveAlarms(std::string unitName);

private:
	AlarmList mAlarmList;
	std::set<AlarmObserverIf*> mAlarmObservers;
	std::string mCurrentEditor;
	std::mutex mAlarmsMutex;
	std::mutex mAlarmObserversMutex;
    std::thread* mAlarmThread;
	std::atomic_bool mAlarmThreadRunning;

	void loadAlarms();
	void saveAlarms();

	void startAlarmThread();
	void stopAlarmThread();

	void alarmThread();

};

} /* namespace App */
#endif /* ALARMMANAGER_H_ */
