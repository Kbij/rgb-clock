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
	// TODO Auto-generated constructor stub
	loadXML();
}

Config::~Config() {
	// TODO Auto-generated destructor stub
}

//bool getAddress(ticpp::Iterator<ticpp::Element>& unit, const std::string& name, uint8_t& addressValue)
bool getAddress(ticpp::Element* element, const std::string& name, uint8_t& addressValue)
{
	ticpp::Element *addressElement = element->FirstChildElement(name, true);
	if ( addressElement != nullptr )
	{
		std::string addressString = addressElement->GetText();
		addressValue = std::stoi(addressString);
		return true;
	}
	return false;
}

void Config::loadXML()
{
	LOG(INFO) << "Reading settings file: " << FLAGS_configfile;
    ticpp::Document configXML(FLAGS_configfile);

    configXML.LoadFile();
    try
    {
        ticpp::Element *settings = configXML.FirstChildElement("settings");

        getAddress(settings, "rtc_addr", mSystemConfig.mRtc);
        getAddress(settings, "radio_addr", mSystemConfig.mRadio);

        ticpp::Iterator<ticpp::Element>  unit(settings->FirstChildElement("clockunit"), "clockunit");
        while ( unit != unit.end() )
        {
        	UnitConfig unitSettings;
        	unit->GetAttribute("name", &unitSettings.mName);
        	LOG(INFO) << "Unit found: " << unitSettings.mName;

        	getAddress(unit.Get(), "light_addr", unitSettings.mLight);
            getAddress(unit.Get(), "keyboard_addr", unitSettings.mKeyboard);
            getAddress(unit.Get(), "amplifier_addr", unitSettings.mAmplifier);
            getAddress(unit.Get(), "lcd_addr", unitSettings.mLCD);
            getAddress(unit.Get(), "lightsensor_addr", unitSettings.mLightSensor);

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

const std::map<std::string, UnitConfig>& Config::configuredUnits()
{
	return mConfiguredUnits;
}

const SystemConfig& Config::systemConfig()
{
	return mSystemConfig;
}


} /* namespace App */
