/*
** EPITECH PROJECT, 2021
** RgbClock
** File description:
** FastLCDisplay
*/

#include "FastLCDisplay.h"
#include "FastLCDisplayCommands.h"
#include <iterator>
#include <thread>
#include <chrono>
#include <glog/logging.h>

namespace
{
const int COMMAND_DELAY_AFTER_SEND_MS = 20;
const int COMMAND_ISREADY_DELAY = 10;
}

namespace Hardware
{
FastLCDisplay::FastLCDisplay(I2C &i2c, uint8_t address):
    mI2c(i2c),
    mAddress(address),
	mDisplayMutex()
{
    init();
}

FastLCDisplay::~FastLCDisplay()
{
}

void FastLCDisplay::initGraphic()
{
}

void FastLCDisplay::clearGraphicDisplay()
{
    while (!isReady())
    {
        VLOG(1) << "Display not ready, waiting";
        std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_ISREADY_DELAY));
    } 

    std::vector<uint8_t> data;
    data.push_back(LCD_CLEAR_SCREEN);
    mI2c.writeData(mAddress, data);    

    std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_DELAY_AFTER_SEND_MS));
}

void FastLCDisplay::writeGraphicText(uint8_t col, uint8_t row, std::string text, FontType font)
{
    while (!isReady())
    {
        VLOG(1) << "Display not ready, waiting";
        std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_ISREADY_DELAY));
    } 

    LOG(INFO) << "Writing text: [" << text << "]";
    std::vector<uint8_t> data;
    data.push_back(LCD_WRITE_TEXT);
    data.push_back(font == FontType::Verdana20 ? 2 : 1);
    data.push_back(col);
    data.push_back(row);
    std::copy(text.begin(), text.end(), std::back_inserter<std::vector<uint8_t> >(data));
    mI2c.writeData(mAddress, data);  
      
    std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_DELAY_AFTER_SEND_MS));
}

void FastLCDisplay::rectangle(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, bool set, bool fill)
{
    while (!isReady())
    {
        VLOG(1) << "Display not ready, waiting";
        std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_ISREADY_DELAY));
    } 

    VLOG(1) << "Draw rectangle,  col1: " << (int) col1 << ", row1: " << (int) row1;
    std::vector<uint8_t> data;
    data.push_back(LCD_RECTANGLE);
    data.push_back(col1);
    data.push_back(row1);
    data.push_back(col2);
    data.push_back(row2);
    data.push_back(fill);
    mI2c.writeData(mAddress, data);  
      
    std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_DELAY_AFTER_SEND_MS));
}

void FastLCDisplay::drawSignal(int8_t signal)
{
}

void FastLCDisplay::drawNTPState(bool ntpSync)
{
}

void FastLCDisplay::init()
{
    std::vector<uint8_t> data;
    data.push_back(LCD_RESET);
    mI2c.writeData(mAddress, data);
}

bool FastLCDisplay::isReady()
{
    uint8_t data = 0;
    mI2c.readData(mAddress, 0, data);
    return (bool) data;
}
}
