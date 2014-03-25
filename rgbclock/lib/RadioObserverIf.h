/*
 * RadioObserverIf.h
 *
 *  Created on: May 28, 2013
 *      Author: koen
 */

#ifndef RADIOOBSERVERIF_H_
#define RADIOOBSERVERIF_H_
#include "RDSInfo.h"

namespace Hardware
{
enum class RadioState
{
	PwrOff,
	PwrOn,
	Wakeup
};

struct RadioInfo
{
	RadioState mState;
	int mVolume;
};

class RadioObserverIf
{
public:
    virtual ~RadioObserverIf() {};

    virtual void radioRdsUpdate(RDSInfo rdsInfo) = 0;
    virtual void radioStateUpdate(RadioInfo radioInfo) = 0;
};
}

#endif /* RADIOOBSERVERIF_H_ */