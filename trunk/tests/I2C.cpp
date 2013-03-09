/*
 * I2C.cpp
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */

#include "I2C.h"
#include <glog/logging.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>


const std::string i2cFileName("/dev/i2c-0");

I2C::I2C() {
	// Open port for reading and writing
	if ((mI2CFile = open(i2cFileName.c_str(), O_RDWR)) < 0)
	{
		throw new std::string("Failed to open I2C port.");
	}
}

I2C::~I2C() {
	close(mI2CFile);
}

bool I2C::writeDataSync(uint8_t address, std::vector<uint8_t> data)
{
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(mI2CFile, I2C_SLAVE, address) < 0)
	{
		LOG(ERROR) << "Unable to get bus access to talk to slave";
		return false;
	}
	return true;
}
