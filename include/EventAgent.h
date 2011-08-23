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

#ifndef JH_EVENT_AGENT_H_
#define JH_EVENT_AGENT_H_

#include "Event.h"

/**
 *	@brief EventAgent class
 *
 *	Technically this should probably be called the AgentEvent because
 *	it is just a specialized Event that has an interface that the
 *	EventDispatcher's in our system know how to dispatch ON the event
 *	based on the event id, as opposed to dispatching the event to an
 *	IEventListener.
 */
class EventAgent : public Event
{
public:
	EventAgent()
	: Event(Event::kAgentEventId) {}
	
	SMART_CASTABLE( Event::kAgentEventId );
	
	/**
	 *	@brief Delivery interface
	 *
	 *	Unlike normal events in the system the EventAgent is special
	 *	in that the event dispatching classes know to invoke this
	 *	method ON the event instead of invoking a method on an IEventListener
	 *	and providing the event.
	 */
	virtual void deliver() = 0;

	/**
	 *  @brief Who is this going to?
	 *
	 *  In order to support "remove all queued events going to this target"
	 *  this needs to be implemented to return the pointer to the object
	 *  that will handle this delivery.
	 */
	virtual void* getDeliveryTarget() = 0;
 protected:
	virtual ~EventAgent() {}

};


/**
 *	@brief AsyncEventAgent
 *
 *	The AsyncEventAgent provides interfaces for sending asynchronously
 *	with or without a timeout to an IEventDispatcher as well as the ability
 *	to remove from an IEventDispatcher.
 *
 *	AsyncEventAgent is an aggregation of the EventAgent class.   However
 *	since we defined the EventAgent with a pure virtual method we need
 *	to do our aggregation via protected inheritance so that we can
 *	implement the EventAgent::deliver method in derived AsyncEventAgent
 *	instances.
 *
 *	Derived templates for the AsyncEventAgent WILL enforce that the user
 *	types specified are NOT references.   This is to prevent users from
 *	making the easy mistake of passing a reference to something on their
 *	stack via an asynchronous event.   Doing so is not valid, so the
 *	templates will force a compiler error so that users see their problem.
 *	Any large data structures that are being sent via an AsyncEventAgent
 *	should be sent via the heap (ie.  new'd pointer).   If you need to
 *	send a large data structure via the stack use the SyncEventAgent
 *	templates
 *
 *	Currently defined there are template classes AsyncEventAgentN where
 *	N is the number of parameters you wish to pass through the agent and
 *	N is in the range 0-10.  The AsyncEventAgentN templates all require
 *	the type of the class that will be handling the processing of the
 *	event.   The first two parameters of the constructor for an AsyncEventAgentN
 *	template will always be a pointer the handler class and a pointer
 *	to the function within the handler class that should be invoked to
 *	handle the event.
 *
 *	Compiler magic makes it so that just instantiating the AsyncEventAgentN
 *	template with types checks that a method of the appropriate type exists
 *	in the handler class.
 *
 *	Example usage:
 *	class Foo
 *	{
 *		public:
 *			void setTime(uint64_t timeMs, int timeOrigin)
 *			{
 *			AsyncEventAgent2<Foo, uint64_t, int> *agent =
 *					jh_new AsyncEventAgent2<Foo, uint64_t, int>(this,
 *					&Foo::handleSetTime, timeMs, timeOrigin);
 *				agent->send(&mEventThread);
 *			}
 *			
 *		protected:
 *			void handleSetTime(uint64_t timeMs, int timeOrigin)
 *			{
 *				LOG_NOTICE("Got time %llu and origin %d", timeMs, timeOrigin);
 *			}
 *		EventThread mEventThread;
 *	};
 *
 *	In addition we're going to use a trick so that we can be reference
 *	counted.  This allows users to hold a reference to an AsyncEventAgent
 *	that they plan to reuse and/or possibly need to removed (timed events
 *	for instance)
 *
 */
class AsyncEventAgent : protected EventAgent
{
public:
	AsyncEventAgent() : EventAgent() {}
	
	// Expose appropriate AddRef method so we can be reference counted
	using EventAgent::AddRef;

	// Expose appropriate Release method so we can be reference counted
	using EventAgent::Release;
	
	/**
	 *	@brief Dispatch asynchronously to the dispatcher specified
	 */
	void send(IEventDispatcher *dispatcher)
	{
		dispatcher->sendEvent(this);
	}
	
