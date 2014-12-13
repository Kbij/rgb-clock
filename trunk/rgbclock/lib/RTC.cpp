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
#include <bitset>

DEFINE_string(rtcStartupFile,"/var/log/rgbclock/rtcstartup.log","Log file containing the RTC startup.");
namespace
{
const int NTP_INTERVAL_SECONDS = 10;
}
namespace Hardware
{
RTC::RTC(I2C &i2c, uint8_t address):
	mI2C(i2c),
	mAddress(address),
	mRTCThread(),
	mRTCThreadRunning(false),
	mRTCStartupLog(),
	mNTPSync(false)
{
	mRTCStartupLog.open(FLAGS_rtcStartupFile);
	mRTCStartupLog << "Checking time accuracy" << std::endl;
	if (!ntpSynchronized())
	{
		log("Time is not synchronised with NTP");
		struct std::tm utcTime = readRTCTime();

		if (rtcValidDateTime(utcTime))
		{
			log("Synchronising systemclock with DS1338");
			setSystemTime(utcTime);
		}
		else
		{
			log("RTC does not contain a valid date/time, where screwed ....");
		}
	}
	else
	{
		log("Time read from RTC clock:");
		readRTCTime();
		log("Time synchronized with ntp server");
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
std::string RTC::runCmd(const std::string& cmd, bool logResult)
{
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe)
    {
		log("Unable to run Cmd: " + cmd);
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
    if (logResult)
    {
		log("Run Cmd: " + cmd);

		std::istringstream resultStream(result);
        std::string line;
        while (std::getline(resultStream, line))
        {
        	log(line);
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
   	    		//log("Synchronised with: " + columns[0]);
   	    		//log("Delay: " + columns[7] + "msec");

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

bool RTC::isNTPSync()
{
	return mNTPSync;
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
	return (utcTime.tm_year > 100) && (utcTime.tm_year < 200);
}

struct std::tm RTC::readRTCTime()
{
	std::vector<uint8_t> response(8); // Vector with size 7
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({0}), response);

	log("Reading RTC Data");

	struct std::tm result;
	result.tm_sec = bcdToInt(response[0] & 0x7F); // ignore bit 7
	result.tm_min = bcdToInt(response[1]);
	result.tm_hour = bcdToInt(response[2]);
	result.tm_wday = bcdToInt(response[3]) - 1;
	result.tm_mday = bcdToInt(response[4]);
	result.tm_mon  = bcdToInt(response[5]);
	result.tm_year =  bcdToInt(response[6]) + (2000 - 1900);

	log("RTC time read (UTC): " + std::string(std::asctime(&result)));
	std::stringstream bitString;
	bitString << "Control Register: " << (std::bitset<8>) response[7];
	log(bitString.str());

	return result;
}

void RTC::writeRTCTime()
{
	std::time_t t = std::time(nullptr);

	log("Write RTC Time, current (local) time: " + std::string(std::ctime(&t)));

	std::tm *utc = gmtime ( &t );

	log("Writing UTC time to RTC: " + std::string(std::asctime(utc)));

	std::vector<uint8_t> rtcData(8);

	rtcData[0] = 0; // Start at register 0
	rtcData[1] = intToBcd(utc->tm_sec);
	rtcData[2] = intToBcd(utc->tm_min);
	rtcData[3] = intToBcd(utc->tm_hour);
	rtcData[4] = intToBcd(utc->tm_wday + 1); // tm->tm_wday: 0..6; RTC: 1..7
	rtcData[5] = intToBcd(utc->tm_mday);
	rtcData[6] = intToBcd(utc->tm_mon);
	rtcData[7] = intToBcd(utc->tm_year - (2000 - 1900));

	mI2C.writeData(mAddress, rtcData);
}

void RTC::setSystemTime(struct std::tm utcTime)
{
	//Convert UTC tm structure to time_t
	std::time_t utcTime_t = timegm(&utcTime);

	// Convert the UTC time to local time
	struct tm * setTime = localtime(&utcTime_t);

	// And now back to time_t for the settimeofday function
	std::time_t setTime_t = mktime( setTime );

    log("Set system time to UTC time: " + std::string(std::asctime(&utcTime)));
	log("Converted to local time: " + std::string(std::asctime(setTime)));
	log("setTime: " + std::string(std::ctime(&setTime_t)));

	struct timeval val;
	val.tv_sec = setTime_t;
	val.tv_usec = 0;

	settimeofday(&val, nullptr); // SetTime uses local time
}

void RTC::log(const std::string& message)
{
	if (mRTCStartupLog.is_open())
	{
		mRTCStartupLog << message << std::endl;

	}
	else
	{
		LOG(INFO) << message;
	}
}

void RTC::startRTCUpdateThread()
{
	mRTCThreadRunning = true;

	mRTCThread = std::unique_ptr<std::thread>(new std::thread(&RTC::rtcThread, this));
}

void RTC::stopRTCUpdateThread()
{
	mRTCThreadRunning = false;

    if (mRTCThread)
    {
    	mRTCThread->join();
    	mRTCThread.reset();
    }
}
void RTC::rtcThread()
{
	pthread_setname_np(pthread_self(), "RTC");

	// sleep interval in minutes at boot
	int secondsInterval = 1 * 60; //1 min
	int secondsPassed = 0;
    while (mRTCThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::seconds(NTP_INTERVAL_SECONDS));
        secondsPassed = secondsPassed + NTP_INTERVAL_SECONDS;
        mNTPSync = ntpSynchronized();

        if (secondsPassed > secondsInterval)
        {
        	secondsPassed = 0;
            if (mNTPSync)
            {
            	// Min 1 minutes after start
    			log("NTP Synchronised, writing RTC");
            	LOG(INFO) << "Testing RTC clock; read current time from RTC:";
        		auto rtcTime = readRTCTime();

        		if (!rtcValidDateTime(rtcTime))
        		{
        			LOG(ERROR) << "== Time read is not valid !!";
        		}

    			secondsInterval = 3 * 24 * 60 * 60; // Every 3 days
    			writeRTCTime();
            }
        }

    }


}
}
