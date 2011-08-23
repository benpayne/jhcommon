/*
 * Copyright (c) 2010, JetHead Development, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the JetHead Development nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Timer.h"
#include "TimerManager.h"

#include "jh_memory.h"
#include "jh_types.h"
#include "logging.h"

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);

TimerManager *TimerManager::mSingleton = NULL;

TimerManager *TimerManager::getInstance()
{
	TRACE_BEGIN(LOG_LVL_INFO);
	if (mSingleton == NULL) 
		mSingleton = jh_new TimerManager;
	return mSingleton; 
}


void TimerManager::destroyManager() 
{ 
	if (mSingleton != NULL)
	{
		delete mSingleton; 
		mSingleton = NULL;
	}
}

TimerManager::TimerManager()
:	mMutex( true )
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	if (mSingleton != NULL)
		LOG_ERR_FATAL("Illegal creation of TimerManager, use getInstance()");
	
	mDefaultTimer = jh_new Timer(kMsPerTick, false);
};

TimerManager::~TimerManager()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	// Release reference to default timer
	mDefaultTimer = NULL;
	
	// At this point all outstanding timers should be
	// gone.  If we still have a timer in our list then
	// log a warning but do not do anything to the memory
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		LOG_WARN("Possibly leaked Timer %p\n", mTimers[i]);
	}
}

Timer* TimerManager::getDefaultTimer()
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	return mDefaultTimer;
}

Timer* TimerManager::getTimer(int tickTimeMs)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		if (mTimers[i]->getTickTime() == tickTimeMs)
		{
			return mTimers[i];
		}
	}
	
	// We didn't find a timer with the specified tick time so
	// we need to create a new one and give it to the user.
	Timer* newTimer = jh_new Timer(tickTimeMs);
	
	return newTimer;
}

void TimerManager::removeTimedEvent( Event::Id eventId,
									 IEventDispatcher *dispatcher )
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		mTimers[i]->removeTimedEvent(eventId, dispatcher);
	}
	
}

void TimerManager::removeTimedEvent( Event *ev )
{
	TRACE_BEGIN(LOG_LVL_NOISE);

	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		mTimers[i]->removeTimedEvent(ev);
	}
	
}

void TimerManager::removeAgentsByReceiver( void* receiver,
										   IEventDispatcher* dispatcher )
{
	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		mTimers[i]->removeAgentsByReceiver(receiver, dispatcher);
	}
}

void TimerManager::addTimer(Timer* timer)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	// Search the vector of timers to see if the specified
	// timer is already present.   If it is then return
	// immediately.
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		if (mTimers[i] == timer)
		{
			// Timer already in our vector so nothing to do
			return;
		}
	}
	
	mTimers.push_back(timer);
}

void TimerManager::removeTimer(Timer* timer)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	// Lock mutex while accessing/modifying mTimers vector
	AutoLock l(mMutex);
	
	for (unsigned i = 0; i < mTimers.size(); ++i)
	{
		// If we find the timer specified in our vector then
		// we need to remove our weak reference.
		if (mTimers[i] == timer)
		{
			mTimers.erase(i);
			return;
		}
	}
}
