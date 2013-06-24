/*
 * KeyboardObserverIf.h
 *
 *  Created on: Jun 23, 2013
 *      Author: koen
 */

#ifndef KEYBOARDOBSERVERIF_H_
#define KEYBOARDOBSERVERIF_H_
#include <stdint.h>

namespace Hardware
{
struct KeyInfo
{
	bool mPressed;
	bool mLongPress;
};

class KeyboardObserverIf
{
public:
    virtual ~KeyboardObserverIf() {};

    virtual void keyboardPressed(std::vector<Hardware::KeyInfo> keyboardInfo) = 0;
};
}

#endif /* KEYBOARDOBSERVERIF_H_ */
