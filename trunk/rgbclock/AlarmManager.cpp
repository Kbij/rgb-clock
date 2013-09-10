/*
 * AlarmManager.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#include "AlarmManager.h"
#include "AlarmObserverIf.h"


#include "tinyxml/ticpp.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>
#include <fstream>

DEFINE_string(alarmfile,"alarms.xml","XML file containing the definition of alarms");


namespace App {

AlarmManager::AlarmManager():
		mAlarmList(),
		mAlarmObservers(),
		mCurrentEditor(),
		mAlarmsMutex(),
		mAlarmObserversMutex(),
		mAlarmThread(nullptr),
		mAlarmThreadRunning(false)
{
	loadAlarms();
	startAlarmThread();
}

AlarmManager::~AlarmManager()
{

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

	        ticpp::Iterator<ticpp::Element>  alarm(alarms->FirstChildElement("alarm"), "alarm");
	        while ( alarm != alarm.end() )
	        {
	        	LOG(INFO) << "Alarm found ";
	        	Alarm alarmSettings;
	        	/*
	        	unit->GetAttribute("name", &unitSettings.mName);

	        	getAddress(unit.Get(), "light_addr", unitSettings.mLight);
	            getAddress(unit.Get(), "keyboard_addr", unitSettings.mKeyboard);
	            getAddress(unit.Get(), "amplifier_addr", unitSettings.mAmplifier);
	            getAddress(unit.Get(), "lcd_addr", unitSettings.mLCD);
	            getAddress(unit.Get(), "lightsensor_addr", unitSettings.mLightSensor);

	        	mConfiguredUnits[unitSettings.mName] = unitSettings;
	        	*/
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
}

void AlarmManager::saveAlarms()
{

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

void AlarmManager::alarmThread()
{
    while (mAlarmThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::seconds(1));
    	std::lock_guard<std::mutex> lk_guard(mAlarmsMutex);
    	if (mCurrentEditor == "")
    	{
    		for (auto& alarm: mAlarmList)
    		{
        			time_t rawTime;
        			struct tm* timeInfo;

        			time(&rawTime);
        			timeInfo = localtime(&rawTime);
        			if ((alarm.mHour == timeInfo->tm_hour) && (alarm.mMinutes == timeInfo->tm_min) && (!alarm.mSignalled))
    				{
    					std::lock_guard<std::mutex> lk_guard2(mAlarmObserversMutex);
    					for (auto& observer : mAlarmObservers)
    					{
    						if ((observer->name() == alarm.mUnit) || (alarm.mUnit == ""))
    						{
    							observer->alarmNotify();
    						}
    					}
    					alarm.mSignalled = true;
    				}

        			// Reset the signalled flag
        			if ((alarm.mHour != timeInfo->tm_hour) && (alarm.mMinutes != timeInfo->tm_min))
        			{
    					alarm.mSignalled = false;
        			}
    		}
    	}
    }
}

} /* namespace App */
