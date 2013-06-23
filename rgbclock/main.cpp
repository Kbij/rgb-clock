/*
 * Raspberry Pi Ultimate Alarm Clock
 */
#include "lib/I2C.h"
#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <time.h>
#include <sstream>
#include <bitset>

DEFINE_bool(daemon, false, "Run rgbclock as Daemon");
DEFINE_string(pidfile,"","Pid file when running as Daemon");

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

	LOG(INFO) << "Raspberry Pi Ultimate Alarm Clock";
	LOG(INFO) << "=================================";
	try
	{
		I2C i2c;
	}
	catch (std::string caught)
	{
		LOG(ERROR) << "Failed to open I2C port:" << caught;
		return EXIT_FAILURE;
	}

	do{
		std::this_thread::sleep_for( std::chrono::milliseconds(1000) );

	} while (runMain);

	close(pidFilehandle);
	return EXIT_SUCCESS;
}

