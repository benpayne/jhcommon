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

#include "jh_types.h"
#include "Timer.h"
#include "logging.h"
#include "jh_memory.h"
#include "Condition.h"
#include "Mutex.h"
#include "TimeUtils.h"
#include "EventAgent.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

TimerManager *TimerManager::mSingleton = NULL;

TimerManager *TimerManager::getInstance()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	if ( mSingleton == NULL ) 
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


TimerManager::TimerManager() : mMutex( true ), mTicks( 0 )
{
	TRACE_BEGIN( LOG_LVL_NOTICE );

	if ( mSingleton != NULL )
		LOG_ERR_FATAL( "Don't create a timer manager use getInstance()" );
	
	mClockThread = jh_new Runnable<TimerManager>("clockThread", this,
												 &TimerManager::clockHandler);
	mClockThread->Start();
}

TimerManager::~TimerManager()
{
	TRACE_BEGIN( LOG_LVL_NOTICE );

	mClockThread->Stop();
	mClockThread->Join();
	
	delete mClockThread;
}

void TimerManager::clockHandler()
{
	TRACE_BEGIN( LOG_LVL_INFO );	
	struct timespec start, cur;
	Condition c;
	Mutex m;
	AutoLock l(m);
	
	TimeUtils::getCurTime( &start );
	while ( !mClockThread->CheckStop() )
	{	
		// Wait for the tick to timeout
		if (not c.Wait(m, kMsPerTick))
		{
			TimeUtils::getCurTime( &cur );
			
			// Check to see if the clock was set back, or jumps
			// forward by more than 10 seconds. If so, update the
			// starting time to the current time and wait for the next
			// tick.
			if ( TimeUtils::getDifference( &cur, &start ) < 0 or
				 TimeUtils::getDifference( &cur, &start ) > 10000 )
			{
				TimeUtils::getCurTime( &start );
				handleTick();
			}
			else
			{	
				// Get the current time and count ticks until cur time and the 
				//  actual tick time are less the kMsPerTick apart.
				while ( TimeUtils::getDifference( &cur, &start ) > kMsPerTick )
				{
					TimeUtils::addOffset( &start, kMsPerTick );
					handleTick();
				}
			}
		}
		else
		{
			LOG_ERR_FATAL("Signal recieved on timer condition????\n");
		}
	}
}

void TimerManager::handleTick()
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	DebugAutoLock( mMutex );
	
	mTicks++;
	
	LOG( "%d ticks, %d items in list", mTicks, mList.size() );
	
	TimerNode timer;
	
	while (not mList.empty())
	{
		timer = *mList.begin();
		
		int32_t diff = timer.mTick - mTicks;
		
		// List is sorted with nearest items first.  Once we find a node
		// that is in the future we are done.
		if ( diff > 0 )
			break;
		
		// Remove the timer from the list now
		mList.pop_front();

		// If the timer doesn't have an event then we call the listener
		if ( timer.mEvent == NULL )
		{
			timer.mListener->onTimeout( timer.mPrivateData );
		}
		// Otherwise send the event
		else
		{
			if (timer.mRepeatMS == 0)
			{
				// Send the event to the specified dispatcher and
				// release our reference to the event
				timer.mDispatcher->sendEvent( timer.mEvent );				

				timer.mEvent = NULL;
			}
			else
			{
				// Send the periodic event
				timer.mDispatcher->sendEvent( timer.mEvent );
			}
		}
		
		// We need to re-send it in a few ticks		
		if (timer.mRepeatMS != 0)
		{
			unsigned newTicks = (timer.mRepeatMS + kMsPerTick - 1 - 
								 timer.mRemainingMS) / kMsPerTick;
			timer.mTick += newTicks;
			timer.mRemainingMS = (timer.mRepeatMS + timer.mRemainingMS) % 
				kMsPerTick;
			addTimerNode(timer);
		}
	}
}

void TimerManager::addTimer( TimerListener *listener, uint32_t msecs,
							 uint32_t private_data )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	DebugAutoLock( mMutex );

	if (listener == NULL) abort();

	// Calculate the timeout time in ticks
	uint32_t ticks = ( msecs +  kMsPerTick - 1 ) / kMsPerTick;
	
	// Initialize new timer node
	TimerNode timer;
	timer.mEvent = NULL;
	timer.mDispatcher = NULL;
	timer.mPrivateData = private_data;
	timer.mListener = listener;
	timer.mTick = mTicks + ticks;
	timer.mRepeatMS = 0;
	timer.mRemainingMS = 0;	

	LOG( "timer at %d ticks", timer.mTick );

	addTimerNode( timer );
}

