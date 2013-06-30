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
enum class InfoType
{
	RdsInfo,
	SignalStrength
};

class RadioObserverIf
{
public:
    virtual ~RadioObserverIf() {};

    virtual void infoAvailable(RDSInfo rdsInfo) = 0;
    virtual void volumeChange(int volume) = 0;
};
}

#endif /* RADIOOBSERVERIF_H_ */
