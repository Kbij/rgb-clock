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

namespace
{
const uint8_t EXP_ADDR = 0x20;
const uint8_t SI468_ADDR = 0x64;
const int HARDWARE_REVISION = 3;
}

TEST(Si4684, Reset)
{
    Hardware::I2C i2c;
	Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, EXP_ADDR, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, mbControl);

	EXPECT_TRUE(si4684->reset());

	delete si4684;
	delete mbControl;
}

TEST(Si4684, InitReset)
{
    Hardware::Si4684Settings settings;
    settings.BootFile = "./firmware/rom00_patch.016.bin";
 	settings.DABFile = "./firmware/dab_radio.bin";

    Hardware::I2C i2c;
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, HARDWARE_REVISION, EXP_ADDR, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, mbControl);

	EXPECT_TRUE(si4684->reset());
	EXPECT_TRUE(si4684->init(settings));

	delete si4684;
	delete mbControl;
}


TEST(Si4684, StuBru)
{
    Hardware::Si4684Settings settings;
    settings.BootFile = "./firmware/rom00_patch.016.bin";
 	settings.DABFile = "./firmware/dab_radio.bin";

    Hardware::I2C i2c;
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, HARDWARE_REVISION, EXP_ADDR, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, mbControl);

	EXPECT_TRUE(si4684->reset());
	EXPECT_TRUE(si4684->init(settings));
    auto freqIndex = si4684->tuneFrequencyIndex(30);

    EXPECT_TRUE(si4684->startService(25348, 8));

	delete si4684;
	delete mbControl;
}

// TEST(Si4684, Init)
// {
//     Hardware::Si4684Settings settings;
//     settings.BootFile = "./firmware/rom00_patch.016.bin";
//     settings.DABFile = "./firmware/dab_radio.bin";

//     Hardware::I2C i2c;
// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
// 	EXPECT_TRUE(si4684->init(settings));

// 	delete si4684;
// }

TEST(Si4684, GetFunctionInfo)
{
    Hardware::I2C i2c;
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, nullptr);
    auto info = si4684->getFunctionInfo();

	LOG(INFO) << info.toString();

	delete si4684;
}

TEST(Si4684, GetFreqList)
{
    Hardware::I2C i2c;
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, nullptr);
    auto freqList = si4684->getFrequencyList();

	EXPECT_EQ((size_t) 41, freqList.mFrequencies.size());
	LOG(INFO) << freqList.toString();

	delete si4684;
}

TEST(Si4684, TuneFreq)
{
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, nullptr);
    auto freqList = si4684->tuneFrequencyIndex(30);

	EXPECT_EQ(223936, freqList.TUNE_FREQ);

	delete si4684;
}

TEST(Si4684, GetServices)
{
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, nullptr);
    auto services = si4684->getServices();

	EXPECT_EQ((size_t) 13, services.mServices.size());
	LOG(INFO) << services.toString();
	delete si4684;
}

TEST(Si4684, StartService)
{
    Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, nullptr);
    EXPECT_TRUE(si4684->startService(25348, 8));

	delete si4684;
}

// TEST(Si4684, GetComponentInfo)
// {
//     Hardware::I2C i2c;
// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
// 	auto info = si4684->getComponentInfo(25348, 8);
//     LOG(INFO) << "Service info: " << info.toString();

// 	delete si4684;
// }

// TEST(Si4684, GetServiceData)
// {
//     Hardware::I2C i2c;
// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
// 	auto status = si4684->getStatus();
// 	while (true)
// 	{
// 		if (status.DSRV_INT)
// 		{
// 			auto serviceData = si4684->getServiceData();
			
// 			if (serviceData.mPayload.size() > 4)
// 			{
// 			//	LOG(INFO) << serviceData.toString();
// 			//	LOG(INFO) << "Payload (" << serviceData.mPayload.size() << "): " << std::endl << vectorToHexString(serviceData.mPayload, true, true);

// 				if (serviceData.mPayload[0] == 0x00)
// 				{
// 					std::string text = std::string(serviceData.mPayload.begin() + 2, serviceData.mPayload.begin() + serviceData.BYTE_COUNT - 1);
// 					LOG(INFO) << "Station: " << text;
// 				}
// 				if (serviceData.mPayload[0] == 0x80)
// 				{
// 					std::string text = std::string(serviceData.mPayload.begin() + 2, serviceData.mPayload.begin() + serviceData.BYTE_COUNT - 1);
// 					LOG(INFO) << "Info: " << text;
// 				}				

				
// 			}
// 		}	

// 		std::this_thread::sleep_for( std::chrono::milliseconds(10));
// 		//LOG(INFO) << "Read Status";
// 		status = si4684->getStatus();
// 	}

// 	delete si4684;
// }

// TEST(Si4684, GetServiceInfo)
// {
//     Hardware::I2C i2c;
// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);

// //	DSRV_INT
// //	si4684->ge
// 	si4684->getServiceInfo();

// 	delete si4684;
// }

// TEST(Si4684, GetRssi)
// {
//     Hardware::I2C i2c;
// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, 0x64, nullptr);
// 	auto info = si4684->getRssi();
//     LOG(INFO) << "Rssi info: " << info.toString();

// 	delete si4684;
// }


TEST(Si4684, Flash)
{
    Hardware::I2C i2c;
	Hardware::SPI spi("/dev/spidev0.0");	
	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, EXP_ADDR, false);

    Hardware::Si4684Settings settings;

	//Need the minipatch file for writing to flash
	settings.MiniPatch = "./firmware/rom00_patch_mini.bin";
	//settings.MiniPatch = "./firmware/rom00_patch.016.bin";

	//Need to write these two files to the flash
	settings.BootFile = "./firmware/rom00_patch.016.bin";
    settings.DABFile = "./firmware/dab_radio.bin";

	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, mbControl);
	si4684->reset();
	EXPECT_TRUE(si4684->writeFlash(settings));

	delete si4684;
	delete mbControl;
}

// TEST(Si4684, FlashNoReset)
// {
//     Hardware::I2C i2c;


//     Hardware::Si4684Settings settings;
//     settings.BootFile = "./firmware/rom00_patch.016.bin";
//     settings.DABFile = "./firmware/dab_radio.bin";

// 	Hardware::Si4684* si4684 = new Hardware::Si4684(i2c, SI468_ADDR, nullptr);
// 	EXPECT_TRUE(si4684->writeFlash2(settings));

// 	delete si4684;
// }

TEST(Si4684, InitResetFromFlash)
{
    Hardware::I2C i2c;
	Hardware::SPI spi("/dev/spidev0.0");	

	Hardware::MainboardControl* mbControl = new Hardware::MainboardControl(i2c, 3, EXP_ADDR, false);
	Hardware::Si4684* si4684 = new Hardware::Si4684(spi, mbControl);

	EXPECT_TRUE(si4684->reset());
    Hardware::Si4684Settings settings;
    settings.MiniPatch = "./firmware/rom00_patch_mini.bin";
//	settings.BootFile = "./firmware/rom00_patch.016.bin";
	settings.LoadFromFlash = true;

	EXPECT_TRUE(si4684->init(settings));

	delete si4684;
	delete mbControl;
}

