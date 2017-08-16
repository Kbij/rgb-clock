/*
 * AlarmManagerTest.cpp
 *
 *  Created on: Aug 9, 2017
 *      Author: koen
 */


#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include "AlarmManager.h"
#include "AlarmObserverIf.h"
#include "lib/WatchDogIf.h"
#include "lib/WatchdogFeederIf.h"
#include "lib/SystemClockIf.h"
#include <thread>

namespace
{
class WatchDogStub: public Hardware::WatchDogIf
{
public:
	WatchDogStub(): mPromised(false), mTimesFeeded(0) {};
	void promiseWatchdog(Hardware::WatchdogFeederIf *watchdogFeeder, int timeoutMiliseconds)
	{
		mPromised = true;
	};
	void removePromise(Hardware::WatchdogFeederIf *watchdogFeeder)
	{
		mPromised = false;
	};
	void signalWatchdog(Hardware::WatchdogFeederIf *watchdogFeeder)
	{

	};
	bool mPromised;
	int mTimesFeeded;
};

class SystemClockStub: public Hardware::SystemClockIf
{
public:
	SystemClockStub(): mTime() {};
	Hardware::LocalTime localTime()
	{
		return mTime;
	}
	Hardware::LocalTime mTime;
};

class AlarmObserverStub: public App::AlarmObserverIf
{
public:
	AlarmObserverStub(): mName(), mAlarmNotifyVolume() {};
	~AlarmObserverStub() {};

	void alarmNotify(int volume) {mAlarmNotifyVolume = volume;}
	std::string name() {return mName;}

	void alarmSnooze() {}
	void alarmOff() {}
	std::string mName;
	int mAlarmNotifyVolume;
};
}

TEST(AlarmManager, Constructor)
{
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	delete manager;
}

TEST(AlarmManager, WatchDogFeeding)
{
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	EXPECT_TRUE(watchdog.mPromised);
	std::this_thread::sleep_for(std::chrono::seconds(5));

	//Feeding each second
	EXPECT_GT(4, watchdog.mTimesFeeded);
	delete manager;
	EXPECT_FALSE(watchdog.mPromised);
}

TEST(AlarmManager, ReadingWritingAlarms)
{
	std::system("rm alarms.xml");
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	App::AlarmList* alarms = manager->editAlarms("testunit");
	App::Alarm alarmWrite1;
	alarmWrite1.mEnabled = true;
	alarmWrite1.mUnit = "testunit1";
	alarmWrite1.mHour = 7;
	alarmWrite1.mMinutes = 10;
	alarmWrite1.mDays = 0b1010101;
	alarmWrite1.mOneTime = true;
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);

	App::Alarm alarmWrite2;
	alarmWrite2.mEnabled = false;
	alarmWrite2.mUnit = "testunit2";
	alarmWrite2.mHour = 8;
	alarmWrite2.mMinutes = 20;
	alarmWrite2.mDays = 0b0101010;
	alarmWrite2.mOneTime = false;
	alarmWrite2.mVolume = 35;
	alarms->push_back(alarmWrite2);

	manager->saveAlarms("testunit");
	delete manager;

	//Read the alarm
	manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	alarms = manager->editAlarms("testunit");
	EXPECT_EQ((size_t) 2, alarms->size());

	App::Alarm alarmRead1 = (*alarms)[0];
	EXPECT_TRUE(alarmRead1.mEnabled);
	EXPECT_EQ("testunit1", alarmRead1.mUnit);
	EXPECT_EQ(7, alarmRead1.mHour);
	EXPECT_EQ(10, alarmRead1.mMinutes);
	EXPECT_EQ(0b1010101, alarmRead1.mDays.to_ulong());
	EXPECT_TRUE(alarmRead1.mOneTime);
	EXPECT_EQ(40, alarmRead1.mVolume);

	App::Alarm alarmRead2 = (*alarms)[1];
	EXPECT_FALSE(alarmRead2.mEnabled);
	EXPECT_EQ("testunit2", alarmRead2.mUnit);
	EXPECT_EQ(8, alarmRead2.mHour);
	EXPECT_EQ(20, alarmRead2.mMinutes);
	EXPECT_EQ(0b0101010, alarmRead2.mDays.to_ulong());
	EXPECT_FALSE(alarmRead2.mOneTime);
	EXPECT_EQ(35, alarmRead2.mVolume);

	delete manager;
	std::system("rm alarms.xml");
}

TEST(AlarmManager, ObserveOneTimeAlarm)
{
	std::system("rm alarms.xml");
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	AlarmObserverStub alarmObserver;
	alarmObserver.mName = "testunit";
	manager->registerAlarmObserver(&alarmObserver);

	//Write one alarm
	App::AlarmList* alarms = manager->editAlarms("testunit");
	App::Alarm alarmWrite1;
	alarmWrite1.mEnabled = true;
	alarmWrite1.mUnit = "testunit";
	alarmWrite1.mHour = 7;
	alarmWrite1.mMinutes = 10;
	alarmWrite1.mOneTime = true;
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);
	manager->saveAlarms("testunit");

	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Must have notified;
	EXPECT_EQ(40, alarmObserver.mAlarmNotifyVolume);

	std::system("rm alarms.xml");
}
