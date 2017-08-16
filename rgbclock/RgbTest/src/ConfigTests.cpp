/*
 * ConfigTests.cpp
 *
 *  Created on: Aug 9, 2017
 *      Author: koen
 */


#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include "Config.h"
#include "ConfigTests.h"
#include <fstream>

TEST(Config, Constructor)
{

	App::Config* config = new App::Config("settingsFile.xml");
	delete config;
}

TEST(Config, ReadConfig)
{
	//Write the file
    std::ofstream configFile("settingsFile.xml");
    configFile << CONFIG;
    configFile.close();

	App::Config* config = new App::Config("settingsFile.xml");
	std::map<std::string, App::UnitConfig> unitConfig = config->configuredUnits();
	EXPECT_EQ((size_t) 2, unitConfig.size());
	EXPECT_EQ(65, unitConfig["Unit1"].mLight);
	EXPECT_EQ(64, unitConfig["Unit1"].mBackLight);
	EXPECT_EQ(90, unitConfig["Unit1"].mKeyboard);
	EXPECT_EQ(108, unitConfig["Unit1"].mAmplifier);
	EXPECT_EQ(33, unitConfig["Unit1"].mLCD);
	EXPECT_EQ(41, unitConfig["Unit1"].mLightSensor);
	EXPECT_TRUE(config->errorFree());
	delete config;
}
