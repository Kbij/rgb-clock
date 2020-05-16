/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABRssiInfo
*/

#ifndef DABRSSIINFO_H_
#define DABRSSIINFO_H_

#include <vector>
#include <string>
#include <sstream>

namespace Hardware
{
class DABRssiInfo
{
public:
    DABRssiInfo(const std::vector<uint8_t>& data): RSSI_HIGH(), RSSI_LOW()
    {
        parse(data);
    }
    ~DABRssiInfo() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "RSII Info: " << " RSSI_HIGH: " << (int) RSSI_HIGH << ", RSSI_LOW: " << (int) RSSI_LOW;
        return ss.str();
    }

    uint8_t RSSI_HIGH;
    uint8_t RSSI_LOW;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 5)
        {
            RSSI_LOW = data[0x04];
            RSSI_HIGH = data[0x05];
        }
    } 
};
}
#endif /* !DABRSSIINFO_H_ */
