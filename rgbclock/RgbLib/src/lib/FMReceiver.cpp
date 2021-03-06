/*
 * FMReceiver.cpp
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#include "FMReceiver.h"
#include "RadioObserverIf.h"
#include "SI4735.h"
#include "MainboardControl.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/logging.h>
#include <algorithm>
#include <pthread.h>

namespace Hardware
{

FMReceiver::FMReceiver(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl) :
		mI2C(i2c),
		mAddress(address),
		mMainboardControl(mainboardControl),
		mPowerCounter(0),
		mPowerMutex(),
		mPowerState(FMPowerState::Unknown),
		mRDSInfo(),
		mReceivingRDSInfo(),
		mReceiverMutex(),
		mRdsInfoMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false),
		mRadioObservers(),
		mRadioObserversMutex()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for(std::chrono::seconds(1));

	powerOff();
	if (mMainboardControl) mMainboardControl->selectInput(InputSelection::RadioIn);

    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    mPowerCounter = 0;

	mI2C.registerAddress(address, "FM Receiver");
}

FMReceiver::~FMReceiver()
{
	powerOff();
	LOG(INFO) << "FMReceiver destructor exit";
}

void FMReceiver::registerRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.insert(observer);
    }
}
void FMReceiver::unRegisterRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.erase(observer);
    }
}
bool FMReceiver::powerOn()
{
	LOG(INFO) << "PowerOn";
    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    if (mPowerCounter == 0)
    {
    	internalPowerOn();
    }
    mPowerCounter++;

    return true;
}

bool FMReceiver::powerOff()
{
	LOG(INFO) << "PowerOff";
    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    if (mPowerCounter <= 1)
    {
    	internalPowerOff();
    }
    mPowerCounter--;

    return true;
}



bool FMReceiver::internalPowerOn()
{
	LOG(INFO) << "Internal PowerOn";
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

	if (!waitForCTS()) return false;

	std::vector<uint8_t> powerupResponse(1); // Vector with size 1
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({POWER_UP, POWER_UP_ARG1_XOSCEN, POWER_UP_AUDIO_OUT_ANALOG}), powerupResponse);
	LOG(INFO) << "POWER_UP Status: " << std::hex << "0x" << (int) powerupResponse[0];

    //if(revision.chip=='D' && revision.firmwareMajor=='6' && revision.firmwareMinor=='0'){
	setProperty(0xFF00, 0);
    //}
/*
	setProperty(PROP_FM_RDS_CONFIG, (FM_RDS_CONFIG_ARG_ENABLE |
                                     FM_RDS_CONFIG_ARG_BLOCK_A_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_B_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_C_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_D_2_BIT_ERRORS) );
*/
	setProperty(PROP_FM_RDS_CONFIG, (FM_RDS_CONFIG_ARG_ENABLE |
                                     FM_RDS_CONFIG_ARG_BLOCK_A_NO_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_B_NO_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_C_NO_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_D_NO_ERRORS) );

	//Enable RDS interrupt sources
    //Generate interrupt when new data arrives and when RDS sync is gained or lost.
    //  setProperty(PROP_FM_RDS_INT_SOURCE, (RDS_RECEIVED_MASK |
      // RDS_SYNC_FOUND_MASK | RDS_SYNC_LOST_MASK) );
	setProperty(PROP_FM_RDS_INT_SOURCE, (RDS_RECEIVED_MASK | RDS_SYNC_FOUND_MASK | RDS_SYNC_LOST_MASK));


	//Setup FM band and spacing
	setProperty(PROP_FM_SEEK_BAND_BOTTOM, 6400);
	setProperty(PROP_FM_SEEK_BAND_TOP, 10800);
	setProperty(PROP_FM_SEEK_FREQ_SPACING, 10); // 100 Khz

	//North America and South Korea use default FM de-emphasis of 75 μs.
	//All others use 50 μs.
	setProperty(PROP_FM_DEEMPHASIS, FM_DEEMPHASIS_ARG_50);

    mRDSInfo.clearAll();
    mReceivingRDSInfo.clearAll(true);
	startReadThread();
	return true;
}

bool FMReceiver::internalPowerOff()
{
	LOG(INFO) << "Internal PowerOff";
	stopReadThread();
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);
	switch (mPowerState)
	{
		case FMPowerState::PowerOff : return true;
		case FMPowerState::PowerOn :
			if (!waitForCTS())
			{
				return false;
			}; // No break is intended
		default:
			std::vector<uint8_t> powerdownResponse(1);
			mI2C.readWriteData(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
			LOG(INFO) << "POWER_DOWN Status: " << std::hex << "0x" << (int) powerdownResponse[0];

		break;
	}
	return true;
}

