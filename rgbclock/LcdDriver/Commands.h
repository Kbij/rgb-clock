#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>


const uint8_t LCD_RESET = 0x01;
const uint8_t LCD_CLEAR_SCREEN = 0x02;
const uint8_t LCD_CLEAR_REGION = 0x03;
const uint8_t LCD_WRITE_TEXT = 0x04;
const uint8_t LCD_RECTANGLE = 0x05;

const uint8_t LCD_FONT_SMALL = 0x01;
const uint8_t LCD_FONT_LARGE = 0x02;

#endif //COMMANDS_H