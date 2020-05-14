/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABServiceList
*/

#ifndef DABSERVICELIST_H_
#define DABSERVICELIST_H_
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <glog/stl_logging.h>


namespace Hardware
{
struct DABService
{
public:    
    uint32_t ServiceId;
    std::vector<uint16_t> Components;
    std::string Label;
}; 

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

class DABServiceList
{
public:
    DABServiceList(const std::vector<uint8_t>& data): SIZE(), mFullParsed(), VERSION(), SERVICECOUNT(), mServices()
    {
        parse(data);
    }
    ~DABServiceList() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "DAB Service List:" << std::endl;
        ss << "SIZE: " << (int) SIZE << std::endl;
        if (mFullParsed)
        {
            ss << "VERSION: " << (int) VERSION << std::endl;
            ss << "SERVICECOUNT: " << (int) SERVICECOUNT << std::endl;
            for(const auto& service: mServices)
            {
                ss << "Service: " << service.Label << ", Id: " << service.ServiceId << ", Components: " << service.Components << std::endl;
            }
        }
 
        return ss.str();
    }

    uint16_t SIZE;
    bool mFullParsed;
    uint16_t VERSION;
    uint8_t SERVICECOUNT;
    std::vector<DABService> mServices;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 0x05) SIZE = (data[0x05] << 8) + data[0x04];
        if (data.size() > SIZE)
        {
            mFullParsed = true;
            VERSION = (data[0x07] << 8) + data[0x06];
            SERVICECOUNT = data[0x08];
            // size of one service with zero component: 24 byte
	        // every component + 4 byte
            size_t pos = 12;
            while (pos < data.size())
            {
                DABService service;
                service.ServiceId = (data[pos + 3] << 24) + (data[pos + 2] << 16) + (data[pos + 1] << 8) + data[pos];
                int componentCount = data[pos + 5] & 0x0F;
                service.Label = std::string(data.begin() + pos + 8, data.begin() + pos + 8 + 16);
                trim(service.Label);
                for(int i = 0; i < componentCount; ++i)
                {
                    uint16_t componentId = (data[pos + 25] << 8) + data[pos + 24];
                    service.Components.push_back(componentId);

                    pos += 4;
                }
                pos += 24;
                mServices.push_back(service);
            }
        }
        
    } 
};
}

#endif /* !DABSERVICELIST_H_ */
