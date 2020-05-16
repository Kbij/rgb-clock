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
#include "AutoPowerOffTimer.h"
#include "AutoPowerOffDeviceIf.h"
#include "UpDownTimer.h"
#include "UpDownDeviceIf.h"

#include <stdint.h>
#include <set>
#include <mutex>
#include <atomic>
#include <thread>

namespace Hardware
{


class DABReceiver;
class I2C;
class ClockDisplay;

class Radio : public Hardware::KeyboardObserverIf, public AutoPowerOffDeviceIf, public UpDownDeviceIf
{
public:
	Radio(I2C &i2c, uint8_t amplifierAddress, DABReceiver &dabReceiver, double frequency);
	virtual ~Radio();

	void registerRadioObserver(RadioObserverIf *observer);
    void unRegisterRadioObserver(RadioObserverIf *observer);

	void pwrOn(bool smooth = false, int volume = 0);
	void pwrOff();

	void up(int step);
	void down(int step);

	void keyboardPressed(const KeyboardInfo& keyboardInfo);

private:
	void writeRegisters();
	void notifyObservers();
	void registerFMReceiver();
	void unRegisterFMReceiver();

	I2C &mI2C;
	const uint8_t mAplifierAddress;
	DABReceiver &mDabReceiver;
	const double mFrequency;
	uint8_t mMaskRegister;
	uint8_t mControlRegister;
	uint8_t mCurrentVolume;
	uint8_t mStoredVolume;
    std::set<RadioObserverIf*> mRadioObservers;
    std::recursive_mutex mRadioObserversMutex;
    std::recursive_mutex mRadioMutex;
	RadioState mState;

    UpDownTimer mUpDownTimer;
    AutoPowerOffTimer mAutoPowerOffTimer;
};
}

#endif /* RADIO_H_ */
