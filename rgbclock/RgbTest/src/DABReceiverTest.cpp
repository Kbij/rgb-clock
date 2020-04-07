/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABReceiverTest
*/

#include "lib/DABReceiver.h"
#include "lib/I2C.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

TEST(DABReceiverTest, Constructor)
{
    Hardware::I2C i2c;
	Hardware::DABReceiver* receiver = new Hardware::DABReceiver(i2c, 0x64, nullptr);
	delete receiver;
}