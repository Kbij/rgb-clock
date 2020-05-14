/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABPartInfo
*/

#ifndef DABPARTINFO_H_
#define DABPARTINFO_H_
#include <vector>
#include <string>
#include <sstream>

namespace Hardware
{
class DABPartInfo
{
public:
    DABPartInfo(const std::vector<uint8_t>& data): CHIPREV(),ROMID(),PART()
    {
        parse(data);
    }
    ~DABPartInfo() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Part Info: " << " ChipRev: " << (int) CHIPREV << ", RomId: " << (int) ROMID << ", Part: " << (int) PART;
        return ss.str();
    }

    uint8_t CHIPREV;
    uint8_t ROMID;
    uint16_t PART;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() >= 10)
        {
            CHIPREV = data[4];
            ROMID = data[5];
            PART = (data[9] << 8) + data[8];
        }
    } 
};
}

#endif /* !DABPARTINFO_H_ */
