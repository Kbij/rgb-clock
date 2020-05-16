/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABComponentInfo
*/

#ifndef DABCOMPONENTINFO_H_
#define DABCOMPONENTINFO_H_

#include <vector>
#include <string>
#include <sstream>
#include "lib/Utils.h"
namespace Hardware
{
class DABComponentInfo
{
public:
    DABComponentInfo(const std::vector<uint8_t>& data): LABEL()
    {
        parse(data);
    }
    ~DABComponentInfo() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Component Info: " << " Label: " << LABEL;
        return ss.str();
    }

    std::string LABEL;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        VLOG(1) << "RAW:" << std::endl << vectorToHexString(data, true, true);
        if (data.size() > 17)
        {
            LABEL = std::string(data.begin() + 8, data.begin() + 17);
        }
    } 
};
}

#endif /* !DABCOMPONENTINFO_H_ */
