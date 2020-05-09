/*
** RgbClock
** File description:
** DABReceiverTest
*/

#include "lib/DABReceiver.h"
#include "lib/MainboardControl.h"
#include "lib/Si4684.h"
#include "lib/I2C.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

TEST(DABReceiverTest, Init)
{
    Hardware::I2C i2c;
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, 0x20, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, mbControl);

	EXPECT_TRUE(si4684->reset());

    Hardware::Si4684Settings settings;
    settings.BootFile = "./firmware/rom00_patch.016.bin";
    settings.DABFile = "./firmware/dab_radio.bin";
	EXPECT_TRUE(si4684->init(settings));

	delete si4684;
	delete mbControl;
}


TEST(DABReceiverTest, Constructor)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);

	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(si4684);
	delete receiver;
	delete si4684;
}

TEST(DABReceiverTest, ServiceScan)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);

	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(si4684);

	receiver->serviceScan();
	
	delete receiver;
	delete si4684;
}