/*
 * AlarmSyncListenerIf.h
 *
 *  Created on: May 9, 2014
 *      Author: koen
 */

#ifndef ALARMSYNCLISTENERIF_H_
#define ALARMSYNCLISTENERIF_H_

namespace App {

class AlarmSyncListenerIf {
public:
	virtual ~AlarmSyncListenerIf() {};

	virtual void alarmSnooze() = 0;
	virtual void alarmOff() = 0;
};

} /* namespace App */


#endif /* ALARMSYNCLISTENERIF_H_ */
