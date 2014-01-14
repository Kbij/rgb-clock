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
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <pthread.h>

DEFINE_string(rtcStartupFile,"/var/log/rgbclock/rtcstartup.log","Log file containing the RTC startup.");

namespace Hardware
{
RTC::RTC(I2C &i2c, uint8_t address):
	mI2C(i2c),
	mRTCThread(),
	mRTCThreadRunning(false),
	mRTCStartupLog()
{
	mRTCStartupLog.open(FLAGS_rtcStartupFile);
	mRTCStartupLog << "Checking time accuracy" << std::endl;

	if (!ntpSynchronized())
	{
		mRTCStartupLog << "Accessing RTC Clock" << std::endl;
		std::ifstream ifile("/sys/bus/i2c/devices/1-0068");
		if (!ifile)
		{
			mRTCStartupLog << "DS1307 not registered, registering..."<< std::endl;
		   // RTC Device not registered on I2C bus
			std::ofstream newDevice("/sys/class/i2c-adapter/i2c-1/new_device");
			if (newDevice)
			{
				newDevice << "ds1307 " << std::to_string(address);
			}
			newDevice.close();

			// Wait a little bit; give the module time to load
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}

		if (rtcValidDateTime())
		{
			mRTCStartupLog << "Synchronising hwclock with DS1307" << std::endl;
        	mI2C.blockI2C();
			runCmd("hwclock -s --debug", true);
        	mI2C.unBlockI2C();
		}
	}
	else
	{
		mRTCStartupLog << "Time synchronized with ntp server" << std::endl;
	}

	startRTCUpdateThread();

	mRTCStartupLog.close();
}

RTC::~RTC()
{
	LOG(INFO) << "RTC destructor";

	stopRTCUpdateThread();

	LOG(INFO) << "RTC destructor exit";
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
    	        if (mRTCStartupLog.is_open())
    	    	{
    	    		mRTCStartupLog << "Synchronised with: " << columns[0] << std::endl;
    	    		mRTCStartupLog << "Delay: " << columns[7] << "msec" << std::endl;
    	    	}
    	        else
    	        {
    	        	LOG(INFO) << "Synchronised with: " << columns[0];
    	        	LOG(INFO) << "Delay: " << columns[7] << "msec";
    	        }
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
	runCmd("ntpq -p", true);
}

bool RTC::rtcValidDateTime()
{
	mI2C.blockI2C();
	std::string rtcStatus =  runCmd("hwclock -r", true);
	mI2C.unBlockI2C();

	if ((rtcStatus.find("invalid") != std::string::npos) || (rtcStatus.find("error") != std::string::npos))
	{
		return false;
	}
	else
	{
		return true;
	}
}
std::string RTC::runCmd(const std::string& cmd, bool log)
{
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe)
    {
    	if (mRTCStartupLog.is_open())
		{
			mRTCStartupLog << "Unable to run Cmd: " << cmd << std::endl;
		}
    	else
    	{
    		LOG(ERROR) << "Unable to run Cmd: " << cmd << std::endl;
    	}
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
    	if (mRTCStartupLog.is_open())
		{
			mRTCStartupLog << "Run Cmd: " << cmd << std::endl;
		}
    	else
    	{
    		LOG(ERROR) << "Run Cmd: " << cmd << std::endl;
    	}
        std::istringstream resultStream(result);
        std::string line;
        while (std::getline(resultStream, line))
        {
        	if (mRTCStartupLog.is_open())
    		{
    			mRTCStartupLog << line << std::endl;
    		}
        	else
        	{
        		LOG(INFO) << line;
        	}
        }

    }

    return result;
}

void RTC::startRTCUpdateThread()
{
	mRTCThreadRunning = true;

	mRTCThread = new std::thread(&RTC::rtcThread, this);
}

void RTC::stopRTCUpdateThread()
{
	mRTCThreadRunning = false;

    if (mRTCThread)
    {
    	mRTCThread->join();

        delete mRTCThread;
        mRTCThread = nullptr;
    }
}
void RTC::rtcThread()
{
	pthread_setname_np(pthread_self(), "RTC");

	// sleep interval in minutes at boot
	int secondsInterval = 5 * 60; //5 min
	int secondsPassed = 0;
    while (mRTCThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++secondsPassed;

        if (secondsPassed > secondsInterval)
        {
        	secondsPassed = 0;
            if (ntpSynchronized())
            {
            	// Min 5 minutes after start: glog already initialised
    			LOG(INFO) << "NTP Synchronised, writing RTC";

    			secondsInterval = 3 * 24 * 60 * 60; // Every 3 days
            	mI2C.blockI2C();
            	runCmd("hwclock --systohc", true);
            	mI2C.unBlockI2C();
            }
        }

    }


}
}
