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
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, 0x21, false);
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

TEST(Si4684, GetFreqList)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
    auto freqList = si4684->getFrequencyList();

	EXPECT_EQ((size_t) 41, freqList.mFrequencies.size());
	LOG(INFO) << freqList.toString();

	delete si4684;
}

TEST(Si4684, TuneFreq)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
    auto freqList = si4684->tuneFrequencyIndex(30);

	EXPECT_EQ(223936, freqList.TUNE_FREQ);

	delete si4684;
}

TEST(Si4684, GetServices)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
    auto services = si4684->getServices();

	EXPECT_EQ((size_t) 13, services.mServices.size());
	LOG(INFO) << services.toString();
	delete si4684;
}

TEST(Si4684, StartService)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
    EXPECT_TRUE(si4684->startService(25348, 8));

	delete si4684;
}

TEST(Si4684, GetServiceData)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
	auto status = si4684->getStatus();
	while (true)
	{
		if (status.DSRV_INT)
		{
			auto serviceData = si4684->getServiceData();
			
			if (serviceData.mPayload.size() > 4)
			{
			//	LOG(INFO) << serviceData.toString();
			//	LOG(INFO) << "Payload (" << serviceData.mPayload.size() << "): " << std::endl << vectorToHexString(serviceData.mPayload, true, true);

				if (serviceData.mPayload[0] == 0x00)
				{
					std::string text = std::string(serviceData.mPayload.begin() + 2, serviceData.mPayload.begin() + serviceData.BYTE_COUNT - 1);
					LOG(INFO) << "Station: " << text;
				}
				if (serviceData.mPayload[0] == 0x80)
				{
					std::string text = std::string(serviceData.mPayload.begin() + 2, serviceData.mPayload.begin() + serviceData.BYTE_COUNT - 1);
					LOG(INFO) << "Info: " << text;
				}				

				
			}
		}	

		std::this_thread::sleep_for( std::chrono::milliseconds(10));
		//LOG(INFO) << "Read Status";
		status = si4684->getStatus();
	}

	delete si4684;
}

TEST(Si4684, GetServiceInfo)
{
    Hardware::I2C i2c;
	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);

//	DSRV_INT
//	si4684->ge
	si4684->getServiceInfo();
    //LOG(INFO) << serviceData.toString();

	delete si4684;
}