/*
** RgbClock
** File description:
** DABReceiverTest
*/

#include "lib/DABReceiver.h"
#include "lib/MainboardControl.h"
#include "lib/I2C.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

TEST(DABReceiverTest, Init)
{
    Hardware::I2C i2c;
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, 0x20, false);
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, mbControl);
	receiver->init();
	delete receiver;
	delete mbControl;
}

TEST(DABReceiverTest, ListFreq)
{
    Hardware::I2C i2c;
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, nullptr);
	receiver->getFrequencyList();
	delete receiver;
}

TEST(DABReceiverTest, TuneFreq)
{
    Hardware::I2C i2c;
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, nullptr);
	receiver->tuneFrequencyIndex(30);

	delete receiver;
}


TEST(DABReceiverTest, GetServiceList)
{
    Hardware::I2C i2c;
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, nullptr);
	receiver->getServiceList();

	delete receiver;
}

TEST(DABReceiverTest, StartService)
{
    Hardware::I2C i2c;
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, nullptr);
	receiver->startService(25348, 8);

	delete receiver;
}