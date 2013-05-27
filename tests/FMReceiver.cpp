/*
 * FMReceiver.cpp
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#include "FMReceiver.h"
#include "SI4735.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/logging.h>



FMReceiver::FMReceiver(I2C &i2c, uint8_t address) :
		mI2C(i2c),
		mAddress(address),
		mPowerState(PowerState::Unknown),
		mRDSInfo(),
		mReceivingRDSInfo(),
		mRdsInfoMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false)
{
	powerOff();
}

FMReceiver::~FMReceiver()
{
	powerOff();
}

bool FMReceiver::init()
{
	/*
	std::vector<uint8_t> powerdownResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
	std::cout << "Powerdown response: " << std::hex << "0x" << (int) powerdownResponse[0] << std::endl;
	//std::cout << "CTS: " << readCTS() << std::endl;
	if (!waitForCTS()) return false;
*/
	/*
	 * ARG1
	 * CTSIEN = 0
	 * GPO2OEN = 0
	 * PATCH = 0
	 * XOSCEN = 1
	 * FUNC = 0000 (FM Receive)
	 *
	 * ARG2:
	 * OPMODE = 00000101 (0x05): Analog audio output
	 */
	std::vector<uint8_t> powerupResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_UP, POWER_UP_ARG1_XOSCEN, POWER_UP_AUDIO_OUT_ANALOG}), powerupResponse);
	std::cout << "POWER_UP Status: " << std::hex << "0x" << (int) powerupResponse[0] << std::endl;
	if (powerupResponse[0] == 0xc0)
	{
//		std::this_thread::sleep_for( std::chrono::milliseconds(1000));
		if (!waitForCTS()) return false;
		std::vector<uint8_t> powerdownResponse(1); // Vector with size 1
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
		std::cout << "Powerdown response: " << std::hex << "0x" << (int) powerdownResponse[0] << std::endl;

//		std::this_thread::sleep_for( std::chrono::milliseconds(2000));
		if (!waitForCTS()) return false;
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_UP, POWER_UP_ARG1_XOSCEN, POWER_UP_AUDIO_OUT_ANALOG}), powerupResponse);
		std::cout << "POWER_UP Status: " << std::hex << "0x" << (int) powerupResponse[0] << std::endl;
	}

	if (!waitForCTS()) return false;

	std::vector<uint8_t> getRevResponse(9); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_REV}), getRevResponse);
	std::cout << "GET_REV Status: " << std::hex << "0x" << (int) getRevResponse[0] << std::endl;
	std::cout << "PN: " << std::dec << (int) getRevResponse[1];
	std::string fw;
	fw += getRevResponse[2];
	fw += getRevResponse[3];
	std::cout << ", FW: " << fw;
	std::string patch;
	patch += getRevResponse[4];
	patch += getRevResponse[5];
	std::cout << ", Patch : " << patch;
	std::string cmp;
	cmp += getRevResponse[6];
	cmp += getRevResponse[7];
	std::cout << ", CMP : " << cmp;
	std::string chipRev;
	chipRev += getRevResponse[8];
	std::cout << ", Chip Rev: " << chipRev << std::endl;

	if (!waitForCTS()) return false;

		//Stubru: 94.5
		/*
		 * ARG1: 0x0
		 * ARG2: Freq H: 0x24
		 * ARG3: Freq L: 0xEA
		 * ARG4: AntCap = 0x0
		 */

	std::vector<uint8_t> tuneFreqResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_FREQ, 0x00, 0x24, 0xEA}), tuneFreqResponse);
	std::cout << "Tune Freq Status: " << std::hex << "0x" << (int) tuneFreqResponse[0] << std::endl;
	std::cout << "CTS: " << readCTS() << std::endl;

	/*
	std::vector<uint8_t> seekResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_SEEK_START, 0x08}), seekResponse);
	std::cout << "FM_SEEK_START Status: " << std::hex << "0x" << (int) seekResponse[0] << std::endl;
*/
	if (!waitForCTS()) return false;

	std::vector<uint8_t> tuneStatusResponse(8); // Vector with size 8
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_STATUS, 0x00}), tuneStatusResponse);
	std::cout << "FM_TUNE_STATUS Status: " << std::hex << "0x" << (int) tuneStatusResponse[0] << std::endl;
	std::cout << "Resp1: " << std::hex << (int) tuneStatusResponse[1];
	std::cout << std::dec;
	std::cout << ", Resp2: " << (int) tuneStatusResponse[2];
	std::cout << ", Resp3: " << (int) tuneStatusResponse[3];
	std::cout << ", Resp4: " << (int) tuneStatusResponse[4];
	std::cout << ", Resp5: " << (int) tuneStatusResponse[5];
	std::cout << ", Resp6: " << (int) tuneStatusResponse[6];
	std::cout << ", Resp7: " << (int) tuneStatusResponse[7] << std::endl;

	return true;
}