void TimerManager::sendTimedEvent( Event *event, IEventDispatcher *dispatcher,
								   uint32_t msecs )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
		
	DebugAutoLock( mMutex );
	
	// Calculate the timeout time in ticks
	uint32_t ticks = ( msecs +  kMsPerTick - 1 ) / kMsPerTick;
	
	// Initialize new timer node
	TimerNode timer;
	timer.mEvent = event; 
	timer.mDispatcher = dispatcher;
	timer.mPrivateData = 0;
	timer.mListener = NULL;
	timer.mTick = mTicks + ticks;
	timer.mRepeatMS = 0;
	timer.mRemainingMS = 0;

	LOG( "timer at %d ticks", timer.mTick );
	
	addTimerNode( timer );
}

void TimerManager::sendPeriodicEvent( Event *event, 
									  IEventDispatcher *dispatcher,
									  uint32_t period )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
		
	DebugAutoLock( mMutex );
	
	// Calculate the timeout time in ticks
	uint32_t ticks = ( period +  kMsPerTick - 1 ) / kMsPerTick;
	
	// Initialize new timer node
	TimerNode timer;
	timer.mEvent = event; 
	timer.mDispatcher = dispatcher;
	timer.mPrivateData = 0;
	timer.mListener = NULL;
	timer.mTick = mTicks + ticks;
	timer.mRepeatMS = period;
	timer.mRemainingMS = 0;
	
	LOG( "timer at %d ticks", timer.mTick );
	
	addTimerNode( timer );
}

void TimerManager::removeTimedEvent( Event *ev )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	TimerNode timer;
	
	DebugAutoLock( mMutex );

	JetHead::list<TimerNode>::iterator node = mList.begin();
	while (node != mList.end())
	{
		// Cast the node as a TimerNode
		timer = *node;
		
		// Check if the timer node has an event for the dispatcher
		// specified
		if ( (Event*)timer.mEvent == ev )
		{
			node = node.erase();
			continue;
		}
		++node;
	}	
}

void TimerManager::removeAgentsByReceiver( void* receiver, 
										   IEventDispatcher* dispatcher )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	TimerNode timer;

	DebugAutoLock( mMutex );

	JetHead::list<TimerNode>::iterator i = mList.begin();
	while (i != mList.end())
	{
		// Get the TimerNode
		timer = *i;

		if ( timer.mEvent->getEventId() == Event::kAgentEventId and
			 timer.mDispatcher == dispatcher )
		{
			LOG_NOTICE("Found an event agent, looking more closely");
			EventAgent* agent = static_cast<EventAgent*>( (Event*)timer.mEvent );
			if (agent->getDeliveryTarget() == receiver)
			{
				LOG_NOTICE("Found a match, removing");
				i = i.erase();
				continue;
			}
		}
		++i;
	}
}

void TimerManager::removeTimedEvent( Event::Id eventId,
									 IEventDispatcher *dispatcher )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	TimerNode timer;
	
	DebugAutoLock( mMutex );

	JetHead::list<TimerNode>::iterator i = mList.begin();
	while (i != mList.end())
	{
		// Get the TimerNode
		timer = *i;
		
		// Check if the timer node has an event for the dispatcher
		// specified
		if ( timer.mDispatcher == dispatcher && timer.mEvent != NULL)
		{
			// Now test if the timer node event has a matching id or if the
			// id is Event::kInvalidEventId.  Event::kInvalidEventId is a
			// special id used to remove all events for a dispatcher
			if ( timer.mEvent->getEventId() == eventId
				 || eventId == Event::kInvalidEventId )
			{
				i = i.erase();
				continue;
			}
		}
		++i;
	}
	
}

void TimerManager::addTimerNode( const TimerNode& newTimer )
{
	TRACE_BEGIN( LOG_LVL_NOISE );

	int32_t diff;
	bool inserted = false;

	// Add new timer node to the list (sorted)	
	for (JetHead::list<TimerNode>::iterator i = mList.begin();
		 i != mList.end(); ++i)
	{
		TimerNode timer = *i;
		diff = newTimer.mTick - timer.mTick;
		
		if ( diff < 0 )
		{
			i.insertBefore(newTimer);			
			inserted = true;
			break;
		}
	}
	
	if ( not inserted )
	{
		mList.push_back( newTimer );
	}
}

