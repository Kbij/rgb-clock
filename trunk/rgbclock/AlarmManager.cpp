/*
 * AlarmManager.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: koen
 */

#include "AlarmManager.h"
#include "tinyxml/ticpp.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>
#include <fstream>

DEFINE_string(alarmfile,"alarms.xml","XML file containing the definition of alarms");


namespace App {

AlarmManager::AlarmManager():
		mAlarmList(new AlarmList),
		mCurrentEditor()
{

}

AlarmManager::~AlarmManager()
{

}

AlarmList* AlarmManager::editAlarms(std::string unitName)
{
	if ((mCurrentEditor == "") || (mCurrentEditor == unitName) )
	{
		mCurrentEditor = unitName;
		return mAlarmList;
	}
	else
	{
		return nullptr;
	}
}

void AlarmManager::saveAlarms(std::string unitName)
{
	if (mCurrentEditor == unitName)
	{
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
}

void AlarmManager::saveAlarms()
{

}


} /* namespace App */
