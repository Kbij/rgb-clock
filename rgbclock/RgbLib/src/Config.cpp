/*
 * Config.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: koen
 */

#include "Config.h"
#include "tinyxml2.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>


namespace App {

Config::Config(const std::string& settingsFile):
	mSettingsFile(settingsFile),
	mErrorFree(false),
	mConfiguredUnits(),
	mSystemConfig()
{
	loadXML();
}

Config::~Config() {
}

//bool getAddress(ticpp::Iterator<ticpp::Element>& unit, const std::string& name, uint8_t& addressValue)
bool getInteger(tinyxml2::XMLElement* element, const std::string& name, uint8_t& value)
{
	tinyxml2::XMLElement *integerElement = element->FirstChildElement(name.c_str());
	if ( integerElement != nullptr )
	{
		std::string integerString = integerElement->GetText();
		value = std::stoi(integerString);
		return true;
	}
	return false;
}

bool getDouble(tinyxml2::XMLElement* element, const std::string& name, double& value, double defaultVal)
{
	tinyxml2::XMLElement *doubleElement = element->FirstChildElement(name.c_str());
	value = defaultVal;
	if ( doubleElement != nullptr )
	{
		std::string doubleString = doubleElement->GetText();
		value = std::stod(doubleString);
		return true;
	}
	return false;
}

void Config::loadXML()
{
	LOG(INFO) << "Reading settings file: " << mSettingsFile;
	tinyxml2::XMLDocument xmlDoc;

	try
	{
		tinyxml2::XMLError eResult = xmlDoc.LoadFile(mSettingsFile.c_str());
		if (eResult != tinyxml2::XML_SUCCESS)
		{
			LOG(ERROR) << "Error reading xml: " << eResult;
			return;
		}

		tinyxml2::XMLElement* settings = xmlDoc.FirstChildElement("settings");
		if (settings == nullptr)
		{
			LOG(ERROR) << "Settings not found";
			return;
		}

		getInteger(settings, "hw_revision", mSystemConfig.mHardwareRevision);
		getInteger(settings, "rtc_addr", mSystemConfig.mRtc);
		getInteger(settings, "radio_addr", mSystemConfig.mRadio);
		getDouble(settings, "frequency", mSystemConfig.mFrequency, 94.5);  //StuBru :-)
		getInteger(settings, "centralio_addr", mSystemConfig.mCentralIO);
		for (tinyxml2::XMLElement* unit = settings->FirstChildElement("clockunit"); unit != NULL; unit = unit->NextSiblingElement())
		{
			UnitConfig unitSettings;
			const char* name = unit->Attribute("name");
			if (name)
			{
				unitSettings.mName = name;
	        	LOG(INFO) << "Unit found: " << unitSettings.mName;

	        	getInteger(unit, "light_addr", unitSettings.mLight);
	        	getInteger(unit, "keyboard_addr", unitSettings.mKeyboard);
	        	getInteger(unit, "amplifier_addr", unitSettings.mAmplifier);
	        	getInteger(unit, "lcd_addr", unitSettings.mLCD);
	        	getInteger(unit, "lightsensor_addr", unitSettings.mLightSensor);
	        	if (mSystemConfig.mHardwareRevision > 1)
	            {
	        		getInteger(unit, "backlight_addr", unitSettings.mBackLight);
	            }
				mConfiguredUnits[unitSettings.mName] = unitSettings;
			}
		}

	}
	catch (const std::exception& ex)
	{
		LOG(ERROR) << "Error reading config file: " << ex.what();
		return;
	}

	mErrorFree = true;
}

bool Config::errorFree()
{
	return mErrorFree;
}

std::map<std::string, UnitConfig> Config::configuredUnits() const
{
	return mConfiguredUnits;
}

SystemConfig Config::systemConfig() const
{
	return mSystemConfig;
}


} /* namespace App */
