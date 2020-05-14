/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABDigitalServiceData
*/

#ifndef DABDIGITALSERVICEDATA_H_
#define DABDIGITALSERVICEDATA_H_
#include <vector>
#include <string>
#include <sstream>
#include "lib/Utils.h"
#include <glog/logging.h>

namespace Hardware
{
class DigitalServiceData
{
public:
    DigitalServiceData(const std::vector<uint8_t>& data): DSRVOVFLINT(), DSRVPCKTINT(), BUFF_COUNT(), SRV_STATE(), DATA_TYPE(), SERVICE_ID(), COMPONENT_ID(), BYTE_COUNT(), SEG_NUM(), NUM_SEGS(), mPayload()
    {
        parse(data);
    }
    ~DigitalServiceData() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "Digital Service Data: DSRVOVFLINT: " << std::boolalpha << DSRVOVFLINT << ", DSRVPCKTINT: " << std::boolalpha << DSRVPCKTINT;
        ss << ", BUFF_COUNT: " << (int) BUFF_COUNT << ", SRV_STATE: " << (int) SRV_STATE << ", DATA_TYPE: " << (int) DATA_TYPE;
        
        ss << ", Byte count: " << (int) BYTE_COUNT << ", ServiceId: " << (int) SERVICE_ID << ", Component: " << (int) COMPONENT_ID << ", SEGNUM: " << (int) SEG_NUM << "/" << (int) NUM_SEGS;
        
        return ss.str();
    }
    bool DSRVOVFLINT;
    bool DSRVPCKTINT;
    uint8_t BUFF_COUNT;
    uint8_t SRV_STATE;
    uint8_t DATA_TYPE;
    uint32_t SERVICE_ID;
    uint32_t COMPONENT_ID;
    uint16_t BYTE_COUNT;
    uint16_t SEG_NUM;
    uint16_t NUM_SEGS;
    std::vector<uint8_t> mPayload;
private:
    void parse(const std::vector<uint8_t>& data)
    {  
        if (data.size() > 0x04) DSRVPCKTINT = data[0x04] & 0x01;
        if (data.size() > 0x04) DSRVOVFLINT = data[0x04] & 0x02;
        if (data.size() > 0x05) BUFF_COUNT = data[0x05];
        if (data.size() > 0x06) SRV_STATE = data[0x06];
        if (data.size() > 0x07) DATA_TYPE = data[0x07];

        

        if (data.size() > 0x0b) SERVICE_ID = (data[0x0b] << 24) + (data[0x0a] << 16) + (data[0x09] << 8) + data[0x08];
        if (data.size() > 0x0f) COMPONENT_ID = (data[0x0f] << 24) + (data[0x0e] << 16) + (data[0x0d] << 8) + data[0x0c];
        if (data.size() > 0x13) BYTE_COUNT = (data[0x13] << 8) + data[0x12];
        if (data.size() > 0x15) SEG_NUM = (data[0x15] << 8) + data[0x14];
        if (data.size() > 0x18) NUM_SEGS = (data[0x18] << 8) + data[0x17];

        if (data.size() > 0x18) mPayload = std::vector<uint8_t>(data.begin() + 0x18, data.end());
    } 

};
}

#endif /* !DABDIGITALSERVICEDATA_H_ */
