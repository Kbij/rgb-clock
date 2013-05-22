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

FMReceiver::FMReceiver(I2C &i2c, uint8_t address) :
		mI2C(i2c),
		mAddress(address)
{
	init();
}

FMReceiver::~FMReceiver()
{

}

void FMReceiver::init()
{
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
	std::cout << "Powerup response: " << std::hex << "0x" << (int) powerupResponse[0] << std::endl;
	std::cout << "CTS: " << readCTS() << std::endl;

	std::chrono::milliseconds dura( 5000 );

	if (readCTS())
	{
		std::vector<uint8_t> getRevResponse(9); // Vector with size 1
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({GET_REV}), getRevResponse);
		std::cout << "Get Rev Status: " << std::hex << "0x" << (int) getRevResponse[0] << std::endl;
		std::cout << "PN: " << std::dec << (int) getRevResponse[1] << std::endl;
		std::cout << "FW Major: " << (int) getRevResponse[2] << std::endl;
		std::cout << "FW Minor: " << (int) getRevResponse[3] << std::endl;
		std::cout << "Patch H: " << (int) getRevResponse[4] << std::endl;
		std::cout << "Patch L: " << (int) getRevResponse[5] << std::endl;
		std::cout << "CMP Major: " << (int) getRevResponse[6] << std::endl;
		std::cout << "CMP Minor: " << (int) getRevResponse[7] << std::endl;
		std::cout << "Chip Rev: " << (int) getRevResponse[8] << std::endl;

		//Stubru: 94.5
		/*
		 * ARG1: 0x0
		 * ARG2: Freq H: 0x24
		 * ARG3: Freq L: 0xEA
		 * ARG4: AntCap = 0x0
		 */
		std::vector<uint8_t> tuneFreqResponse(1); // Vector with size 1
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_FREQ, 0x00, 0x24, 0xEA, 0x00}), tuneFreqResponse);
		std::cout << "Tune Freq Status: " << std::hex << "0x" << (int) tuneFreqResponse[0] << std::endl;
		std::cout << "CTS: " << readCTS() << std::endl;

		std::chrono::milliseconds dura( 1000 );
		std::cout << "CTS: " << readCTS() << std::endl;

		std::vector<uint8_t> tuneStatusResponse(8); // Vector with size 1
		mI2C.writeReadDataSync(mAddress, std::vector<uint8_t>({FM_TUNE_STATUS, 0x00}), tuneStatusResponse);
		std::cout << "TuneStatus Status: " << std::hex << "0x" << (int) tuneStatusResponse[0] << std::endl;
		std::cout << "Resp1: " << std::hex << (int) tuneStatusResponse[1] << std::endl;
		std::cout << std::dec;
		std::cout << "Resp2: " << (int) tuneStatusResponse[2] << std::endl;
		std::cout << "Resp3: " << (int) tuneStatusResponse[3] << std::endl;
		std::cout << "Resp4: " << (int) tuneStatusResponse[4] << std::endl;
		std::cout << "Resp5: " << (int) tuneStatusResponse[5] << std::endl;
		std::cout << "Resp6: " << (int) tuneStatusResponse[6] << std::endl;
		std::cout << "Resp7: " << (int) tuneStatusResponse[7] << std::endl;


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
