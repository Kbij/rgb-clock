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

class I2C {
public:
	I2C();
	virtual ~I2C();

	bool writeByteSync(uint8_t address, uint8_t byte);
	bool writeDataSync(uint8_t address, const std::vector<uint8_t>& data);
	bool readByteSync(uint8_t address, uint8_t reg, uint8_t& byte);
	bool readWordSync(uint8_t address, uint8_t reg, uint16_t& word);
private:
	int mI2CFile;

	bool mI2CWriteError;
	std::mutex mBusMutex;
};

#endif /* I2C_H_ */