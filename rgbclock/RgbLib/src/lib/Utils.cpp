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

std::string vectorToHexString(const std::vector<uint8_t>& vector, bool tryString)
{
	std::ostringstream ss;
	std::for_each( vector.cbegin(), vector.cend(), [&]( int c )
    {
        if ((((c >= 32) && (c <= 126)) || c == 13 || c == 10) && tryString)
        {
            ss << (char) c;
        }
        else
        {
            ss << "[0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 2 ) << c << "]";
        }
    });

	return ss.str();
}

bool readFile(const std::string& fileName, std::vector<uint8_t>& contents)
{
   	std::ifstream ifStream(fileName);
	if (ifStream.is_open())
	{
		std::vector<uint8_t> fileContents((std::istreambuf_iterator<char>(ifStream)),
		                         std::istreambuf_iterator<char>());
        contents = fileContents;  
        
        VLOG(1)  << "File: " << fileName << ", filesize: " << contents.size();
        return true;
    }
    LOG(ERROR) << "Unable to read file: " << fileName;
    return false;
}