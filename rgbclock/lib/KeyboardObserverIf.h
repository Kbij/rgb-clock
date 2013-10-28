/*
 * KeyboardObserverIf.h
 *
 *  Created on: Jun 23, 2013
 *      Author: koen
 */

#ifndef KEYBOARDOBSERVERIF_H_
#define KEYBOARDOBSERVERIF_H_
#include <stdint.h>
#include <vector>

const uint8_t KEY_1         = 0;
const uint8_t KEY_CENTRAL_L = 1;
const uint8_t KEY_3         = 4;
const uint8_t KEY_4         = 3;
const uint8_t KEY_5         = 2;
const uint8_t KEY_CENTRAL_R = 5;
const uint8_t KEY_UP        = 6;
const uint8_t KEY_DOWN      = 7;

namespace Hardware
{

enum class KeyboardState
{
	stNormal,
	stAlarmEdit,
	stAlarmActive
};

struct KeyInfo
{
	bool mShortPressed;
	bool mLongPress;
	bool mReleased;
	bool mPressed;
	bool mRepeat;
};

class KeyboardObserverIf
{
public:
    virtual ~KeyboardObserverIf() {};

    virtual void keyboardPressed(const std::vector<Hardware::KeyInfo>& keyboardInfo, KeyboardState state) = 0;
};
}

#endif /* KEYBOARDOBSERVERIF_H_ */
