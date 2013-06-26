/*
 * RadioObserverIf.h
 *
 *  Created on: May 28, 2013
 *      Author: koen
 */

#ifndef RADIOOBSERVERIF_H_
#define RADIOOBSERVERIF_H_

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

    virtual void infoAvailable(InfoType type) = 0;
};
}

#endif /* RADIOOBSERVERIF_H_ */
