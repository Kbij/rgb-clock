/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684
*/

#include "Si4684.h"
#include "Si4684Const.h"
#include "cmd/DABCommands.h"
#include "lib/MainboardControl.h"
#include "lib/Utils.h"
#include <thread>
#include <glog/logging.h>

namespace 
{
const int FIRMWARE_BLOCK_SIZE = 4092;
const int MAX_RETRIES = 500;
const uint8_t WAIT_CTS = 0x80;
const uint8_t WAIT_CTS_STC = 0x81;
}


namespace Hardware
{
Si4684::Si4684(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl):
	mI2C(i2c),
	mAddress(address),
    mMainboardControl(mainboardControl)
{
	mI2C.registerAddress(address, "Si4684");	
}

Si4684::~Si4684()
{
}

bool Si4684::reset()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for( std::chrono::milliseconds(10));

    return mI2C.probeAddress(mAddress);
}

bool Si4684::init(const Si4684Settings& settings)
{
    LOG(INFO) << "Init Si4684";

    if (settings.BootFile.empty())
    {
        LOG(ERROR) << "Bootfile not specified !";
        return false;
    }
    if (settings.DABFile.empty())
    {
        LOG(ERROR) << "DABfile not specified !";
        return false;
    }    

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

	VLOG(3) << "Writing POWER_UP";
    sendCommand(SI468X_POWER_UP, powerUpParams, 0, WAIT_CTS, 10);
	std::this_thread::sleep_for( std::chrono::milliseconds(10));

    LOG(INFO) << "Loading Bootfile: " << settings.BootFile;
	if (!hostload(settings.BootFile))
    {
        LOG(ERROR) << "Error loading Boot";
        return false;
    }

    LOG(INFO) << "Loading DAB file: " << settings.DABFile;
	if(!hostload(settings.DABFile))
    {
        LOG(ERROR) << "Error laoding DAB file";
        return false;
    }

	std::this_thread::sleep_for( std::chrono::seconds(1));

	VLOG(1) << "Writing BOOT";
    DABStatus bootStatus(sendCommand(SI468X_BOOT, std::vector<uint8_t>({0x00}), 0, WAIT_CTS, 500));
    if (bootStatus.error())
    {
        LOG(ERROR) << "Error booting";
        return false;
    }
	
    auto sysState = readSysState();
    VLOG(1) << "System state after Init: " << sysState.toString();

	DABPartInfo partInfo(sendCommand(SI468X_GET_PART_INFO, std::vector<uint8_t> ({0x00}), 23, WAIT_CTS, 1));
	LOG(INFO) << "Si4684 Radio partinfo:" << partInfo.toString();

	//Set output selection of the mainboard
	if (mMainboardControl) mMainboardControl->selectInput(InputSelection::RadioIn);

	//DSRVOVFLINT and DSRVPCKTINT
	setProperty(SI468X_DIGITAL_SERVICE_INT_SOURCE, 0x03);

    // If DAB is active
    return sysState.IMAGE == 2;
}

DABStatus Si4684::getStatus()
{
	DABStatus status(sendCommand(SI468X_RD_REPLY, std::vector<uint8_t> ({0x00}), 4, WAIT_CTS));
	return status;
}

DABFrequencyList Si4684::getFrequencyList()
{
	LOG(INFO) << "Get Frequency list";
    //Get the size first
	DABFrequencyList freqList(sendCommand(SI468X_DAB_GET_FREQ_LIST, std::vector<uint8_t> ({0x00}), 5, WAIT_CTS));

    //Get the full list
	DABFrequencyList fullFreqList(sendCommand(SI468X_RD_REPLY, std::vector<uint8_t> ({0x00}), 8 + (freqList.NUM_FREQS * 4), WAIT_CTS));
    return fullFreqList;
}

