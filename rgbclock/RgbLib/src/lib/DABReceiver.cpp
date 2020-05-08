/*
** RgbClock
** File description:
** DABReceiver
*/

#include "DABReceiver.h"
#include "DABReceiverDef.h"
#include "DABCommands.h"
#include "FMReceiver.h"
#include "RadioObserverIf.h"
#include "SI4735.h"
#include "MainboardControl.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <glog/stl_logging.h>
#include <glog/logging.h>
#include <algorithm>
#include <pthread.h>
#include <iomanip>

namespace 
{
const int FIRMWARE_BLOCK_SIZE = 4092;
const uint8_t VRT_DAB_FREQUENCY = 41; //(12A)
const int MAX_RETRIES = 500;
const uint8_t WAIT_CTS = 0x80;
const uint8_t WAIT_CTS_STC = 0x81;
}


namespace Hardware
{

DABReceiver::DABReceiver(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl) :
		mI2C(i2c),
		mAddress(address),
		mMainboardControl(mainboardControl),
		mPowerCounter(0),
		mPowerMutex(),
		mPowerState(DABPowerState::Unknown),
        mReceiverMutex(),
		mReadThread(nullptr),
		mReadThreadRunning(false),
		mRadioObservers(),
		mRadioObserversMutex()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for(std::chrono::seconds(1));

	if (mMainboardControl) mMainboardControl->selectInput(InputSelection::RadioIn);

    std::lock_guard<std::mutex> lk_guard(mPowerMutex);
    mPowerCounter = 0;

	mI2C.registerAddress(address, "DAB Receiver");


//	init();
}

DABReceiver::~DABReceiver()
{
	// powerOff();
	// LOG(INFO) << "DABReceiver destructor exit";
}

void DABReceiver::registerRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.insert(observer);
    }
}

void DABReceiver::unRegisterRadioObserver(RadioObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);

        mRadioObservers.erase(observer);
    }
}

void DABReceiver::init()
{
	std::this_thread::sleep_for( std::chrono::milliseconds(10));

    //See Page 154 in the manual
    std::vector<uint8_t> powerUpParams;
	powerUpParams.push_back(0x00); // ARG1 CTSIEN is disabled
	powerUpParams.push_back((1 << 4) | (7 << 0)); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7 (19.2Mhz X-Tal)

    //See page 438 Programming Manual)
    //ESR = 70 Ohm
    powerUpParams.push_back(0x48); // ARG3 IBIAS=0x48 (Sdk: 0x28)

    //19 200 000 hz = 0x0124F800
	powerUpParams.push_back(0x00); // ARG4 XTAL
	powerUpParams.push_back(0xF8); // ARG5 XTAL (ik 0xF8)
	powerUpParams.push_back(0x24); // ARG6 XTAL
	powerUpParams.push_back(0x01); // ARG7 XTAL 19.2MHz

    //See Page 444 of programming manual (ABM8 Xtal = 10pf)
    //CTUN = 2*(Cl-Clpar) -Cx -> 2*(10pf - 0) - 0= 20pf -> 0x28
	powerUpParams.push_back(0x28); // ARG8 CTUN (ik: 0x28) (Sdk: 0x07)

	powerUpParams.push_back(0x00 | (1 << 4)); // ARG9
	powerUpParams.push_back(0x00); // ARG10
	powerUpParams.push_back(0x00); // ARG11
	powerUpParams.push_back(0x00); // ARG12

    //See page 443
	powerUpParams.push_back(0x18); // ARG13 IBIAS_RUN (Ik: 0x12)
	powerUpParams.push_back(0x00); // ARG14
	powerUpParams.push_back(0x00); // ARG15    

	VLOG(1) << "Writing POWER_UP";
    sendCommand(SI468X_POWER_UP, powerUpParams, 0, WAIT_CTS, 10);
	std::this_thread::sleep_for( std::chrono::milliseconds(10));


	hostload("./firmware/rom00_patch.016.bin");
	std::this_thread::sleep_for( std::chrono::seconds(1));
	hostload("./firmware/dab_radio.bin");


	std::this_thread::sleep_for( std::chrono::seconds(1));

	VLOG(1) << "Writing BOOT";
    sendCommand(SI468X_BOOT, std::vector<uint8_t>({0x00}), 0, WAIT_CTS, 2000);
	std::this_thread::sleep_for( std::chrono::seconds(2));
	
	readSysState();
	readPartInfo();

	// setProperty(SI468X_DAB_TUNE_FE_CFG, 0x0001); // front end switch closed
	// setProperty(SI468X_DAB_TUNE_FE_VARM, 0x1710); // Front End Varactor configuration (Changed from '10' to 0x1710 to improve receiver sensitivity - Bjoern 27.11.14)
	// setProperty(SI468X_DAB_TUNE_FE_VARB, 0x1711); // Front End Varactor configuration (Changed from '10' to 0x1711 to improve receiver sensitivity - Bjoern 27.11.14)	
	// setProperty(SI468X_PIN_CONFIG_ENABLE, 0x0002); // enable I2S output (BUG!!! either DAC or I2S seems to work)
	// setProperty(SI468X_DIGITAL_IO_OUTPUT_SELECT, 0x0000); // I2S Slave Mode
}