bool FMReceiver::powerOff()
{
	LOG(INFO) << "PowerOff";
	stopReadThread();

	switch (mPowerState)
	{
		case PowerState::PowerOff : return true;
		case PowerState::PowerOn :
			if (!waitForCTS())
			{
				return false;
			}; // No break is intended
		default:
			std::vector<uint8_t> powerdownResponse(1);
			mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
			LOG(INFO) << "POWER_DOWN Status: " << std::hex << "0x" << (int) powerdownResponse[0];

		break;
	}
	return true;
}

bool FMReceiver::powerOn()
{
	LOG(INFO) << "PowerOn";
	if (!waitForCTS()) return false;

	std::vector<uint8_t> powerupResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_UP, POWER_UP_ARG1_XOSCEN, POWER_UP_AUDIO_OUT_ANALOG}), powerupResponse);
	LOG(INFO) << "POWER_UP Status: " << std::hex << "0x" << (int) powerupResponse[0];

    //if(revision.chip=='D' && revision.firmwareMajor=='6' && revision.firmwareMinor=='0'){
	setProperty(0xFF00, 0);
    //}
	setProperty(PROP_FM_RDS_CONFIG, (FM_RDS_CONFIG_ARG_ENABLE |
                                     FM_RDS_CONFIG_ARG_BLOCK_A_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_B_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_C_2_BIT_ERRORS |
                                     FM_RDS_CONFIG_ARG_BLOCK_D_2_BIT_ERRORS) );
	//Enable RDS interrupt sources
    //Generate interrupt when new data arrives and when RDS sync is gained or lost.
    //  setProperty(PROP_FM_RDS_INT_SOURCE, (RDS_RECEIVED_MASK |
    //   RDS_SYNC_FOUND_MASK | RDS_SYNC_LOST_MASK) );
	setProperty(PROP_FM_RDS_INT_SOURCE, (RDS_RECEIVED_MASK | RDS_SYNC_FOUND_MASK | RDS_SYNC_LOST_MASK));

	//Setup FM band and spacing
	setProperty(PROP_FM_SEEK_BAND_BOTTOM, 64);
	setProperty(PROP_FM_SEEK_BAND_TOP, 108);
	setProperty(PROP_FM_SEEK_FREQ_SPACING, 10); // 100 Khz

	//North America and South Korea use default FM de-emphasis of 75 μs.
	//All others use 50 μs.
	setProperty(PROP_FM_DEEMPHASIS, FM_DEEMPHASIS_ARG_50);
    mRDSInfo.clearAll();
    mReceivingRDSInfo.clearAll();
	//startReadThread();
	return true;
}

bool FMReceiver::seekUp(int timeoutSeconds)
{
	LOG(INFO) << "SeekUp";
	if (!waitForCTS()) return false;

	std::vector<uint8_t> seekResponse(1);
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_SEEK_START, 0x08}), seekResponse);

	if (timeoutSeconds == 0)
	{
		return true;
	}
	int waitTime = 0;
	while (waitTime < timeoutSeconds)
	{
		if (readSTC())
		{
			debugTuningStatus();
			return true;
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::seconds(1));
			++waitTime;
		}
	}

	return false;
}

bool FMReceiver::tuneFrequency(double frequency)
{
	LOG(INFO) << "TuneFrequency: " << frequency << "Mhz";
	if (!waitForCTS()) return false;

	frequency = frequency * 100;
	uint8_t high = static_cast<uint16_t>(frequency) >> 8;
	uint8_t low = static_cast<uint16_t>(frequency) & 0xFF;
	std::vector<uint8_t> tuneFreqResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_FREQ, 0x00, high, low}), tuneFreqResponse);

	debugTuningStatus();

	//startReadThread();
	return true;
}

