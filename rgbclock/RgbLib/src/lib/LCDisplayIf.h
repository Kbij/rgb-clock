/*
** Koen, 2021
** RgbClock
** File description:
** LCDisplayIf
*/

#ifndef LCDISPLAYIF_H_
#define LCDISPLAYIF_H_
#include "FontType.h"
#include <stdint.h>
#include <string>

namespace Hardware
{
class LCDisplayIf
{
public:
	virtual ~LCDisplayIf(){} ;

	virtual void initGraphic() = 0;
	virtual void clearGraphicDisplay() = 0;

	virtual bool writeGraphicText(uint8_t col, uint8_t row, std::string text, FontType font, int clearLength = 0) = 0;
    virtual void rectangle(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, bool set, bool fill) = 0;

	virtual void drawSignal(int8_t signal) = 0;
	virtual void drawNTPState(bool ntpSync) = 0;
};
}

#endif /* !LCDISPLAYIF_H_ */
