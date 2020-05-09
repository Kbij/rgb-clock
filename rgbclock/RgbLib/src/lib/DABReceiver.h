/*
** RgbClock
** File description:
** DABReceiver
*/

#ifndef DABRECEIVER_H_
#define DABRECEIVER_H_

#include "I2C.h"
#include "RadioObserverIf.h"
#include "RDSInfo.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <set>
#include <memory>
#include <vector>


namespace Hardware
{
class Radio;
class MainboardControl;

enum class DABPowerState
{
	Unknown,
	PowerOn,
	PowerOff,

};

class DABReceiver
{
public:
	DABReceiver(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl);
	virtual ~DABReceiver();
	friend Radio;
    
	//Move init, temp to public
	void init();	
	void tuneFrequencyIndex(uint8_t index);
	void getFrequencyList();
	void getServiceList();
	void startService(uint32_t serviceId, uint32_t componentId);
    bool powerOn();

private:
	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

//    void init();
	void readSysState();
//	void readStatus();
	void readPartInfo();
	void hostload(const std::string& fileName);
	//void tuneFrequencyIndex(uint8_t index);

	bool powerOff();

	bool internalPowerOn();
	bool internalPowerOff();

	bool setProperty(uint16_t property, uint16_t value);
	// bool getProperty(int property, int& value);
//	bool waitForCTS();
    std::vector<uint8_t> sendCommand(uint8_t command, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
    std::vector<uint8_t> sendCommand(uint8_t command, const std::vector<uint8_t>& param, int resultLength, uint8_t waitMask, int timeForResponseMilliseconds = 0);
	std::string commandToString(uint8_t command);
	
	void startReadThread();
	void stopReadThread();
	void readThread();

	void notifyObservers();

	I2C &mI2C;
	const uint8_t mAddress;
    Hardware::MainboardControl* mMainboardControl;
	int mPowerCounter;
	std::mutex mPowerMutex;
	DABPowerState mPowerState;
    std::recursive_mutex mReceiverMutex;
    std::unique_ptr<std::thread> mReadThread;
    std::atomic_bool mReadThreadRunning;
    std::set<RadioObserverIf*> mRadioObservers;
    std::recursive_mutex mRadioObserversMutex;
};
}
#endif /* !DABRECEIVER_H_ */
