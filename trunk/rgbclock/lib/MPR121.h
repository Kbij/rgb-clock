/*
 * MPR121.h
 *
 *  Created on: May 5, 2013
 *      Author: koen
 */

#ifndef MPR121_H_
#define MPR121_H_
// I2Cdev library collection - MPR121 I2C device class header file
// Based on Freescale MPR121 datasheet rev. 2, 04/2010 and Freescale App Note 3944, rev 1 3/26/2010
// 9/3/2011 by Andrew Schamp <schamp@gmail.com>
//

#include <inttypes.h>

// this is the 7-bit I2C address with the ADDR pin grounded
const uint8_t MPR121_DEFAULT_ADDRESS = 0x5A;

// MPR121 Registers (from data sheet)
const uint8_t ELE0_ELE7_TOUCH_STATUS = 0x00;
const uint8_t ELE8_ELE11_ELEPROX_TOUCH_STATUS = 0x01;

const uint8_t ELE0_7_OOR_STATUS = 0x02;
const uint8_t ELE8_11_ELEPROX_OOR_STATUS = 0x03;

const uint8_t ELE0_FILTERED_DATA_LSB = 0x04;
const uint8_t ELE0_FILTERED_DATA_MSB = 0x05;
const uint8_t ELE1_FILTERED_DATA_LSB = 0x06;
const uint8_t ELE1_FILTERED_DATA_MSB = 0x07;
const uint8_t ELE2_FILTERED_DATA_LSB = 0x08;
const uint8_t ELE2_FILTERED_DATA_MSB = 0x09;
const uint8_t ELE3_FILTERED_DATA_LSB = 0x0A;
const uint8_t ELE3_FILTERED_DATA_MSB = 0x0B;
const uint8_t ELE4_FILTERED_DATA_LSB = 0x0C;
const uint8_t ELE4_FILTERED_DATA_MSB = 0x0D;
const uint8_t ELE5_FILTERED_DATA_LSB = 0x0E;
const uint8_t ELE5_FILTERED_DATA_MSB = 0x0F;
const uint8_t ELE6_FILTERED_DATA_LSB = 0x10;
const uint8_t ELE6_FILTERED_DATA_MSB = 0x11;
const uint8_t ELE7_FILTERED_DATA_LSB = 0x12;
const uint8_t ELE7_FILTERED_DATA_MSB = 0x13;
const uint8_t ELE8_FILTERED_DATA_LSB = 0x14;
const uint8_t ELE8_FILTERED_DATA_MSB = 0x15;
const uint8_t ELE9_FILTERED_DATA_LSB = 0x16;
const uint8_t ELE9_FILTERED_DATA_MSB = 0x17;
const uint8_t ELE10_FILTERED_DATA_LSB = 0x18;
const uint8_t ELE10_FILTERED_DATA_MSB = 0x19;
const uint8_t ELE11_FILTERED_DATA_LSB = 0x1A;
const uint8_t ELE11_FILTERED_DATA_MSB = 0x1B;
const uint8_t ELEPROX_FILTERED_DATA_LSB = 0x1C;
const uint8_t ELEPROX_FILTERED_DATA_MSB = 0x1D;

const uint8_t ELE0_BASELINE_VALUE = 0x1E;
const uint8_t ELE1_BASELINE_VALUE = 0x1F;
const uint8_t ELE2_BASELINE_VALUE = 0x20;
const uint8_t ELE3_BASELINE_VALUE = 0x21;
const uint8_t ELE4_BASELINE_VALUE = 0x22;
const uint8_t ELE5_BASELINE_VALUE = 0x23;
const uint8_t ELE6_BASELINE_VALUE = 0x24;
const uint8_t ELE7_BASELINE_VALUE = 0x25;
const uint8_t ELE8_BASELINE_VALUE = 0x26;
const uint8_t ELE9_BASELINE_VALUE = 0x27;
const uint8_t ELE10_BASELINE_VALUE = 0x28;
const uint8_t ELE11_BASELINE_VALUE = 0x29;
const uint8_t ELEPROX_BASELINE_VALUE = 0x2A;

const uint8_t MHD_RISING = 0x2B;
const uint8_t NHD_AMOUNT_RISING = 0x2C;
const uint8_t NCL_RISING = 0x2D;
const uint8_t FDL_RISING = 0x2E;
const uint8_t MHD_FALLING = 0x2F;
const uint8_t NHD_AMOUNT_FALLING = 0x30;
const uint8_t NCL_FALLING = 0x31;
const uint8_t FDL_FALLING = 0x32;
const uint8_t NHD_AMOUNT_TOUCHED = 0x33;
const uint8_t NCL_TOUCHED = 0x34;
const uint8_t FDL_TOUCHED = 0x35;
const uint8_t ELEPROX_MHD_RISING = 0x36;
const uint8_t ELEPROX_NHD_AMOUNT_RISING = 0x37;
const uint8_t ELEPROX_NCL_RISING = 0x38;
const uint8_t ELEPROX_FDL_RISING = 0x39;
const uint8_t ELEPROX_MHD_FALLING = 0x3A;
const uint8_t ELEPROX_NHD_AMOUNT_FALLING = 0x3B;
const uint8_t ELEPROX_FDL_FALLING = 0x3C;
const uint8_t ELEPROX_NHD_AMOUNT_TOUCHED = 0x3E;
const uint8_t ELEPROX_NCL_TOUCHED = 0x3F;
const uint8_t ELEPROX_FDL_TOUCHED = 0x40;

