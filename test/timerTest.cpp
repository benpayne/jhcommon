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

#include "EventThread.h"
#include "Selector.h"
#include "Timer.h"
#include "TimeUtils.h"
#include "EventAgent.h"

#include "jh_memory.h"
#include "logging.h"

#include <unistd.h>
SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

enum {
	EVENT_ID_EV1,
	EVENT_ID_EV2,
	EVENT_ID_EV3,
	EVENT_ID_END	
};

int gEventCount = 0;

struct Event1 : public Event
{
	Event1()
	: Event( EVENT_ID_EV1, PRIORITY_HIGH )
	{
		gEventCount++;
		TimeUtils::getCurTime(&mSent);
	}		
	virtual ~Event1() { gEventCount--; }
	
	SMART_CASTABLE( EVENT_ID_EV1 );
	
	struct timeval mSent;
};

struct Event2 : public Event
{
	Event2( uint32_t p1, uint16_t p2 )
	: Event( EVENT_ID_EV2 ), mP1( p1 ), mP2( p2 )
	{
		gEventCount++;
		TimeUtils::getCurTime(&mSent);
	}		
	virtual ~Event2() { gEventCount--; }		
		
	SMART_CASTABLE( EVENT_ID_EV2 );
	
	uint32_t mP1;
	uint16_t mP2;
	struct timeval mSent;
};

struct Event3 : public Event
{
	Event3( Timer* timer )
	: Event( EVENT_ID_EV3 ), mTimer( timer )
	{
		gEventCount++;
		TimeUtils::getCurTime(&mSent);
	}
	virtual ~Event3() { gEventCount--; }		
		
	SMART_CASTABLE( EVENT_ID_EV3 );
	
	Timer* mTimer;
	struct timeval mSent;
};

class TestClass : public TimerListener
{
public:
	TestClass();
	virtual ~TestClass();
	
	void onTimeout( uint32_t private_data );

	void Func1();
	void Func2( uint32_t p1, uint16_t p2 );
	void Func3( Timer* timer = NULL );
	void Cancel2();
	void sendPeriodic( Timer* timer = NULL );
private:
	void ProcessFunc1( Event1 *ev );
	void ProcessFunc2( Event2 *ev );
	void ProcessFunc3( Event3 *ev );
	void handlePeriodic();

	EventMethod<TestClass,Event1>  *mEvent1Handler;
	EventMethod<TestClass,Event2>  *mEvent2Handler;
	EventMethod<TestClass,Event3>  *mEvent3Handler;

	Selector mThread;
//	EventThread mThread;
	Timer *mDefaultTimer;
	
	struct timeval mLastTime;
	
	SmartPtr<AsyncEventAgent> mPeriodEvent;
	int mPeriodElapsed;
	int mPeriodCount;
};


TestClass::TestClass()
{
	mDefaultTimer = TimerManager::getInstance()->getDefaultTimer();

	mEvent1Handler = jh_new EventMethod<TestClass,Event1>(
		this, &TestClass::ProcessFunc1, &mThread );
	mEvent2Handler = jh_new EventMethod<TestClass,Event2>(
		this, &TestClass::ProcessFunc2, &mThread );
	mEvent3Handler = jh_new EventMethod<TestClass,Event3>(
		this, &TestClass::ProcessFunc3, &mThread );
	mPeriodEvent = jh_new AsyncEventAgent0<TestClass>(
		this, &TestClass::handlePeriodic );
		
	mDefaultTimer->addTimer( this, 2000, 2345 );
	mDefaultTimer->addTimer( this, 500, 1234 );
	mDefaultTimer->addTimer( this, 3500, 4567 );
	mDefaultTimer->addTimer( this, 2500, 3456 );
	
	mPeriodElapsed = 0;
	mPeriodCount = 0;
}

TestClass::~TestClass()
{
	delete mEvent1Handler;
	delete mEvent2Handler;
	delete mEvent3Handler;
	
	mThread.removeAll();
	TimerManager::destroyManager();
}

static bool firstFunc1 = true;

void TestClass::onTimeout( uint32_t private_data )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	LOG( "private_data %d", private_data );
	
	// Make sure we can add and remove from the TimerManager while we are
	// handling a timeout via the TimerListener interface (this happens
	// on the TimerManager thread)
	if ( private_data == 2345 )
	{
		LOG( "\n\nRemoving event1 and event2 from clockthread context\n\n" );
		mThread.remove( Event1::GetEventId() );
		mThread.remove( Event2::GetEventId() );
		
		LOG( "\n\nAdding timeout from clockthread context\n\n" );
		mDefaultTimer->addTimer( this, 2000, 6669 );
		
		LOG( "\n\nCalling Func1 and Func2 again to resend timed events\n\n");
		Func1();
		Func2( 10, 15 );
	}
}

