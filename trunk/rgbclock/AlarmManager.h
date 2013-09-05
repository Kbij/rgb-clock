/*
 * AlarmManager.h
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#ifndef ALARMMANAGER_H_
#define ALARMMANAGER_H_
#include <vector>
#include <string>
#include <map>

namespace App {
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
		mVolume(40){};

	uint8_t mHour;
	uint8_t mMinutes;
	bool mOneTime;
	std::map<Day, bool> mDays;
	std::string mUnit;
	uint8_t mVolume;
};

using AlarmList = std::vector<Alarm>;

class AlarmManager {
public:
	AlarmManager();
	virtual ~AlarmManager();

	AlarmList* editAlarms(std::string unitName);
	void saveAlarms(std::string unitName);

private:
	AlarmList* mAlarmList;
	std::string mCurrentEditor;

	void loadAlarms();
	void saveAlarms();

};

} /* namespace App */
#endif /* ALARMMANAGER_H_ */
