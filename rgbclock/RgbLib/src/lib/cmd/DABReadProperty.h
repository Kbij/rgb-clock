/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABReadProperty
*/

#ifndef DABREADPROPERTY_H_
#define DABREADPROPERTY_H_
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace Hardware
{
class DABReadProperty
{
public:
    DABReadProperty(const std::vector<uint8_t>& data, uint16_t id): ID(id),VALUE()
    {
        parse(data);
    }
    ~DABReadProperty() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Read Property: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 4 ) << ID << ", Value: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw( 4 ) <<  VALUE;
        return ss.str();
    }

    uint16_t ID;
    uint16_t VALUE;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 4)
        {
            VALUE = (data[5] << 8) + data[4];
        }
    } 
};
}

#endif /* !DABREADPROPERTY_H_ */
