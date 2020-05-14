/*
** RgbClock
** File description:
** DABCommandsTest
*/

//#include "lib/DABCommands.h"
#include "gtest/gtest.h"
#include "glog/stl_logging.h"
#include "glog/logging.h"

// TEST(DABCommands, FreqList)
// {
//     std::vector<uint8_t> rawList ({0x00, 0x00, 0x00, 0x00, 0x02, 0x00 , 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x11, 0x22, 0x33, 0x44});
//     Hardware::FrequencyList list(rawList);
//     EXPECT_EQ(2, list.mFrequencies.size());
//     LOG(INFO) << "List: " << list.toString();
//     EXPECT_EQ(2018915346, list.mFrequencies[0]);
//     EXPECT_EQ(1144201745, list.mFrequencies[1]);
// }
