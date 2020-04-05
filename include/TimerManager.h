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

#ifndef JH_TIMER_MANAGER_H_
#define JH_TIMER_MANAGER_H_

#include "Timer.h"
#include "Mutex.h"
#include "RefCount.h"
#include "jh_vector.h"

class Timer;

/**
 *  @brief Singleton that tracks Timer's in the system
 *
 *  TimerManager is a singleton class that acts as a factory for Timer
 *  objects in the system.   Timers that are allocated via the TimerManager
 *  will be shared, reducing the number of outstanding threads.
 *
 *  TimerManager is also the owner/allocate of the default timer that is
 *  shared system wide with a 100ms resolution.
 */
class TimerManager
{
public:
	//! Get the singleton TimerManager
	static TimerManager *getInstance();

	//! Destroy the TimerManager (until the next call to getInstance)
	static void destroyManager();
	
	//! Return the default Timer (100ms resolution)
	Timer* getDefaultTimer();
	
	/**
	 *  @brief Fetch a timer with the specified resolution in milliseconds
	 *
	 *  This method will return a Timer that can service the specified
	 *  resolution without in milliseconds.   If one does not exist that
	 *  can service the requested resolution then one will be created.
	 *
	 *	@param  tickTimeMs - Requested tick time in milliseconds for Timer
	 */
	Timer* getTimer(int tickTimeMs);
	
	/**
	 *  @brief For each Timer known by the TimerManager remove
	 *
	 *	This method calls the removeTimedEvent method on all outstanding
	 *  Timer objects known to it.
	 *
	 *  @param event_id - the event id to remove or kInvalidEventId to
	 *  remove all.
	 *  @param dispatcher - the dispatcher for which we are removing events.
	 */
	void removeTimedEvent(Event::Id event_id, IEventDispatcher *dispatcher);

	/**
	 *  @brief For each Timer known by the TimerManager remove
	 *
	 *	This method calls the removeTimedEvent method on all outstanding
	 *  Timer objects known to it.
	 *
	 *  @param ev - the event to remove.
	 */
	void removeTimedEvent( Event *ev );

	/**
	 *  @brief For each Timer known by the TimerManager remove
	 *
	 *	This method calls the removeAgentsByReceiver method on all outstanding
	 *  Timer objects known to it.
	 *
	 *  @param reciever - the delivery target for an event agent
	 *
	 *  @param dispatcher - the dispatcher to clear those agents for or
	 *  NULL for all
	 */
	void removeAgentsByReceiver(void *reciever, IEventDispatcher *dispatcher);
	
	/**
	 *  @brief Add Timer to the TimerManager
	 *
	 *  This method adds a weak reference to the specified Timer to
	 *  the TimerManager.   This is called by a Timer when it is created
	 *  and users should not call this method.
	 */
	void addTimer(Timer* timer);
	
	/**
	 *  @brief Remove Timer from the TimerManager
	 *
	 *  This method removes the weak reference to the specified Timer
	 *  from the TimerManager.   This is called by a Timer when
	 *  it is destroyed and users should not call this method.
	 */
	void removeTimer(Timer* timer);
	
protected:
	//! Prevent non-singleton usage
	TimerManager();
	
	/**
	 *  Prevent anyone from destroying this in any way other than
	 *  destroyManager
	 */
	~TimerManager();

private:
	//! The default timer which will use the default kMsPerTick
	SmartPtr<Timer> mDefaultTimer;
	
	//! How many ms do we wait per tick?
	static const int kMsPerTick = 100;
	
	//! Our singleton 
	static TimerManager *mSingleton;
	
	//! Weakly referenced list of Timers that are being tracked by TimerManager
	JHSTD::vector<Timer*> mTimers;
	
	//! Locking for internal state
	Mutex mMutex;
};


#endif // JH_TIMER_MANAGER_H_

