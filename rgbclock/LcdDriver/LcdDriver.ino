#include <Arduino.h>
#include <U8g2lib.h>
#include "Wire.h"
#include "Commands.h"

U8G2_ST7920_192X32_F_6800 u8g2(U8G2_R0, 4, 5, 6, 7, 8, 9, 10, 11, /*enable=*/ 3, /*cs=*/ U8X8_PIN_NONE, /*dc=*/ 2, /*reset=*/ U8X8_PIN_NONE);

const uint8_t verdana_tf[341] U8G2_FONT_SECTION("verdana") =
  "\13\0\4\4\5\5\4\1\6\22\25\1\0\25\0\25\0\0\0\0\0\1\70\60\31\260j\272\14\252b\343"
  "\302P\11C\313\364\317\26\61*\342\206\225BS\0\61\20\255r\232\12\226\63\224\207\365\37=\370\340\0"
  "\62\35\260nz\216\61j\343d\224\221\200F\215\226-j\264\250I\13\17\232\64\371\340\7\4\63 \260"
  "jzP\71y\361\202\224\11\201e\213\32Kw\62\235Z\263eM\252zp\302I\63T\0\64!\262"
  "f:\315^\212\62!\211r%\312\21)F\246\24\241B\244\312\224*\363\340\7LK\327\15\0\65\34"
  "\260j:\234\370\244l\265\253\332\70q\42\354\250\331\262\7U=\70\341\244\31*\0\66 \260j\372\220"
  ")b\323\344\224\20\243\225\230q\362\342\201\251e\232\255*a\306\204\233F\353\16\1\67\34\260j\32\376"
  "\200\250\321\242F\213\232\64j\322\250I\243E\215\26\65i\324\244I\0\70)\262f\272\20\262r\363 "
  "\210!#\305\212\24+r\246P\63\225\253\334\24:R\254\204\61f&\14\31y\20\306\25CT\0\71"
  "#\261f\272\16\256j\363\304\214\221R\314\66+a\252\304\203 \37\31)[\326L\60C\215X\251C"
  "\7\0:\13\345\355\25\36\250\7\372@\1\0\0\0\4\377\377\0";

volatile bool commandReceived;
volatile bool commandProcessed;
volatile uint8_t receivedCommand;
const int MAX_DATA_SIZE = 25;
uint8_t receivedCommandData[MAX_DATA_SIZE];
int receivedCommandLength;

void setup()
{
    commandReceived = false;
    commandProcessed = true;

    u8g2.begin();
    //R/W pin
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);
    pinMode(13, OUTPUT);

    Serial.begin(115200);

    Wire.begin(4);                    // join i2c bus with address #4
    Wire.onReceive(receiveEvent);     // register event
    Wire.onRequest(requestEvent);
    Wire.setTimeout( 100 );

    u8g2.clearBuffer();				    // clear the internal memory
    u8g2.setFont(u8g2_font_6x10_tf);	// choose a suitable font
    u8g2.drawStr(1,15,"Waiting for I2C commands...");
    u8g2.sendBuffer();
}

void loop()
{
    if (commandReceived)
    {
        Serial.println("processing command");

        switch (receivedCommand)
        {
            case LCD_RESET:
            {
                Serial.println("LCD_RESET");
                lcdReset();
                break;
            }
            case LCD_CLEAR_SCREEN:
            {
                Serial.println("LCD_CLEAR_SCREEN");
                lcdClearScreen();
                break;
            }
            case LCD_CLEAR_REGION:
            {
                Serial.println("LCD_CLEAR_REGION");
                if (receivedCommandLength >= 4)
                {
                    int x1 = receivedCommandData[0];
                    int y1 = receivedCommandData[1];
                    int x2 = receivedCommandData[2];
                    int y2 = receivedCommandData[3];

                    lcdClearRegion(x1, y1, x2, y2);
                }
                break;
            }
            case LCD_WRITE_TEXT:
            {
                Serial.println("LCD_WRITE_TEXT");
                if (receivedCommandLength >= 5)
                {
                    String text;
                    int index = 4;
                    while(index < receivedCommandLength)
                    {
                        text += (char) receivedCommandData[index++];
                    }
                    lcdWriteText(receivedCommandData[0],  receivedCommandData[1], receivedCommandData[2], receivedCommandData[3], text);
                }
                break;
            }
            case LCD_RECTANGLE:
            {
                Serial.println("LCD_RECTANGLE");
                if (receivedCommandLength >= 5)
                {
                    int x1 = receivedCommandData[0];
                    int y1 = receivedCommandData[1];
                    int x2 = receivedCommandData[2];
                    int y2 = receivedCommandData[3];
                    int set = receivedCommandData[4];
                    int fill = receivedCommandData[5];

                    lcdRectangle(x1, y1, x2, y2, (bool) set, (bool) fill);
                }
                break;
            }
            default:
            {
                Serial.print("Unknown command received: ");
                Serial.println((int) receivedCommand);
            }
        }

        Serial.println("Command processed");
        commandReceived = false;
        commandProcessed = true;
    }
}

