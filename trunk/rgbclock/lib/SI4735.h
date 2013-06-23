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

// Properties
const uint16_t PROP_GPO_IEN                             =0x0001;
const uint16_t PROP_DIGITAL_OUTPUT_FORMAT               =0x0102;
const uint16_t PROP_DIGITAL_OUTPUT_SAMPLE_RATE          =0x0104;
const uint16_t PROP_REFCLK_FREQ                         =0x0201;
const uint16_t PROP_REFCLK_PRESCALE                     =0x0202;
const uint16_t PROP_RX_VOLUME                           =0x4000;
const uint16_t PROP_RX_HARD_MUTE                        =0x4001;
//FM mode
const uint16_t PROP_FM_DEEMPHASIS                       =0x1100;
const uint16_t PROP_FM_CHANNEL_FILTER                   =0x1102;
const uint16_t PROP_FM_BLEND_STEREO_THRESHOLD           =0x1105;
const uint16_t PROP_FM_BLEND_MONO_THRESHOLD             =0x1106;
const uint16_t PROP_FM_MAX_TUNE_ERROR                   =0x1108;
const uint16_t PROP_FM_RSQ_INT_SOURCE                   =0x1200;
const uint16_t PROP_FM_RSQ_SNR_HI_THRESHOLD             =0x1201;
const uint16_t PROP_FM_RSQ_SNR_LO_THRESHOLD             =0x1202;
const uint16_t PROP_FM_RSQ_RSSI_HI_THRESHOLD            =0x1203;
const uint16_t PROP_FM_RSQ_RSSI_LO_THRESHOLD            =0x1204;
const uint16_t PROP_FM_RSQ_MULTIPATH_HI_THRESHOLD       =0x1205;
const uint16_t PROP_FM_RSQ_MULTIPATH_LO_THRESHOLD       =0x1206;
const uint16_t PROP_FM_RSQ_BLEND_THRESHOLD              =0x1207;
const uint16_t PROP_FM_SOFT_MUTE_RATE                   =0x1300;
const uint16_t PROP_FM_SOFT_MUTE_SLOPE                  =0x1301;
const uint16_t PROP_FM_SOFT_MUTE_MAX_ATTENUATION        =0x1302;
const uint16_t PROP_FM_SOFT_MUTE_SNR_THRESHOLD          =0x1303;
const uint16_t PROP_FM_SOFT_MUTE_RELEASE_RATE           =0x1304;
const uint16_t PROP_FM_SOFT_MUTE_ATTACK_RATE            =0x1305;
const uint16_t PROP_FM_SEEK_BAND_BOTTOM                 =0x1400;
const uint16_t PROP_FM_SEEK_BAND_TOP                    =0x1401;
const uint16_t PROP_FM_SEEK_FREQ_SPACING                =0x1402;
const uint16_t PROP_FM_SEEK_TUNE_SNR_THRESHOLD          =0x1403;
const uint16_t PROP_FM_SEEK_TUNE_RSSI_THRESHOLD         =0x1404;
const uint16_t PROP_FM_RDS_INT_SOURCE                   =0x1500;
const uint16_t PROP_FM_RDS_INT_FIFO_COUNT               =0x1501;
const uint16_t PROP_FM_RDS_CONFIG                       =0x1502;
const uint16_t PROP_FM_RDS_CONFIDENCE                   =0x1503;
const uint16_t PROP_FM_BLEND_RSSI_STEREO_THRESHOLD      =0x1800;
const uint16_t PROP_FM_BLEND_RSSI_MONO_THRESHOLD        =0x1801;
const uint16_t PROP_FM_BLEND_RSSI_ATTACK_RATE           =0x1802;
const uint16_t PROP_FM_BLEND_RSSI_RELEASE_RATE          =0x1803;
const uint16_t PROP_FM_BLEND_SNR_STEREO_THRESHOLD       =0x1804;
const uint16_t PROP_FM_BLEND_SNR_MONO_THRESHOLD         =0x1805;
const uint16_t PROP_FM_BLEND_SNR_ATTACK_RATE            =0x1806;
const uint16_t PROP_FM_BLEND_SNR_RELEASE_RATE           =0x1807;
const uint16_t PROP_FM_BLEND_MULTIPATH_STEREO_THRESHOLD =0x1808;
const uint16_t PROP_FM_BLEND_MULTIPATH_MONO_THRESHOLD   =0x1809;
const uint16_t PROP_FM_BLEND_MULTIPATH_ATTACK_RATE      =0x180A;
const uint16_t PROP_FM_BLEND_MULTIPATH_RELEASE_RATE     =0x180B;
const uint16_t PROP_FM_HICUT_SNR_HIGH_THRESHOLD         =0x1A00;
const uint16_t PROP_FM_HICUT_SNR_LOW_THRESHOLD          =0x1A01;
const uint16_t PROP_FM_HICUT_ATTACK_RATE                =0x1A02;
const uint16_t PROP_FM_HICUT_RELEASE_RATE               =0x1A03;
const uint16_t PROP_FM_HICUT_MULTIPATH_TRIGGER_THRESHOLD=0x1A04;
const uint16_t PROP_FM_HICUT_MULTIPATH_END_THRESHOLD    =0x1A05;
const uint16_t PROP_FM_HICUT_CUTOFF_FREQUENCY           =0x1A06;


