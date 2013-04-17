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
	mI2CWriteError(false),
	mBusMutex()
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
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

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
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

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

bool I2C::readByteSync(uint8_t address, uint8_t reg, uint8_t& byte)
{
    std::lock_guard<std::mutex> lk_guard(mBusMutex);

	VLOG(1) << "Reading I2C; Addr: 0x" << std::hex << (int) address << "; Register:" << (int) reg << ";";

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

	uint8_t data[2];
	data[0] = reg;
	if (write(mI2CFile, data, 1) != 1)
	{
		if (!mI2CWriteError) // If first occurrence
		{
			LOG(ERROR) << "Failed setting register address: " << strerror(errno);
		}
		mI2CWriteError = true;

		return false;
	}
	if (read(mI2CFile, data, 1) != 1) {
		perror("pca9555ReadRegisterPair read value");
	}
	byte = data[0];

	return true;

#else
	return true;
#endif

/*
 Original Code
	uint8_t data[2];
	data[0] = reg;
	if (write(g_i2cFile, data, 1) != 1) {
		perror("pca9555ReadRegisterPair set register");
	}
	if (read(g_i2cFile, data, 2) != 2) {
		perror("pca9555ReadRegisterPair read value");
	}
	return data[0] | (data[1] << 8);
 */

}
