/*
 * Config.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: koen
 */

#include "Config.h"
#include "tinyxml/ticpp.h"

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <string>

DEFINE_string(configfile,"settings.xml","XML file containing the addresses of all IC's");

namespace App {

Config::Config():
	mErrorFree(false),
	mConfiguredUnits(),
	mSystemConfig()
{
	loadXML();
}

Config::~Config() {
}

//bool getAddress(ticpp::Iterator<ticpp::Element>& unit, const std::string& name, uint8_t& addressValue)
bool getInteger(ticpp::Element* element, const std::string& name, uint8_t& value)
{
	ticpp::Element *integerElement = element->FirstChildElement(name, true);
	if ( integerElement != nullptr )
	{
		std::string integerString = integerElement->GetText();
		value = std::stoi(integerString);
		return true;
	}
	return false;
}

void Config::loadXML()
{
	LOG(INFO) << "Reading settings file: " << FLAGS_configfile;
    ticpp::Document configXML(FLAGS_configfile);

    try
    {
        configXML.LoadFile();

        ticpp::Element *settings = configXML.FirstChildElement("settings");

        getInteger(settings, "hw_revision", mSystemConfig.mHardwareRevision);
        getInteger(settings, "rtc_addr", mSystemConfig.mRtc);
        getInteger(settings, "radio_addr", mSystemConfig.mRadio);
        getInteger(settings, "radio_addr", mSystemConfig.mCentralIO);

        ticpp::Iterator<ticpp::Element>  unit(settings->FirstChildElement("clockunit"), "clockunit");
        while ( unit != unit.end() )
        {
        	UnitConfig unitSettings;
        	unit->GetAttribute("name", &unitSettings.mName);
        	LOG(INFO) << "Unit found: " << unitSettings.mName;

        	getInteger(unit.Get(), "light_addr", unitSettings.mLight);
        	getInteger(unit.Get(), "keyboard_addr", unitSettings.mKeyboard);
        	getInteger(unit.Get(), "amplifier_addr", unitSettings.mAmplifier);
        	getInteger(unit.Get(), "lcd_addr", unitSettings.mLCD);
        	getInteger(unit.Get(), "lightsensor_addr", unitSettings.mLightSensor);

        	mConfiguredUnits[unitSettings.mName] = unitSettings;
        	// advance to next item
        	++unit;
        }
	}
	catch (const ticpp::Exception& ex)
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

const std::map<std::string, UnitConfig>& Config::configuredUnits() const
{
	return mConfiguredUnits;
}

const SystemConfig& Config::systemConfig()
{
	return mSystemConfig;
}


} /* namespace App */
