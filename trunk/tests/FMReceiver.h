/*
 * FMReceiver.h
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#ifndef FMRECEIVER_H_
#define FMRECEIVER_H_

#include "I2C.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>

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
struct RDSInfo {
	uint16_t mProgramId;
	std::string mStationName;
	std::string mText;
	TextType mTextType;
	RDSInfo()
	{
		mProgramId = 0;
		mStationName.resize(9,' ');
		mText.resize(65,' ');
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
		mText.resize(65,' ');
	}
};

class FMReceiver {
public:
	FMReceiver(I2C &i2c, uint8_t address);
	virtual ~FMReceiver();

	bool powerOn();
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);

	bool getRDSInfo();
private:
	bool init();
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

	I2C &mI2C;
	const uint8_t mAddress;
	PowerState mPowerState;
	RDSInfo mRDSInfo;
	RDSInfo mReceivingRDSInfo;
	std::mutex mRdsInfoMutex;
    std::thread* mReadThread;
    std::atomic_bool mReadThreadRunning;
};

#endif /* FMRECEIVER_H_ */
