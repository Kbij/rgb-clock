/*
 * I2C.h
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */

#ifndef I2C_H_
#define I2C_H_
#include <stdint.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>
#include <memory>


namespace
{
int DEFAULT_I2C_RETRY = 3;
}

namespace App 
{
class Config;
}

namespace Hardware
{

struct StatData {
	std::string mName;
	int mByteCount;
	int mBytesPerSecond;
	int mMaxBytesPerSecond;
};

class I2C
{
public:
	I2C();
	virtual ~I2C();

	bool probeAddress(uint8_t address);
	bool writeByte(uint8_t address, uint8_t byte, int retryCount = DEFAULT_I2C_RETRY);
	bool writeRegByte(uint8_t address, uint8_t reg, uint8_t byte, int retryCount = DEFAULT_I2C_RETRY);
	bool writeData(uint8_t address, const std::vector<uint8_t>& data, int retryCount = DEFAULT_I2C_RETRY);
	bool readData(uint8_t address, uint8_t reg, uint8_t& byte, int retryCount = DEFAULT_I2C_RETRY);
	bool readData(uint8_t address, uint8_t reg, uint16_t& word, int retryCount = DEFAULT_I2C_RETRY);
	bool readWriteData(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData, int retryCounts = DEFAULT_I2C_RETRY);

	void blockI2C();
	void unBlockI2C();
	void registerAddress(uint8_t address, const std::string& name);
	void registerAddresses(const App::Config &config);
	void printStatistics();
	void printStatistics(const std::string& name);
	void resetStat();
private:
	bool readWriteDataWithRetry(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData, int retryCount);
	bool readWriteDataNoRetry(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData);

	void startStatisticsThread();
	void stopStatisticsThread();
	void statisticsThread();

	std::map<uint8_t, bool> mI2CWriteError;
	std::mutex mBusMutex;
	std::mutex mStatMutex;
	std::map<uint8_t, StatData> mAddressStatistics;
	StatData mGeneralStatistics;
    std::unique_ptr<std::thread> mStatisticsThread;
    std::atomic_bool mStatisticsThreadRunning;

};
}
#endif /* I2C_H_ */
