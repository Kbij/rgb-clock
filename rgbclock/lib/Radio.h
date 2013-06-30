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
#include <stdint.h>
#include <set>
#include <mutex>

namespace Hardware
{
enum class RadioState
{
	PwrOff,
	PwrOn,
	Wakeup
};

class FMReceiver;
class I2C;
class RadioObserverIf;
class ClockDisplay;

class Radio : public Hardware::KeyboardObserverIf {
public:
	Radio(I2C &i2c, uint8_t apmlifierAddress, FMReceiver &fmReceiver);
	virtual ~Radio();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);
//must be internal
//    void setDisplay(ClockDisplay *clockDisplay);
    bool togglePwr();
    bool powerOn();
	bool powerOff();
	void volume(int volume);
    void volumeUp();
    void volumeDown();
	void readRegisters();
	void writeRegisters();
	bool seekUp(int timeout);
	bool tuneFrequency(double frequency);
	RDSInfo getRDSInfo();

	void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo);
private:

	I2C &mI2C;
	const uint8_t mAplifierAddress;
	FMReceiver &mFMReceiver;
	ClockDisplay *mClockDisplay;
	uint8_t mMaskRegister;
	uint8_t mControlRegister;
	uint8_t mVolume;
	RadioState mState;
    std::set<RadioObserverIf*> mRadioObservers;
    std::recursive_mutex mRadioObserversMutex;

};
}

#endif /* RADIO_H_ */
