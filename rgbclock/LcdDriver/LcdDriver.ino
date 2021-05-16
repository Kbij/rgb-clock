#include <U8g2lib.h>
#include <Wire.h>
#include "verdana.h"
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
bool commandReceived;
bool commandProcessed;
char receivedCommand[255];
int receivedCommandLength;

void setup()
{
    commandReceived = false;
    commandProcessed = false;

    u8g2.begin();
    //R/W pin
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);

    Serial.begin(115200);
    Wire.begin(4);                    // join i2c bus with address #4
    Wire.onReceive(receiveEvent);     // register event
    Wire.onRequest(requestEvent);

    u8g2.clearBuffer();					      // clear the internal memory
    u8g2.setFont(u8g2_font_6x10_tf);	// choose a suitable font
    u8g2.drawStr(1,15,"Waiting for I2C commands...");
    u8g2.sendBuffer();
}

void loop()
{
    if (commandReceived)
    {
        commandProcessed = true;
    }
  /*
    Serial.println("Printing Hello world");

    u8g2.setFont(verdana);	// choose a suitable font
    u8g2.drawStr(30, 22,"18:00");	// write something to the internal memory
    // bijna ok: u8g2_font_courR08_tf
    u8g2.setFont(u8g2_font_6x10_tf    );	// choose a suitable font
    u8g2.drawStr(1,30,"Dit is een lange text ...");	// write something to the internal memory
    u8g2.sendBuffer();					// transfer internal memory to the display
    //u8g2.setFontMode(1);
    for(int i = 0; i > -180;--i)
    {
        u8g2.setDrawColor(0);
        u8g2.drawBox(0,22,160,10);
        u8g2.setDrawColor(1);
        u8g2.drawUTF8(1+i,30,"Dit is een lange text ...");	// write something to the internal memory
        u8g2.sendBuffer();					// transfer internal memory to the display


        delay(50);
    }
    Serial.println("Done ...");
    delay(1000);
    */
}

void receiveEvent(int bytesReceived)
{
  if (bytesReceived > 0)
  {
    uint8_t command = Wire.read();
    switch (command)
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
        if (bytesReceived >= 4)
        {
          int x1 = Wire.read();
          int y1 = Wire.read();
          int x2 = Wire.read();
          int y2 = Wire.read();

          lcdClearRegion(x1, y1, x2, y2);
        }

        break;
      }
      case LCD_WRITE_TEXT:
      {
        Serial.println("LCD_WRITE_TEXT");
        if (bytesReceived >= 5)
        {
          int size = Wire.read();
          int x = Wire.read();
          int y = Wire.read();
          String text;
          int index = 0;
          while(Wire.available() > 0)
          {
            text += (char) Wire.read(); // receive byte as a character
          }
          //text[index] = 0; //Close
          lcdWriteText(size, x, y, text);
        }
        break;
      }
      default:
      {
        Serial.print("Unknown command received: ");
        Serial.println(command);
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
}

void lcdWriteText(int size, int x, int y, String text)
{
  Serial.print("write text, x: ");
  Serial.print(x);
  Serial.print(", y: ");
  Serial.print(y);
  Serial.print(", ");
  int fontHeight = 0;

  if (size == LCD_FONT_SMALL)
  {
    Serial.print("small, ");
    u8g2.setFont(u8g2_font_6x10_tf);
    fontHeight = 8;
  }
  else if (size == LCD_FONT_LARGE)
  {
    Serial.print("large, ");
    u8g2.setFont(verdana_tf);
    fontHeight = 22;
  } else
  {
    Serial.print("small, ");
    u8g2.setFont(u8g2_font_6x10_tf);
    fontHeight = 8;
  }

  Serial.print(text);
  Serial.println("'");

  char charArray[255];
  text.toCharArray(charArray, sizeof(charArray));

  //Clear first
  /*
  int textWidth = u8g2.getStrWidth(charArray);
  u8g2.setDrawColor(0);
  u8g2.drawBox(x-1, y+1, textWidth+2, fontHeight+2);
  */

  u8g2.setFontMode(0);
  //Write text
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(x, y, charArray);
  u8g2.sendBuffer();
}