	/**
	 *	@brief Dispatch timed asynchronously to the dispatcher specified
	 */
	void sendTimed(IEventDispatcher *dispatcher,
				   uint32_t msecs,
				   Timer* timer = NULL)
	{
		dispatcher->sendTimedEvent(this, msecs, timer);
	}
	
	/**
	 *	@brief Dispatch periodically to the dispatcher specified
	 */
	void sendPeriodically(IEventDispatcher *dispatcher,
						  uint32_t msecs,
						  Timer* timer = NULL)
	{
		dispatcher->sendPeriodicEvent(this, msecs, timer);
	}		
	
	/**
	 *	@brief Remove previously dispatched from the dispatcher specified
	 */
	void remove(IEventDispatcher *dispatcher)
	{
		dispatcher->remove(this);
	}
 protected:
	virtual ~AsyncEventAgent() {}

};


/**
 *	@brief SyncEventAgent
 *
 *	The SyncEventAgent provides interfaces for sending synchronously
 *	to an IEventDispatcher.  If the send method is called on the same
 *	thread that the IEventDispatcher uses then the EventAgent is
 *	delivered immediately, otherwise the EventAgent is queued to be
 *	delivered on the IEventDispatcher's thread.   In this way the
 *	SyncEventAgent can be used to assure that code is run to completion on
 *	a specific thread when the SyncEventAgent::send() method is called
 *	without worrying about possible deadlocks.
 *
 *	SyncEventAgent is an aggregation of the EventAgent class.   However
 *	since we defined the EventAgent with a pure virtual method we need
 *	to do our aggregation via protected inheritance so that we can
 *	implement the EventAgent::deliver method in derived SyncEventAgent
 *	instances.
 *
 *	Since the SyncEventAgent dispatches synchronously the user can
 *	specify a SyncEventAgent derived template with output values.
 *
 *	Currently defined there are template classes SyncEventAgentN where
 *	N is the number of parameters you wish to pass through the agent and
 *	N is in the range of 0-10.   The SyncEventAgentN templates all require
 *	the type of the class that will be handling the processing of the
 *	event.   The first two parameters of the constructor for a SyncEventAgentN
 *	template will always be a pointer to the handler class and a
 *	pointer to the function within the handler class that should be invoked
 *	to handle the event.
 *
 *	Compiler magic makes it so that just instantiating the SyncEventAgentN
 *	template with types checks that a method of the appropriate type exists
 *	in the handler class.
 *
 *	Example usage:
 *	class Foo
 *	{
 *		public:
 *			Foo() : mTime(424242) {}
 *			bool getTime(uint64_t &timeMs)
 *			{
 *				bool result;
 *				SyncEventAgent2<Foo, uint64_t&, bool&> *agent =
 *					jh_new SyncEventAgent2<Foo, uint64_t&, bool&>(this,
 *					&Foo::handleGetTime, timeMs, result);
 *				agent->send(&mEventThread);
 *				return result;
 *			}
 *			
 *		protected:
 *			void handleGetTime(uint64_t &timeMs, bool &result)
 *			{
 *				LOG_NOTICE("Getting time...");
 *				timeMs = mTime;
 *				result = true;
 *			}
 *		uint64_t	mTime;
 *		EventThread mEventThread;
 *	};
 *
 *	In addition we're going to use a trick so that we can be reference
 *	counted.  This allows users to hold a reference to a SyncEventAgent
 *	that they plan to reuse.
 *
 */
class SyncEventAgent : protected EventAgent
{
public:
	SyncEventAgent() : EventAgent() {}
	
	// Expose appropriate AddRef method so we can be reference counted
	using EventAgent::AddRef;

	// Expose appropriate Release method so we can be reference counted
	using EventAgent::Release;
	
	void send(IEventDispatcher *dispatcher)
	{
		// AddRef and Release calls are needed to provoke the
		// appropriate cleanup mechanisms for this SyncEventAgent
		// in the case that we do not send the event to the dispatcher.
		AddRef();
		
		if (dispatcher->isThreadCurrent())
		{
			deliver();
		}
		else
		{
			dispatcher->sendEventSync(this);
		}
		
		Release();
	}
protected:
	virtual ~SyncEventAgent() {}

};


