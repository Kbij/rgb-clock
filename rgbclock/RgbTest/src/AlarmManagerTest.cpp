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
		++mTimesFeeded;
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
	AlarmObserverStub(): mName(), mAlarmNotifyVolume(), mSmooth() {};
	~AlarmObserverStub() {};

	void alarmNotify(int volume, bool smooth) {mAlarmNotifyVolume = volume; mSmooth = smooth;}
	std::string name() {return mName;}

	void alarmSnooze() {}
	void alarmOff() {}
	std::string mName;
	int mAlarmNotifyVolume;
	bool mSmooth;
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
	EXPECT_GT(watchdog.mTimesFeeded, 3);
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
	alarmWrite1.mAlarmType = App::AlarmType::OnTimeSmooth;
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);

	App::Alarm alarmWrite2;
	alarmWrite2.mEnabled = false;
	alarmWrite2.mUnit = "testunit2";
	alarmWrite2.mHour = 8;
	alarmWrite2.mMinutes = 20;
	alarmWrite2.mDays = 0b0101010;
	alarmWrite2.mAlarmType = App::AlarmType::Daily;
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
	EXPECT_EQ(App::AlarmType::OnTimeSmooth, alarmRead1.mAlarmType);
	EXPECT_EQ(40, alarmRead1.mVolume);

	App::Alarm alarmRead2 = (*alarms)[1];
	EXPECT_FALSE(alarmRead2.mEnabled);
	EXPECT_EQ("testunit2", alarmRead2.mUnit);
	EXPECT_EQ(8, alarmRead2.mHour);
	EXPECT_EQ(20, alarmRead2.mMinutes);
	EXPECT_EQ(0b0101010, alarmRead2.mDays.to_ulong());
	EXPECT_EQ(App::AlarmType::Daily, alarmRead2.mAlarmType);
	EXPECT_EQ(35, alarmRead2.mVolume);

	delete manager;
//	std::system("rm alarms.xml");
}

TEST(AlarmManager, ReadOldFile)
{
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	std::system("cp ../testfiles/alarms.xml .");
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"Koen", "Heidi"}, watchdog, systemClock);
	delete manager;
}

TEST(AlarmManager, ObserveOneTimeAlarmSmooth)
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
	alarmWrite1.mAlarmType = App::AlarmType::OnTimeSmooth;
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);
	manager->saveAlarms("testunit");
	//Next alarm list must be calculated
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 00;
	EXPECT_EQ("07:10", manager->nextAlarm("testunit"));

	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Must have notified;
	EXPECT_EQ(40, alarmObserver.mAlarmNotifyVolume);
	EXPECT_TRUE(alarmObserver.mSmooth);
	EXPECT_EQ("", manager->nextAlarm("testunit"));

	//Reset alarmobserver
	alarmObserver.mAlarmNotifyVolume = 0;
	alarmObserver.mSmooth = false;

	systemClock.mTime.mHour = 8;
	systemClock.mTime.mMin = 0;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Alarm schould be disabled
	alarms = manager->editAlarms("testunit");
	EXPECT_EQ((size_t) 1 , alarms->size());
	EXPECT_FALSE(alarms->front().mEnabled);
	EXPECT_EQ("", manager->nextAlarm("testunit"));

	manager->saveAlarms("testunit");

	//Let it be 7:10 again ...
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Will not have notified
	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	EXPECT_FALSE(alarmObserver.mSmooth);

	delete manager;

	EXPECT_TRUE(watchdog.mTimesFeeded > 0);
	std::system("rm alarms.xml");
}

TEST(AlarmManager, ObserveOneTimeAlarmLoud)
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
	alarmWrite1.mAlarmType = App::AlarmType::OnTimeLoud;
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);
	manager->saveAlarms("testunit");
	//Next alarm list must be calculated
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 00;
	EXPECT_EQ("07:10", manager->nextAlarm("testunit"));

	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Must have notified;
	EXPECT_EQ(40, alarmObserver.mAlarmNotifyVolume);
	EXPECT_FALSE(alarmObserver.mSmooth);
	EXPECT_EQ("", manager->nextAlarm("testunit"));

	//Reset alarmobserver
	alarmObserver.mAlarmNotifyVolume = 0;
	alarmObserver.mSmooth = false;

	systemClock.mTime.mHour = 8;
	systemClock.mTime.mMin = 0;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Alarm schould be disabled
	alarms = manager->editAlarms("testunit");
	EXPECT_EQ((size_t) 1 , alarms->size());
	EXPECT_FALSE(alarms->front().mEnabled);
	EXPECT_EQ("", manager->nextAlarm("testunit"));

	manager->saveAlarms("testunit");

	//Let it be 7:10 again ...
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Will not have notified
	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	EXPECT_FALSE(alarmObserver.mSmooth);

	delete manager;

	EXPECT_TRUE(watchdog.mTimesFeeded > 0);
	std::system("rm alarms.xml");
}


