/*
 * AlarmSync.cpp
 *
 *  Created on: May 9, 2014
 *      Author: koen
 */

#include "AlarmSync.h"
#include "AlarmSyncListenerIf.h"

namespace App {

AlarmSync::AlarmSync():
		mListenersMutex(),
		mListeners()
{

}

AlarmSync::~AlarmSync()
{
}

void AlarmSync::registerListener(AlarmSyncListenerIf* listener)
{
    if (listener)
    {
        std::lock_guard<std::mutex> lk_guard(mListenersMutex);

        mListeners.insert(listener);
    }
}

void AlarmSync::unRegisterListener(AlarmSyncListenerIf* listener)
{
    if (listener)
    {
        std::lock_guard<std::mutex> lk_guard(mListenersMutex);

        mListeners.erase(listener);
    }
}

void AlarmSync::sendAlarmSnooze()
{
	std::lock_guard<std::mutex> lk_guard(mListenersMutex);
    for (auto listener : mListeners)
    {
    	listener->alarmSnooze();
    }
}

void AlarmSync::sendAlarmOff()
{
	std::lock_guard<std::mutex> lk_guard(mListenersMutex);
    for (auto listener : mListeners)
    {
    	listener->alarmOff();
    }
}

} /* namespace App */
