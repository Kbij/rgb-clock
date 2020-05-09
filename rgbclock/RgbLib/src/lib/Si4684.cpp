/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684
*/

#include "Si4684.h"
#include "lib/MainboardControl.h"
#include <thread>

namespace Hardware
{
Si4684::Si4684(I2C &i2c, uint8_t address, Hardware::MainboardControl* mainboardControl):
	mI2C(i2c),
	mAddress(address),
    mMainboardControl(mainboardControl)
{
}

Si4684::~Si4684()
{
}

bool Si4684::reset()
{
	if (mMainboardControl) mMainboardControl->resetTuner();
    std::this_thread::sleep_for( std::chrono::milliseconds(10));

    return mI2C.probeAddress(mAddress);
}

bool Si4684::init(const Si4684Settings& settings)
{

}

FrequencyList Si4684::getFrequencyList()
{

}

void Si4684::tuneFrequencyIndex(uint8_t index)
{

}

}