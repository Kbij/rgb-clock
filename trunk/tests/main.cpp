/*
 * main.cpp
 *
 *  Created on: Mar 8, 2013
 *      Author: koen
 */
#include "I2C.h"
#include "RgbLed.h"
#include "IOExpander.h"
#include "LCDisplay.h"
#include "LightSensor.h"
#include "ClockDisplay.h"

#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <iostream>
#include <stdio.h>
//#include <cstdlib>
#include <chrono>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
//#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <time.h>


DEFINE_bool(daemon, false, "Run rgbclock as Daemon");
DEFINE_string(pidfile,"","Pid file when running as Daemon");

const uint8_t PCA9685_ADDRESS = 0b01000000;
const uint8_t DISPLAY_ADDRESS = 0x20;
const uint8_t LIGHTSENSOR_ADDRESS = 0x29;

void signal_handler(int sig);
void daemonize();
bool runMain = true;

int pidFilehandle;

void signal_handler(int sig)
{
	switch(sig)
    {
		case SIGHUP:
			LOG(WARNING) << "Received SIGHUP signal.";
			break;
		case SIGINT:
		case SIGTERM:
			LOG(INFO) << "Daemon exiting";
			runMain = false;
			break;
		default:
			LOG(WARNING) << "Unhandled signal " << strsignal(sig);
			break;
    }
}

void daemonize()
{
    int pid, sid, i;
    char str[10];
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Check if parent process id is set */
    if (getppid() == 1)
    {
        /* PPID exists, therefore we are already a daemon */
        return;
    }

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */

    /* Fork*/
    pid = fork();

    if (pid < 0)
    {
        /* Could not fork */
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        /* Child created ok, so exit parent process */
    	LOG(INFO) << "Child process created: " <<  pid;
        exit(EXIT_SUCCESS);
    }


    /* Get a new process group */
    sid = setsid();

    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* close all descriptors */
    for (i = getdtablesize(); i >= 0; --i)
    {
        close(i);
    }

    /* Route I/O connections */

    /* Open STDIN */
    i = open("/dev/null", O_RDWR);

    /* STDOUT */
    dup(i);

    /* STDERR */
    dup(i);

//    chdir(rundir); /* change running directory */

    /* Ensure only one copy */
    pidFilehandle = open(FLAGS_pidfile.c_str(), O_RDWR|O_CREAT, 0600);

    if (pidFilehandle == -1 )
    {
        /* Couldn't open lock file */
    	LOG(ERROR) << "Could not open PID lock file " << FLAGS_pidfile << ", exiting";
        exit(EXIT_FAILURE);
    }

    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1)
    {
        /* Couldn't get lock on lock file */
    	LOG(WARNING) << "Could not lock PID lock file " << FLAGS_pidfile << ", exiting";
        exit(EXIT_FAILURE);
    }

    /* Get and format PID */
    sprintf(str,"%d\n",getpid());

    /* write pid to lockfile */
    write(pidFilehandle, str, strlen(str));
}

int main (int argc, char* argv[])
{
    umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //User: r/w, Group: r, Other: r
	google::InitGoogleLogging("RGBClock");
	//FLAGS_minloglevel = 1;

	std::string usage("This program implements a alarm clock. Sample usage:\n");
	usage += argv[0];
	google::SetUsageMessage(usage);
	google::ParseCommandLineFlags(&argc, &argv, true);

	if (FLAGS_daemon)
	{
		daemonize();
	}

	LOG(INFO) << "Test application for I2C Bus";
	LOG(INFO) << "============================";
	try
	{
//		I2C i2c;
	//	RgbLed rgbLed(i2c, PCA9685_ADDRESS);

/*
		std::string inputValue;
		do
		{
			std::cout << "Please enter On Ratio ('x' to quit): ";
			std::cin >> inputValue;
			std::transform(inputValue.begin(), inputValue.end(), inputValue.begin(), ::tolower);
			if (inputValue.size() >= 2)
			{
				std::string value = inputValue.substr(1, inputValue.size() - 1);
				std::cout << "Value:" << value << std::endl;
				int intValue = atoi(value.c_str());
				if ((intValue >=0 ) && (intValue <= 100))
				{

					switch (inputValue[0])
					{
						case 'r': 	rgbLed.red(intValue);
									break;
						case 'g':	rgbLed.green(intValue);
									break;
						case 'b': 	rgbLed.blue(intValue);
									break;
						case 'h': 	rgbLed.hue(intValue);
									break;
						case 's': 	rgbLed.saturation(intValue);
									break;
						case 'l': 	rgbLed.luminance(intValue);
									break;

					}
					rgbLed.write();
				}
			}
		} while ( inputValue != "x");
*/
/*
		rgbLed.hue(200);
		rgbLed.saturation(4000);

		do{

			rgbLed.pwrOn();

			for (int i = 0; i < 4000; ++i)
			{
				std::chrono::milliseconds dura( 2 );
				std::this_thread::sleep_for( dura );
				rgbLed.luminance(i);
				rgbLed.write();
				if (!runMain) break;
			}

			for (int i = 4000; i > 0; --i)
			{
				std::chrono::milliseconds dura( 2 );
				std::this_thread::sleep_for( dura );
				rgbLed.luminance(i);
				rgbLed.write();
				if (!runMain) break;
			}

//			std::chrono::milliseconds dura( 1000 );
//			std::this_thread::sleep_for( dura );

		} while (runMain);

		rgbLed.pwrOff();
		*/
		//uint8_t counter = 0;
		//IOExpander ioExpander(i2c, 0x20);
/*
		LCDisplay display(i2c, 0x20);
		display.clearDisplay();
		display.writeText(0,"Test");

		do
		{
			std::chrono::milliseconds dura( 1000 );
			std::this_thread::sleep_for( dura );

		} while (runMain);
*/
/*
		I2C i2c;
		RgbLed rgbLed(i2c, PCA9685_ADDRESS);

		LCDisplay display(i2c, DISPLAY_ADDRESS);
		display.initGraphic();
    	display.clearGraphicDisplay();

    	LightSensor lightSensor(i2c, 0x29);

		rgbLed.hue(200);
		rgbLed.saturation(4000);
		rgbLed.pwrOn();

		do{
			std::chrono::milliseconds dura( 2000 );
			std::this_thread::sleep_for( dura );
			double lux = lightSensor.lux();
			rgbLed.luminance((lux * 10)+30);
			rgbLed.write();
			std::stringstream stream;
			stream << lux;
			display.writeGraphicText(0, 0, stream.str(), FontType::Courier15);

		} while (runMain);
*/
		I2C i2c;
		RgbLed rgbLed(i2c, PCA9685_ADDRESS);

		ClockDisplay clockDisplay(i2c, DISPLAY_ADDRESS, LIGHTSENSOR_ADDRESS);
		clockDisplay.showClock();

		rgbLed.hue(200);
		rgbLed.saturation(4000);
		rgbLed.pwrOn();
		struct tm nextAlarm;
		nextAlarm.tm_hour = 6;
		nextAlarm.tm_min = 59;
		uint8_t counter = 0;
		clockDisplay.showSignal(100);
		do{
			std::chrono::milliseconds dura( 200 );
			std::this_thread::sleep_for( dura );
			clockDisplay.showNextAlarm(nextAlarm);
			clockDisplay.showVolume(counter);
			clockDisplay.showSignal(counter);

			counter++;
			if (counter > 100)
			{
				counter = 0;
			}
		} while (runMain);

	}
	catch (std::string caught)
	{
		LOG(ERROR) << "Failed to open I2C port:" << caught;
	}

	close(pidFilehandle);
	return EXIT_SUCCESS;
}

