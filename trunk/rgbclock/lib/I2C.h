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

namespace {
struct StatData {
	std::string mName;
	int mByteCount;
	int mBytesPerSecond;
	int mMaxBytesPerSecond;
};
}

namespace HW
{
class I2C {
public:
	I2C();
	virtual ~I2C();

	bool writeByteSync(uint8_t address, uint8_t byte);
	bool writeRegByteSync(uint8_t address, uint8_t regAddr, uint8_t byte);
	bool writeDataSync(uint8_t address, const std::vector<uint8_t>& data);
	bool readByteSync(uint8_t address, uint8_t reg, uint8_t& byte);
	bool readWordSync(uint8_t address, uint8_t reg, uint16_t& word);
	bool writeReadDataSync(uint8_t address, const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData);
	void blockI2C();
	void unBlockI2C();
	void registerAddress(uint8_t address, std::string name);
	void printStatistics();
	void resetStat();
private:
	void startStatisticsThread();
	void stopStatisticsThread();
	void statisticsThread();

	bool mI2CWriteError;
	std::mutex mBusMutex;
	std::mutex mStatMutex;
	std::map<uint8_t, StatData> mAddressStatistics;
	StatData mGeneralStatistics;
    std::thread* mStatisticsThread;
    std::atomic_bool mStatisticsThreadRunning;

};
}
#endif /* I2C_H_ */
