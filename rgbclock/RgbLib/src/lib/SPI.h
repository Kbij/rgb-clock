/*
** RgbClock, 2020
** RgbClock
** File description:
** SPI
*/

#ifndef SPI_H_
#define SPI_H_
#include <string>
#include <vector>
#include <stdint.h>

namespace Hardware
{

class SPI
{
public:
	SPI(const std::string& deviceName);
	virtual ~SPI();

    bool writeData(const std::vector<uint8_t>& data);
	bool readWriteData(const std::vector<uint8_t>& writeData, std::vector<uint8_t>& readData);
private:
	bool openDevice();
	void closeDevice();	
	std::string mDeviceName;
	int mDeviceHandle;

};
}
#endif /* !SPI_H_ */
