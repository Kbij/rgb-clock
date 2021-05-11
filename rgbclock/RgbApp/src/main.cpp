/*
 * Raspberry Pi Ultimate Alarm Clock
 */
#include "lib/I2C.h"
#include "lib/Si4684.h"
#include "lib/DABReceiver.h"
#include "lib/RTC.h"
#include "lib/MainboardControl.h"
#include "lib/IOExpander.h"
#include "lib/LCDisplay.h"
#include "lib/LCDBacklight.h"
#include "lib/Light.h"
#include "lib/SystemClock.h"
#include "AlarmClock.h"
#include "Config.h"
#include "AlarmManager.h"

#include <string>
#include "glog/logging.h"
#include "gflags/gflags.h"

#include <chrono>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <pthread.h>
#include <iostream>
#include <fstream>

DEFINE_bool(daemon, false, "Run rgbclock as Daemon");
DEFINE_string(pidfile,"","Pid file when running as Daemon");
DEFINE_bool(i2cstatistics, false, "Print I2C statistics");
DEFINE_bool(disablewatchdog, false, "Disable the watchdog");
DEFINE_string(configfile,"settings.xml","XML file containing the addresses of all IC's");
DEFINE_string(alarmfile,"alarms.xml","XML file containing the definition of alarms");
DEFINE_bool(dabscan, false, "Perform DAB Scan");
DEFINE_string(rootfolder, ".", "Root folder for DAB images");


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
		case SIGKILL:
			LOG(INFO) << "Daemon exiting";
			runMain = false;
			break;
		default:
			LOG(WARNING) << "Unhandled signal " << strsignal(sig);
			break;
    }
}

void registerSignals()
{
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigaddset(&newSigSet, SIGKILL);  /* Netbeans terminate process */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */
}

void daemonize()
{
    int pid, sid, i;
    char str[10];

    /* Check if parent process id is set */
 //   if (getppid() == 1)
 //   {
 //   	std::cout << "Already daemon" << std::endl;
 //       /* PPID exists, therefore we are already a daemon */
 //       return;
 //   }

    /* Fork*/
    pid = fork();
    std::cout << "Forked !!" << std::endl;
    if (pid < 0)
    {
        /* Could not fork */
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
    	std::cout << "Exit parent" << std::endl;
        /* Child created ok, so exit parent process */
        exit(EXIT_SUCCESS);
    }

    /* Get a new process group */
    sid = setsid();
    std::cout << "This is the forked process" << std::endl;
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
    std::cout << "pid file wirtten" << std::endl;
}