bool FMReceiver::getRDSInfo()
{
	bool rdsAvailable = true;
	if (!readRDSInt())
	{
		return false;
	}

	while (rdsAvailable)
	{
		std::vector<uint8_t> rdsInfoResponse(13);
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_RDS_STATUS, RDS_STATUS_ARG1_CLEAR_INT}), rdsInfoResponse);
		LOG(INFO) << "Status: " << std::hex << "0x" << (int)rdsInfoResponse[0];
/*
		LOG(INFO) << "RDS:" << static_cast<char>((rdsInfoResponse[4] >> 8)) << static_cast<char>((rdsInfoResponse[4] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[5] >> 8)) << static_cast<char>((rdsInfoResponse[5] & 0xFF))
   		                    << static_cast<char>((rdsInfoResponse[6] >> 8)) <<  static_cast<char>((rdsInfoResponse[6] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[7] >> 8)) << static_cast<char>((rdsInfoResponse[7] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[8] >> 8)) << static_cast<char>( (rdsInfoResponse[8] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[9] >> 8)) << static_cast<char>( (rdsInfoResponse[9] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[10] >> 8)) << static_cast<char>((rdsInfoResponse[10] & 0xFF))
		                    << static_cast<char>((rdsInfoResponse[11] >> 8)) << static_cast<char>( (rdsInfoResponse[11] & 0xFF));
*/
		LOG(INFO) << "RDS:" << static_cast<int>((rdsInfoResponse[4] >> 8)) << static_cast<int>((rdsInfoResponse[4] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[5] >> 8)) << static_cast<int>((rdsInfoResponse[5] & 0xFF))
   		                    << static_cast<int>((rdsInfoResponse[6] >> 8)) <<  static_cast<int>((rdsInfoResponse[6] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[7] >> 8)) << static_cast<int>((rdsInfoResponse[7] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[8] >> 8)) << static_cast<int>( (rdsInfoResponse[8] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[9] >> 8)) << static_cast<int>( (rdsInfoResponse[9] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[10] >> 8)) << static_cast<int>((rdsInfoResponse[10] & 0xFF))
		                    << static_cast<int>((rdsInfoResponse[11] >> 8)) << static_cast<int>( (rdsInfoResponse[11] & 0xFF));

//	    if((rdsInfoResponse[12] & FIELD_RDS_STATUS_RESP12_BLOCK_A) != RDS_STATUS_RESP12_BLOCK_A_UNCORRECTABLE){
	         //Get PI code
//	    	mRDSInfo.mProgramId = (rdsInfoResponse[PI_H] << 8) | rdsInfoResponse[PI_L] ;
	      //  LOG(INFO) << "PI: " << mRDSInfo.mProgramId;