const uint16_t FIELD_FM_DEEMPHASIS_ARG=0b11;
const uint16_t FM_DEEMPHASIS_ARG_75=0b10;  //75 μs (default)
const uint16_t FM_DEEMPHASIS_ARG_50=0b01;  //50 μs
//FM_RDS_INT_SOURCE
/* See RDS interrupts above. */
//FM_RDS_CONFIG
const uint16_t FIELD_FM_RDS_CONFIG_ARG_BLOCK_A=0b11000000<<8;
const uint16_t FIELD_FM_RDS_CONFIG_ARG_BLOCK_B=0b00110000<<8;
const uint16_t FIELD_FM_RDS_CONFIG_ARG_BLOCK_C=0b00001100<<8;
const uint16_t FIELD_FM_RDS_CONFIG_ARG_BLOCK_D=0b00000011<<8;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_A_NO_ERRORS    =0U<<14;  //Block must have no errors
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_A_2_BIT_ERRORS =1U<<14;  //Block may have up to 2 bit errors
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_A_5_BIT_ERRORS =2U<<14;  //Block may have up to 5 bit errors
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_A_UNCORRECTABLE=3U<<14;  //Block may be uncorrectable
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_B_NO_ERRORS    =0U<<12;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_B_2_BIT_ERRORS =1U<<12;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_B_5_BIT_ERRORS =2U<<12;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_B_UNCORRECTABLE=3U<<12;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_C_NO_ERRORS    =0U<<10;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_C_2_BIT_ERRORS =1U<<10;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_C_5_BIT_ERRORS =2U<<10;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_C_UNCORRECTABLE=3U<<10;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_D_NO_ERRORS    =0U<<8;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_D_2_BIT_ERRORS =1U<<8;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_D_5_BIT_ERRORS =2U<<8;
const uint16_t FM_RDS_CONFIG_ARG_BLOCK_D_UNCORRECTABLE=3U<<8;
const uint16_t FM_RDS_CONFIG_ARG_ENABLE=0b1;  //Enable RDS

// Bit masks for RDS interrupts. Use with FM_RDS_INT_SOURCE property and RESP1 of FM_RDS_STATUS command.
const uint16_t RDS_NEW_B_BLOCK_MASK=0b00100000;  //New B block (Si4735-D50 or later)
const uint16_t RDS_NEW_A_BLOCK_MASK=0b00010000;  //New A block (Si4735-D50 or later)
const uint16_t RDS_SYNC_FOUND_MASK =0b00000100;  //RDS sync found
const uint16_t RDS_SYNC_LOST_MASK  =0b00000010;  //RDS sync lost
const uint16_t RDS_RECEIVED_MASK   =0b00000001;   //FM_RDS_INT_FIFO_COUNT or more packets received

// Command responses
// Names that begin with FIELD are argument masks.  Others are argument constants.