const uint8_t ELE0_TOUCH_THRESHOLD = 0x41;
const uint8_t ELE0_RELEASE_THRESHOLD = 0x42;
const uint8_t ELE1_TOUCH_THRESHOLD = 0x43;
const uint8_t ELE1_RELEASE_THRESHOLD = 0x44;
const uint8_t ELE2_TOUCH_THRESHOLD = 0x45;
const uint8_t ELE2_RELEASE_THRESHOLD = 0x46;
const uint8_t ELE3_TOUCH_THRESHOLD = 0x47;
const uint8_t ELE3_RELEASE_THRESHOLD = 0x48;
const uint8_t ELE4_TOUCH_THRESHOLD = 0x49;
const uint8_t ELE4_RELEASE_THRESHOLD = 0x4A;
const uint8_t ELE5_TOUCH_THRESHOLD = 0x4B;
const uint8_t ELE5_RELEASE_THRESHOLD = 0x4C;
const uint8_t ELE6_TOUCH_THRESHOLD = 0x4D;
const uint8_t ELE6_RELEASE_THRESHOLD = 0x4E;
const uint8_t ELE7_TOUCH_THRESHOLD = 0x4F;
const uint8_t ELE7_RELEASE_THRESHOLD = 0x50;
const uint8_t ELE8_TOUCH_THRESHOLD = 0x51;
const uint8_t ELE8_RELEASE_THRESHOLD = 0x52;
const uint8_t ELE9_TOUCH_THRESHOLD = 0x53;
const uint8_t ELE9_RELEASE_THRESHOLD = 0x54;
const uint8_t ELE10_TOUCH_THRESHOLD = 0x55;
const uint8_t ELE10_RELEASE_THRESHOLD = 0x56;
const uint8_t ELE11_TOUCH_THRESHOLD = 0x57;
const uint8_t ELE11_RELEASE_THRESHOLD = 0x58;
const uint8_t ELEPROX_TOUCH_THRESHOLD = 0x59;
const uint8_t ELEPROX_RELEASE_THRESHOLD = 0x5A;
const uint8_t DEBOUNCE_TOUCH_AND_RELEASE = 0x5B;
const uint8_t AFE_CONFIGURATION = 0x5C;

const uint8_t FILTER_CONFIG = 0x5D;
const uint8_t ELECTRODE_CONFIG = 0x5E;
const uint8_t ELE0_CURRENT = 0x5F;
const uint8_t ELE1_CURRENT = 0x60;
const uint8_t ELE2_CURRENT = 0x61;
const uint8_t ELE3_CURRENT = 0x62;
const uint8_t ELE4_CURRENT = 0x63;
const uint8_t ELE5_CURRENT = 0x64;
const uint8_t ELE6_CURRENT = 0x65;
const uint8_t ELE7_CURRENT = 0x66;
const uint8_t ELE8_CURRENT = 0x67;
const uint8_t ELE9_CURRENT = 0x68;
const uint8_t ELE10_CURRENT = 0x69;
const uint8_t ELE11_CURRENT = 0x6A;
const uint8_t ELEPROX_CURRENT = 0x6B;

const uint8_t ELE0_ELE1_CHARGE_TIME = 0x6C;
const uint8_t ELE2_ELE3_CHARGE_TIME = 0x6D;
const uint8_t ELE4_ELE5_CHARGE_TIME = 0x6E;
const uint8_t ELE6_ELE7_CHARGE_TIME = 0x6F;
const uint8_t ELE8_ELE9_CHARGE_TIME = 0x70;
const uint8_t ELE10_ELE11_CHARGE_TIME = 0x71;
const uint8_t ELEPROX_CHARGE_TIME = 0x72;

const uint8_t GPIO_CONTROL_0 = 0x73;
const uint8_t GPIO_CONTROL_1 = 0x74;
const uint8_t GPIO_DATA = 0x75;
const uint8_t GPIO_DIRECTION = 0x76;
const uint8_t GPIO_ENABLE = 0x77;
const uint8_t GPIO_SET = 0x78;
const uint8_t GPIO_CLEAR = 0x79;
const uint8_t GPIO_TOGGLE = 0x7A;
const uint8_t AUTO_CONFIG_CONTROL_0 = 0x7B;
const uint8_t AUTO_CONFIG_CONTROL_1 = 0x7C;
const uint8_t AUTO_CONFIG_USL = 0x7D;
const uint8_t AUTO_CONFIG_LSL = 0x7E;
const uint8_t AUTO_CONFIG_TARGET_LEVEL = 0x7F;

// Other Constants
// these are suggested values from app note 3944
const uint8_t TOUCH_THRESHOLD = 0x0F;
const uint8_t RELEASE_THRESHOLD = 0x0A;
const uint8_t NUM_CHANNELS = 12;



#endif /* MPR121_H_ */
