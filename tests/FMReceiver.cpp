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
		mAddress(address)
{
	init();
}

FMReceiver::~FMReceiver()
{

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
	 * XOSCEN = 0
	 * FUNC = 0000 (FM Receive)
	 *
	 * ARG2:
	 * OPMODE = 00000101 (0x05): Analog audio output
	 */
	std::vector<uint8_t> powerupResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_UP, 0x00, 0x05}), powerupResponse);
	std::cout << "POWER_UP Status: " << std::hex << "0x" << (int) powerupResponse[0] << std::endl;
	if (powerupResponse[0] == 0xc0)
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(1000));
		if (!waitForCTS()) return false;
		std::vector<uint8_t> powerdownResponse(1); // Vector with size 1
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
		std::cout << "Powerdown response: " << std::hex << "0x" << (int) powerdownResponse[0] << std::endl;

		std::this_thread::sleep_for( std::chrono::milliseconds(2000));
		if (!waitForCTS()) return false;
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({POWER_UP, 0x00, 0x05}), powerupResponse);
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
/*
	std::vector<uint8_t> tuneFreqResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_FREQ, 0x00, 0x24, 0xEA}), tuneFreqResponse);
	std::cout << "Tune Freq Status: " << std::hex << "0x" << (int) tuneFreqResponse[0] << std::endl;
	std::cout << "CTS: " << readCTS() << std::endl;
*/
	std::vector<uint8_t> seekResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_SEEK_START, 0x08}), seekResponse);
	std::cout << "FM_SEEK_START Status: " << std::hex << "0x" << (int) seekResponse[0] << std::endl;

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
		std::cout << "Retries: " << retries << std::endl;
		return true;
	}
}

bool FMReceiver::readCTS()
{
	std::vector<uint8_t> readIntStatusResponse(1); // Vector with size 1
	mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_INT_STATUS}), readIntStatusResponse);
	//std::cout << "GET_INT_STATUS response: " << std::hex << "0x" << (int) readIntStatusResponse[0] << std::endl;
	return readIntStatusResponse[0] && 0x80;
}

bool FMReceiver::readSTC()
{
	return false;
}
