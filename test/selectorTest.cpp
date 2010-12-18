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

#include "Selector.h"
#include "jh_memory.h"
#include "logging.h"

#include <unistd.h>
SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class EventTest : public TestCase
{
public:
	EventTest( Selector *s, int number );
	~EventTest();

private:
	void Func1();
	void Func2( uint32_t p1, uint16_t p2 );
	int Func3( uint32_t p1 );

	enum {
		kEventEv1,
		kEventEv2,
		kEventEv3,
		kEventLast	
	};
	
	struct Event1 : public Event
	{
		Event1() : Event( kEventEv1, PRIORITY_HIGH ) { EventTest::sEventCount++; }		
		~Event1() { EventTest::sEventCount--; }		
		SMART_CASTABLE( kEventEv1 );
	};
	
	struct Event2 : public Event
	{
		Event2( uint32_t p1, uint16_t p2 )
			: Event( kEventEv2 ), mP1( p1 ), mP2( p2 ) { EventTest::sEventCount++; }
		~Event2() { EventTest::sEventCount--; }		
			
		SMART_CASTABLE( kEventEv2 );
		
		uint32_t mP1;
		uint16_t mP2;
	};
	
	struct Event3 : public Event
	{
		Event3( uint32_t p1 )
			: Event( kEventEv3 ), mP1( p1 ), mValid( true ) { EventTest::sEventCount++; }
		~Event3() { mValid = false; EventTest::sEventCount--; }		
			
		SMART_CASTABLE( kEventEv3 );
		
		uint32_t mP1;
		bool mValid;
	};

	void ProcessFunc1( Event1 *ev );
	void ProcessFunc2( Event2 *ev );
	void ProcessFunc3( Event3 *ev );

	EventMethod<EventTest,Event1>  *mEvent1Handler;
	EventMethod<EventTest,Event2>  *mEvent2Handler;
	EventMethod<EventTest,Event3>  *mEvent3Handler;

	int mTestNum;
	int mTestState;
	static int sEventCount;	
	Selector *mSelector;
	
	void Run()
	{
		mTestState = 0;
		
		switch( mTestNum )
		{
			case 1:
				Func1();
				// Async Event give it 100ms to be recieved.				
				usleep( 100000 );
				if ( mTestState != 1 )
					TestFailed( "Event not recieved" );
				else if ( sEventCount != 0 )
					TestFailed( "Event count != 0 (%d)", sEventCount );					
				else
					TestPassed();
				break;
				
			case 2:
				Func2( 1, 2 );
				// Sync Event no need to sleep.
				if ( mTestState != 1 )
					TestFailed( "Event not recieved" );
				else if ( sEventCount != 0 )
					TestFailed( "Event count != 0 (%d)", sEventCount );					
				else
					TestPassed();
				break;

			case 3:
				int res = Func3( 3 );
				// Sync Event no need to sleep.
				if ( mTestState != 1 )
					TestFailed( "Event not recieved" );
				else if ( res != 1000 )
					TestFailed( "Result not valid" );					
				else if ( sEventCount != 0 )
					TestFailed( "Event count != 0 (%d)", sEventCount );					
				else
					TestPassed();
				break;
		}
	}	
};

int EventTest::sEventCount = 0;

EventTest::EventTest( Selector *s, int number ) : TestCase( "EventTest" ), 
	mTestNum( number ), mSelector( s )
{
	switch( number )
	{
		case 1:
			SetTestName( "Async Event" );
			break;
		case 2:
			SetTestName( "Sync Event" );
			break;
		case 3:
			SetTestName( "Sync Event with return" );
			break;
	}

	mEvent1Handler = jh_new EventMethod<EventTest,Event1>(
		this, &EventTest::ProcessFunc1, mSelector );
	mEvent2Handler = jh_new EventMethod<EventTest,Event2>(
		this, &EventTest::ProcessFunc2, mSelector );
	mEvent3Handler = jh_new EventMethod<EventTest,Event3>(
		this, &EventTest::ProcessFunc3, mSelector );

}

EventTest::~EventTest()
{
	delete mEvent1Handler;
	delete mEvent2Handler;
	delete mEvent3Handler;
}

void EventTest::Func1()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	mSelector->sendEvent( jh_new Event1() );
}

void EventTest::Func2( uint32_t p1, uint16_t p2 )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	mSelector->sendEventSync( jh_new Event2( p1, p2 ) );
}

int EventTest::Func3( uint32_t p1 )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	SmartPtr<Event3> ev = jh_new Event3( p1 );
	mSelector->sendEventSync( ev );
	if ( ev->mValid == false )
		TestFailed( "Ev had been deleted" );
	return ev->mP1;
}

void EventTest::ProcessFunc1( Event1 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	mTestState++;
}

