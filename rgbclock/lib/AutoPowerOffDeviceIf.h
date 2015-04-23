/*
 * AutoPowerOffIf.h
 *
 *  Created on: Apr 15, 2015
 *      Author: koen
 */

#ifndef AUTOPOWEROFFIF_H_
#define AUTOPOWEROFFIF_H_

namespace Hardware
{

class AutoPowerOffDeviceIf
{
public:

    virtual void pwrOff() = 0;
};
}


#endif /* AUTOPOWEROFFIF_H_ */
