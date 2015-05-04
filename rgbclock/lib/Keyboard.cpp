/*
 * Keyboard.cpp
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#include "Keyboard.h"
#include "MainboardControl.h"
#include <glog/logging.h>
#include "MPR121.h"
#include <bitset>
#include <pthread.h>

namespace Hardware
{
std::string binary(uint16_t number, uint8_t width)
{
	std::bitset<16> bitset(number);
	std::string result = bitset.to_string();

	return result;
}
const int LONG_MASK = 0x80;
const int LONG_MASKREPEAT = 0x100;
const int MONITORED_KEYS = 9;

Keyboard::Keyboard(I2C &i2c, uint8_t address, Hardware::MainboardControl &mainboardControl) :
	mI2C(i2c),
	mAttached(true), // We assume it is attached
	mAddress(address),
	mMainboardControl(mainboardControl),
	mKeyboardStateMutex(),
	mKeyboardState(KeyboardState::stNormal),
	mKeyHistory(0),
	mReadThread(nullptr),
	mReadThreadRunning(false),
	mKeyboardObservers(),
	mKeyboardObserversMutex(),
	mKeyboardWorkerRunning(false),
	mKeyboardWorkerThread(),
	mKeyboardQueue(),
	mKeyboardQueueMutex()
{
	mI2C.registerAddress(address, "MPR121");
	mKeyHistory.resize(MONITORED_KEYS, 0); // Monitoring 9 Keys
	init();
	startReadThread();
	startKeyboardWorkerThread();
}

Keyboard::~Keyboard()
{
	LOG(INFO) << "Keyboard destructor";
	mMainboardControl.removePromise(this);

	stopReadThread();
	stopKeyboardWorkerThread();
	LOG(INFO) << "Keyboard destructor exit";
}

void Keyboard::registerKeyboardObserver(KeyboardObserverIf *observer)
{
    if (observer)
    {
        std::lock_guard<std::recursive_mutex> lk_guard(mKeyboardObserversMutex);

        mKeyboardObservers.insert(observer);
    }
}

void Keyboard::unRegisterKeyboardObserver(KeyboardObserverIf *observer)
{
    if (observer)
    {
    	std::lock_guard<std::recursive_mutex> lk_guard(mKeyboardObserversMutex);

        mKeyboardObservers.erase(observer);
    }
}

void Keyboard::keyboardState(KeyboardState state)
{
    std::lock_guard<std::recursive_mutex> lk_guard(mKeyboardStateMutex);
	mKeyboardState = state;
}

bool Keyboard::isAttached()
{
	return mAttached;
}

std::string Keyboard::feederName() const
{
	return "Keyboard";
}

void Keyboard::init()
{
	  mI2C.writeRegByte(mAddress, SOFT_RESET, 0x63);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // These are the configuration values recommended by app note AN3944
	  // along with the description in the app note.

	  // Section A
	  // Description:
	  // This group of settings controls the filtering of the system
	  // when the data is greater than the baseline. The setting used
	  // allows the filter to act quickly and adjust for environmental changes.
	  // Additionally, if calibration happens to take place while a touch occurs,
	  // the value will self adjust very quickly. This auto-recovery or snap back
	  // provides repeated false negative for a touch detection.
	  // Variation:
	  // As the filter is sensitive to setting changes, it is recommended
	  // that users read AN3891 before changing the values.
	  // In most cases these default values will work.
      mI2C.writeRegByte(mAddress, MHD_RISING, 0x01);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      mI2C.writeRegByte(mAddress, NHD_AMOUNT_RISING, 0x01);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      mI2C.writeRegByte(mAddress, NCL_RISING, 0x00);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      mI2C.writeRegByte(mAddress, FDL_RISING, 0x00);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

	  // Section B
	  // Description:
	  // This group of settings controls the filtering of the system
	  // when the data is less than the baseline. The settings slow down
	  // the filter as the negative charge is in the same direction
	  // as a touch. By slowing down the filter, touch signals
	  // are "rejected" by the baseline filter. While at the same time
	  // long term environmental changes that occur slower than
	  // at a touch are accepted. This low pass filter both allows
	  // for touches to be detected properly while preventing false
	  // positives by passing environmental changes through the filter.
	  // Variation:
	  // As the filter is sensitive to setting changes, it is recommended
	  // that users read AN3891 before changing the values.
	  // In most cases these default values will work.
      mI2C.writeRegByte(mAddress, MHD_FALLING, 0x01);
      mI2C.writeRegByte(mAddress, NHD_AMOUNT_FALLING, 0x01);
      mI2C.writeRegByte(mAddress, NCL_FALLING, 0xFF);
      mI2C.writeRegByte(mAddress, FDL_FALLING, 0x02);

	  // Section C
	  // Description:
	  // The touch threshold registers set the minimum delta from the baseline
	  // when a touch is detected. The value 0x0F (or 15 in decimal) is
	  // an estimate of the minimum value for touch. Most electrodes will
	  // work with this value even if they vary greatly in size and shape.
	  // The value of 0x0A or 10 in the release threshold register allowed
	  // for hysteresis in the touch detection.
	  // Variation:
	  // For very small electrodes, smaller values can be used and for
	  // very large electrodes the reverse is true. One easy method is
	  // to view the deltas actually seen in a system and set the touch
	  // at 80% and release at 70% of delta for good performance.
      mI2C.writeRegByte(mAddress, ELE0_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE0_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE1_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE1_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE2_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE2_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE3_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE3_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE4_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE4_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE5_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE5_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE6_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE6_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE7_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE7_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE8_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE8_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE9_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE9_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE10_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE10_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE11_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByte(mAddress, ELE11_RELEASE_THRESHOLD, RELEASE_THRESHOLD);

	  // Section D
	  // Description:
	  // There are three settings embedded in this register so it is
	  // only necessary to pay attention to one. The ESI controls
	  // the sample rate of the device. In the default, the setting
	  // used is 0x00 for 1ms sample rate. Since the SFI is set to 00,
	  // resulting in 4 samples averaged, the response time will be 4 ms.
	  // Variation:
	  // To save power, the 1 ms can be increased to 128 ms by increasing
	  // the setting to 0x07. The values are base 2 exponential, thus
	  // 0x01 = 2ms, 0x02 = 4 ms; and so on to 0x07 = 128 ms. Most of
	  // the time, 0x04 results in the best compromise between power
	  // consumption and response time.
      mI2C.writeRegByte(mAddress, FILTER_CONFIG, 0x04);

	  // Section E
	  // Description:
	  // This register controls the number of electrodes being enabled
	  // and the mode the device is in. There are only two modes,
	  // Standby (when the value is 0x00) and Run (when the value of
	  // the lower bit is non-zero). The default value shown enables
	  // all 12 electrodes by writing decimal 12 or hex 0x0C to the register.
	  // Typically other registers cannot be changed while the part is running,
	  // so this register should always be written last.
	  // Variation:
	  // During debug of a system, this register will change between
	  // the number of electrodes and 0x00 every time a register needs
	  // to change. In a production system, this register will only need
	  // to be written when the mode is changed from Standby to Run or vice versa.
      mI2C.writeRegByte(mAddress, ELECTRODE_CONFIG, 0x0C);

	  // Section F
	  // Description:
	  // These are the settings used for the Auto Configuration. They enable
	  // AUTO-CONFIG and AUTO_RECONFIG. In addition, they set the target
	  // rate for the baseline. The upper limit is set to 190, the target
	  // is set to 180 and the lower limit is set to 140.
	  // Variation:
	  // In most cases these values will never need to be changed, but if
	  // a case arises, a full description is found in application note AN3889.
//	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_USL, 0x9C);
//	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_TARGET_LEVEL, 0x8C);
//	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_LSL, 0x65);
/*
      mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_USL, 0x1E);
	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_TARGET_LEVEL, 0x1B);
	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_LSL, 0x14);

	  mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_CONTROL_0, 0x0B);

      mI2C.writeRegByteSync(mAddress, ELECTRODE_CONFIG, 0x0C);
*/

}

