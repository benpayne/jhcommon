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

/* The goal of this program is half to test out the logging features
 * and half to make it easy for someone to understand the logging
 * features without having to dig through the logging.h source.
 */

#include "logging.h"

// First set the cat level.
// There are basically two choices: ALL and DEFAULT
// There are a few other choices: TRACE, ERR, .. but some you probably
// will never want and other are on by default.  See logging.h:281 to find out
// more.
SET_LOG_CAT( LOG_CAT_DEFAULT );

// Second set the log level.
// There are basically three choices: INFO, NOTICE, NOISE.
// Just as with cat there are other choices but you probably don't want 
// them.  See logging.h:294 for more information.
SET_LOG_LEVEL( LOG_LVL_NOTICE );

// You need to set the CAT and LEVEL before including this header.
#include "TestCase.h"

class LogTest2 : public TestCase
{

public:
	LogTest2( int test_id );
	virtual ~LogTest2() { ; };

private:
	void Run();
	void Test1(); 
	void Test2();
	void Test3();

	int mTest;

};

LogTest2::LogTest2( int test_id )
	: TestCase( "LogTest2" ), mTest( test_id )
{
	char name[ 32 ];
	sprintf( name, "Log Test2 %d", test_id );
	SetTestName( name );
	
}

void LogTest2::Run()
{
	
	LOG_NOTICE( "LogTest2 Started" ); // Ignore this for now.
	switch( mTest )
	{
	case 1:
		Test1();
		break;
	case 2:
		Test2();
		break;
	case 3:
		Test3();
		break;
	}

	TestPassed();
}

void LogTest2::Test1()
{
	
	LOG_NOTICE( "This will print a notice to the logging file all "
				"LOG_ functions include the file and line number they "
				"came from. But they don't pretty print" );
	
	// All of the LOG_ and ASSERT_ functions work just like printf
	double d = 3.14;
	int i = -275;
	char c = 'c';
	char cstring[] = { 'a', 'b', 'c', 'f', 0x0 };
	LOG_NOTICE( "This is how you print a double(float): %f . "
				"This is how you print a int: %d. "
				"A char:%c  and a c-style string: %s (be sure it is null "
				"terminated).", d, i, c, cstring );

	LOG_WARN( "This will print a warning to the logging file with "
			  " information about the file name and line number");
	
	LOG_WARN_PERROR( "This will do the same as above but also include the "  
					 "value of errno?");
	
		
	// This is not fatal but will log a error message.
	LOG_ERR( "This will print an error to the logging file."  );
	// This is a fatal error message it will stop execution of your program.
	//LOG_ERR_FATAL( "This would do the same as above but also stop the "
	//			   "execution of the program"  );

	
	// This is not fatal; and will only log if the first argument
	// evaluates to false.
	ASSERT_WARN( false, "This will print a warning to the logging file "
				"since the assert failed." );
	// This is fatal if the first argument evaluates to false.
	ASSERT_ERR( true, "Nothing will happen here."  );

	
	
	
}

void LogTest2::Test2()
{

	LOG_ERR_FATAL( "Test2 is here to show what happens when LOG_ERR_FATAL "
				   "gets called.");
}

void LogTest2::Test3()
{
	// Note: Since Test2 doesn't pass this never actually gets called.
	// If you want it to get called comment out the LOG_ERR_FATAL in Test2.
	ASSERT_ERR( false, "Test3 is here to show what happens when an "
				"ASSERT_ERR fires."  );

}


int main( int argc, char* argv[] )
{
	TestRunner runner( argv[ 0 ] );
	TestCase* test_set[ 10 ];

	test_set[ 0 ] = jh_new LogTest2( 1 );
	test_set[ 1 ] = jh_new LogTest2( 2 );
	test_set[ 2 ] = jh_new LogTest2( 3 );

	runner.RunAll( test_set, 3 );
	
	
	return 0;



}

