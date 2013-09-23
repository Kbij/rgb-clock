/*
 * AlarmManager.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#include "AlarmManager.h"
#include "AlarmObserverIf.h"

#include "Config.h"

#include "tinyxml/ticpp.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>
#include <fstream>

DEFINE_string(alarmfile,"alarms.xml","XML file containing the definition of alarms");


namespace App {

AlarmManager::AlarmManager(const Config& config):
		mAlarmList(),
		mAlarmObservers(),
		mCurrentEditor(),
		mAlarmsMutex(),
		mAlarmObserversMutex(),
		mAlarmThread(nullptr),
		mAlarmThreadRunning(false),
		mConfig(config)
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
	        	Alarm alarmSettings;

	        	ticpp::Element *enabledElement = alarm->FirstChildElement("enabled", true);
	        	if ( enabledElement != nullptr )
	        	{
	        		alarmSettings.mEnabled = std::stoi(enabledElement->GetText());
	        	}

	        	ticpp::Element *unitElement = alarm->FirstChildElement("unit", false);
	        	if ( unitElement != nullptr )
	        	{
	        		alarmSettings.mUnit = unitElement->GetText();
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

        			if ((alarm.mHour == timeInfo->tm_hour) && (alarm.mMinutes == timeInfo->tm_min) && (alarm.mDays[Day(timeInfo->tm_wday)] || alarm.mOneTime) && (!alarm.mSignalled))
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

    		//cleanUpSingleAlarms;
    	}
    }
}

} /* namespace App */
