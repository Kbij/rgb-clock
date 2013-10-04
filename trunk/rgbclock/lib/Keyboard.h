/*
 * Keyboard.h
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include "I2C.h"
#include "KeyboardObserverIf.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <set>

namespace Hardware
{

class Keyboard {
public:
	Keyboard(I2C &i2c, uint8_t address);
	virtual ~Keyboard();

	void registerKeyboardObserver(KeyboardObserverIf *observer);
    void unRegisterKeyboardObserver(KeyboardObserverIf *observer);

    void keyboardState(KeyboardState state);

    bool isAttached();
private:
	void init();
	void startReadThread();
	void stopReadThread();
	void readThread();

	I2C &mI2C;
	std::atomic_bool mAttached;
	const uint8_t mAddress;
	std::recursive_mutex mKeyboardStateMutex;
	KeyboardState mKeyboardState;
	std::vector<uint16_t> mKeyHistory;
    std::thread* mReadThread;
    std::atomic_bool mReadThreadRunning;
    std::set<KeyboardObserverIf*> mKeyboardObservers;
    std::recursive_mutex mKeyboardObserversMutex;

};

}
#endif /* KEYBOARD_H_ */
