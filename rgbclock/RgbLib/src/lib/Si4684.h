/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684
*/

#ifndef SI4684_H_
#define SI4684_H_

#include "SPI.h"
#include "lib/cmd/DABCommands.h"
#include <thread>
#include <set>
#include <stdint.h>


namespace Hardware
{
class MainboardControl;

struct Si4684Settings
{
    Si4684Settings(): MiniPatch(), BootFile(), DABFile(), LoadFromFlash(false) {};
	std::string MiniPatch;
    std::string BootFile;
    std::string DABFile;
	bool LoadFromFlash;
};

class Si4684
{
public:    
	Si4684(SPI &spi, Hardware::MainboardControl* mainboardControl);
	virtual ~Si4684();

    bool reset();
	bool init(const Si4684Settings& settings);
	bool writeFlash(const Si4684Settings& settings);
	bool writeFlash2(const Si4684Settings& settings);

	DABStatus getStatus();
   	DABFrequencyList getFrequencyList();
	DABDigiradStatus tuneFrequencyIndex(uint8_t index);
    DABServiceList getServices();
	bool startService(uint32_t serviceId, uint32_t componentId);
	bool stopService(uint32_t serviceId, uint32_t componentId);
	DABComponentInfo getComponentInfo(uint32_t serviceId, uint32_t componentId);
	DigitalServiceData getServiceData();
	void getEnsembleInfo();
	void getServiceInfo();
	DABRssiInfo getRssi();

	
private:
	bool powerUp();
	bool hostLoad(const std::string& fileName);
	bool flashLoad(uint32_t address);
	bool flashLoad(uint32_t address, uint32_t crc, uint32_t size);
	bool writeFlashImage(const std::string& fileName, uint32_t address);
    DABSysState readSysState();
	void readFlashProperties();

    std::vector<uint8_t> sendCommand(uint8_t command, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
    std::vector<uint8_t> sendCommand(uint8_t command, const std::vector<uint8_t>& param, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
	std::string commandToString(uint8_t command);
	bool setProperty(uint16_t property, uint16_t value);
	bool flashSetProperty(uint16_t property, uint16_t value);
	DABReadProperty flashGetProperty(uint16_t property);

	SPI &mSPI;
    Hardware::MainboardControl* mMainboardControl;
};
}

#endif /* !SI4684_H_ */