void Keyboard::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = std::unique_ptr<std::thread>(new std::thread(&Keyboard::readThread, this));
}

void Keyboard::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();
        mReadThread.reset();
    }
}

void Keyboard::readThread()
{
	pthread_setname_np(pthread_self(), "KeyboardReader");

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	mMainboardControl.promiseWatchdog(this, 500);

    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        mMainboardControl.signalWatchdog(this);

        uint16_t keybValue = 0;

        // Must read high and low byte in one read
        mAttached = mAttached & mI2C.readData(mAddress, ELE0_ELE7_TOUCH_STATUS, keybValue);

        std::vector<Hardware::KeyInfo> keyboardInfo(MONITORED_KEYS);
        bool stateChange = false;

        for (int i = 0; i < MONITORED_KEYS; ++i)
        {

        	mKeyHistory[i] <<= 1;// Shift history 1 to the left
        	mKeyHistory[i] |= (keybValue & 0x01); // add 1 bit to the history

        	keybValue >>= 1; // Shift the current values 1 to the right

    		keyboardInfo[i].mShortPressed = false;
    		keyboardInfo[i].mReleased = false;
    		keyboardInfo[i].mLongPress = false;
    		keyboardInfo[i].mRepeat = false;
    		keyboardInfo[i].mPressed = (mKeyHistory[i] & 0x01);

    		// Key just pressed, or released
    		stateChange |= ((mKeyHistory[i] & 0b10) > 0) != ((mKeyHistory[i] & 0b01) >0) ;

        	if (!(mKeyHistory[i] & 0x01)) // if key is released
        	{
        		if (mKeyHistory[i] > 0x01)
        		{
        			if (mKeyHistory[i] < LONG_MASK) //short Pressed
            		{
            			//LOG(INFO) << "S" << i;
            			keyboardInfo[i].mShortPressed = true;
            		}
            		else
            		{
            			//LOG(INFO) << "R" << i;
            			keyboardInfo[i].mReleased = true; // Released after long press
            		}
        		}
        		mKeyHistory[i] = 0; // delete the history
        	}
        	if (mKeyHistory[i] > LONG_MASK) // Long press
        	{
        		//LOG(INFO) << "L" << i;
        		keyboardInfo[i].mLongPress = true;
        		stateChange = true;
        		if (mKeyHistory[i] > LONG_MASKREPEAT) // Long press repeat
        		{
            		keyboardInfo[i].mRepeat = true;
        		}

        	}

        }

        if (stateChange)
        {
        	/*
        	std::lock_guard<std::recursive_mutex> lk_guard1(mKeyboardObserversMutex);
            std::lock_guard<std::recursive_mutex> lk_guard2(mKeyboardStateMutex);
            auto initialKeyboardState = mKeyboardState;
            for (auto observer : mKeyboardObservers)
            {
            	if (initialKeyboardState == mKeyboardState)
            	{
            		KeyboardInfo keyboard(keyboardInfo, mKeyboardState);
                    observer->keyboardPressed(keyboard);
            	}
            }
            */
			std::lock_guard<std::mutex> lk_guard(mKeyboardQueueMutex);
    		KeyboardInfo keyboard(keyboardInfo, mKeyboardState);
        	mKeyboardQueue.push(keyboard);

        }
    }
}

