/*
 * Keyboard.cpp
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#include "Keyboard.h"
#include <glog/logging.h>
#include "MPR121.h"


Keyboard::Keyboard(I2C &i2c, uint8_t address) :
	mI2C(i2c),
	mAddress(address),
	mKeys(0),
	mKeysMutex(),
	mReadThread(nullptr),
	mReadThreadRunning(false)
{
	init();
	startReadThread();
}

Keyboard::~Keyboard() {

}

uint16_t Keyboard::readKeys()
{
    std::lock_guard<std::mutex> lk_guard(mKeysMutex);
	return mKeys;
}

void Keyboard::init()
{
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
      mI2C.writeRegByteSync(mAddress, MHD_RISING, 0x01);
      mI2C.writeRegByteSync(mAddress, NHD_AMOUNT_RISING, 0x01);
      mI2C.writeRegByteSync(mAddress, NCL_RISING, 0x00);
      mI2C.writeRegByteSync(mAddress, FDL_RISING, 0x00);

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
      mI2C.writeRegByteSync(mAddress, MHD_FALLING, 0x01);
      mI2C.writeRegByteSync(mAddress, NHD_AMOUNT_FALLING, 0x01);
      mI2C.writeRegByteSync(mAddress, NCL_FALLING, 0xFF);
      mI2C.writeRegByteSync(mAddress, FDL_FALLING, 0x02);

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
      mI2C.writeRegByteSync(mAddress, ELE0_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE0_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE1_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE1_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE2_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE2_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE3_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE3_RELEASE_THRESHOLD, RELEASE_THRESHOLD);

	  // TODO: enable setting these channels to capsense or GPIO
	  // for now they are all capsense
      mI2C.writeRegByteSync(mAddress, ELE4_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE4_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE5_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE5_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE6_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE6_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE7_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE7_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE8_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE8_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE9_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE9_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE10_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE10_RELEASE_THRESHOLD, RELEASE_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE11_TOUCH_THRESHOLD, TOUCH_THRESHOLD);
      mI2C.writeRegByteSync(mAddress, ELE11_RELEASE_THRESHOLD, RELEASE_THRESHOLD);

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
      mI2C.writeRegByteSync(mAddress, FILTER_CONFIG, 0x04);

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
      mI2C.writeRegByteSync(mAddress, ELECTRODE_CONFIG, 0x0C);

	  // Section F
	  // Description:
	  // These are the settings used for the Auto Configuration. They enable
	  // AUTO-CONFIG and AUTO_RECONFIG. In addition, they set the target
	  // rate for the baseline. The upper limit is set to 190, the target
	  // is set to 180 and the lower limit is set to 140.
	  // Variation:
	  // In most cases these values will never need to be changed, but if
	  // a case arises, a full description is found in application note AN3889.
	  //mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_CONTROL_0, 0x0B);
	  //mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_USL, 0x9C);
	  //mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_LSL, 0x65);
	  //mI2C.writeRegByteSync(mAddress, AUTO_CONFIG_TARGET_LEVEL, 0x8C);
}

void Keyboard::startReadThread()
{
	mReadThreadRunning = true;

    // create read thread object and start read thread
	mReadThread = new std::thread(&Keyboard::readThread, this);
}

void Keyboard::stopReadThread()
{
	mReadThreadRunning = false;

    if (mReadThread)
    {
        // wait for alarm maintenance thread to finish and delete maintenance thread object
    	mReadThread->join();

        delete mReadThread;
        mReadThread = nullptr;
    }
}

void Keyboard::readThread()
{
    while (mReadThreadRunning == true)
    {
        // default sleep interval
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        uint8_t byte0;
        uint8_t byte1;
        uint8_t oor0;
        uint8_t oor1;

        mI2C.readByteSync(mAddress, ELE0_ELE7_TOUCH_STATUS, byte0);
        mI2C.readByteSync(mAddress, ELE8_ELE11_ELEPROX_TOUCH_STATUS, byte1);

        mI2C.readByteSync(mAddress, ELE0_7_OOR_STATUS, oor0);
        mI2C.readByteSync(mAddress, ELE8_11_ELEPROX_OOR_STATUS, oor1);

        LOG(INFO) << "OOR0: " << std::hex << (int) oor0;
        LOG(INFO) << "OOR1: " << std::hex << (int) oor1 << std::dec;

        std::lock_guard<std::mutex> lk_guard(mKeysMutex);

        mKeys = byte1;
        mKeys = mKeys << 8;
        mKeys = mKeys | byte0;
        LOG(INFO) << "mKeys:" << std::hex << (int) mKeys;

    }
}
