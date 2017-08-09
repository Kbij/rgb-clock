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

TEST(Config, Constructor)
{

	App::Config* config = new App::Config("settingsFile.xml");
	delete config;
}

TEST(Config, ReadConfig)
{

	App::Config* config = new App::Config("../testfiles/settingsFile.xml");
	std::map<std::string, App::UnitConfig> unitConfig = config->configuredUnits();
	EXPECT_EQ((size_t) 2, unitConfig.size());

	EXPECT_TRUE(config->errorFree());
	delete config;
}
