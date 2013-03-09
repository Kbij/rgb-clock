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
		I2C *i2c = new I2C();
	}
	catch (std::string* caught)
	{
		LOG(ERROR) << "Failed to open I2C port";
	}

	return 0;
}

