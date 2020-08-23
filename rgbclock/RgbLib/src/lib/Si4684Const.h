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
    const uint8_t SI468X_FLASH_LOAD = 0x05;
    const uint8_t SI468X_LOAD_INIT = 0x06;
    const uint8_t SI468X_BOOT = 0x07;
    const uint8_t SI468X_GET_PART_INFO = 0x08;
    const uint8_t SI468X_GET_SYS_STATE = 0x09;
    const uint8_t SI468X_SET_PROPERTY = 0x13;
    const uint8_t SI468X_GET_PROPERTY = 0x14;
    const uint8_t SI468X_DAB_TUNE_FREQ = 0xB0;
    const uint8_t SI468X_DAB_DIGRAD_STATUS = 0xB2;
    const uint8_t SI468X_DAB_GET_FREQ_LIST = 0xB9;
    const uint8_t SI468X_DAB_GET_COMPONENT_INFO = 0xBB;
    const uint8_t SI468X_GET_DIGITAL_SERVICE_LIST = 0x80;
    const uint8_t SI468X_START_DIGITAL_SERVICE = 0x81;
    const uint8_t SI468X_STOP_DIGITAL_SERVICE = 0x82;
    const uint8_t SI468X_GET_DIGITAL_SERVICE_DATA = 0x84;
    const uint8_t SI468X_DAB_GET_ENSEMBLE_INFO = 0xb4;
    const uint8_t SI468X_DAB_GET_SERVICE_INFO = 0xc0;
    const uint8_t SI468X_TEST_GET_RSSI = 0xE5;
    


    //Properties    
    const uint16_t SI468X_DAB_TUNE_FE_CFG  = 0x1712;
    const uint16_t SI468X_PIN_CONFIG_ENABLE = 0x0800;
    const uint16_t SI468X_DIGITAL_IO_OUTPUT_SELECT = 0x0200;
    const uint16_t SI468X_DAB_TUNE_FE_VARM = 0x171;
    const uint16_t SI468X_DAB_TUNE_FE_VARB = 0x1711;
    const uint16_t SI468X_DIGITAL_SERVICE_INT_SOURCE = 0x8100;

    const uint16_t PROP_FLASH_SPI_CLOCK_FREQ_KHZ = 0x0001;
    const uint16_t PROP_FLASH_SPI_SPI_MODE = 0x0002;
    const uint16_t PROP_HIGH_SPEED_READ_MAX_FREQ_MHZ = 0x0103;
    const uint16_t PROP_WRITE_ERASE_SECTOR_CMD = 0x0202;
    //Flash Write
    const uint8_t FLASH_WRITE_BLOCK_READBACK_VERIFY = 0xF1;
    
}

#endif /* !DABRECEIVERDEF_H_ */
