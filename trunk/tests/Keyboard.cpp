/*
 * Keyboard.cpp
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#include "Keyboard.h"
#include <glog/logging.h>
const uint8_t TOUCH_STATUS0     = 0x00;
const uint8_t TOUCH_STATUS1     = 0x01;
const uint8_t AUTOCONF_CONTROL0 = 0x7B;
const uint8_t AUTOCONF_CONTROL1 = 0x7C;
const uint8_t USL_REG           = 0x7D;
const uint8_t LSL_REG           = 0x7E;
const uint8_t TL_REG            = 0x7F;
const uint8_t SOFT_RESET_REG    = 0x80;
const uint8_t ECR_REG           = 0x5E;
const uint8_t OOR0_REG          = 0x02;
const uint8_t OOR1_REG          = 0x03;

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
	/* Using Auto Config (See Application note: AN3889.pdf
	 * Vdd= 3.3
	 * VSL = (Vdd - 0.7)/Vdd * 256
	 *     = (3.3 - 0.7)/3.3 * 256 = 202
	 * Target = VSL * 0.9
	 *        = 202 * 0.9 = 182
	 * LSL = Target * 0.65
	 *     = 182 * 0.65 = 118
	 * Autoconfig Control register (AUTOCONF_CONTROL0) = 0x0B
	 *
	 * Check: Auto of range status register (must be all 0)
	 */
//	Write SOFT_RESET with 0x63 asserts soft reset.
	std::vector<uint8_t> buffer;

	buffer.push_back(SOFT_RESET_REG); // SOFT_RESET
	buffer.push_back(0x63);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

/*
 *         byte1 = AB;
 *
 */


	buffer.push_back(USL_REG); // Set VSL
	buffer.push_back(202);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();

	buffer.push_back(TL_REG); // Set Target
	buffer.push_back(182);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();

	buffer.push_back(LSL_REG); // Set LSL
	buffer.push_back(118);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();


	buffer.push_back(AUTOCONF_CONTROL0); // Set AUTOCONF_CONTROL0
	/*
	 * FFI7..6: The FFI bits are the same as the FFI bits in register 0x5C for correct auto-configuration and reconfiguration operations.
     * RETRY5..4:  Specifies the number of retries for autoconfiguration and autoreconfiguration if the configuration fails before setting OOR.
     * BVA3..2: Fill the BVA bits same as the CL bits in ECR (0x5E) register.
     * ARE: Auto-Reconfiguration Enable. 1: Enable, 0: Disable. When enabled, if the OOR is set for a channel after autoconfiguration,
     * autoreconfiguration will operate on that channel on each sampling interval until the OOR is cleared.
	 * ACE: Auto-Configuration Enable. 1: Enable, 0: Disable. When Enabled, the autoconfiguration will operate once at the beginning
     * of the transition from Stop Mode to Run Mode. This includes search and update of the CDCx and CDTx for each enabled channel
     * (if SCTS = 0).
	 */
	buffer.push_back(0b00111011);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();

	buffer.push_back(ECR_REG); // Set ECR_REG
	/*
	 * CL1: 1
	 * CL0: 0 Baseline tracking enabled
	 * ELEPROX_EN1: 0
	 * ELEPROX_EN0: 0 Proximity disabled
	 * ELE_EN: 1111: Electrode 0 .. Electrode 11 enabled
	 * Debug: ELE_EN: 0010: Electrode 0 .. Electrode 1 enabled
	 */
	buffer.push_back(0b10000010);
	mI2C.writeDataSync(mAddress, buffer);
	buffer.clear();



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

        mI2C.readByteSync(mAddress, TOUCH_STATUS0, byte0);
        mI2C.readByteSync(mAddress, TOUCH_STATUS1, byte1);

        mI2C.readByteSync(mAddress, OOR0_REG, oor0);
        mI2C.readByteSync(mAddress, OOR1_REG, oor1);

        LOG(INFO) << "OOR0: " << std::hex << (int) oor0;
        LOG(INFO) << "OOR1: " << std::hex << (int) oor1 << std::dec;

        std::lock_guard<std::mutex> lk_guard(mKeysMutex);

        mKeys = byte1;
        mKeys = mKeys << 8;
        mKeys = mKeys | byte0;
    }
}