void DABReceiver::readSysState()
{
	LOG(INFO) << "Reading System State";
	auto sysStateRaw = sendCommand(SI468X_GET_SYS_STATE, std::vector<uint8_t> ({0x00}), 6, WAIT_CTS, 5);
	LOG(INFO) << "Raw sys state: " << vectorToHexString(sysStateRaw, false);
	SysState sysState(sysStateRaw);

	LOG(INFO) << "System State: " << sysState.toString();
}

// void DABReceiver::readStatus()
// {
// 	auto status = sendCommand(SI468X_RD_REPLY,  23);
// 	VLOG(3) << "Raw Status: " << vectorToHexString(status);
// 	Status statusCmd(status);
// 	LOG(INFO) << statusCmd.toString();
// }

void DABReceiver::readPartInfo()
{
	LOG(INFO) << "Reading PartInfo";
	auto partInfoRaw = sendCommand(SI468X_GET_PART_INFO, std::vector<uint8_t> ({0x00}), 23, WAIT_CTS, 1);
	VLOG(1) << "Raw Status: " << vectorToHexString(partInfoRaw);
	PartInfo partInfo(partInfoRaw);
	LOG(INFO) << partInfo.toString();
}

void DABReceiver::hostload(const std::string& fileName)
{
	std::vector<uint8_t> firmware;
	if (readFile(fileName,  firmware))
	{
		VLOG(1) << "Firmware size: " << firmware.size();

		Status loadInitStatus(sendCommand(SI468X_LOAD_INIT, std::vector<uint8_t> ({0x00}), 0, WAIT_CTS, 1));
		if (loadInitStatus.error())
		{
			LOG(ERROR) << "Load init resulted in a error: " << loadInitStatus.toString();
		}
		std::this_thread::sleep_for( std::chrono::milliseconds(50));

		int bytesSend = 0;
		auto filePos = firmware.begin();
		while(filePos != firmware.end())
		{
			int amountToSend = (firmware.end() - filePos) >= FIRMWARE_BLOCK_SIZE ? FIRMWARE_BLOCK_SIZE : firmware.end() - filePos;
			std::vector<uint8_t> firmwareBlock(filePos, (filePos + amountToSend));
			std::vector<uint8_t> hostLoadParams({0x00, 0x00, 0x00});

			std::copy (firmwareBlock.begin(),firmwareBlock.end(),back_inserter(hostLoadParams));

			VLOG(30) << "Sending bytes: " << firmwareBlock.size() << ", already send: " << bytesSend;
			VLOG(40) << vectorToHexString(hostLoadParams, true);

			Status hostLoadStatus(sendCommand(SI468X_HOST_LOAD, hostLoadParams, 0, WAIT_CTS));
			if (hostLoadStatus.error())
			{
				LOG(ERROR) << "Host load resulted in a error: " << hostLoadStatus.toString();
			}

			bytesSend += firmwareBlock.size();

			std::advance(filePos , amountToSend);

			std::this_thread::sleep_for( std::chrono::milliseconds(5));
		}
		VLOG(1) << "Total bytes send: " << bytesSend;
		
		std::this_thread::sleep_for( std::chrono::milliseconds(50));		
	}
}

void DABReceiver::tuneFrequencyIndex(uint8_t index)
{
	LOG(INFO) << "Tune to frequency index: " << (int) index;

	std::vector<uint8_t> tuneFreqParam;
	tuneFreqParam.push_back(0x00); //Automatic Injection
	tuneFreqParam.push_back(index);
	tuneFreqParam.push_back(0x00);

	tuneFreqParam.push_back(0x00); //Auto Ant cap
	tuneFreqParam.push_back(0x00);

	auto tuneFreqResponse = sendCommand(SI468X_DAB_TUNE_FREQ, tuneFreqParam, 4, WAIT_CTS_STC, 1000);
	VLOG(1) << "Raw tuneFreqResponse: " << vectorToHexString(tuneFreqResponse);

	std::this_thread::sleep_for( std::chrono::seconds(10));

	LOG(INFO) << "Read Digirad Status";
	std::vector<uint8_t> params;
	params.push_back((1 << 3) | 1); // set digrad_ack and stc_ack;);
	auto digiRadStatus = sendCommand(SI468X_DAB_DIGRAD_STATUS, params, 40, WAIT_CTS);
	VLOG(1) << "Raw DigiRadStatus: " << vectorToHexString(digiRadStatus);
	DabDigiradStatus radStatus(digiRadStatus);
	LOG(INFO) << radStatus.toString();
}

