/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** Status
*/

#ifndef STATUS_H_
#define STATUS_H_
#include <vector>
#include <string>
#include <sstream>

class DABStatus
{
public:
    DABStatus(const std::vector<uint8_t>& data): CTS(), ERR_CMD(), DACQ_INT(), DSRV_INT(), STC_INT(), PUP_STATE(), RFFE_ERR(), CMDOFERR(), REPOFERR(), ERROR(), mStatus0Complete(), mResp4Complete() 
    {
        parse(data);
    }
    ~DABStatus() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Status: ";
        ss << "CTS: " << std::boolalpha << CTS;
        ss << ", ERR_CMD: " << std::boolalpha << ERR_CMD;
        ss << ", DACQ_INT: " << std::boolalpha << DACQ_INT;
        ss << ", STC_INT: " << std::boolalpha << STC_INT;
        ss << ", DSRV_INT: " << std::boolalpha << DSRV_INT;
        if (mStatus3Complete)
        {
            ss << ", CMDOFERR: " << std::boolalpha << CMDOFERR;
            ss << ", REPOFERR: " << std::boolalpha << REPOFERR;
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
            ss << ", RF Error: " << std::boolalpha << RFFE_ERR;
        }

        if (mResp4Complete)
        {
            if (ERR_CMD)
            {
                ss << ", Error: ";
                switch(ERROR)
                {
                    case 0x01: ss << "unspecified"; break;
                    case 0x02: ss << "reply overflow"; break;
                    case 0x03: ss << "not available"; break;
                    case 0x04: ss << "not supported"; break;
                    case 0x05: ss << "bad frequency"; break;
                    case 0x10: ss << "command not found"; break;
                    case 0x11: ss << "bad arg1"; break;
                    case 0x12: ss << "bad arg2"; break;
                    case 0x13: ss << "bad arg3"; break;
                    case 0x14: ss << "bad arg4"; break;
                    case 0x15: ss << "bad arg5"; break;
                    case 0x16: ss << "bad arg6"; break;
                    case 0x17: ss << "bad arg7"; break;
                    case 0x18: ss << "command busy"; break;
                    case 0x19: ss << "at band limit"; break;
                    case 0x20: ss << "bad NVM"; break;
                    case 0x30: ss << "bad patch"; break;
                    case 0x31: ss << "bad bootmode"; break;
                    case 0x40: ss << "bad property"; break;
                    case 0x50: ss << "not acquired"; break;
                    case 0xff: ss << "APP not supported"; break;
                    default: ss << "Unkown error: " << (int) ERROR;
                }
            }
        }
        return ss.str();
    }

    bool error()
    {
        return ERR_CMD || CMDOFERR || REPOFERR;
    }

    bool CTS;
    bool ERR_CMD;
    bool DACQ_INT;
    bool DSRV_INT;
    bool STC_INT;
    uint8_t PUP_STATE;
    bool RFFE_ERR;
    bool CMDOFERR;
    bool REPOFERR;
    uint8_t ERROR;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 0)
        {
            CTS = data[0] & 0x80;
            ERR_CMD = data[0] & 0x40;
            DACQ_INT = data[0] & 0x20;
            DSRV_INT = data[0] & 0x10;
            STC_INT = data[0] & 0x01;

            mStatus0Complete = true;
        }
        if (data.size() >= 3)
        {
            PUP_STATE = data[3] >> 6;
            RFFE_ERR = (data[3] & 0x20) >> 5;
            REPOFERR = (data[3] & 0x08) >> 3;
            CMDOFERR = (data[3] & 0x04) >> 2;
            mStatus3Complete = true;
        }
        if (data.size() >= 4)
        {
            ERROR = data[4];
            mResp4Complete = true;
        }        
    } 
    bool mStatus0Complete; 
    bool mStatus3Complete; 
    bool mResp4Complete;
};
#endif /* !STATUS_H_ */
