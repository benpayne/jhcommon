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

#include <unistd.h>
#include "TimeUtils.h"
#include "jh_memory.h"
#include "logging.h"

using namespace std;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class TimeUtilsTest : public TestCase
{
public:
	TimeUtilsTest( int test_id );
	virtual ~TimeUtilsTest() {}
	
private:
	void Run();
	
	void test1();
	void test2();
	void test3();
	
	int mTest;
};

TimeUtilsTest::TimeUtilsTest( int test_id ) : TestCase( "TimeUtils" ), mTest( test_id )
{
	char name[ 32 ];
	
	sprintf( name, "Time Utils Test %d", test_id );
	
	SetTestName( name );
}

void TimeUtilsTest::Run()
{	
	LOG_NOTICE( "UDP Test Started" );

	switch ( mTest )
	{
		case 1:
			test1();
			break;
		case 2:
			test2();
			break;
		case 3:
			test2();
			break;
	}
	
	TestPassed();
}

void TimeUtilsTest::test1()
{
	struct timeval tv;
	
	TimeUtils::getCurTime( &tv );
	
	if ( tv.tv_sec == 0 )
		TestFailed( "Cur time with timeval is 0" );
	
	struct timespec ts;
	
	TimeUtils::getCurTime( &ts );
	
	if ( ts.tv_sec == 0 )
		TestFailed( "Cur time with timespec is 0" );
}

struct TV_TestData {
	struct timeval tv1;
	struct timeval tv2;
	int expected_diff;
};

TV_TestData tv_test_data[] = {
	{ { 12, 0       }, { 10, 0       }, 2000 },
	{ { 12, 500000  }, { 10, 500000  }, 2000 },
	{ { 10, 0       }, { 12, 0       }, -2000 },
	{ { 10, 500000  }, { 12, 500000  }, -2000 },
	{ { 12, 0       }, { 10, 500000  }, 1500 },
	{ { 12, 0       }, { 0,  0       }, 12000 }
};

void TimeUtilsTest::test2()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( tv_test_data ); i++ )
	{
		int ms = TimeUtils::getDifference( &tv_test_data[ i ].tv1, 
			&tv_test_data[ i ].tv2 );
		
		if ( ms != tv_test_data[ i ].expected_diff )
		{
			TestFailed( "Test %d failed, expected %d, got %d", i, tv_test_data[ i ].expected_diff, ms );
		}
	}
	
	for ( int i = 0; i < JH_ARRAY_SIZE( tv_test_data ); i++ )
	{
		struct timeval tv = tv_test_data[ i ].tv2;
		TimeUtils::addOffset( &tv, tv_test_data[ i ].expected_diff );
		
		if ( tv.tv_sec != tv_test_data[ i ].tv1.tv_sec || 
			tv.tv_usec != tv_test_data[ i ].tv1.tv_usec )
		{
			TestFailed( "Test %d failed (%d, %d)", i, tv.tv_sec, tv.tv_usec );
		}
	}	
}

struct TS_TestData {
	struct timespec ts1;
	struct timespec ts2;
	int expected_diff;
};

TS_TestData ts_test_data[] = {
	{ { 12, 0          }, { 10, 0          }, 2000 },
	{ { 12, 500000000  }, { 10, 500000000  }, 2000 },
	{ { 10, 0          }, { 12, 0          }, -2000 },
	{ { 10, 500000000  }, { 12, 500000000  }, -2000 },
	{ { 12, 0          }, { 10, 500000000  }, 1500 },
	{ { 12, 0          }, { 0,  0          }, 12000 }
};


void TimeUtilsTest::test3()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( ts_test_data ); i++ )
	{
		int ms = TimeUtils::getDifference( &ts_test_data[ i ].ts1, 
			&ts_test_data[ i ].ts2 );
		
		if ( ms != ts_test_data[ i ].expected_diff )
		{
			TestFailed( "Test %d failed, expected %d, got %d", i, ts_test_data[ i ].expected_diff, ms );
		}
	}
	
	for ( int i = 0; i < JH_ARRAY_SIZE( ts_test_data ); i++ )
	{
		struct timespec ts = ts_test_data[ i ].ts2;
		TimeUtils::addOffset( &ts, ts_test_data[ i ].expected_diff );
		
		if ( ts.tv_sec != ts_test_data[ i ].ts1.tv_sec || 
			ts.tv_nsec != ts_test_data[ i ].ts1.tv_nsec )
		{
			TestFailed( "Test %d failed (%d, %d)", i, ts.tv_sec, ts.tv_nsec );
		}
	}	
}

int main( int argc, char*argv[] )
{	
	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ 10 ];
	
	test_set[ 0 ] = jh_new TimeUtilsTest( 1 );
	test_set[ 1 ] = jh_new TimeUtilsTest( 2 );
	test_set[ 2 ] = jh_new TimeUtilsTest( 3 );
	
	runner.RunAll( test_set, 3 );

	return 0;
}
