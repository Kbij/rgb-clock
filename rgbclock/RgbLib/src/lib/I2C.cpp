/*
 * I2C.cpp
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */

#include "I2C.h"
#include "Config.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <exception>
namespace
{
const int ALL_ADDRESS = 0;
const int NO_LOGGING = 255;
};

DEFINE_int32(logI2CAddress, NO_LOGGING, "A I2C Address to log, log all I2C traffic = 0.");

namespace Hardware
{
const std::string I2C_FILENAME("/dev/i2c-1");

I2C::I2C() :
	mI2CWriteError(),
	mBusMutex(),
	mStatMutex(),
	mAddressStatistics(),
	mGeneralStatistics(),
	mStatisticsThread(),
	mStatisticsThreadRunning(false)
{
	startStatisticsThread();
#ifndef HOSTBUILD
	int i2cFile;
	// Open port for reading and writing
	if ((i2cFile = open(I2C_FILENAME.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus (" + I2C_FILENAME + "): ");
		ex += strerror(errno);
		throw std::runtime_error(ex);
	}

	close(i2cFile);
#endif
}

I2C::~I2C()
{
	stopStatisticsThread();
}

bool I2C::probeAddress(uint8_t address)
{
	std::vector<uint8_t> readData;
	return readWriteDataWithRetry(address, std::vector<uint8_t>({0}), readData, 0);
}

bool I2C::writeByte(uint8_t address, uint8_t byte, int retryCount)
{
	std::vector<uint8_t> readData;
	return readWriteDataWithRetry(address, std::vector<uint8_t>({byte}), readData, retryCount);
}

bool I2C::writeRegByte(uint8_t address, uint8_t reg, uint8_t byte, int retryCount)
{
	std::vector<uint8_t> readData;
	return readWriteDataWithRetry(address, std::vector<uint8_t>({reg, byte}), readData, retryCount);
}

bool I2C::writeData(uint8_t address, const std::vector<uint8_t>& data, int retryCount)
{
	std::vector<uint8_t> readData;
	return readWriteDataWithRetry(address, data, readData, retryCount);
}

bool I2C::readData(uint8_t address, uint8_t reg, uint8_t& byte, int retryCount)
{
	std::vector<uint8_t> readData({0});
	auto result = readWriteDataWithRetry(address, std::vector<uint8_t>({reg}), readData, retryCount);
	byte = readData[0];
	return result;
}

bool I2C::readData(uint8_t address, uint8_t reg, uint16_t& word, int retryCount)
{
	std::vector<uint8_t> readData({0,0});
	auto result = readWriteDataWithRetry(address, std::vector<uint8_t>({reg}), readData, retryCount);
	word = readData[0] | (readData[1] << 8);
	return result;
}

bool I2C::readWriteData(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData, int retryCount)
{
	return readWriteDataWithRetry(address, writeData, readData, retryCount);
}

bool I2C::readWriteDataWithRetry(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData, int retryCount)
{
	VLOG(30) << "readWrite address: " << (int) address;
	int retry = -1;
	bool result = false;
	while (!result && (retry < retryCount))
	{
		if (retry > -1)
		{
			LOG(ERROR) << "Retry (address: " << (int) address << ", " << mAddressStatistics[address].mName << "), retrycount =" << retry;
		}
		result = readWriteDataNoRetry(address, writeData, readData);
		++retry;
	}
	return result;
}

bool I2C::readWriteDataNoRetry(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData)
{
    std::lock_guard<std::mutex> lk_guard(mStatMutex);
    mAddressStatistics[address].mByteCount += 1 + writeData.size() + readData.size();

    std::lock_guard<std::mutex> lk_guard2(mBusMutex);

	if (writeData.size() == 0)
	{
		LOG(ERROR) << "Writing data with 0 size (address: " << (int) address << ", " << mAddressStatistics[address].mName << ")";
		return false;
	}
	std::ostringstream writeLogStream;
	writeLogStream << std::hex << std::setfill('0') << std::setw(2);
	bool writeFirst = true;
	for (auto byte: writeData)
	{
		if (!writeFirst)
		{
			writeLogStream << ", ";
		}
		writeFirst = false;

		writeLogStream << " 0x" << (int) byte;
	}
	LOG_IF(INFO, (address == FLAGS_logI2CAddress) || (FLAGS_logI2CAddress == ALL_ADDRESS)) << "Writing I2C (address: " << (int) address << ", " << mAddressStatistics[address].mName << "): Data:" << writeLogStream.str() << ";";

#ifndef HOSTBUILD
	int i2cFile;
	if ((i2cFile = open(I2C_FILENAME.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus");
		ex += strerror(errno);
		throw ex;
	}
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(i2cFile, I2C_SLAVE, address) < 0)
	{
		if (!mI2CWriteError[address]) // If first occurrence
		{
			LOG(ERROR) << "Failed setting address (address: " << (int) address << ", " << mAddressStatistics[address].mName << "):" << strerror(errno);
		}
		mI2CWriteError[address] = true;

		close(i2cFile);
		return false;
	}

	if (writeData.size() > 0)
	{
		if ((write(i2cFile, writeData.data(), writeData.size())) != static_cast<int>(writeData.size()) )
		{
			if (!mI2CWriteError[address]) // If first occurrence
			{
				LOG(ERROR) << "Failed writing data (address: " << (int) address << ", " << mAddressStatistics[address].mName << "): " << strerror(errno);
			}
			mI2CWriteError[address] = true;

			close(i2cFile);
			return false;
		}
	}
	mI2CWriteError[address] = false;

	if (readData.size() > 0)
	{
		if (read(i2cFile, readData.data(), readData.size()) != static_cast<int>(readData.size()))
		{
			LOG(ERROR) << "Failed reading data (address: " << (int) address << ", " << mAddressStatistics[address].mName << "): " << strerror(errno);

			close(i2cFile);
			return false;
		}

		std::ostringstream writeLogStream;
		writeLogStream << std::hex << std::setfill('0') << std::setw(2);

		bool readFirst = true;
		for (auto byte: readData)
		{
			if (!readFirst)
			{
				writeLogStream << ", ";
			}
			readFirst = false;

			writeLogStream << " 0x" << (int) byte;
		}
		LOG_IF(INFO, (address == FLAGS_logI2CAddress) || (FLAGS_logI2CAddress == ALL_ADDRESS)) << "Data read (address: " << (int) address << ", " << mAddressStatistics[address].mName << "):" << writeLogStream.str() << ";";
	}

	close(i2cFile);
	return true;
#else
	return true;
#endif
}

void I2C::blockI2C()
{
    mBusMutex.lock();
}

void I2C::unBlockI2C()
{
    mBusMutex.unlock();
}

void I2C::registerAddresses(const App::Config &config)
{
	const std::map<std::string, App::UnitConfig>& configuredUnits = config.configuredUnits();
	for (const auto& configUnit : configuredUnits)
	{
		 registerAddress(configUnit.second.mAmplifier, configUnit.second.mName +":Amplifier" );
		 registerAddress(configUnit.second.mBackLight, configUnit.second.mName +":Backlight" );
		 registerAddress(configUnit.second.mKeyboard, configUnit.second.mName +":Keyboard" );
		 registerAddress(configUnit.second.mLCD, configUnit.second.mName +":LCD" );
		 registerAddress(configUnit.second.mLight, configUnit.second.mName +":Light" );
		 registerAddress(configUnit.second.mLightSensor, configUnit.second.mName +":LightSensor" );
	}

	const App::SystemConfig& systemConfig = config.systemConfig();
	registerAddress(systemConfig.mCentralIO, "CentralIO" );
	registerAddress(systemConfig.mRadio , "FMTuner" );
	registerAddress(systemConfig.mRtc, "RTC" );
}

void I2C::registerAddress(uint8_t address, const std::string& name)
{
    std::lock_guard<std::mutex> lk_guard(mStatMutex);

    if (mAddressStatistics.find(address) == mAddressStatistics.end())
    {
        LOG(INFO) << "Register address: " << std::hex << std::setfill('0') << std::setw(2) << (int) address << ", Name: " << name;
        mAddressStatistics[address].mName = name;
    }
}

void I2C::printStatistics()
{
    std::lock_guard<std::mutex> lk_guard(mStatMutex);

    LOG(INFO) << "I2C Address statistics";
    LOG(INFO) << "======================";
    for (auto& stat : mAddressStatistics)
    {
    	LOG(INFO) << "Address: " << std::hex << "0x" << (int) stat.first;
    	LOG(INFO) << "Name: " << stat.second.mName;
    	LOG(INFO) << "Bytes/sec: " << stat.second.mBytesPerSecond;
    	LOG(INFO) << "Max Bytes/sec: " << stat.second.mMaxBytesPerSecond;
        LOG(INFO) << "---------------------";

    }

    LOG(INFO) << "I2C General statistics";
    LOG(INFO) << "======================";
   	LOG(INFO) << "Bytes/sec: " << mGeneralStatistics.mBytesPerSecond;
   	LOG(INFO) << "Max Bytes/sec: " << mGeneralStatistics.mMaxBytesPerSecond;
}

void I2C::printStatistics(const std::string& name)
{
    std::lock_guard<std::mutex> lk_guard(mStatMutex);

    for (auto& stat : mAddressStatistics)
    {
    	if (name == stat.second.mName)
    	{
        	LOG(INFO) << "Statistics for " << name << ", bytes: " << stat.second.mBytesPerSecond<< ", max bytes: "  <<stat.second.mMaxBytesPerSecond;
    	}
    }
}

void I2C::resetStat()
{
    std::lock_guard<std::mutex> lk_guard(mStatMutex);
    mGeneralStatistics.mBytesPerSecond = 0;
    mGeneralStatistics.mMaxBytesPerSecond = 0;
    for (auto& stat : mAddressStatistics)
    {
    	stat.second.mMaxBytesPerSecond = 0;
    }
}

void I2C::startStatisticsThread()
{
	mStatisticsThreadRunning = true;

	mStatisticsThread = std::unique_ptr<std::thread>(new std::thread(&I2C::statisticsThread, this));
}

void I2C::stopStatisticsThread()
{
	mStatisticsThreadRunning = false;

    if (mStatisticsThread)
    {
    	mStatisticsThread->join();
        mStatisticsThread.reset();
    }
}

void I2C::statisticsThread()
{
	pthread_setname_np(pthread_self(), "I2C Statistics");

    while (mStatisticsThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::lock_guard<std::mutex> lk_guard(mStatMutex);
    	mGeneralStatistics.mName = "General Statistics";
    	mGeneralStatistics.mBytesPerSecond = 0;

        for (auto& stat : mAddressStatistics)
        {
        	stat.second.mBytesPerSecond = stat.second.mByteCount;
        	mGeneralStatistics.mBytesPerSecond += stat.second.mBytesPerSecond;

        	if (stat.second.mBytesPerSecond > stat.second.mMaxBytesPerSecond)
        	{
        		stat.second.mMaxBytesPerSecond = stat.second.mBytesPerSecond;
        	}
        	stat.second.mByteCount = 0;
        }
        if (mGeneralStatistics.mBytesPerSecond > mGeneralStatistics.mMaxBytesPerSecond)
        {
        	mGeneralStatistics.mMaxBytesPerSecond = mGeneralStatistics.mBytesPerSecond;
        }
    }
}
}
