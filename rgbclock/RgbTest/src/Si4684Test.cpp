/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Si4684Test
*/

#include "lib/Si4684.h"
#include "lib/MainboardControl.h"
#include "lib/I2C.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"


TEST(Si4684, Reset)
{
    Hardware::I2C i2c;
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, 0x20, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, mbControl);

	EXPECT_TRUE(si4684->reset());

	delete si4684;
	delete mbControl;
}

TEST(Si4684, Init)
{
    Hardware::Si4684Settings settings;
    settings.BootFile = "./firmware/rom00_patch.016.bin";
    settings.DABFile = "./firmware/dab_radio.bin";

    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
	EXPECT_TRUE(si4684->init(settings));

	delete si4684;
}

TEST(Si4684, GetFrequencyList)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
    auto freqList = si4684->getFrequencyList();

	EXPECT_EQ((size_t) 41, freqList.mFrequencies.size());

	delete si4684;
}