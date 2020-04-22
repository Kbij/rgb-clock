/*
** EPITECH PROJECT, 2020
** RgbClock
** File description:
** UtilsTest
*/

#include "lib/Utils.h"
#include <gtest/gtest.h>


TEST(Utils, ReadWriteBinaryFile)
{
    std::vector<uint8_t> writeVector;
    for(uint8_t value = 0; value <=254; value++)
    {
        writeVector.push_back(value);
    }

    writeFile("testfile.bin",  writeVector);

    std::vector<uint8_t> readVector;
    readFile("testfile.bin", readVector);

    EXPECT_EQ(writeVector, readVector);
    readVector[10] = 11;

    EXPECT_NE(writeVector, readVector);

}