void receiveEvent(int bytesReceived)
{
    if (bytesReceived > 0)
    {
        if (commandProcessed)
        {
            receivedCommand = Wire.read();
            if (receivedCommand > 0)
            {
                int index = 0;
                while(Wire.available() > 0)
                {
                    if (index < MAX_DATA_SIZE)
                    {
                        receivedCommandData[index] = (uint8_t) Wire.read();
                        index++;
                    }
                    else
                    {
                        Wire.read();
                    }
                }

                receivedCommandLength = index;
                commandProcessed = false;
                commandReceived = true;
            }
        }
        else
        {
            //Command was not processed; read all data, and throw away
            while(Wire.available() > 0)
            {
                Wire.read();
            }
        }
    }
}

void requestEvent()
{
    Wire.write((uint8_t) commandProcessed);
}

void lcdReset()
{
    u8g2.clearDisplay();
}

void lcdClearScreen()
{
    u8g2.clearDisplay();
}

void lcdClearRegion(int x1, int y1, int x2, int y2)
{
    int width = abs(x2-x1);
    int height = abs(y2-y1);
    u8g2.setDrawColor(0);
    u8g2.drawBox(x1, y1, width, height);
    u8g2.sendBuffer();
}

void lcdWriteText(int size, int x, int y, int clearLength, String text)
{
    Serial.print("write text, x: ");
    Serial.print(x);
    Serial.print(", y: ");
    Serial.print(y);
    Serial.print(", ");
    int fontHeight = 0;

    if (size == LCD_FONT_SMALL)
    {
        Serial.print("small, '");
        u8g2.setFont(u8g2_font_6x10_tf);
        fontHeight = 8;
    }
    else if (size == LCD_FONT_LARGE)
    {
        Serial.print("large, '");
        u8g2.setFont(verdana_tf);
        fontHeight = 22;
    } else
    {
        Serial.print("small, '");
        u8g2.setFont(u8g2_font_6x10_tf);
        fontHeight = 8;
    }
    Serial.print(", clearlength: ");
    Serial.print(clearLength);

    Serial.print("[");
    Serial.print(text);
    Serial.println("]");

    char charArray[MAX_DATA_SIZE];
    text.toCharArray(charArray, sizeof(charArray));

    //Clear first, when '0'; calculate with text to draw
    if (clearLength == 0)
    {
        clearLength = u8g2.getStrWidth(charArray);
    }

    u8g2.setDrawColor(0);
    u8g2.drawBox(x - 1, y - 1, clearLength + 2, fontHeight + 2);

    u8g2.setFontMode(0);
    //Write text
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(x, y + fontHeight, charArray);
    u8g2.sendBuffer();
}

void lcdRectangle(int x1, int y1, int x2, int  y2, bool set, bool fill)
{
    int width = abs(x2-x1);
    int height = abs(y2-y1);
    Serial.print("rectangle, x1: ");
    Serial.print(x1);
    Serial.print(", y1: ");
    Serial.print(y1);
    Serial.print(", width: ");
    Serial.print(width);
    Serial.print(", height: ");
    Serial.print(height);
    Serial.print(", fill: ");
    Serial.println(fill);

    //u8g2.setFontMode(0);
    u8g2.setDrawColor(set ? 1 : 0);
    u8g2.drawBox(x1, y1, width, height);
    u8g2.sendBuffer();
}

