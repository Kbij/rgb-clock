/*
 * Keyboard.cpp
 *
 *  Created on: May 2, 2013
 *      Author: koen
 */

#include "Keyboard.h"
#include <glog/logging.h>
#include "MPR121.h"
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
const uint8_t TouchThre         = 10;   //15//30//10
const uint8_t ReleaThre         = 6;  //8//25//8

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

	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({SOFT_RESET_REG, 0x63}));// SOFT_RESET
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));



	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x5E, 0x00})); //Stop mode
	//touch pad baseline filter
	//rising
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x2B,0x01})); // MAX HALF DELTA Rising
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x2C,0x01})); // NOISE HALF DELTA Rising
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x2D,0x0E})); // NOISE COUNT LIMIT Rising
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x2E,0x00})); // DELAY LIMIT Rising

	//falling
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x2F,0x01})); // MAX HALF DELTA Falling
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x30,0x05})); // NOISE HALF DELTA Falling
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x31,0x01})); // NOISE COUNT LIMIT Falling
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x32,0x00})); // DELAY LIMIT Falling
	//touched

	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x33,0x00})); // Noise half delta touched
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x34,0x00})); // Noise counts touched
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x35,0x00})); // Filter delay touched

	//Touch pad threshold
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x41,TouchThre})); // ELE0 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x42,ReleaThre})); // ELE0 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x43,TouchThre})); // ELE1 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x44,ReleaThre})); // ELE1 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x45,TouchThre})); // ELE2 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x46,ReleaThre})); // ELE2 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x47,TouchThre})); // ELE3 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x48,ReleaThre})); // ELE3 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x49,TouchThre})); // ELE4 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4A,ReleaThre})); // ELE4 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4B,TouchThre})); // ELE5 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4C,ReleaThre})); // ELE5 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4D,TouchThre})); // ELE6 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4E,ReleaThre})); // ELE6 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x4F,TouchThre})); // ELE7 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x50,ReleaThre})); // ELE7 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x51,TouchThre})); // ELE8 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x52,ReleaThre})); // ELE8 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x53,TouchThre})); // ELE9 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x54,ReleaThre})); // ELE9 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x55,TouchThre})); // ELE10 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x56,ReleaThre})); // ELE10 RELEASE THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x57,TouchThre})); // ELE11 TOUCH THRESHOLD
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x58,ReleaThre})); // ELE11 RELEASE THRESHOLD

	//touch /release debounce
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x5B,0x00}));
	// response time = SFI(10) X ESI(8ms) = 80ms
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x5D,0x13}));
	//FFI=18
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({0x5C,0x80}));


	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({USL_REG, 202}));// Set VSL (USL)
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({TL_REG, 182}));// // Set Target
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({LSL_REG, 118})); // Set LSL

	/*
	 * FFI7..6: The FFI bits are the same as the FFI bits in register 0x5C for correct auto-configuration and reconfiguration operations.
	 * RETRY5..4:  Specifies the number of retries for autoconfiguration and autoreconfiguration if the configuration fails before setting OOR.
	 * BVA3..2: Fill the BVA bits same as the CL bits in ECR (0x5E) register.
	 * ARE: Auto-Reconfiguration Enable. 1: Enable, 0: Disable. When enabled, if the OOR is set for a channel after autoconfiguration,
	 * autoreconfiguration will operate on that channel on each sampling interval until the OOR is cleared.
     * ACE: Auto-Configuration Enable. 1: Enable, 0: Disable. When Enabled, the autoconfiguration will operate once at the beginning
	 * of the transition from Stop Mode to Run Mode. This includes search and update of the CDCx and CDTx for each enabled channel
	 * (if SCTS = 0).
	 * Example Code; write to 0x8F
	 * My Code: 0b00111011
	 */
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({AUTOCONF_CONTROL0, 0x8F}));

	/*
	 * CL1: 1
	 * CL0: 0 Baseline tracking enabled
	 * ELEPROX_EN1: 0
	 * ELEPROX_EN0: 0 Proximity disabled
	 * ELE_EN: 1111: Electrode 0 .. Electrode 11 enabled
	 * Debug: ELE_EN: 0010: Electrode 0 .. Electrode 1 enabled
	 * Example code: write to 0xCC
	 * My value 0b10000010
	*/
	mI2C.writeDataSync(mAddress,std::vector<uint8_t>({ECR_REG, 0xCC})); //Stop mode

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
        LOG(INFO) << "mKeys:" << std::hex << (int) mKeys;

    }
}
