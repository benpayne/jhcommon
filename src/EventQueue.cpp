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

#include "EventQueue.h"
#include "logging.h"
#include "EventAgent.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

EventQueue::EventQueue() : mLock( "EventQueue" )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
}

EventQueue::~EventQueue()
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	// look at all event in the queue and delete them.
}

void EventQueue::SendEvent( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	DebugAutoLock( mLock );
	
	ev->AddRef();

	if ( ev->getPriority() == PRIORITY_NORMAL )
	{
		mQueue.push_back( ev );
	}
	else
	{
		int done = false;
		for (JetHead::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end(); ++i)
		{
			if (ev->getPriority() > (*i)->getPriority())
			{
				i.insertBefore(ev);
				done = true;
				break;
			}
		}

		// if we didn't put the event in the list, we add it to the end.
		//  this will happend if all events in the list are of greater priority.
		if ( !done )
		{
			mQueue.push_back( ev );
		}
	}
	
	LOG( "queue size %d", mQueue.size() );	

	mWait.Signal();
}

Event *EventQueue::WaitEvent( uint32_t mstimeout )
{
	TRACE_BEGIN( LOG_LVL_NOISE );

	DebugAutoLock( mLock );
	Event *ev = pollEventInternal();

	while ( ev == NULL )
	{
		LOG( "timeout %d", mstimeout );
		if ( mWait.Wait( mLock, mstimeout ) )
		{
			LOG( "signalled" );
			ev = pollEventInternal();
			if ( ev == NULL )
			{
				LOG_ERR( "Signalled empty queue" );
			}
		}
		else if ( mstimeout > 0 )
			return NULL;
		
		LOG( "done" );
	}	
	
	return ev;
}

Event *EventQueue::pollEventInternal()
{
	if (mQueue.empty()) return NULL;
	
	Event* ret = mQueue.front();
	mQueue.pop_front();
	return ret;
}

Event *EventQueue::PollEvent()
{
	TRACE_BEGIN( LOG_LVL_INFO );

	DebugAutoLock( mLock );
	return pollEventInternal();
}

void EventQueue::Remove( Event::Id id )
{
	DebugAutoLock( mLock );
	
	for (JetHead::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end(); ++i)
	{
		if ((*i)->getEventId() == id)
		{
			(*i)->Release();
			i = i.erase();
			--i;
		}
	}
}

void EventQueue::Remove( Event *ev )
{
	DebugAutoLock( mLock );

	for (JetHead::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end(); ++i)
	{
		if ((*i) == ev)
		{
			(*i)->Release();
			i = i.erase();
			--i;
		}
	}	
}

void EventQueue::RemoveAgentsByReceiver( void* receiver )
{
	DebugAutoLock( mLock );

	for (JetHead::list<Event*>::iterator i = mQueue.begin(); i != mQueue.end(); ++i)
	{
		if ((*i)->getEventId() == Event::kAgentEventId)
		{
			EventAgent* agent = static_cast<EventAgent*>(*i);
			if (agent->getDeliveryTarget() == receiver)
			{
				(*i)->Release();
				i = i.erase();
				--i;
			}
		}
	}
}

void EventQueue::Flush()
{
	DebugAutoLock( mLock );

	// Scan through all events, remove them from the queue and
	// release a reference from them.
	while (not mQueue.empty())
	{
		Event* temp = mQueue.front();
		mQueue.pop_front();
		temp->Release();
	}
}

