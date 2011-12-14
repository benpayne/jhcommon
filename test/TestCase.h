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
#include <string>
#include <stdlib.h>
#include <stdarg.h>

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
		printf( " PASSED\n" );
		mTestComplete = true;
		mTestPassed = true;
	}
	
	void TestFailed( const char *fmt, ... )
	{
		va_list params;

		va_start(params, fmt);
		
		printf( " FAILED\n" );
		vprintf( fmt, params );
		printf( "\n" );
		// when Test Fails we exit the thread.
		mTestComplete = true;
		mTestPassed = false;
		Thread::Exit();
	}
		
	virtual void Run() = 0;
	
private:
	bool		mTestComplete;
	bool		mTestPassed;
	std::string	mTestName;
	Runnable<TestCase>	mThread;
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
};

#endif // JH_TEST_CASE_H_

