/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABDigiradStatus
*/

#ifndef DABDIGIRADSTATUS_H_
#define DABDIGIRADSTATUS_H_
#include <vector>
#include <string>
#include <sstream>

namespace Hardware
{
class DABDigiradStatus
{
public:
    DABDigiradStatus(const std::vector<uint8_t>& data): VALID(), RSSI(), SNR(), FIC_QUALITY(), CNR(), FIB_ERROR_COUNT(),
        TUNE_FREQ(), TUNE_INDEX(), FFT_OFFSET(), READANTCAP(), CU_LEVEL(), FAST_DECT(), SAMPLE_RATE_OFFSET(), FREQ_OFFSET(), FIC_BIT_CNT(), FIC_ERR_CNT()
    {
        parse(data);
    }
    ~DABDigiradStatus() {};

    std::string toString()
    {
        std::stringstream ss;
        ss << "DAB Digirad Status:" << std::endl;
        ss << "VALID: " << std::boolalpha << VALID << std::endl;
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

    bool VALID;
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
        if (data.size() > 5) VALID = data[5] & 0x01;
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
}
#endif /* !DABDIGIRADSTATUS_H_ */
