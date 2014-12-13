/*
 * Radio.h
 *
 *  Created on: Jun 12, 2013
 *      Author: koen
 */

#ifndef RADIO_H_
#define RADIO_H_
#include "RDSInfo.h"
#include "KeyboardObserverIf.h"
#include "RadioObserverIf.h"

#include <stdint.h>
#include <set>
#include <mutex>
#include <atomic>
#include <thread>

namespace Hardware
{


class FMReceiver;
class I2C;
class ClockDisplay;

class Radio : public Hardware::KeyboardObserverIf {
public:
	Radio(I2C &i2c, uint8_t amplifierAddress, FMReceiver &fmReceiver, double frequency);
	virtual ~Radio();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

    bool powerOn();
    bool slowPowerOn(int volume);
	bool powerOff();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();

	void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state);



private:
	bool powerOff(bool autoPowerOff);
    void volumeUp();
    void volumeDown();
	void writeRegisters();
	void notifyObservers();
	void registerFMReceiver();
	void unRegisterFMReceiver();

	void startMaintenanceThread();
	void stopMaintenanceThread();
	void maintenanceThread();

	void startAutoOffThread();
	void stopAutoOffThread();
	void autoOffThread();

	I2C &mI2C;
	const uint8_t mAplifierAddress;
	FMReceiver &mFMReceiver;
	const double mFrequency;
	uint8_t mMaskRegister;
	uint8_t mControlRegister;
	uint8_t mVolume;
    std::set<RadioObserverIf*> mRadioObservers;
    std::recursive_mutex mRadioObserversMutex;
    std::recursive_mutex mRadioMutex;
	RadioState mState;
    std::thread* mMaintenanceThread;
    std::atomic_bool mMaintenanceThreadRunning;
    std::atomic_int mTargetVolume;
    std::unique_ptr<std::thread> mAutoOffThread;
    std::atomic_bool mAutoOffThreadRunning;

};
}

#endif /* RADIO_H_ */
