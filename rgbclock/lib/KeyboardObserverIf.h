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

const uint8_t KEY_LEFT      = 0;
const uint8_t KEY_CENTRAL_L = 1;
const uint8_t KEY_CENTRAL_R = 2;
const uint8_t KEY_UP        = 3;
const uint8_t KEY_DOWN      = 4;
const uint8_t KEY_REL1      = 5;
const uint8_t KEY_REL2      = 6;
const uint8_t KEY_REL3      = 7;
const uint8_t KEY_REL4      = 8;

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
