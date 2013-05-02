/*
 * Keyboard.h
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include "I2C.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>

class Keyboard {
public:
	Keyboard(I2C &i2c, uint8_t address);
	virtual ~Keyboard();

	uint16_t readKeys();
private:
	void init();
	void startReadThread();
	void stopReadThread();
	void readThread();

	I2C &mI2C;
	const uint8_t mAddress;
	uint16_t mKeys;
	std::mutex mKeysMutex;
    std::thread* mReadThread;
    std::atomic_bool mReadThreadRunning;

};

#endif /* KEYBOARD_H_ */
