/*
 * main.cpp
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */
#include "I2C.h"
#include <string>
#include <glog/logging.h>

#include <iostream>
int main (int argc, char* argv[])
{
	google::InitGoogleLogging("I2C Test");
	FLAGS_logtostderr = 1;

	std::cout << "Test application for I2C Bus"<< std::endl;
	std::cout << "============================"<< std::endl;
	try
	{
		I2C i2c;

		i2c.writeByteSync(0x10, 0x80);
		std::vector<uint8_t> data;
		data.push_back(0x02);
		data.push_back(0x03);
		data.push_back(0x04);

		i2c.writeDataSync(0x11, data);
	}
	catch (std::string* caught)
	{
		LOG(ERROR) << "Failed to open I2C port";
	}

	return 0;
}

