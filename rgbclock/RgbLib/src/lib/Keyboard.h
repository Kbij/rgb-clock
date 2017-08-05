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
#include "WatchdogFeederIf.h"
#include <stdint.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <set>
#include <memory>
#include <queue>

namespace Hardware
{
class MainboardControl;

class Keyboard: public Hardware::WatchdogFeederIf {
public:
	Keyboard(I2C &i2c, uint8_t address, Hardware::MainboardControl &mainboardControl);
	virtual ~Keyboard();

	void registerKeyboardObserver(KeyboardObserverIf *observer);
    void unRegisterKeyboardObserver(KeyboardObserverIf *observer);

    void keyboardState(KeyboardState state);

    bool isAttached();

    std::string feederName() const;
private:
	void init();
	void startReadThread();
	void stopReadThread();
	void readThread();

	void startKeyboardWorkerThread();
	void stopKeyboardWorkerThread();
	void keyboardWorkerThread();

	I2C &mI2C;
	std::atomic_bool mAttached;
	const uint8_t mAddress;
    Hardware::MainboardControl &mMainboardControl;
	std::recursive_mutex mKeyboardStateMutex;
	KeyboardState mKeyboardState;
	std::vector<uint16_t> mKeyHistory;
    std::unique_ptr<std::thread> mReadThread;
    std::atomic_bool mReadThreadRunning;
    std::set<KeyboardObserverIf*> mKeyboardObservers;
    std::recursive_mutex mKeyboardObserversMutex;

    std::atomic_bool mKeyboardWorkerRunning;
    std::unique_ptr<std::thread> mKeyboardWorkerThread;
    std::queue<KeyboardInfo> mKeyboardQueue;
    std::mutex	mKeyboardQueueMutex;
};

}
#endif /* KEYBOARD_H_ */
