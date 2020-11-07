/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684
*/

#include "Si4684.h"
#include "Si4684Const.h"
#include "Crc32.hpp"
#include "cmd/DABCommands.h"
#include "lib/MainboardControl.h"
#include "lib/Utils.h"
#include <thread>
#include <glog/logging.h>

namespace 
{
const int FIRMWARE_BLOCK_SIZE = 4084;
const int MAX_RETRIES = 500;
const uint8_t WAIT_CTS = 0x80;
const uint8_t WAIT_CTS_STC = 0x81;
const uint32_t BOOT_IMAGE_ADDRESS = 0x00002000;
const uint32_t DAB_IMDAGE_ADDRESS = 0x00006000;
}


namespace Hardware
{
Si4684::Si4684(SPI &spi, Hardware::MainboardControl* mainboardControl):
	mSPI(spi),
    mMainboardControl(mainboardControl)
{
}

Si4684::~Si4684()
{
}

bool Si4684::reset()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for( std::chrono::milliseconds(10));

    return true;
}

bool Si4684::init(const Si4684Settings& settings)
{
    LOG(INFO) << "Init Si4684";

	if (!settings.LoadFromFlash)
	{
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
	}    

    //See Page 154 in the manual
    std::vector<uint8_t> powerUpParams;
	powerUpParams.push_back(0x00); // ARG1 CTSIEN is disabled
	powerUpParams.push_back((1 << 4) | (7 << 0)); // ARG2 CLK_MODE=0x1 TR_SIZE=0x7 (19.2Mhz X-Tal)

    //See page 438 Programming Manual)
    //ESR = 70 Ohm
    powerUpParams.push_back(0x28); // ARG3 IBIAS=0x48 (Sdk: 0x28)

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
	powerUpParams.push_back(0x12); // ARG13 IBIAS_RUN (Ik: 0x12)
	powerUpParams.push_back(0x00); // ARG14
	powerUpParams.push_back(0x00); // ARG15    


	VLOG(3) << "Writing POWER_UP";
    DABStatus powerUpStatus(sendCommand(SI468X_POWER_UP, powerUpParams, 0, WAIT_CTS, 100));
	if (powerUpStatus.error())
	{
		LOG(ERROR) << "Si4684 Powerup failed";
		return false;
	}

//	return true;

	if (settings.LoadFromFlash)
	{
		LOG(INFO) << "Loading Mini Patch: " << settings.MiniPatch;
		if (!hostLoad(settings.MiniPatch))
		{
			LOG(ERROR) << "Error loading Mini Patch";
			return false;
		}		

		// auto propValue = flashGetProperty(PROP_WRITE_ERASE_SECTOR_CMD);
		// LOG(INFO) << "PROP_WRITE_ERASE_SECTOR_CMD: " << propValue.toString();

	//	return true;

	//	flashSetProperty(PROP_FLASH_SPI_CLOCK_FREQ_KHZ, 100); 
	//	flashSetProperty(PROP_HIGH_SPEED_READ_MAX_FREQ_MHZ, 0x0001); // Set flash high speed read speed to 127MHz	

		//Recommended wait = 4ms
		std::this_thread::sleep_for( std::chrono::milliseconds(10));

		LOG(INFO) << "Loading Boot from Flash";
		if (!flashLoad(BOOT_IMAGE_ADDRESS, 0xE523ED8B, 5796))
		{
			LOG(ERROR) << "Error loading Boot Image";
			return false;
		}

		// auto propValue = flashGetProperty(PROP_HIGH_SPEED_READ_MAX_FREQ_MHZ);
		// LOG(INFO) << "PROP_HIGH_SPEED_READ_MAX_FREQ_MHZ: " << propValue.toString();		

		LOG(INFO) << "Loading DAB from Flash";
		if (!flashLoad(DAB_IMDAGE_ADDRESS, 0x9E6FCCDB, 499356))
		{
			LOG(ERROR) << "Error laoding DAB Image";
			return false;
		}
	}
	else
	{
		LOG(INFO) << "Loading Bootfile: " << settings.BootFile;
		if (!hostLoad(settings.BootFile))
		{
			LOG(ERROR) << "Error loading Boot";
			return false;
		}

		LOG(INFO) << "Loading DAB file: " << settings.DABFile;
		if(!hostLoad(settings.DABFile))
		{
			LOG(ERROR) << "Error laoding DAB file";
			return false;
		}
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

bool Si4684::writeFlash(const Si4684Settings& settings)
{
    LOG(INFO) << "Init Si4684";

    if (settings.MiniPatch.empty())
    {
        LOG(ERROR) << "MiniPatch not specified !";
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
    DABStatus powerUpStatus(sendCommand(SI468X_POWER_UP, powerUpParams, 0, WAIT_CTS, 10));
	std::this_thread::sleep_for( std::chrono::milliseconds(10));
	if (powerUpStatus.error())
	{
		LOG(ERROR) << "Si4684 Powerup failed";
		return false;
	}

    LOG(INFO) << "Loading Mini Patch: " << settings.MiniPatch;
	if (!hostLoad(settings.MiniPatch))
    {
        LOG(ERROR) << "Error loading Mini Patch";
        return false;
    }

	if (settings.BootFile.empty())
	{
		LOG(ERROR) << "Bootfile is empty, unable to flash";
		return false;
	}

	if (settings.DABFile.empty())
	{
		LOG(ERROR) << "DABFile is empty, unable to flash";
		return false;
	}	

    auto sysState2 = readSysState();
    VLOG(1) << "System state after Init: " << sysState2.toString();

	VLOG(1) << "Erase Flash";
    std::vector<uint8_t> flashEraseParams;
	flashEraseParams.push_back(0xFF); // Full chip
	flashEraseParams.push_back(0xDE);
	flashEraseParams.push_back(0xC0);

    DABStatus flashEraseStatus(sendCommand(SI468X_FLASH_LOAD, flashEraseParams, 0, WAIT_CTS, 10));	
	if (flashEraseStatus.error())
	{
		LOG(ERROR) << "Error erasing Flash: " << flashEraseStatus.toString();
		return false;
	}
	
	VLOG(1) << "Write bootfile " << settings.BootFile << " to flash.";
	if (!writeFlashImage(settings.BootFile, BOOT_IMAGE_ADDRESS))
	{
		LOG(ERROR) << "Error writing Boot to flash";
		return false;
	}

	VLOG(1) << "Write DAB image " << settings.BootFile << " to flash.";
	if (!writeFlashImage(settings.DABFile, DAB_IMDAGE_ADDRESS))
	{
		LOG(ERROR) << "Error writing Boot to flash";
		return false;
	}	
}

bool Si4684::writeFlash2(const Si4684Settings& settings)
{
	// VLOG(1) << "Write Flash properties";
	// //flashSetProperty(PROP_FLASH_SPI_CLOCK_FREQ_KHZ, 0x9C40); // Set flash speed to 40MHz
	// //flashSetProperty(PROP_HIGH_SPEED_READ_MAX_FREQ_MHZ, 0x00FF); // Set flash high speed read speed to 127MHz	

	// VLOG(1) << "Erase Flash";
    // std::vector<uint8_t> flashEraseParams;
	// flashEraseParams.push_back(0xFF); // Full chip
	// flashEraseParams.push_back(0xDE);
	// flashEraseParams.push_back(0xC0);

    // DABStatus flashEraseStatus(sendCommand(SI468X_FLASH_LOAD, flashEraseParams, 0, WAIT_CTS, 1000));	

	

	return true;
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

bool Si4684::stopService(uint32_t serviceId, uint32_t componentId)
{
	VLOG(1) << "Stop Service: " << (int) serviceId << ", Component: " << (int) componentId;

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

	DABStatus status(sendCommand(SI468X_STOP_DIGITAL_SERVICE, params, 4, WAIT_CTS));
	VLOG(1) << "Stop service result: " << status.toString();

	return !status.error();
}
DABComponentInfo Si4684::getComponentInfo(uint32_t serviceId, uint32_t componentId)
{
	VLOG(1) << "Get Component Info: " << (int) serviceId << ", Component: " << (int) componentId;

	std::vector<uint8_t> params;
	params.push_back(0x00);
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

	DABComponentInfo info(sendCommand(SI468X_DAB_GET_COMPONENT_INFO, params, 37, WAIT_CTS));
	VLOG(1) << "Start service result: " << info.toString();

	return info;
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

DABRssiInfo Si4684::getRssi()
{
	VLOG(1) << "Get Rssi Info";
	auto result = DABRssiInfo(sendCommand(SI468X_TEST_GET_RSSI, std::vector<uint8_t> ({0x00}), 6, WAIT_CTS));
	
	return result;
}

bool Si4684::hostLoad(const std::string& fileName)
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
			VLOG(40) << std::endl << vectorToHexString(hostLoadParams, false, true);

			DABStatus hostLoadStatus(sendCommand(SI468X_HOST_LOAD, hostLoadParams, 0, WAIT_CTS, 0));
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

bool Si4684::flashLoad(uint32_t address)
{
	VLOG(1) << "Flash LOAD_INIT";
	DABStatus loadInitStatus(sendCommand(SI468X_LOAD_INIT, std::vector<uint8_t> ({0x00}), 0, WAIT_CTS, 1));
	if (loadInitStatus.error())
	{
		LOG(ERROR) << "Load init resulted in a error: " << loadInitStatus.toString();
		return false;
	}
	std::this_thread::sleep_for( std::chrono::milliseconds(50));

	LOG(INFO) << "Flash FLASH_LOAD IMG, address: 0x0" <<  std::hex << address;
	std::vector<uint8_t> params;
	params.push_back(0x00); //CMD
	params.push_back(0x00); //PAD0
	params.push_back(0x00); //PAD1
	params.push_back(address & 0xFF);
	params.push_back((address >> 8) & 0xFF);
	params.push_back((address >> 16) & 0xFF);
	params.push_back((address >> 24) & 0xFF);
	params.push_back(0x00);
	params.push_back(0x00);
	params.push_back(0x00);
	params.push_back(0x00);

	//std::this_thread::sleep_for( std::chrono::milliseconds(3000));

	DABStatus status(sendCommand(SI468X_FLASH_LOAD, params, 4, WAIT_CTS));
	LOG(INFO) << "Flash load status: " << status.toString();

	return !status.error();
}

bool Si4684::flashLoad(uint32_t address, uint32_t crc, uint32_t size)
{
	VLOG(1) << "Flash LOAD_INIT";
	DABStatus loadInitStatus(sendCommand(SI468X_LOAD_INIT, std::vector<uint8_t> ({0x00}), 0, WAIT_CTS, 1));
	if (loadInitStatus.error())
	{
		LOG(ERROR) << "Load init resulted in a error: " << loadInitStatus.toString();
		return false;
	}
	std::this_thread::sleep_for( std::chrono::milliseconds(50));

	LOG(INFO) << "Flash FLASH_LOAD IMG (Witch CRC Check), address: 0x0" <<  std::hex << address << ", crc: 0x" <<  std::uppercase << std::hex << crc << ", size: " << std::dec << size;
	std::vector<uint8_t> params;
	params.push_back(0x01); //CMD (0x01: With CRC Check)
	params.push_back(0x00); //PAD0
	params.push_back(0x00); //PAD1

	params.push_back(crc & 0xFF);
	params.push_back((crc >> 8) & 0xFF);
	params.push_back((crc >> 16) & 0xFF);
	params.push_back((crc >> 24) & 0xFF);

	params.push_back(address & 0xFF);
	params.push_back((address >> 8) & 0xFF);
	params.push_back((address >> 16) & 0xFF);
	params.push_back((address >> 24) & 0xFF);

	params.push_back(size & 0xFF);
	params.push_back((size >> 8) & 0xFF);
	params.push_back((size >> 16) & 0xFF);
	params.push_back((size >> 24) & 0xFF);

	//std::this_thread::sleep_for( std::chrono::milliseconds(3000));

	DABStatus status(sendCommand(SI468X_FLASH_LOAD, params, 4, WAIT_CTS));
	if (status.error())
	{
		LOG(ERROR) << "Flash load status: " << status.toString();
	}
	return !status.error();
}

bool Si4684::writeFlashImage(const std::string& fileName, uint32_t address)
{
	LOG(INFO) << "Writing file: " << fileName << " to flash (with CRC check). Address: 0x" << std::hex << address;
	std::vector<uint8_t> firmware;
	uint32_t currentAddress = address;
	if (readFile(fileName,  firmware))
	{
		LOG(INFO) << "Firmware size: " << firmware.size();
		auto fileCrc = crc32<IEEE8023_CRC32_POLYNOMIAL>(/*0xFFFFFFFF*/0, firmware.begin(), firmware.end());
		LOG(INFO) << "File CRC: 0x" << std::uppercase << std::hex << fileCrc;

		int bytesSend = 0;
		auto filePos = firmware.begin();
		while(filePos != firmware.end())
		{
			int amountToSend = (firmware.end() - filePos) >= FIRMWARE_BLOCK_SIZE ? FIRMWARE_BLOCK_SIZE : firmware.end() - filePos;
			std::vector<uint8_t> firmwareBlock(filePos, (filePos + amountToSend));

			auto blockCrc = crc32<IEEE8023_CRC32_POLYNOMIAL>(/*0xFFFFFFFF*/ 0, firmwareBlock.begin(), firmwareBlock.end());
			
			VLOG(3) << "CRC32: 0x" << std::hex << blockCrc;

			std::vector<uint8_t> writeFlashParams({FLASH_WRITE_BLOCK_READBACK_VERIFY, 0x0C, 0xED});
			writeFlashParams.push_back(blockCrc & 0xFF);
			writeFlashParams.push_back((blockCrc >> 8) & 0xFF);
			writeFlashParams.push_back((blockCrc >> 16) & 0xFF);
			writeFlashParams.push_back((blockCrc >> 24) & 0xFF);

			writeFlashParams.push_back(currentAddress & 0xFF);
			writeFlashParams.push_back((currentAddress >> 8) & 0xFF);
			writeFlashParams.push_back((currentAddress >> 16) & 0xFF);
			writeFlashParams.push_back((currentAddress >> 24) & 0xFF);

			uint32_t blockSize = firmwareBlock.size();
			writeFlashParams.push_back(blockSize & 0xFF);
			writeFlashParams.push_back((blockSize >> 8) & 0xFF);
			writeFlashParams.push_back((blockSize >> 16) & 0xFF);
			writeFlashParams.push_back((blockSize >> 24) & 0xFF);

			currentAddress += blockSize;
			VLOG(10) << "Current Address: 0x" << std::hex << currentAddress << ", size: " << blockSize;

			std::copy (firmwareBlock.begin(),firmwareBlock.end(),back_inserter(writeFlashParams));

			VLOG(30) << "Sending bytes: " << firmwareBlock.size() << ", already send: " << bytesSend;
			VLOG(40) << vectorToHexString(writeFlashParams, false, true);

			DABStatus flashWriteStatus(sendCommand(SI468X_FLASH_LOAD, writeFlashParams, 0, WAIT_CTS, 50));
			if (flashWriteStatus.error())
			{
				LOG(ERROR) << "Flash write resulted in a error: " << flashWriteStatus.toString();
                return false;
			}

			bytesSend += firmwareBlock.size();

			std::advance(filePos , amountToSend);
		}

		VLOG(1) << "Total bytes written: " << bytesSend;
		
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
	VLOG(10) << "Sending command: " << (int) command << ", waitmask: 0x" << std::hex << (int) waitMask << ", expected result length: " << resultLength << ", wait time: " << timeForResponseMilliseconds << "ms" ;

	std::vector<uint8_t> fullCmd;
	fullCmd.push_back(command);
	fullCmd.insert(fullCmd.end(), param.begin(), param.end());

	//Write the command first
    mSPI.writeData(fullCmd);

	if (timeForResponseMilliseconds > 0)
	{
		VLOG(1) << "Sleep: " <<  timeForResponseMilliseconds << "ms" << " for command: " << commandToString(command);
		std::this_thread::sleep_for( std::chrono::milliseconds(timeForResponseMilliseconds));
	}

	std::vector<uint8_t> rawStatus(5);
	int retries = 0;
	while ((rawStatus[0] & waitMask) != waitMask && (retries < MAX_RETRIES))
	{
		std::this_thread::sleep_for( std::chrono::milliseconds(10));

		mSPI.readWriteData(std::vector<uint8_t> ({SI468X_RD_REPLY}), rawStatus);
		DABStatus status(rawStatus);
		if (status.error())
		{
			LOG(ERROR) << "Command: " << commandToString(command) << ", Error status returned: " << status.toString();
			std::vector<uint8_t> rawError(6);
			mSPI.readWriteData(std::vector<uint8_t> ({SI468X_RD_REPLY}), rawError);			
			DABStatus error(rawError);
			LOG(ERROR) << "Full error: " << error.toString();
			return rawStatus;
		}
		VLOG(10)  << "Waiting for waitmask: 0x" << std::hex << (int) waitMask << ", current status: " << std::hex << (int) (rawStatus[0] & waitMask) << ", retry count: " << std::dec << retries;
		++retries;
	}

	if (retries < MAX_RETRIES)
	{
		if (resultLength > 5)
		{
			//Need to read the full command result
			std::vector<uint8_t> cmdResult(resultLength);
			mSPI.readWriteData(std::vector<uint8_t> ({SI468X_RD_REPLY}), cmdResult);
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
		case SI468X_FLASH_LOAD: return "FLASH_LOAD";
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

bool Si4684::flashSetProperty(uint16_t property, uint16_t value)
{
	std::vector<uint8_t> setPropertyParams;
	setPropertyParams.push_back(0x10);
	setPropertyParams.push_back(0x00);
	setPropertyParams.push_back(0x00);
	setPropertyParams.push_back(property & 0xFF);
	setPropertyParams.push_back((property >> 8) & 0xFF);
	setPropertyParams.push_back(value & 0xFF);
	setPropertyParams.push_back((value >> 8) & 0xFF);

	DABStatus status(sendCommand(SI468X_FLASH_LOAD, setPropertyParams, 0, WAIT_CTS));
	return !status.error();
}

DABReadProperty Si4684::flashGetProperty(uint16_t property)
{
	std::vector<uint8_t> getPropertyParams;
	getPropertyParams.push_back(0x11);
	getPropertyParams.push_back(property & 0xFF);
	getPropertyParams.push_back((property >> 8) & 0xFF);

	auto resultData = sendCommand(SI468X_FLASH_LOAD, getPropertyParams, 32, WAIT_CTS);
	LOG(INFO) << "Raw Data: " << std::endl << vectorToHexString(resultData, false, true);
	DABReadProperty result(resultData, property);	
	return result;
}

}