void EventTest::ProcessFunc2( Event2 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	LOG_NOTICE( "p1 %d p2 %d", ev->mP1, ev->mP2 );
	
	mTestState++;
	if ( ev->mP1 != 1 || ev->mP2 != 2 )
		TestFailed( "P1 and/or P2 not expected values" ); 		
}

void EventTest::ProcessFunc3( Event3 *ev )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	LOG_NOTICE( "p1 %d", ev->mP1 );

	mTestState++;
	if ( ev->mP1 != 3 )
		TestFailed( "P1 not expected values" );
	ev->mP1 = 1000;
}

class OtherListenerClass;

class SelectorTest : public TestCase, public SelectorListener
{
public:
	SelectorTest( Selector *s );
	virtual ~SelectorTest();
	
private:

	OtherListenerClass *other;
	Selector *mSelector;
	int		mPipe[ 2 ];

	int mTestState;	
	int mEventCount;	

	void SendData();
	void processFileEvents( int fd, short events, jh_ptr_int_t private_data );
	
	void Run()
	{
		mTestState = 0;
		
		SendData();
		// wait 100ms for selector to wake up.
		usleep( 100000 );
	
		if ( mTestState != 2 )
			TestFailed( "Both listeners not called" );
	
		SendData();
		// wait 100ms for selector to wake up.
		usleep( 100000 );
	
		if ( mTestState != 4 )
			TestFailed( "Both listeners not called" );

		SendData();
		// wait 100ms for selector to wake up.
		usleep( 100000 );
	
		if ( mTestState != 6 )
			TestFailed( "Both listeners not called" );

		TestPassed();
	}

	friend class OtherListenerClass;
};

class OtherListenerClass : public SelectorListener
{
public:
	OtherListenerClass( int fd, int pd, SelectorTest *testCase ) : 
		mFd( fd ), mPrivateData( pd ), mTestCase( testCase ) {}
	virtual ~OtherListenerClass() {}
	
	void processFileEvents( int fd, short events, jh_ptr_int_t private_data );

private:
	int mFd;
	int mPrivateData;
	SelectorTest *mTestCase;	
};

SelectorTest::SelectorTest( Selector *s ) : TestCase( "SelectorTest" ),
	mSelector( s )
{
	SetTestName( "File Events" );
	int res = pipe( mPipe );
		
	if ( res != 0 )
		LOG_ERR_FATAL( "failed to create pipe" );

	other = jh_new OtherListenerClass( mPipe[ 0 ], 11, this );
	
	mSelector->addListener( mPipe[ 0 ], POLLIN, this, 10 );
	mSelector->addListener( mPipe[ 0 ], POLLIN, other, 11 );
}

SelectorTest::~SelectorTest()
{
	mSelector->removeListener( mPipe[ 0 ], this );
	mSelector->removeListener( mPipe[ 0 ], other );
	close( mPipe[ 0 ] );
	close( mPipe[ 1 ] );
}


void SelectorTest::SendData()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	write( mPipe[ 1 ], "FOOBAR", 6 );	
}

void SelectorTest::processFileEvents( int fd, short events, jh_ptr_int_t private_data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	uint8_t buffer[ 10 ];

	LOG_NOTICE( "fd %d, events %x private data %d", fd, events, private_data );
	
	if ( events & POLLNVAL )
	{
		TestFailed( "Recieved POLLNVAL" );
	}
	
	if ( fd == mPipe[ 0 ] && private_data == 10 && ( events & POLLIN ) )
	{
		mTestState++;
	}
	else
	{
		TestFailed( "First Listener Bad Params" );
	}
	
	if ( events & POLLIN )
	{
		int res = read( fd, buffer, 10 );

		print_buffer( "READ", buffer, res );
	}
}

void OtherListenerClass::processFileEvents( int fd, short events, jh_ptr_int_t private_data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	if ( events & POLLNVAL )
	{
		mTestCase->TestFailed( "Recieved POLLNVAL" );
	}
	
	if ( fd == mFd && private_data == mPrivateData && ( events & POLLIN ) != 0 )
	{
		mTestCase->mTestState++;
	}
	else
	{
		mTestCase->TestFailed( "Second Listener Bad Params" );
	}	
}

int main( int argc, char*argv[] )
{
	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ 10 ];
	
	Selector testSelector;
	test_set[ 0 ] = jh_new EventTest( &testSelector, 1 );
	test_set[ 1 ] = jh_new EventTest( &testSelector, 2 );
	test_set[ 2 ] = jh_new EventTest( &testSelector, 3 );
	test_set[ 3 ] = jh_new SelectorTest( &testSelector );
	
	runner.RunAll( test_set, 4 );

	return 0;
}