int main (int argc, char* argv[])
{
	std::string usage("Raspberry Pi Ultimate Alarm Clock. Sample usage:\n");
	usage += argv[0];
	gflags::SetUsageMessage(usage);
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	std::cout << "Starting, registering signals..." << std::endl;

	registerSignals();

	if (FLAGS_daemon)
	{
		std::cout << "Daemonize..." << std::endl;
		daemonize();
	}
	if (FLAGS_dabscan)
	{
		try
		{
			google::InitGoogleLogging("RGBClock");
    		Hardware::I2C i2c;

			// Create the config object; load the XML file with the settings
			App::Config config(FLAGS_configfile);
			if (!config.errorFree())
			{
				LOG(ERROR) << "Errors found in the config, exiting";
				// No errorfree load; exit the application
				return EXIT_FAILURE;
			}
			const App::SystemConfig& systemConfig = config.systemConfig();
			Hardware::SPI spi("/dev/spidev0.0");
			Hardware::MainboardControl mainboardControl(i2c, systemConfig.mHardwareRevision, systemConfig.mCentralIO, !FLAGS_disablewatchdog);
			Hardware::Si4684 si4684(spi, &mainboardControl);
			Hardware::DABReceiver dabReceiver(&si4684, &mainboardControl, 0, 0, 0);
			dabReceiver.serviceScan();
		}
		catch(const std::exception& ex)
		{
			std::cerr << "Exception performing DAB scan: " << ex.what() << std::endl;
		}
		
		return EXIT_SUCCESS;
	}

    try
    {
		// Start the RTC clock first; need to have a valid date/time for logging
		// Don't use any glog functions @constructor time of RTC
		google::InitGoogleLogging("RGBClock");

    	Hardware::I2C i2c;
        Hardware::RTC rtc(i2c, 0x68);
		Hardware::SPI spi("/dev/spidev0.0");

		LOG(INFO) << "Raspberry Pi Ultimate Alarm Clock";
		LOG(INFO) << "=================================";

		// Create the config object; load the XML file with the settings
		App::Config config(FLAGS_configfile);
		if (!config.errorFree())
		{
			LOG(ERROR) << "Errors found in the config, exiting";
			// No errorfree load; exit the application
			return EXIT_FAILURE;
		}

    	i2c.registerAddresses(config);

		std::map<std::string, std::unique_ptr<App::AlarmClock>> startedUnits;
		std::map<std::string, std::unique_ptr<Hardware::Light>> startedLights;

		const std::map<std::string, App::UnitConfig>& configuredUnits = config.configuredUnits();
		const App::SystemConfig& systemConfig = config.systemConfig();
		LOG(INFO) << "Number of configured units: " << configuredUnits.size();

		try
		{
			// Reset all PCA9685's
			i2c.writeByte(0x00, 0x06); // General Call Address, Send SWRST data byte 1):

			Hardware::MainboardControl mainboardControl(i2c, systemConfig.mHardwareRevision, systemConfig.mCentralIO, !FLAGS_disablewatchdog);
			Hardware::Si4684 si4684(spi, &mainboardControl);
			si4684.reset();
			std::this_thread::sleep_for( std::chrono::seconds(1));
    		Hardware::Si4684Settings settings;
			settings.BootFile = FLAGS_rootfolder + R"(/firmware/rom00_patch.016.bin)";
 			settings.DABFile = FLAGS_rootfolder + R"(/firmware/dab_radio.bin)";
			si4684.init(settings);

			Hardware::DABReceiver dabReceiver(&si4684, &mainboardControl, systemConfig.mFrequencyId, systemConfig.mServiceId, systemConfig.mComponentId);
			Hardware::SystemClock systemClock;
			App::AlarmManager alarmManager(FLAGS_alarmfile, config.units(), mainboardControl, systemClock);

			do{

				for (const auto& configUnit : configuredUnits)
				{
					if (i2c.probeAddress(configUnit.second.mKeyboard))
					{
						if (startedUnits.find(configUnit.first) == startedUnits.end())
						{
							LOG(INFO) << "Creating clock unit: " << configUnit.first;
							// Unit not found; create a unit
							startedUnits[configUnit.first] = std::unique_ptr<App::AlarmClock>(new App::AlarmClock(i2c, rtc, dabReceiver, systemConfig, alarmManager, mainboardControl, configUnit.second));
						}

						if (i2c.probeAddress(configUnit.second.mLight))
						{
							if (!startedUnits[configUnit.first]->hasRegisteredLight())
							{ // It has no registered light
								LOG(INFO) << "Unit found, without light registered";

								// Is there a light present for this unit ?
								auto lightUnit = startedLights.find(configUnit.first);
								if (lightUnit == startedLights.end())
								{
									LOG(INFO) << "Found no existing light; creating: " << configUnit.first;
									startedLights[configUnit.first] = std::unique_ptr<Hardware::Light>(new Hardware::Light(i2c, configUnit.second.mLight));
								}

								LOG(INFO) << "Registering the light";
								startedUnits[configUnit.first]->registerLight(startedLights[configUnit.first].get());
							}
						}
					}
				}

				// Sleep for 10 seconds; check for disconnected hardware after this time
				std::this_thread::sleep_for( std::chrono::seconds(10) );

				if (FLAGS_i2cstatistics)
				{
					i2c.printStatistics();
				}

				// Now run thru the connected devices; and see if they are still connected
				// Start with the clock devices
				auto unit_it = startedUnits.begin();
				while (unit_it != startedUnits.end())
				{
					if (!unit_it->second->isAttached())
					{
						LOG(ERROR) << "Clock unit is no longer attached: " << unit_it->first;

						// Need to remove the clock unit; first find (if any) the light unit, and unregister
						auto lightUnit = startedLights.find(unit_it->first);
						if (lightUnit != startedLights.end())
						{
							LOG(ERROR) << "The Clock unit had a registered light; unregistering the light";
							unit_it->second->unRegisterLight(lightUnit->second.get());
						}

						LOG(ERROR) << "Deleting the clock unit";
						startedUnits.erase(unit_it);
					}
					unit_it++;
				}


				// Do the same with the light devices
				auto light_it = startedLights.begin();
				while (light_it != startedLights.end())
				{
					if (!light_it->second->isAttached())
					{
						LOG(ERROR) << "Licht unit no longer attached: " << light_it->first;
						// Need to remove the light unit; first find (if any) the clock unit, and unregister
						auto clockUnit = startedUnits.find(light_it->first);
						if (clockUnit != startedUnits.end())
						{
							LOG(ERROR) << "Found the corresponding clock unit; unregistering light";
							clockUnit->second->unRegisterLight(light_it->second.get());
						}
						LOG(ERROR) << "Deleting the light";
						startedLights.erase(light_it);
					}

					light_it++;
				}


			} while (runMain);

			LOG(INFO) << "Exit application, cleaning up ...";

			// Delete the clock units
			startedUnits.clear();

			// Do the same with the light devices
			startedLights.clear();

		}
		catch (std::string &ex)
		{
			LOG(ERROR) << "Exception occurred:" << ex;
			return EXIT_FAILURE;
		}


		close(pidFilehandle);
		LOG(INFO) << "Exit application";
	}
	catch (std::string &ex)
	{
		std::cerr << "Exception: " << ex << std::endl;
	}

	return EXIT_SUCCESS;
}

