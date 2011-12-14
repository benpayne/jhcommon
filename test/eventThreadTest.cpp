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
#include "jh_memory.h"
#include "logging.h"
#include "Timer.h"

#include <unistd.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

int gEventCount = 0;

class TestClass
{
public:
	TestClass();
	~TestClass();
	
	void Func1();
	void Func2( uint32_t p1, uint16_t p2 );
	void Func3( uint32_t p1 );

	void end( int i );
	void cancel();
	
private:
	enum {
		EVENT_ID_EV1,
		EVENT_ID_EV2,
		EVENT_ID_EV3,
		EVENT_ID_END	
	};

	struct Event1 : public Event
	{
		Event1() : Event( EVENT_ID_EV1, PRIORITY_HIGH ) { gEventCount++; }		
		~Event1() { gEventCount--; }		
		SMART_CASTABLE( EVENT_ID_EV1 );
	};
	
	struct Event2 : public Event
	{
		Event2( uint32_t p1, uint16_t p2 )
			: Event( EVENT_ID_EV2 ), mP1( p1 ), mP2( p2 ) { gEventCount++; }
		~Event2() { gEventCount--; }		
			
		SMART_CASTABLE( EVENT_ID_EV2 );
		
		uint32_t mP1;
		uint16_t mP2;
	};
	
	struct Event3 : public Event
	{
		Event3( uint32_t p1 )
			: Event( EVENT_ID_EV3 ), mP1( p1 ) { gEventCount++; }
		~Event3() { gEventCount--; }		
			
		SMART_CASTABLE( EVENT_ID_EV3 );
		
		uint32_t mP1;
	};


	void ProcessFunc1( Event1 *ev );
	void ProcessFunc2( Event2 *ev );
	void ProcessFunc3( Event3 *ev );

	EventMethod<TestClass,Event1>  *mEvent1Handler;
	EventMethod<TestClass,Event2>  *mEvent2Handler;
	EventMethod<TestClass,Event3>  *mEvent3Handler;

	EventThread mThread;
};


TestClass::TestClass()
{
	mEvent1Handler = jh_new EventMethod<TestClass,Event1>(
		this, &TestClass::ProcessFunc1, &mThread );
	mEvent2Handler = jh_new EventMethod<TestClass,Event2>(
		this, &TestClass::ProcessFunc2, &mThread );
	mEvent3Handler = jh_new EventMethod<TestClass,Event3>(
		this, &TestClass::ProcessFunc3, &mThread );
}

TestClass::~TestClass()
{
	delete mEvent1Handler;
	delete mEvent2Handler;
	delete mEvent3Handler;
}

void TestClass::Func1()
{
	mThread.sendEvent( jh_new Event1() );
}

void TestClass::Func2( uint32_t p1, uint16_t p2 )
{
	mThread.sendEventSync( jh_new Event2( p1, p2 ) );
}

void TestClass::Func3( uint32_t p1 )
{
	mThread.sendEventSync( jh_new Event3( p1 ) );
}

void TestClass::end( int i )
{
	mThread.sendTimedEvent( jh_new Event3( 0 ), i * 1000 );
}

void TestClass::cancel()
{
	mThread.remove( Event3::GetEventId() );
}

void TestClass::ProcessFunc1( Event1 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
}

void TestClass::ProcessFunc2( Event2 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	LOG_NOTICE( "p1 %d p2 %d", ev->mP1, ev->mP2 );
}

void TestClass::ProcessFunc3( Event3 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	LOG_NOTICE( "p1 %d", ev->mP1 );	
}


int main( int argc, char*argv[] )
{
	LOG_NOTICE( "Test Started" );

	TimerManager* timer = TimerManager::getInstance();
	
	TestClass *c = jh_new TestClass;
	
	c->Func1();
	c->Func2( 1, 2 );
	c->Func3( 3 );

	c->end( 1 );
	c->end( 2 );
	
	//c->cancel();
	
	LOG_NOTICE( "Event Count is %d", gEventCount );
	
	sleep( 3 );
	
	delete c;
	
	LOG_NOTICE( "Event Count is %d", gEventCount );
	
	if ( gEventCount != 0 )
		LOG_ERR_FATAL( "Events leaked %d", gEventCount );
	
	return 0;
}

