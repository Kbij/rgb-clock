/*
 * AlarmManager.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#include "AlarmManager.h"
#include "AlarmObserverIf.h"
#include "lib/SystemClockIf.h"

#include "Config.h"
#include "lib/WatchDogIf.h"

#include "tinyxml2.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>
#include <fstream>
#include <pthread.h>
#include <algorithm>

namespace
{
const int ALARM_CHECK_SECONDS = 1;
}
namespace App {

AlarmManager::AlarmManager(const std::string& alarmFile, const std::vector<std::string> unitList, Hardware::WatchDogIf& watchDog, Hardware::SystemClockIf& systemClock):
		mAlarmFile(alarmFile),
		mUnitList(unitList),
		mWatchdDog(watchDog),
		mSystemClock(systemClock),
		mAlarmList(),
		mAlarmObservers(),
		mCurrentEditor(),
		mAlarmsMutex(),
		mAlarmObserversMutex(),
		mAlarmThread(nullptr),
		mAlarmThreadRunning(false),
		mNextAlarmMap(),
		mNextAlarmMapMutex(),
		mSendAlarmSnooze(false),
		mSendAlarmOff(false)
{
	loadAlarms();
	startAlarmThread();
	mWatchdDog.promiseWatchdog(this, 6000);
}

AlarmManager::~AlarmManager()
{
	mWatchdDog.removePromise(this);
	stopAlarmThread();
}

void AlarmManager::registerAlarmObserver(AlarmObserverIf* observer)
{
	if (observer)
	{
		std::lock_guard<std::mutex> lk_guard(mAlarmObserversMutex);
		mAlarmObservers.insert(observer);
	}

}

void AlarmManager::unRegisterAlarmObserver(AlarmObserverIf* observer)
{
	if (observer)
	{
		std::lock_guard<std::mutex> lk_guard(mAlarmObserversMutex);
		mAlarmObservers.erase(observer);
	}
}

AlarmList* AlarmManager::editAlarms(std::string unitName)
{
	std::lock_guard<std::mutex> lk_guard(mAlarmsMutex);

	if ((mCurrentEditor == "") || (mCurrentEditor == unitName) )
	{
		mCurrentEditor = unitName;
		return &mAlarmList;
	}
	else
	{
		return nullptr;
	}
}

void AlarmManager::saveAlarms(std::string unitName)
{
	std::lock_guard<std::mutex> lk_guard(mAlarmsMutex);

	if (mCurrentEditor == unitName)
	{
		mCurrentEditor = "";
		saveAlarms();
	}
}

std::string AlarmManager::nextAlarm(std::string unitName)
{
	std::lock_guard<std::mutex> lk_guard1(mNextAlarmMapMutex);
	std::lock_guard<std::mutex> lk_guard2(mAlarmsMutex);

	if ((mCurrentEditor == "") && (mNextAlarmMap.find(unitName) != mNextAlarmMap.end()))
	{
		return mNextAlarmMap[unitName].to_string();
	}
	else
	{
		return "";
	}
}

std::string AlarmManager::nextUnitName(std::string currentUnitName)
{
	LOG(INFO) << "Currentname: " << currentUnitName;
	auto current = std::find(mUnitList.begin(), mUnitList.end(), currentUnitName);
	if ((currentUnitName == "") || (current == mUnitList.end()))
	{
		LOG(INFO) << "return first:" << mUnitList.front();
		return mUnitList.front();
	}

	current++;
	if (current != mUnitList.end())
	{
		LOG(INFO) << "Return: " << (*current);
		return (*current);
	}
	else
	{
		LOG(INFO) << "Return empty";
		return "";
	}

}

std::string AlarmManager::feederName() const
{
	return "AlarmManager";
}

void AlarmManager::sendAlarmSnooze()
{
	mSendAlarmSnooze = true;
}

void AlarmManager::sendAlarmOff()
{
	mSendAlarmOff = true;
}

bool fileExists(std::string fileName)
{
   std::ifstream infile(fileName);
   return infile.good();
}

void AlarmManager::loadAlarms()
{
	if (fileExists(mAlarmFile))
	{
		LOG(INFO) << "Reading Alarm file: " << mAlarmFile;
		tinyxml2::XMLDocument alarmsXML;
		tinyxml2::XMLError eResult = alarmsXML.LoadFile(mAlarmFile.c_str());
		if (eResult != tinyxml2::XML_SUCCESS)
		{
			LOG(ERROR) << "Error reading alarm xml: " << eResult;
			return;
		}

		tinyxml2::XMLElement* alarms = alarmsXML.FirstChildElement("alarms");
		if (alarms == nullptr)
		{
			LOG(ERROR) << "Alarms not found";
			return;
		}
    	std::lock_guard<std::mutex> lk_guard2(mAlarmsMutex);
    	mAlarmList.clear();
		for (tinyxml2::XMLElement* alarm = alarms->FirstChildElement("alarm"); alarm != NULL; alarm = alarm->NextSiblingElement())
		{
			Alarm alarmSettings;
			tinyxml2::XMLElement* enabledElement = alarm->FirstChildElement("enabled");
        	if ( enabledElement != nullptr && enabledElement->GetText() != nullptr )
        	{
        		alarmSettings.mEnabled = std::stoi(enabledElement->GetText());
        	}

        	tinyxml2::XMLElement *unitElement = alarm->FirstChildElement("unit");
        	if ( (unitElement != nullptr) && (unitElement->GetText() != nullptr) )
        	{
        		alarmSettings.mUnit = unitElement->GetText();// Do not throw when no text is filled in
        	}

        	tinyxml2::XMLElement *hourElement = alarm->FirstChildElement("hour");
        	if ( hourElement != nullptr && hourElement->GetText() != nullptr )
        	{
        		alarmSettings.mHour = std::stoi(hourElement->GetText());
        	}

        	tinyxml2::XMLElement *minutesElement = alarm->FirstChildElement("minutes");
        	if ( minutesElement != nullptr && minutesElement->GetText() != nullptr )
        	{
        		alarmSettings.mMinutes = std::stoi(minutesElement->GetText());
        	}

        	tinyxml2::XMLElement *typeElement = alarm->FirstChildElement("type");

        	if (typeElement != nullptr )
        	{
        		std::string readValue(typeElement->GetText());
        		if (readValue.compare("D") == 0) alarmSettings.mAlarmType = AlarmType::Daily;
        		if (readValue.compare("L") == 0) alarmSettings.mAlarmType = AlarmType::OnTimeLoud;
        		if (readValue.compare("E") == 0) alarmSettings.mAlarmType = AlarmType::OnTimeSmooth;
        	}

        	tinyxml2::XMLElement *volumeElement = alarm->FirstChildElement("volume");
        	if ( volumeElement != nullptr )
        	{
        		alarmSettings.mVolume = std::stoi(volumeElement->GetText());
        	}

        	tinyxml2::XMLElement *daysElement = alarm->FirstChildElement("days");
        	if ( daysElement != nullptr )
        	{
        		alarmSettings.mDays = std::bitset<7>(daysElement->GetText());
        	}

        	LOG(INFO) << "Alarm loaded: " << alarmSettings.to_string_long();
        	mAlarmList.push_back(alarmSettings);
		}
	}
	else
	{
		LOG(INFO) << "Alarm file (" << mAlarmFile << ") not found.";
	}

	LOG(INFO) << mAlarmList.size() << " alarms loaded";
}

void AlarmManager::saveAlarms()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLDeclaration* decl = xmlDoc.NewDeclaration();
	xmlDoc.InsertFirstChild(decl);

	tinyxml2::XMLNode* alarms = xmlDoc.NewElement("alarms");
	xmlDoc.LinkEndChild(alarms);

	for (const auto& alarm: mAlarmList)
	{
		tinyxml2::XMLNode* alarmElement = xmlDoc.NewElement("alarm");
		alarms->InsertEndChild( alarmElement );

		tinyxml2::XMLElement* enabledElement = xmlDoc.NewElement("enabled");
		enabledElement->SetText(std::to_string(alarm.mEnabled).c_str());
		alarmElement->InsertEndChild(enabledElement);

		tinyxml2::XMLElement* unitElement = xmlDoc.NewElement("unit");
		unitElement->SetText(alarm.mUnit.c_str());
		alarmElement->InsertEndChild(unitElement);

		tinyxml2::XMLElement* hourElement = xmlDoc.NewElement("hour");
		hourElement->SetText(std::to_string(alarm.mHour).c_str());
		alarmElement->InsertEndChild(hourElement);

		tinyxml2::XMLElement* minutesElement = xmlDoc.NewElement("minutes");
		minutesElement->SetText(std::to_string(alarm.mMinutes).c_str());
		alarmElement->InsertEndChild(minutesElement);

		tinyxml2::XMLElement* typeElement = xmlDoc.NewElement("type");
		typeElement->SetText(alarm.typeString().c_str());
		alarmElement->InsertEndChild(typeElement);

		tinyxml2::XMLElement* daysElement = xmlDoc.NewElement("days");
		daysElement->SetText(alarm.mDays.to_string().c_str());
		alarmElement->InsertEndChild(daysElement);

		tinyxml2::XMLElement* volumeElement = xmlDoc.NewElement("volume");
		volumeElement->SetText(std::to_string(alarm.mVolume).c_str());
		alarmElement->InsertEndChild(volumeElement);
	}

	xmlDoc.SaveFile(mAlarmFile.c_str());
}

void AlarmManager::startAlarmThread()
{
	mAlarmThreadRunning = true;

	mAlarmThread = std::unique_ptr<std::thread>(new std::thread(&AlarmManager::alarmThread, this));
}

void AlarmManager::stopAlarmThread()
{
	mAlarmThreadRunning = false;

    if (mAlarmThread)
    {
    	mAlarmThread->join();
        mAlarmThread.reset();
    }
}

int minutesUntilFired(const Alarm& alarm, Hardware::SystemClockIf& systemClock)
{
	VLOG(1) << "Alarm: " << alarm.to_string_short() << ", calculating minutes until fired";
	if (!alarm.mEnabled)
	{
		VLOG(1) << "Alarm not enabled";
		return -1; // alarm not active
	}
	Hardware::LocalTime time = systemClock.localTime();
	// nextday = if before saturday then (current day + 1) else sunday
	int nextDay =  time.mDay < Hardware::DayOfWeek::Saturday ? ((int)time.mDay + 1) : 0;

	int nowMinutes = time.mHour * 60 + time.mMin;
	int almMinutes = alarm.mHour * 60 + alarm.mMinutes;
	bool oneTimeAlarm = alarm.mAlarmType == AlarmType::OnTimeLoud || alarm.mAlarmType == AlarmType::OnTimeSmooth;

	if (!oneTimeAlarm)
	{
		// Not a one time alarm. If (already passed or no alarm today) and not for tomorrow
		if ( ((alarm.mDays[(int)time.mDay] && (almMinutes < nowMinutes)) || !alarm.mDays[(int)time.mDay]) && !alarm.mDays[nextDay] )
		{
			VLOG(1) << "Not a one time alarm. If (already passed or no alarm today) and not for tomorrow";
			return -1;
		}
	}

	// If a onetime alarm that will happen in less than 24h; add 24h to get a positive result
	if (oneTimeAlarm && (almMinutes < nowMinutes))
	{
		almMinutes += 24 * 60; // add 1 day
	}
	else
	{
		// If not today but tommorow
		if (!oneTimeAlarm &&  !(alarm.mDays[(int)time.mDay] && (almMinutes >= nowMinutes))  && alarm.mDays[nextDay]) // Tomorrow
		{
			almMinutes += 60 * 24; // add 1 day (in minutes)
		}
	}
	VLOG(1) << "Result: " << almMinutes - nowMinutes;
	return almMinutes - nowMinutes;
}

void AlarmManager::alarmThread()
{
	pthread_setname_np(pthread_self(), "AlarmManager");

    while (mAlarmThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::seconds(ALARM_CHECK_SECONDS));
        mWatchdDog.signalWatchdog(this);

    	std::lock_guard<std::mutex> lk_guard(mAlarmsMutex);
    	if (mCurrentEditor == "")
    	{
    		bool saveAlm = false;
    		mNextAlarmMap.clear();

    		for (auto& alarm: mAlarmList)
    		{

        		int minutesLeft = minutesUntilFired(alarm, mSystemClock);
        		if ((minutesLeft ==  0) && (!alarm.mSignalled))
    			{
    				std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
    				VLOG(1) << "Fire alarm: " << alarm.to_string_long();
    				for (auto& observer : mAlarmObservers)
    				{
    					if ((observer->name() == alarm.mUnit) || (alarm.mUnit == ""))
    					{
    						LOG(INFO) << "Send notify to: " << observer->name();
    						observer->alarmNotify(alarm.mVolume, (alarm.mAlarmType == AlarmType::Daily || alarm.mAlarmType == AlarmType::OnTimeSmooth));
    					}
    				}
    				alarm.mSignalled = true;
    				if (alarm.mAlarmType == AlarmType::OnTimeLoud || alarm.mAlarmType == AlarmType::OnTimeSmooth)
    				{
    					alarm.mEnabled = false;
    					saveAlm = true;
    				}
    			}
        		else //not yet there; store in the next alarmmap
        		{
        			if (minutesLeft > 0)
        			{
        				for (auto& observer : mAlarmObservers)
        				{
        					if ((observer->name() == alarm.mUnit) || (alarm.mUnit == ""))
							{
								if (mNextAlarmMap.find(observer->name()) != mNextAlarmMap.end())
								{
									if (mNextAlarmMap[observer->name()].mIntervalMinutes > minutesLeft)
									{
										mNextAlarmMap[observer->name()].mIntervalMinutes = minutesLeft;
										mNextAlarmMap[observer->name()].mHour = alarm.mHour;
										mNextAlarmMap[observer->name()].mMinutes = alarm.mMinutes;
									}

								}
								else
								{
									if (minutesLeft < ( 23 * 60))
									{
										mNextAlarmMap[observer->name()].mIntervalMinutes = minutesLeft;
										mNextAlarmMap[observer->name()].mHour = alarm.mHour;
										mNextAlarmMap[observer->name()].mMinutes = alarm.mMinutes;
									}
								}
							}
        				}
        			}
        		}
        		Hardware::LocalTime time = mSystemClock.localTime();

        		// Reset the signalled flag
        		if ((alarm.mHour != time.mHour) || (alarm.mMinutes != time.mMin))
        		{
    				VLOG(1) << "Reset signalled alarm: " << alarm.to_string_long();
    				alarm.mSignalled = false;
        		}
    		}

    		if (saveAlm)
    		{
				saveAlarms();
    		}


    		if (mSendAlarmSnooze)
    		{
				std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
				for (auto& observer : mAlarmObservers)
				{
					observer->alarmSnooze();
				}
				mSendAlarmSnooze = false;
    		}

    		if (mSendAlarmOff)
    		{
				std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
				for (auto& observer : mAlarmObservers)
				{
					observer->alarmOff();
				}
				mSendAlarmOff = false;
    		}

    	}
    }
}

} /* namespace App */
