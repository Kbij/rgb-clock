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
		mHour(7),
		mMinutes(0),
		mOneTime(false),
		mDays(),
		mUnit(""),
		mVolume(40),
		mSignalled(false){};
	int mHour;
	int mMinutes;
	bool mOneTime;
	std::bitset<7> mDays;
	std::string mUnit;
	int mVolume;
	bool mSignalled;
	std::string to_string()
	{
		std::string result;
		result = "Unit: " + mUnit + ": " + std::to_string(mHour) + ":" + std::to_string(mMinutes) + ", OneTime: " + std::to_string(mOneTime) +  ", Days: " + mDays.to_string() + ", Volume: " + std::to_string(mVolume);
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
