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

#include "EventDispatcher.h"
#include "EventAgent.h"
#include "Timer.h"
#include "logging.h"
#include "jh_memory.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

EventDispatcherHelper::EventDispatcherHelper()
:	mLock( true )
{
	TRACE_BEGIN(LOG_LVL_INFO);
	// Initialize EventDispatcherHelper with a recursive lock
}

EventDispatcherHelper::~EventDispatcherHelper()
{
	TRACE_BEGIN(LOG_LVL_INFO);

	for (JetHead::list<EventListenerNode*>::iterator i = mEventList.begin();
		 i != mEventList.end(); ++i)
	{
		delete *i;
	}

	mEventList.clear();
}

void EventDispatcherHelper::dispatchEvent( Event *ev )
{	
	DebugAutoLock( mLock );
	
	if ( ev == NULL )
	{
		LOG_WARN( "Attempt to dispatch NULL event" );
		return;
	}
	
	if ( ev->getEventId() == Event::kAgentEventId )
	{
		EventAgent *agent = event_cast<EventAgent>( ev );
		if ( agent != NULL )
			agent->deliver();
	}
	else
	{
		lookupListeners( ev );
	}
}

void EventDispatcherHelper::lookupListeners( Event *ev )
{
	for (JetHead::list<EventListenerNode*>::iterator i = mEventList.begin();
		 i != mEventList.end(); ++i)
	{
		// Prune dead listener nodes
		if ( (*i)->mListener == NULL )
		{
			delete *i;
			i = i.erase();
			--i;
		}
		else if ( (*i)->mEventId == ev->getEventId() || 
				  (*i)->mEventId == Event::kInvalidEventId )
		{
			IEventListener *listener = (*i)->mListener;
			listener->receiveEvent( ev );
		}
	}	
}

int EventDispatcherHelper::addEventListener( IEventListener *listener, int event_id )
{
	DebugAutoLock( mLock );
	
	EventListenerNode *node = jh_new EventListenerNode;
	
	node->mListener = listener;
	node->mEventId = event_id;
	
	mEventList.push_back( node );

	return 0;
}

int EventDispatcherHelper::removeEventListener( IEventListener *listener, int event_id )
{
	DebugAutoLock( mLock );

	for (JetHead::list<EventListenerNode*>::iterator i = mEventList.begin();
		 i != mEventList.end(); ++i)
	{
		EventListenerNode *lnode = *i;
		
		if ( lnode->mEventId == event_id && lnode->mListener == listener )
		{
			// Update mListener 
			lnode->mListener = NULL;
			return 0;
		}
	}

	// Didn't find it
	return -1;
}

EventDispatcher::EventDispatcher()
{
	// NOTE:  This is a sort of hacky way of preventing a bad condition from
	// occuring.  It was found that when we are processing a signal to do
	// a shutdown (ie.  ctrl-c in tad) the creation of the thread in the
	// TimerManager hangs the system.  I traced this down to a hang up in
	// a call to pthread_create.  The odd thing is that the new thread is
	// spawned (the main is run) but the pthread_create method never returns.
	// To prevent this from hanging us up in the future we will make sure
	// that the TimerManager is initialized when an EventDispatcher is
	// created.
//	TimerManager::getInstance();
}

EventDispatcher::~EventDispatcher()
{
}

void EventDispatcher::sendEventSync( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	// event ref count handled by holder.
	SyncEventHolder holder( ev );

	mQueue.SendEvent( &holder );
	wakeThread();
	
	if (isThreadCurrent())
		LOG_ERR_FATAL("Sending sync event to your current thread, I will die now...");
	
	mSyncLock.Lock();
	while ( holder.mDone == false )
		mSyncWait.Wait( mSyncLock );
	mSyncLock.Unlock();
}

void EventDispatcher::sendEvent( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	mQueue.SendEvent( ev );
	wakeThread();
}

void EventDispatcher::sendTimedEvent( Event *ev, uint32_t msecs, Timer* timer)
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	if ( timer == NULL )
	{
		timer = TimerManager::getInstance()->getDefaultTimer();
	}
	timer->sendTimedEvent( ev, this, msecs );
}

void EventDispatcher::sendPeriodicEvent( Event *ev, uint32_t msecs, Timer* timer)
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	if ( timer == NULL )
	{
		timer = TimerManager::getInstance()->getDefaultTimer();
	}
	timer->sendPeriodicEvent( ev, this, msecs );
}


