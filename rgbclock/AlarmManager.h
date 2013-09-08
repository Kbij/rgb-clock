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
#include <map>
#include <mutex>
#include <atomic>
#include <thread>

namespace App {

class AlarmObserverIf;

enum class Day
{
	Monday    = 1,
	Thusday   = 2,
	Wednesday = 3,
	Thursday  = 4,
	Friday    = 5,
	Saturday  = 6,
	Sunday    = 7
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
		mSignaled(false){};

	uint8_t mHour;
	uint8_t mMinutes;
	bool mOneTime;
	std::map<Day, bool> mDays;
	std::string mUnit;
	uint8_t mVolume;
	bool mSignaled;
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