bool FMReceiver::seekUp(int timeoutSeconds)
{
	LOG(INFO) << "SeekUp";
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

	if (!waitForCTS()) return false;
	if (!waitForSTC()) return false; // Previous seek not complete

	std::vector<uint8_t> seekResponse(1);
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({FM_SEEK_START, 0x0C}), seekResponse); // seek up & wrap
	mReceivingRDSInfo.clearAll(true);
	mRDSInfo.clearAll();
	mRDSInfo.mValidRds = false;
	mRDSInfo.mStationName = "Seek...";
	notifyObservers();
	return true;
}

bool FMReceiver::tuneFrequency(double frequency)
{
	LOG(INFO) << "TuneFrequency: " << frequency << "Mhz";
	std::stringstream freqStream;
	freqStream << frequency << "Mhz";
	mRDSInfo.mStationName = freqStream.str();
	mRDSInfo.mText = "";
	mRDSInfo.mValidRds = false;

	notifyObservers();

    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

	if (!waitForCTS()) return false;

	frequency = frequency * 100;
	uint8_t high = static_cast<uint16_t>(frequency) >> 8;
	uint8_t low = static_cast<uint16_t>(frequency) & 0xFF;
	std::vector<uint8_t> tuneFreqResponse(1); // Vector with size 1
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({FM_TUNE_FREQ, 0x00, high, low}), tuneFreqResponse);




	return true;
}

RDSInfo FMReceiver::getRDSInfo()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRdsInfoMutex);
    return mRDSInfo; // Should be copy constructor
}

TextType ABToTextType(bool ab)
{
	return ab ? TextType::TypeA: TextType::TypeB;
}

const std::string whiteSpaces( " \f\n\r\t\v" );

std::string trimRight(const std::string& str,
      const std::string& trimChars = whiteSpaces )
{
	std::string result = str;
	std::string::size_type pos = result.find_last_not_of( trimChars );
	result.erase( pos + 1 );

	return result;
}


std::string trimLeft(const std::string& str,
      const std::string& trimChars = whiteSpaces )
{
	std::string result = str;
	std::string::size_type pos = result.find_first_not_of( trimChars );
    result.erase( 0, pos );

    return result;
}


std::string trim(const std::string& str, const std::string& trimChars = whiteSpaces )
{
	std::string result = trimRight( str, trimChars );
	result = trimLeft( result, trimChars );

	return result;
}

void FMReceiver::readRDSInfo()
{
    std::lock_guard<std::recursive_mutex> lk_guard(mRdsInfoMutex);

    bool rdsAvailable = true;

	if (!readRDSInt())
	{
		return;
	}

	while (rdsAvailable)
	{
		std::vector<uint8_t> rdsInfoResponse(13);
		mI2C.readWriteData(mAddress, std::vector<uint8_t>({FM_RDS_STATUS, RDS_STATUS_ARG1_CLEAR_INT}), rdsInfoResponse);

		rdsAvailable = (rdsInfoResponse[3] > 0);

		// Only if no errors found
	    if (rdsInfoResponse[12] != 0)
	    {
	    	return;
	    }
    	//mRDSInfo.mProgramId = (rdsInfoResponse[PI_H] << 8) | rdsInfoResponse[PI_L] ;

	    if (!(rdsInfoResponse[Block_B_H] & 0x01)) // If not RDSSync
	    {
       		mRDSInfo.mText = "";
       		notifyObservers();
	    	return;
	    }

	    uint8_t type =  rdsInfoResponse[Block_B_H]>>4U;
	    bool version = rdsInfoResponse[Block_B_H] & 0b00001000;

	    if (type == 0)
	    {
	        uint8_t segment =  rdsInfoResponse[Block_B_L] & 0b00000011;
	        uint8_t pos = segment * 2;

	 		if ((pos + 1) < static_cast<uint8_t>(mReceivingRDSInfo.mStationName.size()))
			{
		 		mReceivingRDSInfo.mStationName[pos] = rdsInfoResponse[Block_D_H];
		 		mReceivingRDSInfo.mStationName[pos + 1] = rdsInfoResponse[Block_D_L];
			}

	        if (mReceivingRDSInfo.stationNameComplete() && (trim(mRDSInfo.mStationName) != trim(mReceivingRDSInfo.mStationName)))
		    {
	        	mRDSInfo.mStationName = trim(mReceivingRDSInfo.mStationName);
		    	mRDSInfo.mValidRds = true;
		        LOG(INFO) << "Station: " << mRDSInfo.mStationName;
		        notifyObservers();
		    }
	    }

	    if (type == 2)
	    {
	    	bool new_ab = bool(rdsInfoResponse[Block_B_L] & 0b00010000);
	        uint8_t segment = rdsInfoResponse[Block_B_L] & 0x0F;

	        uint8_t startPos = 0;
	        uint8_t count = 0;
	        if (!version)
	        {
	        	count = 4;
	        	startPos = Block_C_H;
	        }
	        else
	        {
	        	count = 2;
	        	startPos = Block_D_H;
	        }

	        for (uint8_t pos = 0; pos < count; ++pos)
	        {
	        	if (mReceivingRDSInfo.mTextType != ABToTextType(new_ab))
	        	{
	        		mReceivingRDSInfo.clearAll(true);
	        		mReceivingRDSInfo.mTextType = ABToTextType(new_ab);
	        	}

	        	uint8_t textPos = (segment * count) + pos;
	        	uint8_t recPos = startPos + pos;

	        	if (recPos < rdsInfoResponse.size())
	        	{
					if (rdsInfoResponse[recPos] == '\r')
					{
						mReceivingRDSInfo.mText = mReceivingRDSInfo.mText.substr(0, textPos);
						if (mReceivingRDSInfo.textComplete())
						{
							if (mRDSInfo.mText != mReceivingRDSInfo.mText)
							{
								mRDSInfo.mText = mReceivingRDSInfo.mText;
								mRDSInfo.mTextType = mReceivingRDSInfo.mTextType;
								LOG(INFO) << "Clean Text: " << mRDSInfo.mText;

								notifyObservers();
								mReceivingRDSInfo.clearText(true);
							}
						}
					}
					else
					{
						if (textPos < mReceivingRDSInfo.mText.size())
						{
							mReceivingRDSInfo.mText[textPos] = rdsInfoResponse[recPos];
						}
					}
	        	}
	        }
	    }
	}

	return;
}

