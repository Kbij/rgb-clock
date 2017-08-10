/*
 * WatchDogIf.h
 *
 *  Created on: Aug 10, 2017
 *      Author: koen
 */

#ifndef SRC_LIB_WATCHDOGIF_H_
#define SRC_LIB_WATCHDOGIF_H_

namespace Hardware
{
class WatchdogFeederIf;

class WatchDogIf
{
public:
	virtual void promiseWatchdog(WatchdogFeederIf *watchdogFeeder, int timeoutMiliseconds) = 0;
	virtual void removePromise(WatchdogFeederIf *watchdogFeeder) = 0;
	virtual void signalWatchdog(WatchdogFeederIf *watchdogFeeder) = 0;
};

}


#endif /* SRC_LIB_WATCHDOGIF_H_ */