/**
 *	@brief SyncRetEventAgent
 *
 *	The SyncRetEventAgent template provides additional interfaces for
 *  sending synchronous messages to an IEventDispatcher.   If the send
 *	method is called on the same thread that the IEventDispatcher uses
 *	then the EventAgent is delivered immediately, otherwise the EventAgent
 *	is queued to be delivered on the IEventDispatcher's thread.   In this
 *	way the SyncRetEventAgent can be used to assure that code is run to
 *	completion on a specific thread when the SyncRetEventAgent::send()
 *	method is called without worrying about possible deadlocks.
 *
 *	SyncRetEventAgent is an aggregation of the EventAgent class.   However
 *	since we defined the EventAgent with a pure virtual method we need
 *	to do our aggregation via protected inheritance so that we can
 *	implement the EventAgent::deliver method in derived SyncRetEventAgent
 *	instances.
 *
 *	SyncRetEventAgent allows the user to dispatch to a method with a
 *	simple return value from any IEventDispatcher.   The SyncRetEventAgent
 *	does NOT handle reference types and will cause a compliation error if
 *	that form of return value is specified.
 *
 *	Currently defined there are template classes SyncRetEventAgentN where
 *	N is the number of parameters you wish to pass through the agent and
 *	N is in the range of 0-10.   The SyncRetEventAgentN templates all require
 *	the type of the class that will be handling the processing of the
 *	event.   The first two parameters of the constructor for a SyncEventAgentN
 *	template will always be a pointer to the handler class and a
 *	pointer to the function within the handler class that should be invoked
 *	to handle the event.
 *
 *	Compiler magic makes it so that just instantiating the SyncRetEventAgentN
 *	template with types checks that a method of the appropriate type exists
 *	in the handler class.
 *
 *	Example usage:
 *	class Foo
 *	{
 *		public:
 *			Foo() : mTime(424242) {}
 *			bool getTime(uint64_t &timeMs)
 *			{
 *				bool result = false;
 *				SyncRetEventAgent1<Foo, bool, uint64_t&> *agent =
 *					jh_new SyncRetEventAgent1<Foo, bool, uint64_t&>(this,
					&Foo::handleGetTime, timeMs);
				result = agent->send(&mEventThread);
 *				return result;
 *			}
 *			
 *		protected:
 *			bool handleGetTime(uint64_t &timeMs)
 *			{
 *				LOG_NOTICE("Getting time...");
 *				timeMs = mTime;
 *				return true;
 *			}
 *		uint64_t	mTime;
 *		EventThread mEventThread;
 *	};
 *
 *	In addition we're going to use a trick so that we can be reference
 *	counted.  This allows users to hold a reference to a SyncEventAgent
 *	that they plan to reuse.
 *
 */
template<typename ReturnType>
class SyncRetEventAgent : protected EventAgent
{
public:
	SyncRetEventAgent() : EventAgent() {}
	
	// Cause a compilation error if this template is instantiated with
	// a reference type as the return value.   This is a more advanced
	// feature and is not supported yet because it is VERY easy to
	// muck up.
	typedef ReturnType& NoReferenceReturn;
	
	// Expose appropriate AddRef method so we can be reference counted
	using EventAgent::AddRef;

	// Expose appropriate Release method so we can be reference counted
	using EventAgent::Release;
	
	ReturnType send(IEventDispatcher *dispatcher)
	{
		// The SyncRetEventAgent needs to make sure that after it
		// is dispatched and delivered the agent remains valid until
		// we are ready to return the value we filled in in the deliver
		// method.   In this case though we are going to use the
		// SmartPtr<> template and C++ scoping to do the AddRef and
		// Release so that we can return mRetValue directly without
		// making an explicit copy on the stack.
		
		// This is necessary because as soon as the dispatcher->sendEventSync
		// returns the only gauranteed reference to "this" object is the
		// one that is taken in this method.   Once we release that reference
		// "this" is possibly invalid and should not be accessed by again
		// before the method returns.
		
		SmartPtr<EventAgent> holdMe = this;
		
		if (dispatcher->isThreadCurrent())
		{
			deliver();
		}
		else
		{
			dispatcher->sendEventSync(this);
		}
		
		// Return the filled in return value.   This will make
		// an appropriate copy of mRetValue before the SmartPtr<>
		// falls out of scope and releases this object.
		return mRetValue;
	}
	
protected:
	virtual ~SyncRetEventAgent() {}
	
	ReturnType		mRetValue;
};


// Include the template definitions
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#include "EventAgentT.h"
#endif

#endif // JH_EVENT_AGENT_H_


