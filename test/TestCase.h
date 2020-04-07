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

#ifndef JH_TEST_CASE_H_
#define JH_TEST_CASE_H_

#include "Thread.h"
#include "Mutex.h"
#include <stdlib.h>
#include <cstdarg>
#include "jh_string.h"

#define TestFailed( str... ) TestFailedInternal( __FILE__, __LINE__, ## str )

class TestCase
{
public:
	TestCase( const char *thread_name ) : mThread( thread_name, this, &TestCase::Run ) 
	{
	}
	
	virtual ~TestCase() 
	{
	}
	
	bool Start() 
	{
		if ( mTestName == "" )
		{
			printf( "Bad Test Case, please call SetTestName in TestCase's constructor\n" );
			return false;
		}

		printf( "Running test %s:", mTestName.c_str() );
		fflush( stdout );
		
		// start thread to run test.
		mThread.Start();
		// wait for test to complete.
		mThread.Join();

		// ensure that it completed.		
		if ( not mTestComplete )
		{
			printf( " FAILED\nBad Test Case, must call TestPassed or TestFailed before returning\n" );
			return false;
		}

		if ( mTestPassed )
			printf( " PASSED\n" );
		else
		{
			printf( " FAILED at %s:%d\n", mFilename.c_str(), mLineNumber );
			printf( "%s\n", mErrorString.c_str() );
		}
		// return test status.
		return mTestPassed;
	}

	const char *GetTestName()
	{
		return mTestName.c_str();
	}
	
protected:
	void SetTestName( const char *name )
	{
		mTestName = name;
	}

	void TestPassed()
	{
		mMutex.Lock();
		
		if ( mTestComplete )
		{
			if ( mTestPassed )
				printf( " BAD TEST CASE, TestPassed called twice\n" );

			mTestPassed = false;
		}
		else
		{
			mTestComplete = true;
			mTestPassed = true;
		}
		mMutex.Unlock();
	}
	
	void TestFailedInternal( const char *filename, int line_num, const char *fmt, ... ) __attribute__ ((__format__ (__printf__, 4, 5)))
	{
		va_list params;

		mMutex.Lock();

		mFilename = filename;
		mLineNumber = line_num;
		
		va_start( params, fmt );
		JetHead::stl_vsprintf( mErrorString, fmt, params );
		va_end( params );

		if ( mTestComplete )
			printf( " BAD TEST CASE, TestPassed/TestFailed already called\n" );

		mTestComplete = true;
		mTestPassed = false;
		mMutex.Unlock();
		
		if ( *(Thread::GetCurrent()) == mThread )
			Thread::Exit();
		else
			mThread.Stop();
	}
		
	virtual void Run() = 0;
	
private:
	bool		mTestComplete;
	bool		mTestPassed;
	std::string	mTestName;
	Runnable<TestCase>	mThread;
	Mutex		mMutex;
	JHSTD::string mFilename;
	int			mLineNumber;
	JHSTD::string mErrorString;
};

class TestSuite
{
public:
	TestSuite() {}

	virtual ~TestSuite()
	{
		for ( int i = 0; i < (int)mTestCases.size(); i++ )
		{
			delete mTestCases[ i ];
		}
	}
	
	void AddTestCase( TestCase *tc )
	{
		mTestCases.push_back( tc );
	}
	
private:
	JHSTD::vector<TestCase*>	mTestCases;

	friend class TestRunner;
};

class TestRunner
{
public:
	TestRunner( const char *ts_name )
	{
		std::string filename( ts_name );
		std::string::size_type i = filename.rfind( "/" );
		filename.erase( 0, i + 1 );
		filename += ".log";	
		//printf( "logging to file %s\n", filename.c_str() );
		FILE *log_file = fopen( filename.c_str(), "w+" );
		logging_set_file( log_file );
	}
	
	void RunAll( TestCase **array, int numTests )
	{
		for( int i = 0; i < numTests; i++ )
		{
			LOG_NOTICE( "Running test %s", array[ i ]->GetTestName() );
			if ( array[ i ]->Start() == false )
			{
				exit( EXIT_FAILURE );
			}
			delete array[ i ];
		}
	}

	void RunAll( TestSuite &ts )
	{
		for( int i = 0; i < (int)ts.mTestCases.size(); i++ )
		{
			LOG_NOTICE( "Running test %s", ts.mTestCases[ i ]->GetTestName() );
			if ( ts.mTestCases[ i ]->Start() == false )
			{
				exit( EXIT_FAILURE );
			}
			delete ts.mTestCases[ i ];
			ts.mTestCases[ i ] = NULL;
		}
	}	
};

#endif // JH_TEST_CASE_H_

