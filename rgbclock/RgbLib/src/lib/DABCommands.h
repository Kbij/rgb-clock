/*
** RgbClock
** File description:
** DABCommands
*/

#ifndef DABCOMMANDS_H_
#define DABCOMMANDS_H_

#include <string>
#include <vector>
#include <sstream>

namespace Hardware
{
class Status
{
public:
    Status(const std::vector<uint8_t>& data): CTS(), ERR_CMD(), DACQ_INT(), DSRV_INT(), PUP_STATE(), mStatus0Complete() 
    {
        parse(data);
    }
    ~Status() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Status: ";
        ss << "CTS: " << std::boolalpha << CTS;
        ss << ", ERR_CMD: " << std::boolalpha << ERR_CMD;
        ss << ", DACQ_INT: " << std::boolalpha << DACQ_INT;
        ss << ", DSRV_INT: " << std::boolalpha << DSRV_INT;
        if (mStatus3Complete)
        {
            ss << ", CMDOFERR: " << std::boolalpha << CMDOFERR;
            ss << ", PUP_STATE: ";
            switch(PUP_STATE)
            {
                case 0:
                {
                    ss << "Waiting for POWER_UP";
                    break;
                }
                case 1:
                {
                    ss << "Reserved";
                    break;
                }
                case 2:
                {
                    ss << "Bootloader running";
                    break;
                }
                case 3:
                {
                    ss << "Application running";
                    break;
                }
            }
        }
        return ss.str();
    }

    bool CTS;
    bool ERR_CMD;
    bool DACQ_INT;
    bool DSRV_INT;
    uint8_t PUP_STATE;
    bool CMDOFERR;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 0)
        {
            CTS = data[0] & 0x80;
            ERR_CMD = data[0] & 0x40;
            DACQ_INT = data[0] & 0x20;
            DSRV_INT = data[0] & 0x10;

            mStatus0Complete = true;
        }
        if (data.size() >= 3)
        {
            PUP_STATE = data[3] >> 6;
            CMDOFERR = (data[3] & 0x04) >> 3;
            mStatus3Complete = true;
        }
    } 
    bool mStatus0Complete; 
    bool mStatus3Complete; 


};

class SysState
{
public:
    SysState(const std::vector<uint8_t>& data): IMAGE() 
    {
        parse(data);
    }
    ~SysState() {};

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

class PartInfo
{
public:
    PartInfo(const std::vector<uint8_t>& data): CHIPREV(),ROMID(),PART()
    {
        parse(data);
    }
    ~PartInfo() {};

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
        if (data.size() >= 5)
        {
            CHIPREV = data[4];
            ROMID = data[5];
            PART = (data[9] << 8) + data[8];
        }
    } 
};
}
#endif /* !DABCOMMANDS_H_ */
