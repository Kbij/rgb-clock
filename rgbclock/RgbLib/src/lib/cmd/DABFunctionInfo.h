/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABFunctionInfo
*/

#ifndef DABFUNCTIONINFO_H_
#define DABFUNCTIONINFO_H_
#include "lib/Utils.h"
#include <vector>
#include <string>
#include <sstream>
#include <glog/logging.h>

namespace Hardware
{
class DABFunctionInfo
{
public:
    DABFunctionInfo(const std::vector<uint8_t>& data): NUM_FREQS(), mFrequencies()
    {
        parse(data);
    }
    ~DABFunctionInfo() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Function Info: ";
        
        return ss.str();
    }

    uint8_t NUM_FREQS;
    std::vector<int> mFrequencies;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        LOG(INFO) << "Raw function info:";
        LOG(INFO) << "\n" <<  vectorToHexString(data, false, true);
    } 
};
}
#endif /* !DABFUNCTIONINFO_H_ */
