/*
 * Raspberry Pi Ultimate Alarm Clock
 */
#include "lib/I2C.h"
#include "lib/FMReceiver.h"
#include "lib/RTC.h"
#include "Light.h"
#include "AlarmClock.h"
#include "Config.h"
#include "AlarmManager.h"

#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <chrono>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>



DEFINE_bool(daemon, false, "Run rgbclock as Daemon");
DEFINE_string(pidfile,"","Pid file when running as Daemon");
DEFINE_bool(i2cstatistics, false, "Print I2C statistics");


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

	std::string usage("Raspberry Pi Ultimate Alarm Clock. Sample usage:\n");
	usage += argv[0];
	google::SetUsageMessage(usage);
	google::ParseCommandLineFlags(&argc, &argv, true);

	if (FLAGS_daemon)
	{
		daemonize();
	}

	// Create the config object; load the XML file with the settings
	App::Config config;
	App::AlarmManager alarmManager;
	auto alarms = alarmManager.editAlarms("Main");
	App::Alarm testAlarm;
	testAlarm.mHour = 23;
	testAlarm.mMinutes = 18;
	alarms->push_back(testAlarm);

	alarmManager.saveAlarms("Main");


	if (!config.errorFree())
	{
		// No errorfree load; exit the application
		return -1;
	}

	std::map<std::string, std::unique_ptr<App::AlarmClock>> startedUnits;
	std::map<std::string, std::unique_ptr<App::Light>> startedLights;

	const std::map<std::string, App::UnitConfig>& configuredUnits = config.configuredUnits();
	const App::SystemConfig& systemConfig = config.systemConfig();

	LOG(INFO) << "Raspberry Pi Ultimate Alarm Clock";
	LOG(INFO) << "=================================";
	LOG(INFO) << "Number of configured units: " << configuredUnits.size();

	try
	{
		Hardware::I2C i2c;
		Hardware::RTC rtc(i2c, systemConfig.mRtc);

		Hardware::FMReceiver fmReceiver(i2c, systemConfig.mRadio);

		do{
			for (const auto& configUnit : configuredUnits)
			{
				if (startedUnits.find(configUnit.first) == startedUnits.end())
				{
					LOG(INFO) << "Creating clock unit: " << configUnit.first;
					// Unit not found; create a unit
					startedUnits[configUnit.first] = std::unique_ptr<App::AlarmClock>(new App::AlarmClock(i2c, fmReceiver, alarmManager, configUnit.second));
				}

				if (!startedUnits[configUnit.first]->hasRegisteredLight())
				{ // It has no registered light
					LOG(INFO) << "Unit found, without light registered";

					// Is there a light present for this unit ?
					auto lightUnit = startedLights.find(configUnit.first);
					if (lightUnit == startedLights.end())
					{
						LOG(INFO) << "Found no existing light; creating: " << configUnit.first;
						startedLights[configUnit.first] = std::unique_ptr<App::Light>(new App::Light(i2c, configUnit.second.mLight));
					}

					LOG(INFO) << "Registering the light";
					startedUnits[configUnit.first]->registerLight(startedLights[configUnit.first].get());
				}
			}

			// Sleep for 3 seconds; check for disconnected hardware after this time
			std::this_thread::sleep_for( std::chrono::milliseconds(10000) );

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

	}
	catch (std::string &ex)
	{
		LOG(ERROR) << "Exception occurred:" << ex;
		return EXIT_FAILURE;
	}


	close(pidFilehandle);
	return EXIT_SUCCESS;
}

