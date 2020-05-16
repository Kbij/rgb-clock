/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684
*/

#ifndef SI4684_H_
#define SI4684_H_

#include "I2C.h"
#include "lib/cmd/DABCommands.h"
#include <thread>
#include <set>
#include <stdint.h>

namespace Hardware
{
class MainboardControl;

struct Si4684Settings
{
    Si4684Settings(): BootFile(), DABFile() {};
    std::string BootFile;
    std::string DABFile;
};

class Si4684
{
public:    
	Si4684(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl);
	virtual ~Si4684();

    bool reset();
	bool init(const Si4684Settings& settings);

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
	bool hostload(const std::string& fileName);
    DABSysState readSysState();

    std::vector<uint8_t> sendCommand(uint8_t command, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
    std::vector<uint8_t> sendCommand(uint8_t command, const std::vector<uint8_t>& param, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
	std::string commandToString(uint8_t command);
	bool setProperty(uint16_t property, uint16_t value);

	I2C &mI2C;
	const uint8_t mAddress;
    Hardware::MainboardControl* mMainboardControl;
};
}

#endif /* !SI4684_H_ */
