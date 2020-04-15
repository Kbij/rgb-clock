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
    const uint8_t SI468X_BOOT = 0x07;
    const uint8_t SI468X_GET_PART_INFO = 0x08;
    const uint8_t SI468X_GET_SYS_STATE = 0x09;
    const uint8_t SI468X_LOAD_INIT = 0x06;
    const uint8_t SI468X_DAB_TUNE_FREQ = 0xB0;
    const uint8_t SI468X_DAB_GET_FREQ_LIST = 0xB9;
}

#endif /* !DABRECEIVERDEF_H_ */
