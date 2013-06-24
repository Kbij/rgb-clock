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

class KeyboardObserverIf
{
public:
    virtual ~KeyboardObserverIf() {};

    virtual void keyboardPressed(uint16_t value) = 0;
};
}

#endif /* KEYBOARDOBSERVERIF_H_ */
