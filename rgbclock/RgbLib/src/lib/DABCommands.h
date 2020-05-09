/*
** RgbClock
** File description:
** DABCommands
*/

#ifndef DABCOMMANDSOLD_H_
#define DABCOMMANDSOLD_H_

//#include "DABReceiverDef.h"
#include "DABService.h"
#include "Utils.h"
#include <string>
#include <vector>
#include <sstream>
#include <glog/stl_logging.h>
#include <glog/logging.h>

namespace Hardware
{
class Status
{
public:
    Status(const std::vector<uint8_t>& data): CTS(), ERR_CMD(), DACQ_INT(), DSRV_INT(), STC_INT(), PUP_STATE(), RFFE_ERR(), CMDOFERR(), REPOFERR(), ERROR(), mStatus0Complete(), mResp4Complete() 
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
        if (data.size() >= 10)
        {
            CHIPREV = data[4];
            ROMID = data[5];
            PART = (data[9] << 8) + data[8];
        }
    } 
};



class DabDigiradStatus
{
public:
    DabDigiradStatus(const std::vector<uint8_t>& data): RSSI(), SNR(), FIC_QUALITY(), CNR(), FIB_ERROR_COUNT(),
        TUNE_FREQ(), TUNE_INDEX(), FFT_OFFSET(), READANTCAP(), CU_LEVEL(), FAST_DECT(), SAMPLE_RATE_OFFSET(), FREQ_OFFSET(), FIC_BIT_CNT(), FIC_ERR_CNT()
    {
        parse(data);
    }
    ~DabDigiradStatus() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "DAB Digirad Status:" << std::endl;
        ss << "RSSI: " << (int) RSSI << std::endl;
        ss << "SNR: " << (int) SNR << std::endl;
        ss << "FIC_QUALITY: " << (int) FIC_QUALITY << std::endl;
        ss << "CNR: " << (int) FIC_QUALITY << std::endl;
        ss << "FIB_ERROR_COUNT: " << (int) FIB_ERROR_COUNT << std::endl;
        ss << "TUNE_FREQ: " << (double) TUNE_FREQ/1000 <<  "Mhz" << std::endl;
        ss << "TUNE_INDEX: " << (int) TUNE_INDEX << std::endl;
        ss << "FFT_OFFSET: " << (int) FFT_OFFSET << std::endl;
        ss << "READANTCAP: " << (int) READANTCAP << std::endl;
        ss << "CU_LEVEL: " << (int) CU_LEVEL << std::endl;
        ss << "FAST_DECT: " << (int) FAST_DECT << std::endl;
        ss << "SAMPLE_RATE_OFFSET: " << (int) SAMPLE_RATE_OFFSET << std::endl;
        ss << "FREQ_OFFSET: " << (int) FREQ_OFFSET << std::endl;
        ss << "FIC_BIT_CNT: " << (int) FIC_BIT_CNT << std::endl;
        ss << "FIC_ERR_CNT: " << (int) FIC_BIT_CNT << std::endl;
 
        return ss.str();
    }

    uint8_t RSSI;
    uint8_t SNR;
    uint8_t FIC_QUALITY;
    uint8_t CNR;
    uint16_t FIB_ERROR_COUNT;
    uint32_t TUNE_FREQ;
    uint8_t TUNE_INDEX;
    uint8_t FFT_OFFSET;
    uint16_t READANTCAP;
    uint16_t CU_LEVEL;
    uint8_t FAST_DECT;
    uint16_t SAMPLE_RATE_OFFSET;
    uint32_t FREQ_OFFSET;
    uint32_t FIC_BIT_CNT;
    uint32_t FIC_ERR_CNT;
private:
    void parse(const std::vector<uint8_t>& data)
    {
        if (data.size() > 6) RSSI = data[6];
        if (data.size() > 7) SNR = data[7];
        if (data.size() > 8) FIC_QUALITY = data[8];
        if (data.size() > 9) CNR = data[9];
        if (data.size() > 0x0b) FIB_ERROR_COUNT = (data[0x0b] << 8) + data[0x0a];
        if (data.size() > 0x0f) TUNE_FREQ = (data[0x0f] << 24) + (data[0x0e] << 16) + (data[0x0d] << 8) + data[0x0c];
        if (data.size() > 0x10) TUNE_INDEX = data[0x10];
        if (data.size() > 0x11) FFT_OFFSET = data[0x11];
        if (data.size() > 0x13) READANTCAP = (data[0x13] << 8) + data[0x12];
        if (data.size() > 0x15) CU_LEVEL = (data[0x15] << 8) + data[0x14];
        if (data.size() > 0x16) FAST_DECT = data[0x16];
        if (data.size() > 0x1b) SAMPLE_RATE_OFFSET = (data[0x1b] << 8) + data[0x1a];
        if (data.size() > 0x1f) FREQ_OFFSET = (data[0x1f] << 24) + (data[0x1e] << 16) + (data[0x1d] << 8) + data[0x1c];
        if (data.size() > 0x23) FIC_BIT_CNT = (data[0x23] << 24) + (data[0x22] << 16) + (data[0x21] << 8) + data[0x20];
        if (data.size() > 0x27) FIC_ERR_CNT = (data[0x27] << 24) + (data[0x26] << 16) + (data[0x25] << 8) + data[0x24];
    } 
};

class DabServiceList
{
public:
    DabServiceList(const std::vector<uint8_t>& data): SIZE(), mFullParsed(), VERSION(), SERVICECOUNT(), mServices()
    {
        parse(data);
    }
    ~DabServiceList() {};

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
        LOG(INFO) << "-->Text: " << std::endl << vectorToHexString(data, true, true);
        LOG(INFO) << "-->Hex: " << std::endl << vectorToHexString(data, false,  true);
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
#endif /* !DABCOMMANDSOLD_H_ */
