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
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <set>

enum class PowerState
{
	Unknown,
	PowerOn,
	PowerOff,

};
enum class TextType
{
	Unknown,
	TypeA,
	TypeB
};
const char EMPTY_CHAR = '~';

struct RDSInfo {
	uint16_t mProgramId;
	bool mValidRds;
	std::string mStationName;
	std::string mText;
	TextType mTextType;
	RDSInfo()
	{
		mValidRds = false;
		mProgramId = 0;
		clearAll();
		mTextType = TextType::Unknown;
	}
	void clearAll()
	{
		clearStationName();
		clearText();
	}
	void clearStationName()
	{
		mStationName = "";
		mStationName.resize(9,' ');
	}

	void clearText()
	{
		mText = "";
		mText.resize(65,EMPTY_CHAR);
	}
};

class FMReceiver {
public:
	FMReceiver(I2C &i2c, uint8_t address);
	virtual ~FMReceiver();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

    bool powerOn();
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();
private:
	void readRDSInfo();
	bool setProperty(int property, int value);
	bool getProperty(int property, int& value);
	void debugTuningStatus();
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
