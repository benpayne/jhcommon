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

#ifndef _JH_EVENTDISPATCHER_H_
#define _JH_EVENTDISPATCHER_H_

#include "EventQueue.h"
#include "Mutex.h"
#include "EventAgent.h"

/**
 * A helper class for anyone implementing IEventDispatcher.  This class will 
 *  track the list of EventListeners and send an event to all the interested
 *  listeners when dispatchEvent is called.
 */
class EventDispatcherHelper
{
public:
	EventDispatcherHelper();
	~EventDispatcherHelper();

	void dispatchEvent( Event *ev );
	
	int addEventListener( IEventListener *listener, int event_id );	
	int removeEventListener( IEventListener *listener, int event_id );

private:
	void lookupListeners( Event *ev );
	
	struct EventListenerNode 
	{
		IEventListener *mListener;
		int mEventId;
	};
	
	Mutex	mLock;
	JetHead::list<EventListenerNode*> mEventList;
};

class EventDispatcher : public IEventDispatcher
{
public:
	EventDispatcher();
	virtual ~EventDispatcher();
	
	/**
	 * Send a event to the queue.
	 */
	void sendEventSync( Event *ev );

	/**
	 * Send a event to the queue.
	 */
	void sendEvent( Event *ev );

	/**
	 * Send a event at a later time.
	 */
	void sendTimedEvent( Event *ev, uint32_t msecs, Timer* timer = NULL );

	/**
	 * Send a recurring Event with a regular period
	 */
	void sendPeriodicEvent( Event *ev, uint32_t msecs, Timer* timer = NULL );

	/**
	 * Remove all events with given eventId
	 */
	int remove( int eventId );
	
	/**
	 * Remove this event.
	 */
	int remove( Event *ev );
	
	/**
	 * Remove any EventAgent events that will be delivered to this object
	 */
	int removeAgentsByReceiver(void* recipient);

	/**
	 * remove all events from the queue
	 */
	int removeAll();
	
	/**
	 *	@brief Check if thread is current
	 *
	 *	This method checks if the caller's thread is the same as the
	 *	thread used by this IEventDispatcher.   If it is then this
	 *	method will return true, otherwise returns false
	 */
	bool isThreadCurrent();
	
	/**
	 * add an event listener to be called when event_id is recieved, if event_id
	 *  is kInvalidEventId then all events recieved will be sent to the listener.
	 */
	int addEventListener( IEventListener *listener, int event_id );
	
	/**
	 * Remove this event listener for this event_id, if and exact match is not 
	 *  found it will fail.
	 */
	int removeEventListener( IEventListener *listener, int event_id );

protected:
	struct SyncEventHolder : public Event
	{
		SyncEventHolder( Event *ev ) : Event( Event::kSyncEventId, ev->getPriority() ),
			mRealEvent( ev ), mDone( false ) {}

		~SyncEventHolder() {}

		SMART_CASTABLE( Event::kSyncEventId );
		
		SmartPtr<Event> mRealEvent;
		bool mDone;
	};
	
	virtual void wakeThread() {}
	virtual void onThreadExit() {}
	
	/**
	 * This method is used to check if a sync event is going to be sent to 
	 *  the same thread as the sender.  Today we just log a fatal error in this
	 *  case.
	 */
	virtual const Thread *getDispatcherThread() { return NULL; }
	
	bool handleEvent( Event *ev );

	EventQueue mQueue;
	
private:	
	void handleSyncEvent( Event *ev );
	
	EventDispatcherHelper mDispatcher;
	Condition mSyncWait;
	Mutex mSyncLock;
};

#endif // _JH_EVENTDISPATCHER_H_
