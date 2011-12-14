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

#ifndef _JH_EVENTQUEUE_H_
#define _JH_EVENTQUEUE_H_

#include "jh_types.h"
#include "Condition.h"
#include "Mutex.h"
#include "Event.h"
#include "list.h"

/**
 * A Class for queuing events.  This is used internally by EventDispatcher.  
 *  If this class is used on its own the owner must be aware of event ref counts.
 */ 
class EventQueue
{
public:
	EventQueue();
	virtual ~EventQueue();
	
	/**
	 * Send a event to the queue.
	 */
	void SendEvent( Event *ev );
	
	/**
	 * Wait for an event to arrive.  User must call release on Event when done
	 *  with it.
	 * 
	 * @param mstimeout Number of milleseconds to wait before timing out.  If 
	 *  zero, the default, wait forever.
	 * @return an event or NULL if timed out.  You must delete the event when using it.
	 */
	Event *WaitEvent( uint32_t mstimeout = 0 );

	/**
	 * Check if an event is pending on the queue.  User must call release on 
	 *  Event when done with it.
	 *
	 * @return an event or NULL non available.  You must delete the event when using it.
	 */
	Event *PollEvent();
	
	/**
	 * Remove all items from the queue with a specified event_id.
	 */
	void Remove( int event_id );
	
	/**
	 * Remove a specific event from the queue
	 */
	void Remove( Event *ev );

	/**
	 * Remove any EventAgents who are going to be delivered to this object
	 */
	void RemoveAgentsByReceiver( void *reciever );

	/**
	 * Remove all items from the queue.
	 */
	void Flush();
	
private:
	Event *pollEventInternal();
	
	JetHead::list<Event*> mQueue;
	Mutex		mLock;
	Condition	mWait;
};



#endif // _JH_EVENTQUEUE_H_
