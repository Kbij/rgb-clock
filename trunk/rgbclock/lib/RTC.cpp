/*
 * RTC.cpp
 *
 *  Created on: Jun 13, 2013
 *      Author: koen
 */

#include "RTC.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <glog/logging.h>
#include <fstream>

std::string runCmd(const std::string& cmd, bool log)
{
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe)
    {
    	LOG(ERROR) << "Unable to run Cmd: " << cmd;
    	return "";
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
    	if (fgets(buffer, 128, pipe) != NULL)
    	{
    		result += buffer;
    	}
    }
    pclose(pipe);
    if (log)
    {
    	LOG(INFO) << "Run Cmd: " << cmd;
        std::istringstream resultStream(result);
        std::string line;
        while (std::getline(resultStream, line))
        {
        	LOG(INFO) << line;
        }

    }

    return result;
}


RTC::RTC(I2C &i2c):
	mI2C(i2c)
{

	LOG(INFO) << "Checking time accuracy";

	if (!ntpSynchronized())
	{
		LOG(INFO) << "Accessing RTC Clock";
		std::ifstream ifile("/sys/bus/i2c/devices/1-0068");
		if (!ifile)
		{
			LOG(INFO) << "DS1307 not registered, registering...";
		  // RTC Device not registered on I2C bus
			std::ofstream newDevice("/sys/class/i2c-adapter/i2c-1/new_device");
			if (newDevice)
			{
				newDevice << "ds1307 0x68";
			}
			newDevice.close();
		}
		LOG(INFO) << "Synchronising hwclock with DS1307";
		runCmd("hwclock -s --debug", true);
	}
	else
	{
		LOG(INFO) << "Time synchronized with ntp server";
	}
}

RTC::~RTC()
{

}

bool RTC::ntpSynchronized()
{
	std::string ntpStatus =  runCmd("ntpq -p", false);
    std::istringstream resultStream(ntpStatus);
    std::string line;
    while (std::getline(resultStream, line)) {

    	if ((line.size() > 0) && (line[0] == '*'))
    	{
    	    std::vector<std::string> columns;
    	    std::istringstream lineStream(line);
    	    std::string cell;
    	    while (std::getline(lineStream, cell, ' ')) {
    	    	if (cell.size() > 0)
    	    	{
        	    	columns.push_back(cell);
    	    	}
    	    }

    	    if (columns.size() > 7)
    	    {
				LOG(INFO) << "Synchronised with: " << columns[0];
				LOG(INFO) << "Delay: " << columns[7] << "msec";
				std::istringstream input(columns[7]);
				double delay;
				if (!(input >> delay))
				{
					delay = 0;
				}

				if ((delay > 5) && (delay < 100))
				{
					return true;
				}
				else
				{
					return false;
				}

    	    }
    	}
    }
    return false;
}

void RTC::showNTPStatus()
{
	std::string ntpStatus =  runCmd("ntpq -p", true);
}


