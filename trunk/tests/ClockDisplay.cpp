/*
 * ClockDisplay.cpp
 *
 *  Created on: Apr 28, 2013
 *      Author: koen
 */

#include "ClockDisplay.h"
#include "I2C.h"
#include <time.h>
#include <sstream>

ClockDisplay::ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress) :
	mLCDisplay(i2c, lcdAddress),
	mLightSensor(i2c, lsAddress),
	mRefreshThread(nullptr),
	mRefreshThreadRunning(false),
	mPrevMin(-1)
{
	mLCDisplay.initGraphic();
	mLCDisplay.clearGraphicDisplay();
	startRefreshThread();
}

ClockDisplay::~ClockDisplay()
{
	stopRefreshThread();
}

void ClockDisplay::showClock()
{

}

void ClockDisplay::hideClock()
{

}

void ClockDisplay::showVolume(uint8_t vol)
{

}

void ClockDisplay::hideVolume()
{

}

void ClockDisplay::showSignal(uint8_t vol)
{

}

void ClockDisplay::hideSignal()
{

}

void ClockDisplay::showRDSInfo(std::string rdsInfo)
{

}

void ClockDisplay::hideRDSInfo()
{

}
void ClockDisplay::startRefreshThread()
{
	mRefreshThreadRunning = true;

    // create refresh thread object and start read thread
	mRefreshThread = new std::thread(&ClockDisplay::refreshThread, this);
}

void ClockDisplay::stopRefreshThread()
{
	mRefreshThreadRunning = false;

    if (mRefreshThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mRefreshThread->join();

        delete mRefreshThread;
        mRefreshThread = nullptr;
    }
}

void ClockDisplay::refreshThread()
{
    while (mRefreshThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		time_t rawTime;
		struct tm* timeInfo;

		time(&rawTime);
		timeInfo = localtime(&rawTime);
		if (timeInfo->tm_min != mPrevMin)
		{
			std::stringstream hourStream;
			hourStream.width(2);
			hourStream.fill('0');
			hourStream << timeInfo->tm_hour;
			mLCDisplay.writeGraphicText(10,0, hourStream.str(), FontType::Verdana20);
			mLCDisplay.writeGraphicText(55,0, ":", FontType::Verdana20);

			std::stringstream minStream;
			minStream.width(2);
			minStream.fill('0');
			minStream << timeInfo->tm_min;
			mLCDisplay.writeGraphicText(70,0, minStream.str(), FontType::Verdana20);

		}

		mPrevMin = timeInfo->tm_min;

		double lux = mLightSensor.lux();
		std::stringstream stream;
		stream << "Measured Lux: " << lux;
		mLCDisplay.writeGraphicText(0, 24, stream.str(), FontType::Terminal8);
    }
}
