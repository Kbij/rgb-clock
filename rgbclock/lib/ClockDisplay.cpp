/*
 * ClockDisplay.cpp
 *
 *  Created on: Apr 28, 2013
 *      Author: koen
 */

#include "ClockDisplay.h"
#include "I2C.h"
#include <sstream>
#include <glog/logging.h>
#include <iostream>

namespace Hardware
{
ClockDisplay::ClockDisplay(I2C &i2c, uint8_t lcdAddress, uint8_t lsAddress) :
	mLCDisplay(i2c, lcdAddress),
	mLightSensor(i2c, lsAddress),
	mRefreshThread(nullptr),
	mRefreshThreadRunning(false),
	mPrevMin(-1),
	mRadioInfoMutex(),
	mRDSVisible(false),
	mVolumeVisible(false),
	mSignalVisible(false),
	mRDSStationName(),
	mRDSText(),
	mRDSTextPos(0),
	mReceiveLevel(0),
	mVolume(0)
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

void ClockDisplay::showVolume()
{
	mVolumeVisible = true;
}

void ClockDisplay::hideVolume()
{
	mVolumeVisible = false;
}

void ClockDisplay::showSignal()
{
	mSignalVisible = true;
}

void ClockDisplay::hideSignal()
{
	mSignalVisible = false;
}

void ClockDisplay::showRDSInfo()
{
   // std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);
	mRDSVisible = true;
 //   mRDSStationName = "";
  //  mRDSText = "";
}

void ClockDisplay::hideRDSInfo()
{
  //  std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);
	mRDSVisible = false;
  //  mRDSStationName = "";
  //  mRDSText = "";
}

void ClockDisplay::showNextAlarm(const struct tm& nextAlarm)
{
	std::stringstream hourStream;
	hourStream.width(2);
	hourStream.fill('0');
	hourStream << nextAlarm.tm_hour;
	mLCDisplay.writeGraphicText(0,0, hourStream.str(), FontType::Terminal8);
	mLCDisplay.writeGraphicText(13,0, ":", FontType::Terminal8);

	std::stringstream minStream;
	minStream.width(2);
	minStream.fill('0');
	minStream << nextAlarm.tm_min;
	mLCDisplay.writeGraphicText(18,0, minStream.str(), FontType::Terminal8);
}

void ClockDisplay::hideNextAlarm()
{

}

void ClockDisplay::radioRdsUpdate(RDSInfo rdsInfo)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);
	if (mRDSStationName != rdsInfo.mStationName)
	{
		mRDSStationName = rdsInfo.mStationName;
		mRDSStationName = mRDSStationName.substr(0, 7);
		mRDSStationName.append(7 - mRDSStationName.size(), ' ');
	}

	if (mRDSText != rdsInfo.mText)
	{
		mRDSText = rdsInfo.mText;
		mRDSTextPos = 0;
	}

	mReceiveLevel = rdsInfo.mReceiveLevel;
	mReceiveLevel = static_cast<int>(static_cast<double> (mReceiveLevel) / 65 * 100);
}

void ClockDisplay::radioStateUpdate(RadioInfo radioInfo)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);
	if (radioInfo.mState == RadioState::PwrOn)
	{
		showRDSInfo();
		showVolume();
		showSignal();
		mVolume = radioInfo.mVolume;
	}
	if (radioInfo.mState == RadioState::PwrOff)
	{
		hideRDSInfo();
		hideVolume();
		hideSignal();
	}
}

void ClockDisplay::drawVolume()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);

	if (mVolume > 100)
	{
		mVolume = 100;
	}
	const uint8_t top = 10;
	const uint8_t bottom = 31;

	const double step = ((double)bottom - (double)top) / 100.0;

	uint8_t length = mVolume * step;

	// top part: clear
	mLCDisplay.rectangle(158, top, 159, 31-length - 1, false, false);
	// bottom part: set
	mLCDisplay.rectangle(158, 31-length, 159, 31, true, false);
}

void ClockDisplay::eraseVolume()
{
	mLCDisplay.rectangle(158, 10, 159, 31, false, false);
}

void ClockDisplay::drawSignal()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);

	if (mReceiveLevel >= 75)
	{
		mLCDisplay.hLine(159-3, 159, 0, true);
	}
	else
	{
		mLCDisplay.hLine(159-3, 159, 0, false);
	}

	if (mReceiveLevel >= 50)
	{
		mLCDisplay.hLine(159-2, 159, 1, true);
	}
	else
	{
		mLCDisplay.hLine(159-2, 159, 1, false);
	}
	if (mReceiveLevel >= 25)
	{
		mLCDisplay.hLine(159-1, 159, 2, true);
	}
	else
	{
		mLCDisplay.hLine(159-1, 159, 2, false);
	}

	if (mReceiveLevel >= 0)
	{
		mLCDisplay.point(159, 3, true);
	}
	else
	{
		mLCDisplay.point(159, 3, false);
	}

}

void ClockDisplay::eraseSignal()
{
	mLCDisplay.rectangle(156, 0, 159, 3, false, true);
}

void ClockDisplay::drawRDS()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioInfoMutex);


	mLCDisplay.writeGraphicText(0, 14, mRDSStationName, FontType::Terminal8);
	if (mRDSText.size() > 0)
	{
		std::string localRDSText = mRDSText.substr(mRDSTextPos, std::string::npos);
		if (localRDSText.size()  > 26)
		{
			localRDSText = localRDSText.substr(0, 26);
			mRDSTextPos +=2;
		}
		else
		{
			localRDSText.append(26 - localRDSText.size(), ' ');
			mRDSTextPos = 0;
		}
		mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
	}
	else
	{
		std::string localRDSText(26, ' ');
		mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
	}
}

void ClockDisplay::eraseRDS()
{
	std::string stationName(7, ' ');
	mLCDisplay.writeGraphicText(0, 14, stationName, FontType::Terminal8);

	std::string localRDSText(26, ' ');
	mLCDisplay.writeGraphicText(0, 24, localRDSText, FontType::Terminal8);
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
			mLCDisplay.writeGraphicText(44, 0, hourStream.str(), FontType::Verdana20);
			mLCDisplay.writeGraphicText(90, 0, ":", FontType::Verdana20);

			std::stringstream minStream;
			minStream.width(2);
			minStream.fill('0');
			minStream << timeInfo->tm_min;
			mLCDisplay.writeGraphicText(104,0, minStream.str(), FontType::Verdana20);

		}

		mPrevMin = timeInfo->tm_min;

	    if (mRDSVisible)
		{
	    	drawRDS();
		}
	    else
	    {
	    	eraseRDS();
	    }
	    if (mVolumeVisible)
	    {
	    	drawVolume();
	    }
	    else
	    {
	    	eraseVolume();
	    }
	    if (mSignalVisible)
	    {
	    	drawSignal();
	    }
	    else
	    {
	    	eraseSignal();
	    }

/*
		double lux = mLightSensor.lux();
		std::stringstream stream;

		stream << "Measured Lux: " << lux;
		showRDSInfo(stream.str());
		*/
    }
}
}