void DABReceiver::getFrequencyList()
{
	LOG(INFO) << "Get Frequency list";
	auto freqListResponse = sendCommand(SI468X_DAB_GET_FREQ_LIST, std::vector<uint8_t> ({0x00}), 5, WAIT_CTS);
	FrequencyList freqList(freqListResponse);
	LOG(INFO) << "List: " << freqList.toString();

	freqListResponse = sendCommand(SI468X_DAB_GET_FREQ_LIST, std::vector<uint8_t> ({0x00}), 8 + (freqList.NUM_FREQS * 4), WAIT_CTS);
	VLOG(1) << "Raw list Freq: " << vectorToHexString(freqListResponse);
	FrequencyList fullFreqList(freqListResponse);
	LOG(INFO) << "Full List: " << fullFreqList.toString();
}

bool DABReceiver::powerOn()
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

bool DABReceiver::powerOff()
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

bool DABReceiver::internalPowerOn()
{
	LOG(INFO) << "Internal PowerOn";
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);

	startReadThread();
	return true;
}

bool DABReceiver::internalPowerOff()
{
	LOG(INFO) << "Internal PowerOff";
	stopReadThread();
    std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);
	switch (mPowerState)
	{
		case DABPowerState::PowerOff : return true;
		case DABPowerState::PowerOn :
			// if (!waitForCTS())
			// {
			// 	return false;
			// }; // No break is intended
		default:
			// std::vector<uint8_t> powerdownResponse(1);
			// mI2C.readWriteData(mAddress, std::vector<uint8_t>({POWER_DOWN}), powerdownResponse);
			// LOG(INFO) << "POWER_DOWN Status: " << std::hex << "0x" << (int) powerdownResponse[0];

		break;
	}
	return true;
}

bool DABReceiver::setProperty(uint16_t property, uint16_t value)
{
	std::vector<uint8_t> setPropertyParams;
	setPropertyParams.push_back(0x00);
	setPropertyParams.push_back(property & 0xFF);
	setPropertyParams.push_back((property >> 8) & 0xFF);
	setPropertyParams.push_back(value & 0xFF);
	setPropertyParams.push_back((value >> 8) & 0xFF);

	sendCommand(SI468X_SET_PROPERTY, setPropertyParams, 0, WAIT_CTS);
	return true;
}

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, uint8_t resultLength, uint8_t waitMask, int timeForResponseMilliseconds)
{
    return sendCommand(command, std::vector<uint8_t>({}), resultLength, waitMask, timeForResponseMilliseconds);
}

