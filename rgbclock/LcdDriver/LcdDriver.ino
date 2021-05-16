#include <U8g2lib.h>
#include "verdana.h"
#include <Wire.h>




//U8G2_ST7920_192X32_F_8080(rotation, d0, d1, d2, d3, d4, d5, d6, d7, enable, cs, dc [, reset]) [full framebuffer, size = 768
U8G2_ST7920_192X32_F_6800 u8g2(U8G2_R0, 4, 5, 6, 7, 8, 9, 10, 11, /*enable=*/ 3, /*cs=*/ U8X8_PIN_NONE, /*dc=*/ 2, /*reset=*/ U8X8_PIN_NONE);
//U8G2_ST7920_192X32_F_8080
//U8G2_ST7920_192X32_F_6800
void setup()
{
    u8g2.begin();
    Serial.begin(115200);
   // u8g2.setFontMode(1);
//    Wire.begin(4);                // join i2c bus with address #4
//    Wire.onReceive(receiveEvent); // register event
}

void loop()
{
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);

    Serial.println("Printing Hello world");

    u8g2.clearBuffer();					// clear the internal memory
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
}

void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}