DABDigiradStatus Si4684::tuneFrequencyIndex(uint8_t index)
{
	VLOG(1) << "Tune to frequency index: " << (int) index;

	std::vector<uint8_t> tuneFreqParam;
	tuneFreqParam.push_back(0x00); //Automatic Injection
	tuneFreqParam.push_back(index);
	tuneFreqParam.push_back(0x00);

	tuneFreqParam.push_back(0x00); //Auto Ant cap
	tuneFreqParam.push_back(0x00);

	DABStatus tuneFreqStatus(sendCommand(SI468X_DAB_TUNE_FREQ, tuneFreqParam, 4, WAIT_CTS_STC, 1000));
    if (tuneFreqStatus.error())
    {
        LOG(ERROR) << "Error tuning to frequency: " << tuneFreqStatus.toString();
        return DABDigiradStatus(std::vector<uint8_t>());
    }

	std::this_thread::sleep_for( std::chrono::seconds(2));

	VLOG(1) << "Read Digirad Status";
	std::vector<uint8_t> params;
	params.push_back((1 << 3) | 1); // set digrad_ack and stc_ack;);
	DABDigiradStatus radStatus(sendCommand(SI468X_DAB_DIGRAD_STATUS, params, 40, WAIT_CTS));
	VLOG(1) << radStatus.toString();
    return radStatus;
}

DABServiceList Si4684::getServices()
{
	VLOG(1) << "Get Service list";
	//Get the size first
	DABServiceList serviceSize(sendCommand(SI468X_GET_DIGITAL_SERVICE_LIST, std::vector<uint8_t> ({0x00}), 6, WAIT_CTS));

    //Get the ful size
	DABServiceList services(sendCommand(SI468X_RD_REPLY, std::vector<uint8_t> ({0x00}), serviceSize.SIZE+6, WAIT_CTS));
	VLOG(1) << "Services: " << services.toString();
    return services;
}

bool Si4684::startService(uint32_t serviceId, uint32_t componentId)
{
	VLOG(1) << "Start Service: " << (int) serviceId << ", Component: " << (int) componentId;

	std::vector<uint8_t> params;
	params.push_back(0x00); //Audio service
	params.push_back(0x00);
	params.push_back(0x00);
	params.push_back(serviceId & 0xFF);
	params.push_back((serviceId >> 8) & 0xFF);
	params.push_back((serviceId >> 16) & 0xFF);
	params.push_back((serviceId >> 24) & 0xFF);
	params.push_back(componentId & 0xFF);
	params.push_back((componentId >> 8) & 0xFF);
	params.push_back((componentId >> 16) & 0xFF);
	params.push_back((componentId >> 24) & 0xFF);

	DABStatus status(sendCommand(SI468X_START_DIGITAL_SERVICE, params, 4, WAIT_CTS));
	VLOG(1) << "Start service result: " << status.toString();

	return !status.error();
}

DigitalServiceData Si4684::getServiceData()
{
	VLOG(1) << "Get Service Data";
	DigitalServiceData serviceDataSize(sendCommand(SI468X_GET_DIGITAL_SERVICE_DATA, std::vector<uint8_t> ({0x01}), 20, WAIT_CTS));


	DigitalServiceData serviceData(sendCommand(SI468X_RD_REPLY, std::vector<uint8_t> (), serviceDataSize.BYTE_COUNT + 24, WAIT_CTS));
	return serviceData;
}

void Si4684::getEnsembleInfo()
{
	VLOG(1) << "Get Ensemble Info";
	auto result = sendCommand(SI468X_DAB_GET_ENSEMBLE_INFO, std::vector<uint8_t> ({0x00}), 32, WAIT_CTS);
	LOG(INFO) << "info: " << std::endl << vectorToHexString(result, true, true);
}

void Si4684::getServiceInfo()
{
	VLOG(1) << "Get Service Info";
	auto result = sendCommand(SI468X_DAB_GET_SERVICE_INFO, std::vector<uint8_t> ({0x00}), 32, WAIT_CTS);
	LOG(INFO) << "info: " << std::endl << vectorToHexString(result, true, true);
}

