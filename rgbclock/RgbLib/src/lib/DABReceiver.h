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
    bool powerOn();

private:
	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

    void init();
	void readSysState();
	void readStatus();
	void readPartInfo();
	void hostload(const std::string& fileName);

	bool powerOff();

	bool internalPowerOn();
	bool internalPowerOff();

	bool setProperty(int property, int value);
	bool getProperty(int property, int& value);
	bool waitForCTS();
    std::vector<uint8_t> sendCommand(uint8_t command, uint8_t resultLength);
    std::vector<uint8_t> sendCommand(uint8_t command, const std::vector<uint8_t>& param, uint8_t resultLength);

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
