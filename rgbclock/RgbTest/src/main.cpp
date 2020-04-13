/*
 * main.cpp
 *
 *  Created on: Aug 6, 2017
 *      Author: koen
 */

#include "gtest/gtest.h"
#include <glog/logging.h>
#include <gflags/gflags.h>


GTEST_API_ int main(int argc, char **argv)
{
	google::InitGoogleLogging("RgbClock Test");
	testing::InitGoogleTest(&argc, argv);
	google::ParseCommandLineFlags(&argc, &argv, true);

	FLAGS_logtostderr=1;
	//FLAGS_v = 3;

    int result = RUN_ALL_TESTS();

	google::ShutdownGoogleLogging();
	gflags::ShutDownCommandLineFlags();
	return result;
}




