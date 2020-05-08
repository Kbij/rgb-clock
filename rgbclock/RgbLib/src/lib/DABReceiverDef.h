/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** DABReceiverDef
*/

#ifndef DABRECEIVERDEF_H_
#define DABRECEIVERDEF_H_
#include <stdint.h>

namespace Hardware
{
    const uint8_t SI468X_RD_REPLY = 0x00;
    const uint8_t SI468X_POWER_UP = 0x01;
    const uint8_t SI468X_HOST_LOAD = 0x04;
    const uint8_t SI468X_LOAD_INIT = 0x06;
    const uint8_t SI468X_BOOT = 0x07;
    const uint8_t SI468X_GET_PART_INFO = 0x08;
    const uint8_t SI468X_GET_SYS_STATE = 0x09;
    const uint8_t SI468X_SET_PROPERTY = 0x13;
    const uint8_t SI468X_GET_PROPERTY = 0x14;
    const uint8_t SI468X_DAB_TUNE_FREQ = 0xB0;
    const uint8_t SI468X_DAB_DIGRAD_STATUS = 0xB2;
    const uint8_t SI468X_DAB_GET_FREQ_LIST = 0xB9;


    //Properties    
    const uint16_t SI468X_DAB_TUNE_FE_CFG  = 0x1712;
    const uint16_t SI468X_PIN_CONFIG_ENABLE = 0x0800;
    const uint16_t SI468X_DIGITAL_IO_OUTPUT_SELECT = 0x0200;
    const uint16_t SI468X_DAB_TUNE_FE_VARM = 0x171;
    const uint16_t SI468X_DAB_TUNE_FE_VARB = 0x1711;
}

#endif /* !DABRECEIVERDEF_H_ */
