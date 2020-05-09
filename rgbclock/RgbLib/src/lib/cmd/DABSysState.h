/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABSysState
*/

#ifndef DABSYSSTATE_H_
#define DABSYSSTATE_H_

#include <vector>
#include <string>
#include <sstream>

namespace Hardware
{
class DABSysState
{
public:
    DABSysState(const std::vector<uint8_t>& data): IMAGE() 
    {
        parse(data);
    }
    ~DABSysState() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Sys State: ";
        switch (IMAGE)
        {
            case 0:
            {
                ss << "Bootloader is active";
                break;
            }
            case 1:
            {
                ss << "FMHD is active";
                break;
            }
            case 2:
            {
                ss << "DAB is active";
                break;
            }
            case 3:
            {
                ss << "TDMB or data only DAB image is active";
                break;
            }
            case 4:
            {
                ss << "FMHD Demod is active";
                break;
            } 
            case 5:
            {
                ss << "AMHD is active";
                break;
            }
            case 6:
            {
                ss << "AMHD Demod is active";
                break;
            }
            case 7:
            {
                ss << "DAB Demod is active";
                break;
            }
            default:
            {
                ss << "Unknown image: " << (int) IMAGE;
            }
        }

        return ss.str();
    }

    uint8_t IMAGE;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() >= 5)
        {
            IMAGE = data[4];
        }
    } 
};
}

#endif /* !DABSYSSTATE_H_ */
