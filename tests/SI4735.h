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



const uint8_t POWER_UP_ARG1_CTSIEN   = 0b10000000;  //CTS interrupt enable
const uint8_t POWER_UP_ARG1_GPO2OEN  = 0b01000000;  //GPO2/INT output enable
const uint8_t POWER_UP_ARG1_PATCH    = 0b00100000;  //Patch enable
const uint8_t POWER_UP_ARG1_XOSCEN   = 0b00010000;  //Enable internal oscillator with external 32768 Hz crystal
const uint8_t POWER_UP_ARG1_FUNC_FM  = 0x0;  //FM receive mode
const uint8_t POWER_UP_ARG1_FUNC_AM  = 0x1;  //AM receive mode
const uint8_t POWER_UP_ARG1_FUNC_TX  = 0x2;  //FM transmit mode - not Si4735 or Si4707
const uint8_t POWER_UP_ARG1_FUNC_WB  = 0x3;  //WB receive mode - not Si4735
const uint8_t POWER_UP_ARG1_FUNC_AUX = 0x4;  //Auxiliary input mode - Si4735-D60 or later
const uint8_t POWER_UP_ARG1_FUNC_REV = 0xF;  //Query chip's hardware and firmware revisions
//FM_TUNE_FREQ, AM_TUNE_FREQ
const uint8_t FM_TUNE_FREQ_ARG1_FREEZE = 0b10;
const uint8_t TUNE_FREQ_ARG1_FAST      = 0b01;  //Fast, inaccurate tune
//FM_SEEK_START, AM_SEEK_START
const uint8_t SEEK_START_ARG1_SEEK_UP = 0b1000;  //1 = Seek up, 0 = Seek down
const uint8_t SEEK_START_ARG1_WRAP    = 0b0100;  //Wrap when band limit reached
//FM_TUNE_STATUS, AM_TUNE_STATUS, WB_TUNE_STATUS
const uint8_t TUNE_STATUS_ARG1_CANCEL_SEEK = 0b10;  //Cancel seek operation - not WB
const uint8_t TUNE_STATUS_ARG1_CLEAR_INT   = 0b01;  //Clear STC interrupt
//FM_RSQ_STATUS, AM_RSQ_STATUS, WB_RSQ_STATUS
const uint8_t RSQ_STATUS_ARG1_CLEAR_INT = 0b1;  //Clear RSQ and related interrupts
//FM_RDS_STATUS
const uint8_t RDS_STATUS_ARG1_STATUS_ONLY = 0b100;
const uint8_t RDS_STATUS_ARG1_CLEAR_FIFO  = 0b010;  //Clear RDS receive FIFO
const uint8_t RDS_STATUS_ARG1_CLEAR_INT   = 0b001;  //Clear RDS interrupt
//WB_SAME_STATUS
const uint8_t SAME_STATUS_ARG1_CLEAR_BUFFER = 0b10;  //Clear SAME receive buffer
const uint8_t SAME_STATUS_ARG1_CLEAR_INT    = 0b01;  //Clear SAME interrupt
//AUX_ASQ_STATUS, WB_ASQ_STATUS
const uint8_t ASQ_STATUS_ARG1_CLEAR_INT = 0b1;  //Clear ASQ interrupt
//FM_AGC_OVERRIDE, AM_AGC_OVERRIDE, WB_AGC_OVERRIDE
const uint8_t AGC_OVERRIDE_ARG1_DISABLE_AGC = 0b1;  //Disable AGC
//GPIO_CTL, GPIO_SET
const uint8_t GPIO_ARG1_GPO3 = 0b1000;  //GPO3
const uint8_t GPIO_ARG1_GPO2 = 0b0100;  //GPO2
const uint8_t GPIO_ARG1_GPO1 = 0b0010;  //GPO1

const uint8_t POWER_UP_AUDIO_OUT_ANALOG  = 0b00000101;  //Enable analog audio output only

#endif /* SI4735_H_ */
