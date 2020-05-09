/*
** RgbClock
** File description:
** Utils
*/

#include "Utils.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <glog/logging.h>
#include <fstream>

std::string vectorToHexString(const std::vector<uint8_t>& vector, bool tryString, bool blockMode)
{
	std::ostringstream ss;
    int pos = 0;

    if (blockMode)
    {
        ss << "     ";
        for(int i = 0; i < 16; ++i)
        {
            ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 2 ) << i << "|";
        }
        ss << std::endl << "=====================================================================================";
    }

    for(int pos = 0; pos < vector.size(); ++pos)
    {
        if (blockMode)
        {
            div_t divResult = div (pos, 16);
            if (divResult.rem == 0)
            {
                ss << std::endl;
                ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 2 ) << divResult.quot << "|";
            }
        }

        uint8_t c = vector[pos];
        if ((((c >= 32) && (c <= 126))) && tryString)
        {
            if (blockMode)
            {
                ss << "  " << c << "  ";
            }
            else
            {
                ss << (char) c;
            }
        }
        else
        {
            ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 2 ) << (int) c << "|";
        }
    }
    
	return ss.str();
}

bool readFile(const std::string& fileName, std::vector<uint8_t>& contents)
{
   	std::ifstream ifStream(fileName, std::ios::in | std::ios::binary);
	if (ifStream.is_open())
	{
		std::vector<uint8_t> fileContents((std::istreambuf_iterator<char>(ifStream)),
		                         std::istreambuf_iterator<char>());
        contents = fileContents;  
        
        VLOG(1)  << "File: " << fileName << ", filesize: " << contents.size();
        return true;

        ifStream.close();
    }
    LOG(ERROR) << "Unable to read file: " << fileName;
    return false;
}

bool writeFile(const std::string& fileName, const std::vector<uint8_t>& contents)
{
   	std::ofstream ofStream(fileName, std::ios::out | std::ios::binary);
	if (ofStream.is_open())
	{
        ofStream.write((const char*)contents.data(), contents.size());
        ofStream.close();
        return true;
    }
    LOG(ERROR) << "Unable to read file: " << fileName;
    return false;    
}