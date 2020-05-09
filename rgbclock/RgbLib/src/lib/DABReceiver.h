/*
** RgbClock
** File description:
** DABReceiver
*/

#ifndef DABRECEIVER_H_
#define DABRECEIVER_H_

#include "RadioObserverIf.h"
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
class Si4684;

enum class DABPowerState
{
	Unknown,
	PowerOn,
	PowerOff,

};

class DABReceiver
{
public:
	DABReceiver(Si4684* siChip);
	virtual ~DABReceiver();
	friend Radio;

	void serviceScan();
    
    bool powerOn();

private:
	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

	bool powerOff();

	bool internalPowerOn();
	bool internalPowerOff();
	
	void startReadThread();
	void stopReadThread();
	void readThread();

	void notifyObservers();

	Si4684* mSiChip;
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