void TestClass::Func1()
{
	// Send event via EventThread (proper way to use timed events)
	mThread.sendTimedEvent( jh_new Event1(), 3000 );
}

void TestClass::Func2( uint32_t p1, uint16_t p2 )
{
	// Send event via TimerManager.  Also works but not recommended
	mDefaultTimer->sendTimedEvent( jh_new Event2( p1, p2 ), &mThread, 3500 );
}

void TestClass::Func3( Timer* timer )
{
	// Send using either specified timer or the default timer
	if ( timer == NULL )           
	{
		timer = mDefaultTimer;
	}
	timer->sendTimedEvent( jh_new Event3( timer ), &mThread, 200 );
}

void TestClass::sendPeriodic( Timer* testTimer )
{
	sleep(2);
	mPeriodEvent->sendPeriodically( &mThread, 166, testTimer );
	TimeUtils::getCurTime(&mLastTime);
}

void TestClass::Cancel2()
{
	// Remove event2 from the EventThread.  This will remove all timed
	// events and any events that have already queued from the timer into
	// the EventThread dispatcher.
	mThread.remove( Event2::GetEventId() );
}

void TestClass::ProcessFunc1( Event1 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	struct timeval curTime;
	TimeUtils::getCurTime( &curTime );
	
	int elapsed = TimeUtils::getDifference( &curTime, &ev->mSent );
	
	LOG("Received Func1 timed event, elapsed time is %d", elapsed);
	// Run once
	if (firstFunc1)
	{
		// Event2 should be coming immediately following this call.  Try to
		// remove it before it arrives
		LOG( "Received Func1 timeout, remove Func2 timed event(s)" );
		Cancel2();
		
		// Now re-send both events
		LOG( "And... do Func1, Func2 again" );
		Func1();
		Func2( 15, 20 );
		
		firstFunc1 = false;
	}
}

void TestClass::ProcessFunc2( Event2 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	LOG_NOTICE( "p1 %d p2 %d", ev->mP1, ev->mP2 );
	struct timeval curTime;
	TimeUtils::getCurTime( &curTime );
	
	int elapsed = TimeUtils::getDifference( &curTime, &ev->mSent );
	
	LOG("Received Func2 timed event, elapsed time is %d", elapsed);
}

void TestClass::ProcessFunc3( Event3 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	struct timeval curTime;
	TimeUtils::getCurTime( &curTime );
	
	int elapsed = TimeUtils::getDifference( &curTime, &ev->mSent );
	LOG_NOTICE( "Timer %p with tick time %d, elapsed is %d\n",
				ev->mTimer, ev->mTimer->getTickTime(), elapsed );
}

void TestClass::handlePeriodic()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	struct timeval curTime;
	TimeUtils::getCurTime( &curTime );
	
	int elapsed = TimeUtils::getDifference( &curTime, &mLastTime );
	mPeriodElapsed += elapsed;
	mPeriodCount++;
	LOG_NOTICE( "Periodic elapsed is %d, total periodic is %d, total elapsed is %d\n",
				elapsed, mPeriodCount, mPeriodElapsed );
	mLastTime = curTime;
}

int main( int argc, char*argv[] )
{	
	LOG_NOTICE( "Test Started" );

	TestClass *c = jh_new TestClass;
	SmartPtr<Timer> testTimer = jh_new Timer(166);

	// Initiate test by making back-to-back calls to Func1 and Func2
	c->Func1();
	c->Func2( 5, 10 );
	
	// Send two timed events.  One using default timer, one using
	// a timer with a resolution of 50ms.   Timed event is set to
	// fire after 250ms.   Default timer should actually be ~200
	// and testTimer should be ~250
	c->Func3();
	c->Func3( testTimer );
	
	c->sendPeriodic( testTimer );
	
	// Wait for a while until the test completes
	sleep( 10 );
	
	LOG_NOTICE( "Sleep done" );
	
	testTimer->stop();
	testTimer = NULL;
	delete c;
	
	if ( gEventCount != 0 )
		LOG_ERR_FATAL( "Events leaked %d", gEventCount );

	return 0;
}


