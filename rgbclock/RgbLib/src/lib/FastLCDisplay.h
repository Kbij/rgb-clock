/*
** EPITECH PROJECT, 2021
** RgbClock
** File description:
** FastLCDisplay
*/

#ifndef FASTLCDISPLAY_H_
#define FASTLCDISPLAY_H_

#include "LCDisplayIf.h"
#include "I2C.h"
#include "FontType.h"
#include <mutex>

namespace Hardware
{
class FastLCDisplay: public LCDisplayIf
{
public:
	FastLCDisplay(I2C &i2c, uint8_t address);
	virtual ~FastLCDisplay();

	void initGraphic();
	void clearGraphicDisplay();

	bool writeGraphicText(uint8_t col, uint8_t row, std::string text, FontType font, int clearLength = 0);
	void rectangle(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, bool set, bool fill);

	void drawSignal(int8_t signal);
	void drawNTPState(bool ntpSync);
private:
    void init();
    bool waitForReady();
	bool isReady();
    I2C &mI2c;
    const uint8_t mAddress;
	std::mutex mDisplayMutex;
};
}
#endif /* !FASTLCDISPLAY_H_ */