void Keyboard::startKeyboardWorkerThread()
{
	mKeyboardWorkerRunning = true;

    // create worker thread and start it
	mKeyboardWorkerThread = std::unique_ptr<std::thread>(new std::thread(&Keyboard::keyboardWorkerThread, this));
}

void Keyboard::stopKeyboardWorkerThread()
{
	mKeyboardWorkerRunning = false;

    if (mKeyboardWorkerThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mKeyboardWorkerThread->join();
    	mKeyboardWorkerThread.reset();
    }
}

void Keyboard::keyboardWorkerThread()
{
	pthread_setname_np(pthread_self(), "KeyboardWorker");

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	// Radio off can take some time (processed from in this thread)
	mMainboardControl.promiseWatchdog(this, 5000);

    while (mKeyboardWorkerRunning == true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        // Lock the queue for a minimal of time
    	int queueSize = 0;
        {
			std::lock_guard<std::mutex> lk_guard(mKeyboardQueueMutex);
        	queueSize = mKeyboardQueue.size();
        }
    	while (queueSize > 0)
    	{
    		KeyboardInfo keyboardInfo;
    		{
    			std::lock_guard<std::mutex> lk_guard(mKeyboardQueueMutex);
    			keyboardInfo = mKeyboardQueue.front();
    			mKeyboardQueue.pop();
    		}

        	std::lock_guard<std::recursive_mutex> lk_guard(mKeyboardObserversMutex);
            for (auto observer : mKeyboardObservers)
            {
            	observer->keyboardPressed(keyboardInfo);
            }

            {
    			std::lock_guard<std::mutex> lk_guard(mKeyboardQueueMutex);
            	queueSize = mKeyboardQueue.size();
            }
    	}

    }
}

}
