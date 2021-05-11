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

bool getInteger(tinyxml2::XMLElement* element, const std::string& name, int& value, int defaultVal)
{
	value = defaultVal;
	tinyxml2::XMLElement *integerElement = element->FirstChildElement(name.c_str());
	if ( integerElement != nullptr )
	{
		std::string integerString = integerElement->GetText();
		value = std::stoi(integerString);
		return true;
	}
	return false;
}

bool getByte(tinyxml2::XMLElement* element, const std::string& name, uint8_t& value)
{
	int intValue = 0;
	bool result = getInteger(element, name, intValue, 0);
	value = intValue;

	return result;
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

		getByte(settings, "hw_revision", mSystemConfig.mHardwareRevision);
		getByte(settings, "rtc_addr", mSystemConfig.mRtc);
		getByte(settings, "radio_addr", mSystemConfig.mRadio);
		getInteger(settings, "frequency_id", mSystemConfig.mFrequencyId, 30);  //StuBru :-)
		getInteger(settings, "service_id", mSystemConfig.mServiceId, 25348);  //StuBru :-)
		getInteger(settings, "component_id", mSystemConfig.mComponentId, 8);  //StuBru :-)
		getByte(settings, "centralio_addr", mSystemConfig.mCentralIO);
		for (tinyxml2::XMLElement* unit = settings->FirstChildElement("clockunit"); unit != NULL; unit = unit->NextSiblingElement())
		{
			UnitConfig unitSettings;
			const char* name = unit->Attribute("name");
			if (name)
			{
				unitSettings.mName = name;
	        	LOG(INFO) << "Unit found: " << unitSettings.mName;

	        	getByte(unit, "light_addr", unitSettings.mLight);
	        	getByte(unit, "keyboard_addr", unitSettings.mKeyboard);
	        	getByte(unit, "amplifier_addr", unitSettings.mAmplifier);
	        	getByte(unit, "lcd_addr", unitSettings.mLCD);
	        	getByte(unit, "lightsensor_addr", unitSettings.mLightSensor);
	        	if (mSystemConfig.mHardwareRevision > 1)
	            {
	        		getByte(unit, "backlight_addr", unitSettings.mBackLight);
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

std::vector<std::string> Config::units() const
{
	std::vector<std::string> result;
	for(auto const& unit: mConfiguredUnits)	result.push_back(unit.first);

	return result;
}

SystemConfig Config::systemConfig() const
{
	return mSystemConfig;
}


} /* namespace App */