TEST(AlarmManager, ObserveDailyAlarm)
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
	alarmWrite1.mAlarmType = App::AlarmType::Daily;
	alarmWrite1.mDays = 0b0000010; //Monday
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);
	manager->saveAlarms("testunit");

	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Sunday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("", manager->nextAlarm("testunit"));

	//Must not have notified
	EXPECT_EQ(0, alarmObserver.mAlarmNotifyVolume);
	EXPECT_FALSE(alarmObserver.mSmooth);

	//Go to alarm time - 24h
	systemClock.mTime.mHour = 23;
	systemClock.mTime.mMin = 10;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Sunday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("07:10", manager->nextAlarm("testunit"));

	//Goto monday
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Monday;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Must have notified
	EXPECT_EQ(40, alarmObserver.mAlarmNotifyVolume);
	EXPECT_TRUE(alarmObserver.mSmooth);

	//Reset alarmobserver
	alarmObserver.mAlarmNotifyVolume = 0;
	alarmObserver.mSmooth = false;

	//Let the time pass 7u10
	systemClock.mTime.mHour = 8;
	systemClock.mTime.mMin = 0;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("", manager->nextAlarm("testunit")); //Alarm not visible

	//Let it be 7u10 again
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 10;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Monday;
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Must have notified
	EXPECT_EQ(40, alarmObserver.mAlarmNotifyVolume);
	EXPECT_TRUE(alarmObserver.mSmooth);

	delete manager;
	EXPECT_TRUE(watchdog.mTimesFeeded > 0);

	std::system("rm alarms.xml");
}

TEST(AlarmManager, MixedAlarms)
{
	std::system("rm alarms.xml");
	WatchDogStub watchdog;
	SystemClockStub systemClock;
	App::AlarmManager* manager = new App::AlarmManager("alarms.xml", {"unit1"}, watchdog, systemClock);
	AlarmObserverStub alarmObserverUnit1;
	alarmObserverUnit1.mName = "unit1";
	manager->registerAlarmObserver(&alarmObserverUnit1);

	AlarmObserverStub alarmObserverUnit2;
	alarmObserverUnit2.mName = "unit2";
	manager->registerAlarmObserver(&alarmObserverUnit2);

	//Write one alarm
	App::AlarmList* alarms = manager->editAlarms("unit1");

	//Daily alarm on Monday 7u10
	App::Alarm alarmWrite1;
	alarmWrite1.mEnabled = true;
	alarmWrite1.mUnit = "unit1"; // Only for testunit1
	alarmWrite1.mHour = 7;
	alarmWrite1.mMinutes = 10;
	alarmWrite1.mAlarmType = App::AlarmType::Daily;
	alarmWrite1.mDays = 0b0000010; //Monday
	alarmWrite1.mVolume = 40;
	alarms->push_back(alarmWrite1);

	//Ontime alarm @7u30, all units
	App::Alarm alarmWrite2;
	alarmWrite2.mEnabled = true;
	alarmWrite2.mUnit = "";
	alarmWrite2.mHour = 7;
	alarmWrite2.mMinutes = 30;
	alarmWrite2.mAlarmType = App::AlarmType::OnTimeSmooth;
	alarmWrite2.mVolume = 40;
	alarms->push_back(alarmWrite2);

	//Daily alarm D,W,D@7u00 all units
	App::Alarm alarmWrite3;
	alarmWrite3.mEnabled = true;
	alarmWrite3.mUnit = "";
	alarmWrite3.mHour = 7;
	alarmWrite3.mMinutes = 00;
	alarmWrite3.mAlarmType = App::AlarmType::Daily;
	alarmWrite3.mDays = 0b0011100; //D,W,D
	alarmWrite3.mVolume = 40;
	alarms->push_back(alarmWrite3);

	manager->saveAlarms("unit1");

	EXPECT_EQ(0, alarmObserverUnit1.mAlarmNotifyVolume);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//Sunday evening
	systemClock.mTime.mHour = 23;
	systemClock.mTime.mMin = 00;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Sunday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("07:10", manager->nextAlarm("unit1")); //Next alarm for testunit: 7:10
	EXPECT_EQ("07:30", manager->nextAlarm("unit2")); //Next alarm for other: 7:30


	//Monday morning, after first alarm
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 20;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Monday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("07:30", manager->nextAlarm("unit1")); //Next alarm for testunit: 7:10
	EXPECT_EQ("07:30", manager->nextAlarm("unit2")); //Next alarm for other: 7:30

	//Monday morning, after second alarm
	systemClock.mTime.mHour = 7;
	systemClock.mTime.mMin = 35;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Monday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("", manager->nextAlarm("unit1"));
	EXPECT_EQ("", manager->nextAlarm("unit2"));

	//Monday morning, @9am
	systemClock.mTime.mHour = 9;
	systemClock.mTime.mMin = 0;
	systemClock.mTime.mDay = Hardware::DayOfWeek::Monday;
	std::this_thread::sleep_for(std::chrono::seconds(2));
	EXPECT_EQ("07:00", manager->nextAlarm("unit1"));
	EXPECT_EQ("07:00", manager->nextAlarm("unit2"));

	delete manager;
	EXPECT_TRUE(watchdog.mTimesFeeded > 0);

	std::system("rm alarms.xml");
}

