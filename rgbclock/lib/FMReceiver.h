/*
 * FMReceiver.h
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#ifndef FMRECEIVER_H_
#define FMRECEIVER_H_

#include "I2C.h"
#include "RadioObserverIf.h"
#include "RDSInfo.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <set>

class Radio;

enum class PowerState
{
	Unknown,
	PowerOn,
	PowerOff,

};


class FMReceiver {
public:
	FMReceiver(I2C &i2c, uint8_t address);
	virtual ~FMReceiver();
	friend Radio;
private:
	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

    bool powerOn();
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();

	void readRDSInfo();
	bool setProperty(int property, int value);
	bool getProperty(int property, int& value);
	bool waitForCTS();
	bool waitForSTC();
	bool readCTS();
	bool readSTC();
	bool readRDSInt();

	void startReadThread();
	void stopReadThread();
	void readThread();

	void notifyObservers(InfoType type);

	I2C &mI2C;
	const uint8_t mAddress;
	PowerState mPowerState;
	RDSInfo mRDSInfo;
	RDSInfo mReceivingRDSInfo;
	std::recursive_mutex mReceiverMutex;
	std::recursive_mutex mRdsInfoMutex;
    std::thread* mReadThread;
    std::atomic_bool mReadThreadRunning;
    std::set<RadioObserverIf*> mRadioObservers;
    std::recursive_mutex mRadioObserversMutex;
};

#endif /* FMRECEIVER_H_ */
