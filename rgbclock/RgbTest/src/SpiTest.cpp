/*
** EPITECH PROJECT, 2020
** RgbCLock
** File description:
** SpiTest
*/

#include "lib/SPI.h"
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <vector>
#include <stdint.h>

TEST(Spi, Constructor)
{
    Hardware::SPI* spi = new Hardware::SPI("/dev/spidev0.0");
    delete spi;
}

TEST(Spi, Single)
{
    Hardware::SPI* spi = new Hardware::SPI("/dev/spidev0.0");
    std::vector<uint8_t> data;
    data.push_back(1);
    data.push_back(0);
    data.push_back(2);
    spi->writeData(data);

    delete spi;
}

TEST(Spi, Write)
{
    Hardware::SPI* spi = new Hardware::SPI("/dev/spidev0.0");
    for(int i = 0; i < 100000; i++)
    {
        std::vector<uint8_t> data;
        data.push_back(0xEE);
        data.push_back(0xFF);
        spi->writeData(data);
    }
    delete spi;
}