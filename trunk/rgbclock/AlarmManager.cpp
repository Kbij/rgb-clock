/*
 * AlarmManager.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#include "AlarmManager.h"
#include "AlarmObserverIf.h"

#include "Config.h"
#include "lib/MainboardControl.h"

#include "tinyxml/ticpp.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>
#include <fstream>
#include <pthread.h>

DEFINE_string(alarmfile,"alarms.xml","XML file containing the definition of alarms");


namespace App {

AlarmManager::AlarmManager(const Config& config, Hardware::MainboardControl &mainboardControl):
		mMainboardControl(mainboardControl),
		mAlarmList(),
		mAlarmObservers(),
		mCurrentEditor(),
		mAlarmsMutex(),
		mAlarmObserversMutex(),
		mAlarmThread(nullptr),
		mAlarmThreadRunning(false),
		mNextAlarmMap(),
		mNextAlarmMapMutex(),
		mConfig(config)
{
	loadAlarms();
	startAlarmThread();
	mMainboardControl.promiseWatchdog(this, 2000);
}

AlarmManager::~AlarmManager()
{
	mMainboardControl.removePromise(this);
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
		std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
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
	std::lock_guard<std::mutex> lk_guard(mNextAlarmMapMutex);

	if (mNextAlarmMap.find(unitName) != mNextAlarmMap.end())
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
	const auto& config = mConfig.configuredUnits();
	auto current = config.find(currentUnitName);
	if ((currentUnitName == "") || (current == config.end()))
	{
		LOG(INFO) << "return first:" << mConfig.configuredUnits().begin()->first;
		return mConfig.configuredUnits().begin()->first;
	}

	current++;
	if (current != config.end())
	{
		LOG(INFO) << "Return: " << current->first;
		return current->first;
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

bool fileExists(std::string fileName)
{
   std::ifstream infile(fileName);
   return infile.good();
}

void AlarmManager::loadAlarms()
{
	if (fileExists(FLAGS_alarmfile))
	{
		LOG(INFO) << "Reading Alarm file: " << FLAGS_alarmfile;
	    ticpp::Document alarmsXML(FLAGS_alarmfile);

	    alarmsXML.LoadFile();
	    try
	    {
	        ticpp::Element *alarms = alarmsXML.FirstChildElement("alarms");
	    	std::lock_guard<std::mutex> lk_guard2(mAlarmsMutex);

	    	mAlarmList.clear();

	        ticpp::Iterator<ticpp::Element>  alarm(alarms->FirstChildElement("alarm", false), "alarm");
	        while ( alarm != alarm.end() )
	        {
	        	Alarm alarmSettings;

	        	ticpp::Element *enabledElement = alarm->FirstChildElement("enabled", true);
	        	if ( enabledElement != nullptr )
	        	{
	        		alarmSettings.mEnabled = std::stoi(enabledElement->GetText());
	        	}

	        	ticpp::Element *unitElement = alarm->FirstChildElement("unit", false);
	        	if ( unitElement != nullptr )
	        	{
	        		alarmSettings.mUnit = unitElement->GetText(false);// Do not throw when no text is filled in
	        	}

	        	ticpp::Element *hourElement = alarm->FirstChildElement("hour", true);
	        	if ( hourElement != nullptr )
	        	{
	        		alarmSettings.mHour = std::stoi(hourElement->GetText());
	        	}

	        	ticpp::Element *minutesElement = alarm->FirstChildElement("minutes", true);
	        	if ( minutesElement != nullptr )
	        	{
	        		alarmSettings.mMinutes = std::stoi(minutesElement->GetText());
	        	}

	        	ticpp::Element *onetimeElement = alarm->FirstChildElement("onetime", true);
	        	if ( onetimeElement != nullptr )
	        	{
	        		alarmSettings.mOneTime = onetimeElement->GetText() == "1";
	        	}

	        	ticpp::Element *volumeElement = alarm->FirstChildElement("volume", true);
	        	if ( volumeElement != nullptr )
	        	{
	        		alarmSettings.mVolume = std::stoi(volumeElement->GetText());
	        	}

	        	ticpp::Element *daysElement = alarm->FirstChildElement("days", true);
	        	if ( daysElement != nullptr )
	        	{
	        		alarmSettings.mDays = std::bitset<7>(daysElement->GetText());
	        	}

	        	LOG(INFO) << "Alarm loaded: " << alarmSettings.to_string_long();
	        	mAlarmList.push_back(alarmSettings);

	        	// advance to next item
	        	++alarm;
	        }
		}
		catch (const ticpp::Exception& ex)
		{
			LOG(ERROR) << "Error reading alarm file: " << ex.what();
			return;
		}
	}
	else
	{
		LOG(INFO) << "Alarm file (" << FLAGS_alarmfile << ") not found.";
	}

	LOG(INFO) << mAlarmList.size() << " alarms loaded";
}

void AlarmManager::saveAlarms()
{
	TiXmlDocument doc;
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );


	TiXmlElement * alarms = new TiXmlElement("alarms");
	doc.LinkEndChild( alarms );

	for (const auto& alarm: mAlarmList)
	{
		TiXmlElement* alarmElement = new TiXmlElement("alarm");
		//alarmElement->LinkEndChild( new TiXmlText("test"));
		alarms->LinkEndChild( alarmElement );

		TiXmlElement* enabledElement = new TiXmlElement("enabled");
		enabledElement->LinkEndChild( new TiXmlText(std::to_string(alarm.mEnabled)));
		alarmElement->LinkEndChild(enabledElement);

		TiXmlElement* unitElement = new TiXmlElement("unit");
		unitElement->LinkEndChild( new TiXmlText(alarm.mUnit));
		alarmElement->LinkEndChild(unitElement);

		TiXmlElement* hourElement = new TiXmlElement("hour");
		hourElement->LinkEndChild( new TiXmlText(std::to_string(alarm.mHour)));
		alarmElement->LinkEndChild(hourElement);

		TiXmlElement* minutesElement = new TiXmlElement("minutes");
		minutesElement->LinkEndChild( new TiXmlText(std::to_string(alarm.mMinutes)));
		alarmElement->LinkEndChild(minutesElement);

		TiXmlElement* onetimeElement = new TiXmlElement("onetime");
		onetimeElement->LinkEndChild( new TiXmlText(std::to_string(alarm.mOneTime)));
		alarmElement->LinkEndChild(onetimeElement);

		TiXmlElement* daysElement = new TiXmlElement("days");
		daysElement->LinkEndChild( new TiXmlText(alarm.mDays.to_string()));
		alarmElement->LinkEndChild(daysElement);

		TiXmlElement* volumeElement = new TiXmlElement("volume");
		volumeElement->LinkEndChild( new TiXmlText(std::to_string(alarm.mVolume)));
		alarmElement->LinkEndChild(volumeElement);
	}

	doc.SaveFile(FLAGS_alarmfile);
}

void AlarmManager::startAlarmThread()
{
	mAlarmThreadRunning = true;

	mAlarmThread = new std::thread(&AlarmManager::alarmThread, this);
}

void AlarmManager::stopAlarmThread()
{
	mAlarmThreadRunning = false;

    if (mAlarmThread)
    {
    	mAlarmThread->join();

        delete mAlarmThread;
        mAlarmThread = nullptr;
    }
}

int minutesUntilFired(const Alarm& alarm)
{
	if (!alarm.mEnabled)
	{
		return -1; // alarm not active
	}
	time_t rawTime;
	struct tm* timeInfo;

	time(&rawTime);
	timeInfo = localtime(&rawTime);
	// nextday = if before saturday then (current day + 1) else sunday
	int nextDay =  timeInfo->tm_wday < 6 ? (timeInfo->tm_wday + 1) : 0;

	int nowMinutes = timeInfo->tm_hour * 60 + timeInfo->tm_min;
	int almMinutes = alarm.mHour * 60 + alarm.mMinutes;


	if (!alarm.mOneTime)
	{
		// Not a one time alarm. If already passed and not for tomorrow
		if ( (alarm.mDays[Day(timeInfo->tm_wday)] && (almMinutes < nowMinutes)) && !alarm.mDays[Day(nextDay)] )
		{
			return -1;
		}
	}

	// If a onetime alarm that will happen in less than 24h; add 24h to get a positive result
	if (alarm.mOneTime && (almMinutes < nowMinutes))
	{
		almMinutes += 24 * 60; // add 1 day
	}
	else
	{
		// If not today but tommorow
		if (!(alarm.mDays[Day(timeInfo->tm_wday)] && (almMinutes >= nowMinutes))  && alarm.mDays[Day(nextDay)]) // Tomorrow
		{
			almMinutes += 60 * 24; // add 1 day (in minutes)
		}
	}

	return almMinutes - nowMinutes;
}

void AlarmManager::alarmThread()
{
	pthread_setname_np(pthread_self(), "AlarmManager");

    while (mAlarmThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mMainboardControl.signalWatchdog(this);

    	std::lock_guard<std::mutex> lk_guard(mAlarmsMutex);
    	if (mCurrentEditor == "")
    	{
    		bool saveAlm = false;
    		mNextAlarmMap.clear();

    		for (auto& alarm: mAlarmList)
    		{

        		int minutesLeft = minutesUntilFired(alarm);
        		if ((minutesLeft ==  0) && (!alarm.mSignalled))
    			{
    				std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
    				for (auto& observer : mAlarmObservers)
    				{
    					if ((observer->name() == alarm.mUnit) || (alarm.mUnit == ""))
    					{
    						LOG(INFO) << "Send notify to: " << observer->name();
    						observer->alarmNotify(alarm.mVolume);
    					}
    				}
    				alarm.mSignalled = true;
    				if (alarm.mOneTime)
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

        		time_t rawTime;
        		struct tm* timeInfo;

        		time(&rawTime);
        		timeInfo = localtime(&rawTime);

        		// Reset the signalled flag
        		if ((alarm.mHour != timeInfo->tm_hour) || (alarm.mMinutes != timeInfo->tm_min))
        		{
    				alarm.mSignalled = false;
        		}
    		}

    		if (saveAlm)
    		{
				saveAlarms();
    		}
    	}
    }
}

} /* namespace App */
