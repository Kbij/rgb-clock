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
#include "lib/WatchDogIf.h"
#include "lib/WatchdogFeederIf.h"


namespace
{
class WatchDogStub: public Hardware::WatchDogIf
{
public:
	void promiseWatchdog(Hardware::WatchdogFeederIf *watchdogFeeder, int timeoutMiliseconds) {};
	void removePromise(Hardware::WatchdogFeederIf *watchdogFeeder) {};
	void signalWatchdog(Hardware::WatchdogFeederIf *watchdogFeeder) {};

};
}
TEST(AlarmManager, Constructor)
{
	WatchDogStub watchdog;
	App::AlarmManager* manager = new App::AlarmManager({"unit1"}, watchdog);
	delete manager;
}
