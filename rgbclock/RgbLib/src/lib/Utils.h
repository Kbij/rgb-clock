/*
** RgbClock
** File description:
** Utils
*/

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>
#include <stdint.h>

std::string vectorToHexString(const std::vector<uint8_t>& vector, bool tryString = false, bool blockMode = false);
bool readFile(const std::string& fileName, std::vector<uint8_t>& contents);
bool writeFile(const std::string& fileName, const std::vector<uint8_t>& contents);

#endif /* !UTILS_H_ */
