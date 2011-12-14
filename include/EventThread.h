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

#ifndef _JH_EVENTTHREAD_H_
#define _JH_EVENTTHREAD_H_

#include "EventDispatcher.h"
#include "Thread.h"

class EventThread : public EventDispatcher
{
public:
	EventThread( const char *name = NULL );
	virtual ~EventThread();
		
private:
	void threadMain();	

	const Thread *getDispatcherThread() { return &mThread; }

	Runnable<EventThread> mThread;	
};

/**
 * This is a class that can adapt a method into a EventListener.  
 */
template<class ClassType, class EventType>
class EventMethod : public IEventListener
{
public:
	typedef void (ClassType::*event_method_t)( EventType *ev );

	/**
	 * Construct a EventMethod.
	 *
	 * @param object The instance of T that they method sould be called on.
	 * @param method The member function that will be the events handler.
	 */
	EventMethod( ClassType *object, event_method_t method, IEventDispatcher *dispatcher ) : 
		mObject( object ), mFunc( method ), mDispatcher( dispatcher )
	{
		mDispatcher->addEventListener( this, EventType::GetEventId() );
	}
	
	virtual ~EventMethod()
	{
		mDispatcher->removeEventListener( this, EventType::GetEventId() );
	}
	
	void receiveEvent( Event *ev )
	{
		EventType *real_event = event_cast<EventType>( ev );
		(mObject->*mFunc)( real_event );
	}
	
private:
	ClassType*		mObject;
	event_method_t	mFunc;
	IEventDispatcher *mDispatcher;
};

#endif // _JH_EVENTTHREAD_H_
