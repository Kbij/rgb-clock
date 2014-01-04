/*
 * WatchdogFeederIf.h
 *
 *  Created on: Dec 1, 2013
 *      Author: koen
 */

#ifndef WATCHDOGFEEDERIF_H_
#define WATCHDOGFEEDERIF_H_
#include <string>


namespace Hardware {

class WatchdogFeederIf {

public:
	virtual ~WatchdogFeederIf() {};

	virtual std::string feederName() const = 0;
};

} /* namespace Hardware */
#endif /* WATCHDOGFEEDERIF_H_ */