//	    }

		// Only if no errors found
	    if (rdsInfoResponse[12] != 0)
	    {
	    	return false;
	    }
    	mRDSInfo.mProgramId = (rdsInfoResponse[PI_H] << 8) | rdsInfoResponse[PI_L] ;

	    uint8_t type =  rdsInfoResponse[Block_B_H]>>4U;
	    bool version = rdsInfoResponse[Block_B_H] & 0b00001000;
	    if (type == 0)
	    {
	         uint8_t segment =  rdsInfoResponse[Block_B_L] & 0b00000011;
	         uint8_t pos = segment * 2;
	         mReceivingRDSInfo.mStationName[pos] = rdsInfoResponse[Block_D_H];
	         mReceivingRDSInfo.mStationName[pos + 1] = rdsInfoResponse[Block_D_L];

	         if (mReceivingRDSInfo.mStationName !=mRDSInfo.mStationName)
	         {
	        	 mRDSInfo.mStationName = mReceivingRDSInfo.mStationName;
	        	 LOG(INFO) << "Station: " << mRDSInfo.mStationName;
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
	        std::cout << "Receiving" << std::endl;

	        for (uint8_t pos = 0; pos < count; ++pos)
	        {
	        	if (rdsInfoResponse[startPos + pos] == '\r')
	        	{
	   	        	if (mRDSInfo.mText != mReceivingRDSInfo.mText)
	   	        	{
	   	        		mRDSInfo.mText = mReceivingRDSInfo.mText;
		   	        	mRDSInfo.mTextType = new_ab ? TextType::TypeA: TextType::TypeB;
		   	        	LOG(INFO) << "Text: " << mRDSInfo.mText;
		   	        	mReceivingRDSInfo.clearText();
	   	        	}
	   	        	mReceivingRDSInfo.clearText();
	        	}
	        	else
	        	{
	        		mReceivingRDSInfo.mText[(segment * count) + pos] = rdsInfoResponse[startPos + pos];
	        		std::cout << mReceivingRDSInfo.mText << std::endl;
	        	}
	        }
	/*

	        if (new_ab)
	        {
		        for (uint8_t pos = 0; pos < count; ++pos)
		        {
		        	if (rdsInfoResponse[startPos + pos] == '\r')
		        	{
		   	        	 mRDSInfo.mTextA = mReceivingRDSInfo.mTextA;
		   	        	 LOG(INFO) << "TextA: " << mRDSInfo.mTextA;
		   	        	mReceivingRDSInfo.mTextA = "";
		        	};
		        	mReceivingRDSInfo.mTextA[(segment * count) + pos] = rdsInfoResponse[startPos + pos];
		        }

	        }
	        else
	        {
		        for (uint8_t pos = 0; pos < count; ++pos)
		        {
		        	if (rdsInfoResponse[startPos + pos] == '\r')
		        	{
		   	        	 mRDSInfo.mTextB = mReceivingRDSInfo.mTextB;
		   	        	 LOG(INFO) << "TextB: " << mRDSInfo.mTextB;
		   	        	mReceivingRDSInfo.mTextB = "";
		        	};
		        	mReceivingRDSInfo.mTextB[(segment * count) + pos] = rdsInfoResponse[startPos + pos];
		        }

	        }
*/
	    }
		rdsAvailable = (rdsInfoResponse[3] > 0);
	}

	return true;

}

bool FMReceiver::setProperty(int property, int value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	std::vector<uint8_t> setPropertyResponse(1);
	uint8_t propH = property >> 8;
	uint8_t propL = property & 0xFF;
	uint8_t valueH = value >> 8;
	uint8_t valueL = value & 0xFF;
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({SET_PROPERTY, 0x00, propH, propL, valueH, valueL}), setPropertyResponse);

	return true;
}

bool FMReceiver::getProperty(int property, int& value)
{
	if (!waitForCTS()) return false;  // Wait for Clear To Send

	return true;
}

void FMReceiver::debugTuningStatus()
{
	//FM_RSQ_STATUS gebruiken ??

	if (!waitForCTS()) return;  // Wait for Clear To Send
	if (!waitForSTC()) return; // Wait for tuning complete

	std::vector<uint8_t> tuneStatusResponse(8); // Vector with size 8
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_STATUS, 0x00}), tuneStatusResponse);

	uint16_t frequency;
	frequency = (tuneStatusResponse[2] << 8) | tuneStatusResponse[3];

	LOG(INFO) << "Tuning Report";
	LOG(INFO) << "=============";
	LOG(INFO) << "Freq: " << static_cast<double>(frequency) / 100 << "Mhz";
	LOG(INFO) << "RSSI: " << (int) tuneStatusResponse[4] << "dBuV";
	LOG(INFO) << "SNR: " << (int) tuneStatusResponse[5] << "dB";
	LOG(INFO) << "MULT: " << (int) tuneStatusResponse[6];
}

bool FMReceiver::waitForCTS()
{
	const int MAX_RETRIES = 20;

	int retries = 0;
	bool cts = false;
	while ((retries < MAX_RETRIES) && !cts)
	{
		//std::cout << "Read CTS" << std::endl;
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
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0x80;
}

bool FMReceiver::readSTC()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0x01;
}

bool FMReceiver::readRDSInt()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	return readIntStatusResponse[0] && 0b0100;
}

void FMReceiver::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = new std::thread(&FMReceiver::readThread, this);
}

void FMReceiver::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();

        delete mReadThread;
        mReadThread = nullptr;
    }
}
void FMReceiver::readThread()
{
    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
   //     getRDSInfo();
    }
}