std::vector<uint8_t> DABReceiver::sendCommand(uint8_t command, const std::vector<uint8_t>& param, uint8_t resultLength, uint8_t waitMask, int timeForResponseMilliseconds)
{
	VLOG(10) << "Sending command: " << (int) command << ", waitmask: 0x" << std::hex << (int) waitMask;

	std::vector<uint8_t> fullCmd;
	fullCmd.push_back(command);
	fullCmd.insert(fullCmd.end(), param.begin(), param.end());

	//Write the command first
    mI2C.writeData(mAddress, fullCmd);

	if (timeForResponseMilliseconds > 0)
	{
		VLOG(1) << "Sleep: " <<  timeForResponseMilliseconds << "ms" << " for command: " << (int) command;
		std::this_thread::sleep_for( std::chrono::milliseconds(timeForResponseMilliseconds));
	}

	std::vector<uint8_t> rawStatus(4);
	int retries = 0;
	while ((rawStatus[0] & waitMask) != waitMask && (retries < MAX_RETRIES))
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(10));

		mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), rawStatus);
		Status status(rawStatus);
		if (status.error())
		{
			LOG(ERROR) << "Command: " << commandToString(command) << ", Error status returned: " << status.toString();
			std::vector<uint8_t> rawError(6);
			mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), rawError);			
			Status error(rawError);
			LOG(ERROR) << "Full error: " << error.toString();
			return rawStatus;
		}
		VLOG(10)  << "Waiting for waitmask: 0x" << std::hex << (int) waitMask << ", current status: " << std::hex << (int) (rawStatus[0] & waitMask) << ", retry count: " << std::dec << retries;
		++retries;
	}

	if (retries < MAX_RETRIES)
	{
		if (resultLength > 0)
		{
			//Need to read the full command result
			std::vector<uint8_t> cmdResult(resultLength);
			mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), cmdResult);
			return cmdResult;
		}
		else
		{
			return rawStatus;
		}		
	}
	else
	{
		LOG(WARNING) << "Timeout for Command: " << (int) command;
	}

	return std::vector<uint8_t>();


	// VLOG(10) << "Sending command: " << (int) command << ", waitmask: 0x" << std::hex << (int) waitMask;

	// std::vector<uint8_t> cmdResult((resultLength > 0) ? resultLength : 1);
	// std::vector<uint8_t> fullCmd;
	// fullCmd.push_back(command);
	// fullCmd.insert(fullCmd.end(), param.begin(), param.end());

	// bool timeout = false;

	// //Write the command first, and read only the first status byte (containing CTS)
    // mI2C.readWriteData(mAddress, fullCmd, cmdResult);

	// if ((cmdResult[0] & waitMask) != waitMask)
	// {
	// 	int retries = 0;
	// 	while ((cmdResult[0] & waitMask) != waitMask && (retries < MAX_RETRIES))
	// 	{
	// 		int sleepTimeMilliseconds = (retries == 0 && timeForResponseMilliseconds > 0) ? timeForResponseMilliseconds : 10;
	// 		VLOG(10) << "Sleeping for " << sleepTimeMilliseconds << " Ms";
	// 		std::this_thread::sleep_for( std::chrono::milliseconds(sleepTimeMilliseconds));

	// 		mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), cmdResult);
	// 		VLOG(10)  << "Waiting for waitmask: 0x" << std::hex << (int) waitMask << ", current status: " << std::hex << (int) (cmdResult[0] & waitMask) << ", retry count: " << std::dec << retries;
	// 		++retries;
	// 	}
	// 	timeout = retries >= MAX_RETRIES;
	// }
	// if (!timeout)
	// {
	// 	return cmdResult;
	// }
	// else
	// {
	// 	LOG(WARNING) << "Timeout for Command: " << (int) command;
	// }

	// return std::vector<uint8_t>();

}
std::string DABReceiver::commandToString(uint8_t command)
{
    switch(command)
    {
        case SI468X_RD_REPLY: return "RD_REPLY";
        case SI468X_POWER_UP: return "POWER_UP";
        case SI468X_HOST_LOAD: return "HOST_LOAD";
        case SI468X_LOAD_INIT: return "LOAD_INIT";
        case SI468X_BOOT: return "BOOT";
        case SI468X_GET_PART_INFO: return "GET_PART_INFO";
        case SI468X_GET_SYS_STATE: return "GET_SYS_STATE";
        case SI468X_SET_PROPERTY: return "SET_PROPERTY";
        case SI468X_GET_PROPERTY: return "GET_PROPERTY";
        case SI468X_DAB_TUNE_FREQ: return "DAB_TUNE_FREQ";
        case SI468X_DAB_DIGRAD_STATUS: return "DAB_DIGRAD_STATUS";
        case SI468X_DAB_GET_FREQ_LIST: return "DAB_GET_FREQ_LIST";
        default: return "Unknown";
    }
}
void DABReceiver::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = std::unique_ptr<std::thread>(new std::thread(&DABReceiver::readThread, this));
}

void DABReceiver::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();
        mReadThread.reset();
    }
}
void DABReceiver::readThread()
{
	pthread_setname_np(pthread_self(), "DAB Receiver");

    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        std::lock_guard<std::recursive_mutex> lk_guard(mReceiverMutex);
    }
}

void DABReceiver::notifyObservers()
{
	std::lock_guard<std::recursive_mutex> lk_guard(mRadioObserversMutex);
    for (auto observer : mRadioObservers)
    {
       // observer->radioRdsUpdate(mRDSInfo);
    }
}
// std::string commandToString(uint8_t command)
// {
//     switch(command)
//     {
//         case SI468X_RD_REPLY: return "RD_REPLY";
//         case SI468X_POWER_UP: return "POWER_UP";
//         case SI468X_HOST_LOAD: return "HOST_LOAD";
//         case SI468X_LOAD_INIT: return "LOAD_INIT";
//         case SI468X_BOOT: return "BOOT";
//         case SI468X_GET_PART_INFO: return "GET_PART_INFO";
//         case SI468X_GET_SYS_STATE: return "GET_SYS_STATE";
//         case SI468X_SET_PROPERTY: return "SET_PROPERTY";
//         case SI468X_GET_PROPERTY: return "GET_PROPERTY";
//         case SI468X_DAB_TUNE_FREQ: return "DAB_TUNE_FREQ";
//         case SI468X_DAB_DIGRAD_STATUS: return "DAB_DIGRAD_STATUS";
//         case SI468X_DAB_GET_FREQ_LIST: return "DAB_GET_FREQ_LIST";
//         default: return "Unknown";
//     }
// }
}
