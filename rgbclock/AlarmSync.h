/*
 * AlarmSync.h
 *
 *  Created on: May 9, 2014
 *      Author: koen
 */

#ifndef ALARMSYNC_H_
#define ALARMSYNC_H_
#include <set>
#include <mutex>

namespace App {
class AlarmSyncListenerIf;

class AlarmSync {
public:
	AlarmSync();
	virtual ~AlarmSync();

	void registerListener(AlarmSyncListenerIf* listener);
	void unRegisterListener(AlarmSyncListenerIf* listener);

	void sendAlarmSnooze();
	void sendAlarmOff();
private:
    std::mutex mListenersMutex;
	std::set<AlarmSyncListenerIf*> mListeners;
};

} /* namespace App */
#endif /* ALARMSYNC_H_ */
