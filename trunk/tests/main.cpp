/*
 * main.cpp
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */
#include "I2C.h"
#include "RgbLed.h"

#include <string>
#include <glog/logging.h>

#include <iostream>
#include <stdio.h>
#include <cstdlib>

const uint8_t PCA9685_ADDRESS = 0b10000000;

int main (int argc, char* argv[])
{
	google::InitGoogleLogging("I2C Test");
	FLAGS_logtostderr = 1;

	std::cout << "Test application for I2C Bus"<< std::endl;
	std::cout << "============================"<< std::endl;
	try
	{
		I2C i2c;
		RgbLed rgbLed(i2c, PCA9685_ADDRESS);
		std::string inputValue;

		do
		{
			std::cout << "Please enter On Ratio ('x' to quit): ";
			std::cin >> inputValue;
			int intValue = atoi(inputValue.c_str());
			if ((intValue >=0 ) && (intValue <= 100))
			{
				std::cout << intValue << std::endl;
				rgbLed.intensity(intValue);
				rgbLed.write();

			}
		} while ( inputValue != "x");

/*
		i2c.writeByteSync(0x10, 0x80);
		std::vector<uint8_t> data;
		data.push_back(0x02);
		data.push_back(0x03);
		data.push_back(0x04);

		i2c.writeDataSync(0x11, data);
		*/
	}
	catch (std::string* caught)
	{
		LOG(ERROR) << "Failed to open I2C port";
	}

	return 0;
}

