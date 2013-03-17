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
#include <chrono>
#include <thread>
#include <algorithm>


const uint8_t PCA9685_ADDRESS = 0b01000000;

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
			std::transform(inputValue.begin(), inputValue.end(), inputValue.begin(), ::tolower);
			if (inputValue.size() >= 2)
			{
				std::string value = inputValue.substr(1, inputValue.size() - 1);
				std::cout << "Value:" << value << std::endl;
				int intValue = atoi(value.c_str());
				if ((intValue >=0 ) && (intValue <= 100))
				{

					switch (inputValue[0])
					{
						case 'r': 	rgbLed.red(intValue);
									break;
						case 'g':	rgbLed.green(intValue);
									break;
						case 'b': 	rgbLed.blue(intValue);
									break;
					}
					rgbLed.write();
				}
			}
		} while ( inputValue != "x");

		/*
		do{
		for (int i = 0; i < 100; ++i)
		{
		    std::chrono::milliseconds dura( 50 );
		    std::this_thread::sleep_for( dura );
			rgbLed.intensity(i);
			rgbLed.write();
		}
		for (int i = 100; i > 0; --i)
		{
		    std::chrono::milliseconds dura( 50 );
		    std::this_thread::sleep_for( dura );
			rgbLed.intensity(i);
			rgbLed.write();
		}
		} while (1);
*/
	}
	catch (std::string* caught)
	{
		LOG(ERROR) << "Failed to open I2C port";
	}

	return 0;
}