int EventDispatcher::remove( Event::Id eventId )
{
	if (not isThreadCurrent())
	{
		SyncRetEventAgent1<EventDispatcher, int, Event::Id>* e = 
			jh_new SyncRetEventAgent1<EventDispatcher, int, Event::Id>
			(this, &EventDispatcher::remove, eventId);

		// I am a bad person, but it's either this or make all
		// EventAgent's publicly inherit from Event (instead of
		// Protected inheritance), and that's a big ugly change for
		// little benefit.
		((Event*)e)->setPriority(PRIORITY_HIGH);

		return e->send(this);
	}

	//  It is important that we remove events from the TimerManager first.
	//  This ensure that an event does not get dispatched to the queue after we
	//  clear the queue.
	TimerManager *timerMan = TimerManager::getInstance();
	timerMan->removeTimedEvent( eventId, this );
	mQueue.Remove( eventId );	
	return 0;
}

int EventDispatcher::remove( Event *ev )
{
	if (not isThreadCurrent())
	{
		SyncRetEventAgent1<EventDispatcher, int, Event*>* e = 
			jh_new SyncRetEventAgent1<EventDispatcher, int, Event*>
			(this, &EventDispatcher::remove, ev);

		// I am a bad person, but it's either this or make all
		// EventAgent's publicly inherit from Event (instead of
		// Protected inheritance), and that's a big ugly change for
		// little benefit.
		((Event*)e)->setPriority(PRIORITY_HIGH);

		return e->send(this);
	}

	//  It is important that we remove events from the TimerManager first.  
	//  This ensure that an event does not get dispatched to the queue after we
	//  clear the queue.
	TimerManager *timerMan = TimerManager::getInstance();
	timerMan->removeTimedEvent( ev );
	mQueue.Remove( ev );	
	return 0;
}

int EventDispatcher::removeAgentsByReceiver(void* recipient)
{
	if (not isThreadCurrent())
	{
		SyncRetEventAgent1<EventDispatcher, int, void*>* e = 
			jh_new SyncRetEventAgent1<EventDispatcher, int, void*>
			(this, &EventDispatcher::removeAgentsByReceiver, recipient);

		// I am a bad person, but it's either this or make all
		// EventAgent's publicly inherit from Event (instead of
		// Protected inheritance), and that's a big ugly change for
		// little benefit.
		((Event*)e)->setPriority(PRIORITY_HIGH);

		return e->send(this);
	}

	TimerManager *timerMan = TimerManager::getInstance();
	timerMan->removeAgentsByReceiver(recipient, this);
	mQueue.RemoveAgentsByReceiver(recipient);
	return 0;
}

int EventDispatcher::removeAll()
{
	if (not isThreadCurrent())
	{
		SyncRetEventAgent0<EventDispatcher, int>* e = 
			jh_new SyncRetEventAgent0<EventDispatcher, int>
			(this, &EventDispatcher::removeAll);

		// I am a bad person, but it's either this or make all
		// EventAgent's publicly inherit from Event (instead of
		// Protected inheritance), and that's a big ugly change for
		// little benefit.
		((Event*)e)->setPriority(PRIORITY_HIGH);

		return e->send(this);
	}

	// Same a remove exept we give TimerManager an invalid id so that he will
	//  remove all events for this dispatcher.
	TimerManager *timerMan = TimerManager::getInstance();
	timerMan->removeTimedEvent( Event::kInvalidEventId, this );
	mQueue.Flush();
	return 0;
}


bool EventDispatcher::isThreadCurrent()
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	if (Thread::GetCurrent() == getDispatcherThread())
		return true;
	return false;
}


int EventDispatcher::addEventListener( IEventListener *listener, int event_id )
{
	return mDispatcher.addEventListener( listener, event_id );
}

int EventDispatcher::removeEventListener( IEventListener *listener, int event_id )
{
	return mDispatcher.removeEventListener( listener, event_id );
}

void EventDispatcher::handleSyncEvent( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	SyncEventHolder *holder = event_cast<SyncEventHolder>( ev );

	if ( holder == NULL)
	{
		LOG_ERR_FATAL("event must not be NULL");
		return;
	}	

	mDispatcher.dispatchEvent( holder->mRealEvent );
	mSyncLock.Lock();
	holder->mDone = true;
	mSyncLock.Unlock();
	mSyncWait.Broadcast();
	// ev will get deleted in the caller.
}

bool EventDispatcher::handleEvent( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	bool done = false;
	
	if ( ev == NULL )
	{
		LOG_WARN( "called with NULL event" );
		return false;
	}
	
	switch ( ev->getEventId() )
	{
		case Event::kShutdownEventId:
			LOG( "kShutdownEventId received" );
			removeAll();
			done = true;
			ev->Release();
			break;

		case Event::kSyncEventId:
			handleSyncEvent( ev );
			break;
		
		default:
			mDispatcher.dispatchEvent( ev );
			ev->Release();
			break;
	}
	
	return done;
}