//FM_TUNE_STATUS, AM_TUNE_STATUS, WB_TUNE_STATUS
const uint16_t FIELD_TUNE_STATUS_RESP1_SEEK_LIMIT=0b10000000;  //Seek hit search limit - not WB
const uint16_t FIELD_TUNE_STATUS_RESP1_AFC_RAILED=0b10;  //AFC railed
const uint16_t FIELD_TUNE_STATUS_RESP1_SEEKABLE  =0b01;  //Station could currently be found by seek,
const uint16_t FIELD_TUNE_STATUS_RESP1_VALID     =0b01;  //that is, the station is valid
//FM_RSQ_STATUS, AM_RSQ_STATUS, WB_RSQ_STATUS
/* See RSQ interrupts above for RESP1. */
const uint16_t FIELD_RSQ_STATUS_RESP2_SOFT_MUTE =0b1000;  //Soft mute active - not WB
const uint16_t FIELD_RSQ_STATUS_RESP2_AFC_RAILED=0b0010;  //AFC railed
const uint16_t FIELD_RSQ_STATUS_RESP2_SEEKABLE  =0b0001;  //Station could currently be found by seek,
const uint16_t FIELD_RSQ_STATUS_RESP2_VALID     =0b0001;  //that is, the station is valid
const uint16_t FIELD_RSQ_STATUS_RESP3_STEREO=0b10000000;  //Stereo pilot found - FM only
const uint16_t FIELD_RSQ_STATUS_RESP3_STEREO_BLEND=0b01111111;  //Stereo blend in % (100 = full stereo, 0 = full mono) - FM only
//FM_RDS_STATUS
/* See RDS interrupts above for RESP1. */
const uint16_t FIELD_RDS_STATUS_RESP2_FIFO_OVERFLOW=0b00000100;  //FIFO overflowed
const uint16_t FIELD_RDS_STATUS_RESP2_SYNC         =0b00000001;  //RDS currently synchronized
const uint16_t FIELD_RDS_STATUS_RESP12_BLOCK_A=0b11000000;
const uint16_t FIELD_RDS_STATUS_RESP12_BLOCK_B=0b00110000;
const uint16_t FIELD_RDS_STATUS_RESP12_BLOCK_C=0b00001100;
const uint16_t FIELD_RDS_STATUS_RESP12_BLOCK_D=0b00000011;
const uint16_t RDS_STATUS_RESP12_BLOCK_A_NO_ERRORS    =0U<<6;  //Block had no errors
const uint16_t RDS_STATUS_RESP12_BLOCK_A_2_BIT_ERRORS =1U<<6;  //Block had 1-2 bit errors
const uint16_t RDS_STATUS_RESP12_BLOCK_A_5_BIT_ERRORS =2U<<6;  //Block had 3-5 bit errors
const uint16_t RDS_STATUS_RESP12_BLOCK_A_UNCORRECTABLE=3U<<6;  //Block was uncorrectable
const uint16_t RDS_STATUS_RESP12_BLOCK_B_NO_ERRORS    =0U<<4;
const uint16_t RDS_STATUS_RESP12_BLOCK_B_2_BIT_ERRORS =1U<<4;
const uint16_t RDS_STATUS_RESP12_BLOCK_B_5_BIT_ERRORS =2U<<4;
const uint16_t RDS_STATUS_RESP12_BLOCK_B_UNCORRECTABLE=3U<<4;
const uint16_t RDS_STATUS_RESP12_BLOCK_C_NO_ERRORS    =0U<<2;
const uint16_t RDS_STATUS_RESP12_BLOCK_C_2_BIT_ERRORS =1U<<2;
const uint16_t RDS_STATUS_RESP12_BLOCK_C_5_BIT_ERRORS =2U<<2;
const uint16_t RDS_STATUS_RESP12_BLOCK_C_UNCORRECTABLE=3U<<2;
const uint16_t RDS_STATUS_RESP12_BLOCK_D_NO_ERRORS    =0U<<0;
const uint16_t RDS_STATUS_RESP12_BLOCK_D_2_BIT_ERRORS =1U<<0;
const uint16_t RDS_STATUS_RESP12_BLOCK_D_5_BIT_ERRORS =2U<<0;
const uint16_t RDS_STATUS_RESP12_BLOCK_D_UNCORRECTABLE=3U<<0;
//WB_SAME_STATUS - TODO

//AUX_ASQ_STATUS, WB_ASQ_STATUS
/* See ASQ interrupts above for RESP1. */
const uint16_t FIELD_AUX_ASQ_STATUS_RESP2_OVERLOAD=0b1;  //Audio input is currently overloading ADC
const uint16_t FIELD_WB_ASQ_STATUS_RESP2_ALERT    =0b1;  //Alert tone is present
//FM_AGC_STATUS, AM_AGC_STATUS, WB_AGC_STATUS
const uint16_t FIELD_AGC_STATUS_RESP1_DISABLE_AGC=0b1;  //True if AGC disabled

//RDS Response bytes
const uint8_t PI_H = 4;  //Also "Block A"
const uint8_t PI_L = 5;
const uint8_t Block_B_H = 6;
const uint8_t Block_B_L = 7;
const uint8_t Block_C_H = 8;
const uint8_t Block_C_L = 9;
const uint8_t Block_D_H = 10;
const uint8_t Block_D_L = 11;
#endif /* SI4735_H_ */
