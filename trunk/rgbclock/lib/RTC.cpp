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
#include <vector>
#include <ctime>
#include <iomanip>
#include <sys/time.h>

DEFINE_string(rtcStartupFile,"/var/log/rgbclock/rtcstartup.log","Log file containing the RTC startup.");

namespace Hardware
{
RTC::RTC(I2C &i2c, uint8_t address):
	mI2C(i2c),
	mAddress(address),
	mRTCThread(),
	mRTCThreadRunning(false),
	mRTCStartupLog()
{
	mRTCStartupLog.open(FLAGS_rtcStartupFile);
	mRTCStartupLog << "Checking time accuracy" << std::endl;
	readRTCTime();
	writeRTCTime();
	if (!ntpSynchronized())
	{
		mRTCStartupLog << "Accessing RTC Clock" << std::endl;
		struct std::tm utcTime = RTC::readRTCTime();

		if (rtcValidDateTime(utcTime))
		{
			mRTCStartupLog << "Synchronising hwclock with DS1307" << std::endl;
			setSystemTime(utcTime);
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

uint8_t bcdToInt(uint8_t bcd)
{
	return (bcd & 0x0F) + ((bcd >> 4)*10);
}
uint8_t intToBcd(uint8_t value)
{
	return (value / 10 << 4) + (value % 10);
}

bool RTC::rtcValidDateTime(struct std::tm utcTime)
{
	return (utcTime.tm_year > 2000) && (utcTime.tm_year < 2100);
}


struct std::tm RTC::readRTCTime()
{

	std::vector<uint8_t> response(7); // Vector with size 7
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({0}), response);

	if (mRTCStartupLog.is_open())
	{
		mRTCStartupLog << "Reading RTC Data" << std::endl;
	}
	else
	{
		LOG(INFO) << "Reading RTC Data";
	}

	struct std::tm result;
	result.tm_sec = bcdToInt(response[0] & 0x7F); // ignore bit 7
	result.tm_min = bcdToInt(response[1]);
	result.tm_hour = bcdToInt(response[2]);
	result.tm_wday = bcdToInt(response[3]) - 1;
	result.tm_mday = bcdToInt(response[4]);
	result.tm_mon  = bcdToInt(response[5]);
	result.tm_year =  bcdToInt(response[6]) + 100;

	if (mRTCStartupLog.is_open())
	{
		mRTCStartupLog << "RTC time read: " << std::asctime(&result) << std::endl;
	}
	else
	{
		LOG(INFO) << "RTC time read: " << std::asctime(&result);
	}
	return result;
}

void RTC::writeRTCTime()
{
	std::time_t t = std::time(nullptr);
	std::cout << "Current (local) time: " << std::ctime(&t) << std::endl;
	std::tm *utc = gmtime ( &t );

	std::cout << "Current UTC time: " << std::asctime(utc) << std::endl;
	std::time_t utcTime = timegm(utc);
	struct tm * setTime = localtime(&utcTime);
	std::cout << "Local time: " << std::asctime(setTime) << std::endl;


	std::time_t setTime2 = mktime( setTime );
	std::cout << "setTime: " << std::ctime(&setTime2) << std::endl;

	struct timeval val;
	val.tv_sec = setTime2;
	val.tv_usec = 0;
}

void RTC::setSystemTime(struct std::tm utcTime)
{
	std::cout << "Current UTC time: " << std::asctime(utcTime) << std::endl;
	std::time_t utcTime = timegm(utc);
	struct tm * setTime = localtime(&utcTime1);
	std::cout << "Local time: " << std::asctime(setTime) << std::endl;


	std::time_t setTime2 = mktime( setTime );
	std::cout << "setTime: " << std::ctime(&setTime2) << std::endl;

	struct timeval val;
	val.tv_sec = setTime2;
	val.tv_usec = 0;
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
