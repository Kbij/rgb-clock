/*
** EPITECH PROJECT, 2020
** RgbCLock
** File description:
** SPI
*/

#include "SPI.h"
#include "Utils.h"
#include <exception>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <glog/logging.h>
//https://raspberry-projects.com/pi/programming-in-c/spi/using-the-spi-interface
//https://docs.huihoo.com/doxygen/linux/kernel/3.7/structspi__ioc__transfer.html

namespace
{
uint32_t SPEED = 500000;
}

namespace Hardware
{
SPI::SPI(const std::string& deviceName):
	mDeviceName(deviceName),
	mDeviceHandle()
{
	openDevice();
}

SPI::~SPI()
{
	closeDevice();
}

bool SPI::openDevice()
{
	LOG(INFO) << "Open SPI device: " << mDeviceName;
	// Open port for reading and writing
	if ((mDeviceHandle = open(mDeviceName.c_str(), O_RDWR)) < 0)
	{
		std::string  ex("Failed to open bus (" + mDeviceName + "): ");
		ex += strerror(errno);
		throw std::runtime_error(ex);
	}

	uint8_t mode = SPI_MODE_0;
	uint8_t bits = 8;
    uint32_t speed = SPEED;
	/*
	 * spi mode
	 */
	int ret = ioctl(mDeviceHandle, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
    {
		LOG(ERROR) << "can't set spi mode";
		close(mDeviceHandle);
        return false;
    }
	ret = ioctl(mDeviceHandle, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
    {
        LOG(ERROR) <<  "can't get spi mode";
		close(mDeviceHandle);
        return false;
    }

	/*
	 * bits per word
	 */
	ret = ioctl(mDeviceHandle, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
    {
		LOG(ERROR) << "can't set bits per word";
		close(mDeviceHandle);
        return false;
    }

	ret = ioctl(mDeviceHandle, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
    {
        LOG(ERROR) << "can't get bits per word";
		close(mDeviceHandle);
        return false;
    }

	/*
	 * max speed hz
	 */
	ret = ioctl(mDeviceHandle, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
    {
	    LOG(ERROR) << "can't set max speed hz";
		close(mDeviceHandle);
        return false;
    }

	ret = ioctl(mDeviceHandle, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
    {
        LOG(ERROR) << "can't get max speed hz";
		close(mDeviceHandle);
        return false;
    }

	return true;
}

void SPI::closeDevice()
{
	LOG(INFO) << "Closing SPI device: " << mDeviceName;
	close(mDeviceHandle);
}

bool SPI::writeData(const std::vector<uint8_t>& data)
{
	std::vector<uint8_t> readData;
	return readWriteData(data, readData);
}

bool SPI::readWriteData(const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData)
{
	VLOG(3) << "SPI write bytes: " << writeData.size() << ", read bytes: " << readData.size();
	VLOG(30) << "SPI Write data:\n" << vectorToHexString(writeData, false, true);
	bool result = false;

	//Only read, or write/read
	int commandCount = readData.size() == 0 ? 1 : 2;
	VLOG(3) << "SPI Command count: " << commandCount;

	//Read and write command
	struct spi_ioc_transfer xfer[commandCount];
	memset(&xfer[0], 0, sizeof(xfer[0]));
	if (commandCount > 1) memset(&xfer[1], 0, sizeof(xfer[1]));

	//Fill the write command
	xfer[0].tx_buf = (long long unsigned int) writeData.data();
	xfer[0].len = writeData.size();
	xfer[0].speed_hz = SPEED;
	xfer[0].cs_change = 0; //Keep CS
	xfer[0].bits_per_word = 8;
	xfer[0].delay_usecs = 0;

	if (commandCount > 1)
	{
		//Fill the read command
		xfer[1].rx_buf = (long long unsigned int) readData.data();
		xfer[1].len = readData.size();
		xfer[1].speed_hz = SPEED;
		xfer[1].cs_change = 0; //Keep CS
		xfer[1].bits_per_word = 8;
		xfer[1].delay_usecs = 0;
	}

	auto res = ioctl(mDeviceHandle, SPI_IOC_MESSAGE(commandCount), xfer);
	if (res < 0)
	{
		LOG(ERROR) << "ioctl error: " << strerror(errno);
		result = false;
	}
	else
	{
		VLOG(3) << "SPI ioctl result: " << (int) res;
		VLOG(30) << "SPI Data read:\n" << vectorToHexString(readData, false, true);
		result = true;
	}

	return result;
}
}
