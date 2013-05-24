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
		mPowerState(PowerState::UNKNOWN)
{
	powerOff();
	//init();
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
	switch (mPowerState)
	{
		case PowerState::POWEROFF : return true;
		case PowerState::POWERON :
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
         FM_RDS_CONFIG_ARG_BLOCK_A_UNCORRECTABLE |
         FM_RDS_CONFIG_ARG_BLOCK_B_5_BIT_ERRORS |
         FM_RDS_CONFIG_ARG_BLOCK_C_5_BIT_ERRORS |
         FM_RDS_CONFIG_ARG_BLOCK_D_5_BIT_ERRORS) );
        //Enable RDS interrupt sources
        //Generate interrupt when new data arrives and when RDS sync is gained or lost.
        setProperty(PROP_FM_RDS_INT_SOURCE, (RDS_RECEIVED_MASK |
         RDS_SYNC_FOUND_MASK | RDS_SYNC_LOST_MASK) );

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
	return true;
}
bool FMReceiver::setProperty(int property, int value)
{
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
	return true;
}

void FMReceiver::debugTuningStatus()
{
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
