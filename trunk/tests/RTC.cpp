/*
 * RTC.cpp
 *
 *  Created on: Jun 13, 2013
 *      Author: koen
 */

#include "RTC.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <glog/logging.h>

RTC::RTC(I2C &i2c):
	mI2C(i2c)
{

}

RTC::~RTC()
{

}

std::string readNTPStatus()
{
	std::string ntpqCmd("ntpq -p");
    FILE* pipe = popen(ntpqCmd.c_str(), "r");

    if (!pipe)
    {
    	return "ERROR";
    }

    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
    {
    	if (fgets(buffer, 128, pipe) != NULL)
    	{
    		result += buffer;
    	}
    }
    pclose(pipe);
    return result;
}

void RTC::showNTPStatus()
{
	std::string ntpStatus =  readNTPStatus();
    std::istringstream resultStream(ntpStatus);
    std::string line;
    while (std::getline(resultStream, line)) {

    	if ((line.size() > 0) && (line[0] == '*'))
    	{
    	    std::vector<std::string> columns;
    	    std::istringstream lineStream(line);
    	    std::string cell;
    	    while (std::getline(lineStream, cell, ' ')) {
    	    	if (cell.size() > 0)
    	    	{
        	    	columns.push_back(cell);
    	    	}
    	    }

    	    if (columns.size() > 7)
    	    {
				LOG(INFO) << "Synchronised with: " << columns[0];
				LOG(INFO) << "Delay: " << columns[7] << "msec";
    	    }
    	}
    }

   // LOG(INFO) << readNTPStatus();
}


