/*
 * UpDownDeviceIf.h
 *
 *  Created on: Apr 16, 2015
 *      Author: koen
 */

#ifndef UPDOWNDEVICEIF_H_
#define UPDOWNDEVICEIF_H_

namespace Hardware
{

class UpDownDeviceIf
{
public:

    virtual void up(int step) = 0;
    virtual void down(int step) = 0;
};
}


#endif /* UPDOWNDEVICEIF_H_ */
