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

class I2C {
public:
	I2C();
	virtual ~I2C();

	bool writeByteSync(uint8_t address, uint8_t byte);
	bool writeDataSync(uint8_t address, const std::vector<uint8_t>& data);
	bool readByteSync(uint8_t address, uint8_t reg, uint8_t& byte);
private:
	int mI2CFile;

	bool mI2CWriteError;
};

#endif /* I2C_H_ */
