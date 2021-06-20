/*
** EPITECH PROJECT, 2021
** RgbClock
** File description:
** FastLCDisplayTest
*/

#include "lib/FastLCDisplay.h"
#include "lib/I2C.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"
#include <thread>
#include <chrono>
#include <sstream>




TEST(FastLCDisplayTest, Init)
{
    Hardware::I2C i2c;
	Hardware::FastLCDisplay* display = new  Hardware::FastLCDisplay(i2c, 0x04);
	delete display;
}

TEST(FastLCDisplayTest, WriteText)
{
    Hardware::I2C i2c;
	Hardware::FastLCDisplay* display = new  Hardware::FastLCDisplay(i2c, 0x04);
    
    int i = 0;
    while (i < 100000)
    {
        std::stringstream text1;
        text1 << "Hello world: "  << i;
        display->clearGraphicDisplay();
        display->writeGraphicText(1, 15, text1.str(), Hardware::FontType::Terminal8);
        i++;
        std::stringstream text2;
        text2 << i;
        display->clearGraphicDisplay();
        display->writeGraphicText(10, 10, text2.str(), Hardware::FontType::Verdana20);
    }

	delete display;
}

TEST(FastLCDisplayTest, WriteLargeTest)
{
    Hardware::I2C i2c;
	Hardware::FastLCDisplay* display = new  Hardware::FastLCDisplay(i2c, 0x04);

    int i = 0;
    display->clearGraphicDisplay();
    bool result = true;
    while (i < 1000 && result)
    {    
        LOG(INFO) << "i:" << i++;
        result = display->writeGraphicText(1, 15, "12345678901234567890123456789012346", Hardware::FontType::Terminal8);
    } 
	delete display;
}

TEST(FastLCDisplayTest, WriteTextClearLength)
{
    Hardware::I2C i2c;
	Hardware::FastLCDisplay* display = new  Hardware::FastLCDisplay(i2c, 0x04);
    
    display->clearGraphicDisplay();
    display->writeGraphicText(1, 15, "0123456789", Hardware::FontType::Terminal8);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    display->writeGraphicText(1, 15, "9876543210", Hardware::FontType::Terminal8);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    display->writeGraphicText(1, 15, "1", Hardware::FontType::Terminal8, 128);

	delete display;
}