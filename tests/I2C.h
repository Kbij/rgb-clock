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

	bool syncWriteData(uint8_t address, std::vector<uint8_t> data);
private:
	int mI2CFile;
};

#endif /* I2C_H_ */
