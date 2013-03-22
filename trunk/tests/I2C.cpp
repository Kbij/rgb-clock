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
#include <iomanip>

const std::string i2cFileName("/dev/i2c-1");

I2C::I2C() :
	mI2CFile(0),
	mI2CWriteError(false)
{
#ifndef HOSTBUILD
	// Open port for reading and writing
	if ((mI2CFile = open(i2cFileName.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus");
		ex += strerror(errno);
		throw ex;
	}
	else
	{
		LOG(INFO) << "I2C bus open for write" ;
	}
#endif
}

I2C::~I2C() {
	close(mI2CFile);
}

bool I2C::writeByteSync(uint8_t address, uint8_t byte)
{
	VLOG(1) << "Writing I2C; Addr: 0x" << std::hex << (int) address << "; Data: 0x" << (int) byte << ";";

#ifndef HOSTBUILD
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(mI2CFile, I2C_SLAVE, address) < 0)
	{
		if (!mI2CWriteError) // If first occurrence
		{
			LOG(ERROR) << "Failed setting address: " << strerror(errno);
		}
		mI2CWriteError = true;

		return false;
	}
	if ((write(mI2CFile, &byte, 1)) != 1)
	{
		if (!mI2CWriteError) // If first occurrence
		{
			LOG(ERROR) << "Failed writing data: " << strerror(errno);
		}
		mI2CWriteError = true;

		return false;
	}
	mI2CWriteError = false;

	return true;
#else
	return true;
#endif
}

bool I2C::writeDataSync(uint8_t address, const std::vector<uint8_t>& data)
{
	if (data.size() == 0)
	{
		LOG(ERROR) << "Writing data with 0 size";
		return false;
	}
	std::ostringstream logStream;
	logStream << std::hex << std::setfill('0') << std::setw(2);
	bool first = true;
	for (auto byte: data)
	{
		if (!first)
		{
			logStream << ", ";
		}
		first = false;

		logStream << " 0x" << (int) byte;
	}
	VLOG(1) << "Writing I2C; Addr: 0x" << std::hex << (int) address << "; Data:" << logStream.str() << ";";

#ifndef HOSTBUILD
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(mI2CFile, I2C_SLAVE, address) < 0)
	{
		if (!mI2CWriteError) // If first occurrence
		{
			LOG(ERROR) << "Failed setting address: " << strerror(errno);
		}
		mI2CWriteError = true;

		return false;
	}
	if ((write(mI2CFile, data.data(), data.size())) != static_cast<int>(data.size()) )
	{
		if (!mI2CWriteError) // If first occurrence
		{
			LOG(ERROR) << "Failed writing data: " << strerror(errno);
		}
		mI2CWriteError = true;

		return false;
	}
	mI2CWriteError = false;

	return true;
#else
	return true;
#endif
}
