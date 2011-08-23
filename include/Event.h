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

#ifndef JH_EVENT_H_
#define JH_EVENT_H_

#include "jh_types.h"
#include "RefCount.h"

enum {
	PRIORITY_NORMAL,
	PRIORITY_HIGH
};

class EventQueue;
class EventAgent;
class Timer;

/**
 * An Event class.  Your should use postive event id's.  negative event id are
 *  reserved for the event system it's self.
 */ 
class Event : public RefCount
{
public:
	typedef int Id;
	
	Event( Id event_id, int priority = PRIORITY_NORMAL ) : 
		mEventId( event_id ), mPriority( priority ) {}
	virtual ~Event() {}
	
	static const Id kInvalidEventId = -1;
	static const Id kShutdownEventId = -2;
	static const Id kSyncEventId = -3;
	static const Id kSelectorUpdateEventId = -4;
	static const Id kAgentEventId = -5;
	
	Id	getEventId() { return mEventId; }
	int getPriority() { return mPriority; }
	void setPriority(int priority) 
	{
		if (priority > PRIORITY_HIGH) priority = PRIORITY_HIGH;
		mPriority = priority;
	}
	
private:
	Id		mEventId;
	int 	mPriority;
	
	friend class EventQueue;
};

/**
 * This interface is implemented by any class that will recieve events for an
 *  event dispatcher.  This interface can be registered with the EventThread
 *  or the selector classes.
 */ 
class IEventListener
{
public:
	/**
	 * Send a event to the queue.
	 */
	virtual void receiveEvent( Event *ev ) = 0;

protected:
	virtual ~IEventListener() {}  // just for compile warning	
};

/**
 * This interface is implemented by any class that can recieve messages.  There
 *  are currently three type of event dispatchers, the EventThread, EventQueue 
 *  and Selector.
 */ 
class IEventDispatcher
{
public:
	/**
	 * Send a event to the queue sync.
	 */
	virtual void sendEventSync( Event *ev ) = 0;

	/**
	 * Send a event to the queue.
	 */
	virtual void sendEvent( Event *ev ) = 0;

	/**
	 * Send a event at a later time.
	 */
	virtual void sendTimedEvent( Event *ev,
								 uint32_t msecs,
								 Timer* timer = NULL ) = 0;
	
	/**
	 * Send a recurring event with a regular period
	 */
	virtual void sendPeriodicEvent( Event *ev,
									uint32_t msecs,
									Timer* timer = NULL ) = 0;
	
	/**
	 * Remove all events with the eventId from the queue
	 */
	virtual int remove( Event::Id eventId ) = 0;

	/**
	 * Remove a specific event from the queue
	 */
	virtual int remove( Event *ev ) = 0;
	
	/**
	 * Removal all event from the queue 
	 */
	virtual int removeAll() = 0;
	
	/**
	 *	@brief Check if thread is current
	 *
	 *	This method checks if the caller's thread is the same as the
	 *	thread used by this IEventDispatcher.   If it is then this
	 *	method will return true, otherwise returns false
	 */
	virtual bool isThreadCurrent() = 0;
	
	/**
	 * add an event listener to be called when event_id is recieved, if event_id
	 *  is kInvalidEventId then all events recieved will be sent to the listener.
	 */
	virtual int addEventListener( IEventListener *listener, int event_id ) = 0;
	
	/**
	 * Remove this event listener for this event_id, if and exact match is not 
	 *  found it will fail.
	 */
	virtual int removeEventListener( IEventListener *listener, int event_id ) = 0;

protected:
	virtual ~IEventDispatcher() {}  // just for compile warning	
};

#define SMART_CASTABLE( id ) static int GetEventId() { return id; }

template <class T>
T *event_cast( Event *ev )
{
	if ( ev->getEventId() == T::GetEventId() )
		return static_cast<T*>( ev );
	else 
		return NULL;
}

#endif // JH_EVENT_H_