bool FMReceiver::setProperty(int property, int value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	std::vector<uint8_t> setPropertyResponse(1);
	uint8_t propH = property >> 8;
	uint8_t propL = property & 0xFF;
	uint8_t valueH = value >> 8;
	uint8_t valueL = value & 0xFF;
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({SET_PROPERTY, 0x00, propH, propL, valueH, valueL}), setPropertyResponse);

	return true;
}

bool FMReceiver::getProperty(int property, int& value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	return true;
}

bool FMReceiver::waitForCTS()
{
	const int MAX_RETRIES = 20;

	int retries = 0;
	bool cts = false;
	while ((retries < MAX_RETRIES) && !cts)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(100));
		cts = readCTS();
		++retries;
	}
	if (retries >= MAX_RETRIES)
	{
		LOG(ERROR) << "Timeout waiting for CTS";
		return false;
	}
	else
	{
		return true;
	}
}

bool FMReceiver::waitForSTC()
{
	const int MAX_RETRIES = 20;

	int retries = 0;
	bool stc = false;
	while ((retries < MAX_RETRIES) && !stc)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(100));
		stc = readSTC();
		++retries;
	}
	if (retries >= MAX_RETRIES)
	{
		LOG(ERROR) << "Timeout waiting for STC";
		return false;
	}
	else
	{
		return true;
	}
}

bool FMReceiver::readCTS()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0x80;
}

bool FMReceiver::readSTC()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0x01;
}

bool FMReceiver::readRDSInt()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.readWriteData(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0b0100;
}

void FMReceiver::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = std::unique_ptr<std::thread>(new std::thread(&FMReceiver::readThread, this));
}

void FMReceiver::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();
        mReadThread.reset();
    }
}
void FMReceiver::readThread()
{
	pthread_setname_np(pthread_self(), "FM Receiver");

    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

    	if (!waitForCTS()) return;  // Wait for Clear To Send

    	std::vector<uint8_t> tuneStatusResponse(8); // Vector with size 8
    	mI2C.readWriteData(mAddress, std::vector<uint8_t>({FM_TUNE_STATUS, 0x00}), tuneStatusResponse);

    	std::lock_guard<std::recursive_mutex> lk_rdsguard(mRdsInfoMutex);

    	if 	(tuneStatusResponse[0] && 0x01) // If STC
    	{
    		if (!mRDSInfo.mValidRds)
    		{
    	    	uint16_t frequency;
    	    	frequency = (tuneStatusResponse[2] << 8) | tuneStatusResponse[3];
    			std::stringstream freqStream;
    			freqStream << static_cast<double>(frequency) / 100 << "Mhz";

    	    	if (mRDSInfo.mStationName != freqStream.str())
    	    	{
    	    		mRDSInfo.mStationName = freqStream.str();
        	    	notifyObservers();

    	    	}
    		}
    	}

    	if (!waitForCTS()) return;  // Wait for Clear To Send

    	std::vector<uint8_t> rsqStatusResponse(8); // Vector with size 8
    	mI2C.readWriteData(mAddress, std::vector<uint8_t>({FM_RSQ_STATUS, 0x00}), rsqStatusResponse);

    	if 	(rsqStatusResponse[0] && 0x01) // If STC
    	{
    		mRDSInfo.mReceiveLevel = rsqStatusResponse[4];
    	}

    	readRDSInfo();
    }
}

void FMReceiver::notifyObservers()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
        //observer->radioRdsUpdate(mRDSInfo);
    }
}

}