bool Si4684::hostload(const std::string& fileName)
{
	std::vector<uint8_t> firmware;
	if (readFile(fileName,  firmware))
	{
		VLOG(1) << "Firmware size: " << firmware.size();

		DABStatus loadInitStatus(sendCommand(SI468X_LOAD_INIT, std::vector<uint8_t> ({0x00}), 0, WAIT_CTS, 1));
		if (loadInitStatus.error())
		{
			LOG(ERROR) << "Load init resulted in a error: " << loadInitStatus.toString();
            return false;
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

			DABStatus hostLoadStatus(sendCommand(SI468X_HOST_LOAD, hostLoadParams, 0, WAIT_CTS));
			if (hostLoadStatus.error())
			{
				LOG(ERROR) << "Host load resulted in a error: " << hostLoadStatus.toString();
                return false;
			}

			bytesSend += firmwareBlock.size();

			std::advance(filePos , amountToSend);

			std::this_thread::sleep_for( std::chrono::milliseconds(5));
		}
		VLOG(1) << "Total bytes send: " << bytesSend;
		
		std::this_thread::sleep_for( std::chrono::milliseconds(50));
        return true;	
	}
    else
    {
        LOG(ERROR) << "Error reading file: " << fileName;
        return false;
    }
    
}

DABSysState Si4684::readSysState()
{
	DABSysState sysState(sendCommand(SI468X_GET_SYS_STATE, std::vector<uint8_t> ({0x00}), 6, WAIT_CTS, 5));
    return sysState;
}

std::vector<uint8_t> Si4684::sendCommand(uint8_t command, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds)
{
    return sendCommand(command, std::vector<uint8_t>({}), resultLength, waitMask, timeForResponseMilliseconds);
}

std::vector<uint8_t> Si4684::sendCommand(uint8_t command, const std::vector<uint8_t>& param, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds)
{
	VLOG(10) << "Sending command: " << (int) command << ", waitmask: 0x" << std::hex << (int) waitMask;

	std::vector<uint8_t> fullCmd;
	fullCmd.push_back(command);
	fullCmd.insert(fullCmd.end(), param.begin(), param.end());

	//Write the command first
    mI2C.writeData(mAddress, fullCmd);

	if (timeForResponseMilliseconds > 0)
	{
		VLOG(1) << "Sleep: " <<  timeForResponseMilliseconds << "ms" << " for command: " << commandToString(command);
		std::this_thread::sleep_for( std::chrono::milliseconds(timeForResponseMilliseconds));
	}

	std::vector<uint8_t> rawStatus(4);
	int retries = 0;
	while ((rawStatus[0] & waitMask) != waitMask && (retries < MAX_RETRIES))
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(10));

		mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), rawStatus);
		DABStatus status(rawStatus);
		if (status.error())
		{
			LOG(ERROR) << "Command: " << commandToString(command) << ", Error status returned: " << status.toString();
			std::vector<uint8_t> rawError(6);
			mI2C.readWriteData(mAddress, std::vector<uint8_t> ({SI468X_RD_REPLY}), rawError);			
			DABStatus error(rawError);
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
		LOG(WARNING) << "Timeout for Command: " << commandToString(command);
	}

	return std::vector<uint8_t>();
}

std::string Si4684::commandToString(uint8_t command)
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
        default: return "Unknown (" + std::to_string(command) + ")";
    }
}

bool Si4684::setProperty(uint16_t property, uint16_t value)
{
	std::vector<uint8_t> setPropertyParams;
	setPropertyParams.push_back(0x00);
	setPropertyParams.push_back(property & 0xFF);
	setPropertyParams.push_back((property >> 8) & 0xFF);
	setPropertyParams.push_back(value & 0xFF);
	setPropertyParams.push_back((value >> 8) & 0xFF);

	DABStatus status(sendCommand(SI468X_SET_PROPERTY, setPropertyParams, 0, WAIT_CTS));
	return !status.error();
}
}