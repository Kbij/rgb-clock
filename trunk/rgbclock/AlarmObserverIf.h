/*
 * AlarmObserverIf.h
 *
 *  Created on: Sep 8, 2013
 *      Author: koen
 */

#ifndef ALARMOBSERVERIF_H_
#define ALARMOBSERVERIF_H_


namespace App {

class AlarmObserverIf {
public:
	virtual ~AlarmObserverIf() {};

	virtual void alarmNotify() = 0;
};

} /* namespace App */

#endif /* ALARMOBSERVERIF_H_ */
