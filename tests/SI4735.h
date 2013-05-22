/*
 * SI4735.h
 *
 *  Created on: May 15, 2013
 *      Author: koen
 */

#ifndef SI4735_H_
#define SI4735_H_
#include <inttypes.h>

const uint8_t POWER_UP = 0x01;
const uint8_t GET_REV = 0x10;
const uint8_t POWER_DOWN = 0x11;
const uint8_t SET_PROPERTY = 0x12;
const uint8_t GET_PROPERTY = 0x13;
const uint8_t GET_INT_STATUS = 0x14;
const uint8_t FM_TUNE_FREQ = 0x20;
const uint8_t FM_SEEK_START = 0x21;
const uint8_t FM_TUNE_STATUS = 0x22;
const uint8_t FM_RSQ_STATUS = 0x23;
const uint8_t FM_RDS_STATUS = 0x24;
const uint8_t FM_AGC_STATUS = 0x27;
const uint8_t FM_AGC_OVERRIDE = 0x28;
const uint8_t GPIO_CTL = 0x80;
const uint8_t GPIO_SET = 0x81;






#endif /* SI4735_H_ */
