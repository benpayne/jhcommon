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
 
#include "logging.h"
#include "jh_memory.h"

#include <string.h>


SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

#define TEST_JH_STRING	1

#ifdef TEST_JH_STRING
#define USE_JETHEAD_STRING
#endif

#include "jh_string.h"

using namespace JHSTD;

class StringTest : public TestCase
{
public:
	StringTest( int number ) : 
		TestCase( "StringTest" ), mTestNum( number ) 
	{
		char name[ 32 ];
		snprintf( name, 32, "StringTest%d", number + 1 );
		name[ 31 ] = '\0';
		SetTestName( name );	
	}

	virtual ~StringTest() {}
	
private:
	int mTestNum;
	
	void Run()
	{	
		switch( mTestNum )
		{
		case 0:
			Test1();
			break;
		case 1:
			Test2();
			break;
		case 2:
			Test3();
			break;
		case 3:
			Test4();
			break;
		case 4:
			Test5();
			break;
		case 5:
			Test6();
			break;
		case 6:
			Test7();
			break;
		case 7:
			Test8();
			break;
		case 8:
			Test9();
			break;
		case 9:
			Test10();
			break;
		case 10:
			Test11();
			break;
		case 11:
			Test12();
			break;
		default:
			break;
		}
	}

	void Test1()
	{
		// test constructors and assignments, requiers that c_str, length and size, be working also.
		string t;
		string t1 = "this is t1";
		string t2 = t1;
		string t3( "this is t3 " );
		string t4( t3 );
		string t5( "this is t5    ", 12 );
		string t6;
		string t7;
		string t8;
		
		t6 = 'a';
		t7.assign( "this is t5    ", 7 );
		t8.assign( "this is t5    ", 5, 2 );
		
		if ( t.length() != 0 || t.size() != 0 )
			TestFailed( "default contrutor made non-zero sized string" );

		if ( t1.length() != 10 || strcmp( t1.c_str(), "this is t1" ) != 0 )
			TestFailed( "t1 not correct" );

		if ( t2.length() != 10 || strcmp( t2.c_str(), "this is t1" ) != 0 )
			TestFailed( "t2 not correct" );

		if ( t3.length() != 11 || strcmp( t3.c_str(), "this is t3 " ) != 0 )
			TestFailed( "t3 not correct" );

		if ( t4.length() != 11 || strcmp( t4.c_str(), "this is t3 " ) != 0 )
			TestFailed( "t4 not correct" );

		if ( t5.length() != 12 || strcmp( t5.c_str(), "this is t5  " ) != 0 )
			TestFailed( "t5 not correct" );

		if ( t6.length() != 1 || strcmp( t6.c_str(), "a" ) != 0 )
			TestFailed( "t6 not correct" );

		if ( t7.length() != 7 || strcmp( t7.c_str(), "this is" ) != 0 )
			TestFailed( "t7 not correct" );
		
		if ( t8.length() != 2 || strcmp( t8.c_str(), "is" ) != 0 )
			TestFailed( "t8 not correct" );

		TestPassed();
	}

	void Test2()
	{
		const char *s = "this is t1";
		const char *s2 = "i am a new";
		string t1( s );

		for ( int i = 0; i < (int)strlen( s ); i++ )
		{
			if ( t1[ i ] != s[ i ] )
				TestFailed( "operator[] failed" );
			
			t1[ i ] = s2[ i ];
		}

		Test2Sub( t1, s2 );		
		TestPassed();
	}
	
	void Test2Sub( const string &s, const char *s2 )
	{
		for ( int i = 0; i < (int)s.length(); i++ )
		{
			if ( s[ i ] != s2[ i ] )
				TestFailed( "operator[] failed after assignment" );			
		}
	}
	
	void Test3()
	{
		string t1 = "abc";
		string t2 = "def";
		string t3 = "abc";
		string t4 = "abcdef";
		
		if ( t1.compare( t2 ) >= 0 )
			TestFailed( "t1 compare t2" );
		
		if ( t2.compare( t1 ) <= 0 )
			TestFailed( "t2 compare t1" );

		if ( t1.compare( t3 ) != 0 )
			TestFailed( "t1 compare t3" );

		if ( t1.compare( t4 ) >= 0 )
			TestFailed( "t1 compare t4" );

		if ( t4.compare( t1 ) <= 0 )
			TestFailed( "t4 compare t1" );

		if ( t1 != t3 )
			TestFailed( "t1 != t3" );

		if ( t1 == t2 )
			TestFailed( "t1 == t2" );

		if ( t1 != "abc" )
			TestFailed( "t1 != abc" );

		if ( t1 == "def" )
			TestFailed( "t1 == def" );

		if ( "abc" != t1 )
			TestFailed( "abc != t1" );

		if ( "def" == t1 )
			TestFailed( "def == t1" );
		
		TestPassed();
	}
	
	void Test4()
	{
		// Test operator+=
		string t1 = "Start of";
		string t2 = " end";
		
		t1 += t2;
		
		if ( t1 != "Start of end" )
		{
			LOG_NOTICE( "string is %s", t1.c_str() );
			TestFailed( "+= string failed" );
		}
		
		t1 += " now";		

		if ( t1 != "Start of end now" )
			TestFailed( "+= const char * failed" );

		t1 += '!';

		if ( t1 != "Start of end now!" )
			TestFailed( "+= const char * failed" );
	
		TestPassed();
	}

	void Test5()
	{
		// Test append
		string t1 = "Start of";
		string t2 = " end";
		
		t1.append( t2 );
		
		if ( t1 != "Start of end" )
			TestFailed( "append string failed" );
		
		t1.append( " now" );

		if ( t1 != "Start of end now" )
			TestFailed( "append const char * failed" );

		t1.append( " and then I will fail", 9 );

		if ( t1 != "Start of end now and then" )
			TestFailed( "append const char *, int failed" );
		
		t1.append( 3, '!' );

		if ( t1 != "Start of end now and then!!!" )
			TestFailed( "append char failed" );

		TestPassed();
	}

	void Test6()
	{
		// clear, empty, length, size, resize, capacity, reserve, substr
		string t1;
		
		if ( !t1.empty() )
			TestFailed( "string not empty" );
		
		t1 = "Hello World";
		
		if ( t1.empty() )
			TestFailed( "string is empty" );
		
		t1.erase();
		
		if ( !t1.empty() || t1.length() != 0 || t1.size() != 0 )
			TestFailed( "string should be empty" );

		t1 = "Hello World2";

		if ( t1.capacity() < t1.length() || t1.capacity() < 12 )
			TestFailed( "capacity too small" );

		t1.resize( 6 );
		
		if ( t1.length() != 6 || t1 != "Hello " )
			TestFailed( "resize down failed" );

		t1.resize( 10, 'a' );

		if ( t1.length() != 10 || t1 != "Hello aaaa" )
		{
			TestFailed( "resize up failed" );
		}
		
		unsigned cur_size = t1.capacity();
		t1.reserve( cur_size + 10 );
		
		if ( t1.capacity() < cur_size + 10 )
			TestFailed( "reserve failed" );

		string t2 = t1.substr( 6, 3 );
		
		if ( t2 != "aaa" )
			TestFailed( "substr1 failed" );

		t2 = t1.substr( 0, 5 );
		
		if ( t2 != "Hello" )
			TestFailed( "substr2 failed" );

		t2 = t1.substr( 6, 4 );
		
		if ( t2 != "aaaa" )
			TestFailed( "substr3 failed" );
		
		TestPassed();
	}	

	void Test7()
	{
		// erase, insert, replace

		string t1 = "Hello World, this is a string";
		string t2 = t1;
		
		t1.erase( 11, 1 );
		
		LOG_NOTICE( "string is %s", t1.c_str() );
		
		if ( t1 != "Hello World this is a string" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );

		t1.erase( t1.length(), 1 );

		if ( t1 != "Hello World this is a string" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );

#ifdef USE_JH_STRING		
		t1.erase( t1.length() + 1, 1 );

		LOG_NOTICE( "string is %s", t1.c_str() );
		LOG_NOTICE( "string is t2 %s", t2.c_str() );
		
		if ( t1 != "Hello World this is a string" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );
#endif

		t1.erase( t1.length() - 1, 2 );

		if ( t1 != "Hello World this is a strin" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );

		t1.erase( 0, 2 );

		if ( t1 != "llo World this is a strin" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );

		t1.erase();

		if ( t1 != "" || t2 != "Hello World, this is a string" )
			TestFailed( "Error on erase" );
		
		t1 = "Hello World";
		t2 = t1;
		string t3 = "to the ";
		
		LOG_NOTICE( "starting insert tests" );
		
		t1.insert( 6, t3 );
		
		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );

		t1.insert( 6, t3, 3, 4 );
		
		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello the to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );

		t1.insert( 10, "matador " );
		
		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello the matador to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );

		t1.insert( 5, 1, ',' );
		
		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello, the matador to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );

		t1.insert( 18, 3, '!' );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello, the matador!!! to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );

		t1.insert( 22, "welcome my son", 8 );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "Hello, the matador!!! welcome to the World" || t2 != "Hello World" )
			TestFailed( "Error on insert" );
		
		LOG_NOTICE( "starting replace tests" );

		t1.replace( 0, 7, t3 );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "to the the matador!!! welcome to the World" )
			TestFailed( "Error on replace" );

		t1.replace( 11, 10, t3, 3, 3 );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "to the the the welcome to the World" )
			TestFailed( "Error on replace" );

		t1.replace( 15, 8, "the world", 4 );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "to the the the the to the World" )
			TestFailed( "Error on replace" );

		t1.replace( 26, 5, "the end" );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "to the the the the to the the end" )
			TestFailed( "Error on replace" );

		t1.replace( 2, 23, 3, '.' );

		LOG_NOTICE( "string is %s", t1.c_str() );

		if ( t1 != "to... the end" )
			TestFailed( "Error on replace" );
		
		TestPassed();
	}	
	
	void Test8()
	{
		string t1 = "Hello World, I'd like to look for a string";
		string t2 = "or";
		
		unsigned i = t1.find( t2 );
		
		if ( i != 7 )
			TestFailed( "Failed to find first string( \"or\" )" );
		
		i = t1.find( t2, i );

		LOG_NOTICE( "i = %d", i );
		
		if ( i != 7 )
			TestFailed( "Failed to refind first string( \"or\" )" );

		i = t1.find( t2, i + 1 );

		LOG_NOTICE( "i = %d", i );
		
		if ( i != 31 )
			TestFailed( "Failed to find second string( \"or\" )" );
		
		i = t1.find( "or" );

		if ( i != 7 )
			TestFailed( "Failed to find first \"or\"" );

		i = t1.find( "or", i );

		if ( i != 7 )
			TestFailed( "Failed to refind first \"or\"" );

		i = t1.find( "or", i + 1 );

		if ( i != 31 )
			TestFailed( "Failed to find second \"or\"" );

		i = t1.find( 'W' );

		if ( i != 6 )
			TestFailed( "Failed to find \'W\'" );

		i = t1.find( 'o', 4 );

		if ( i != 4 )
			TestFailed( "Failed to find first \'o\'" );
		
		i = t1.find( 'o', 5 );

		if ( i != 7 )
			TestFailed( "Failed to find second \'o\'" );
		
		i = t1.find( "orlando", 0, 2 );

		if ( i != 7 )
			TestFailed( "Failed to find first \"or\" (full)" );

		i = t1.find( "orlando", 8, 2 );

		if ( i != 31 )
			TestFailed( "Failed to find second \"or\" (full)" );

		i = t1.find( "orlando", 7, 2 );

		if ( i != 7 )
			TestFailed( "Failed to refind first \"or\" (full)" );
		
		// find
		TestPassed();
	}	

	void Test9()
	{
		// rfind
		string t1 = "Hello World, I'd like to look for a string";
		string t2 = "or";
		
		unsigned i = t1.rfind( t2 );
		
		if ( i != 31 )
			TestFailed( "Failed to rfind last string( \"or\" )" );
		
		i = t1.rfind( t2, i );

		LOG_NOTICE( "i = %d", i );
		
		if ( i != 31 )
			TestFailed( "Failed to refind last string( \"or\" )" );

		i = t1.rfind( t2, i - 1 );

		LOG_NOTICE( "i = %d", i );
		
		if ( i != 7 )
			TestFailed( "Failed to rfind second string( \"or\" )" );
		
		i = t1.rfind( "or" );

		if ( i != 31 )
			TestFailed( "Failed to rfind first \"or\"" );

		i = t1.rfind( "or", i );

		if ( i != 31 )
			TestFailed( "Failed to rerfind first \"or\"" );

		i = t1.rfind( "or", i - 1 );

		if ( i != 7 )
			TestFailed( "Failed to rfind second \"or\"" );

		i = t1.rfind( 'W' );

		if ( i != 6 )
			TestFailed( "Failed to rfind \'W\'" );

		i = t1.rfind( 'd', 31 );

		if ( i != 15 )
			TestFailed( "Failed to rfind first \'o\'" );
		
		i = t1.rfind( 'd', 14 );

		if ( i != 10 )
			TestFailed( "Failed to rfind second \'o\'" );
		
		i = t1.rfind( "orlando", string::npos, 2 );

		if ( i != 31 )
			TestFailed( "Failed to rfind first \"or\" (full)" );

		i = t1.rfind( "orlando", 30, 2 );

		if ( i != 7 )
			TestFailed( "Failed to rfind second \"or\" (full)" );

		i = t1.rfind( "orlando", 31, 2 );

		if ( i != 31 )
			TestFailed( "Failed to rerfind first \"or\" (full)" );
		
		TestPassed();
	}	

	void Test10()
	{
		// find_first_of
		string t1 = "This is y a sentence with vowels.";
		string t2 = "aeiou";
		
		unsigned i = t1.find_first_of( t2 );
		
		if ( i != 2 )
			TestFailed( "Failed to find first vowel" );

		i = t1.find_first_of( t2, 2 );

		if ( i != 2 )
			TestFailed( "Failed to find first vowel" );

		i = t1.find_first_of( t2, 3 );

		if ( i != 5 )
			TestFailed( "Failed to find second vowel" );

		i = t1.find_first_of( "aeiou", 3 );

		if ( i != 5 )
			TestFailed( "Failed to find second vowel" );

		i = t1.find_first_of( "aeiouy", 6, 5 );

		if ( i != 10 )
			TestFailed( "Failed to find third vowel" );

		i = t1.find_first_of( 'y' );

		if ( i != 8 )
			TestFailed( "Failed to find \'y\'" );

		i = t1.find_first_of( 's', 8 );

		if ( i != 12 )
			TestFailed( "Failed to find \'s\'" );
		
		TestPassed();
	}	

	void Test11()
	{
		// find_first_of
		string t1 = "This is y a sentence with vowels.";
		string t2 = "aeiou";
		
		unsigned i = t1.find_last_of( t2 );
		
		LOG_NOTICE( "i is %d", i );
		
		if ( i != 29 )
			TestFailed( "Failed to find first vowel" );

		i = t1.find_last_of( t2, 29 );

		LOG_NOTICE( "i is %d", i );

		if ( i != 29 )
			TestFailed( "Failed to find first vowel" );

		i = t1.find_last_of( t2, 28 );

		LOG_NOTICE( "i is %d", i );

		if ( i != 27 )
			TestFailed( "Failed to find second vowel" );

		i = t1.find_last_of( "aeiou", 26 );

		LOG_NOTICE( "i is %d", i );

		if ( i != 22 )
			TestFailed( "Failed to find second vowel" );

		i = t1.find_last_of( "aeiouy", 9, 5 );

		LOG_NOTICE( "i is %d", i );

		if ( i != 5 )
			TestFailed( "Failed to find third vowel" );

		i = t1.find_last_of( 'y' );

		LOG_NOTICE( "i is %d", i );

		if ( i != 8 )
			TestFailed( "Failed to find \'y\'" );

		i = t1.find_last_of( 's' );

		LOG_NOTICE( "i is %d", i );

		if ( i != 31 )
			TestFailed( "Failed to find \'s\'" );

		i = t1.find_last_of( 's', 8 );

		LOG_NOTICE( "i is %d", i );

		if ( i != 6 )
			TestFailed( "Failed to find \'s\'" );
		
		TestPassed();
	}	
	
	void Test12()
	{
		// JetHead split test
		JHSTD::string s( "This is a\ttest" );
		JHSTD::vector<JHSTD::string> parts;
		
		int res = JetHead::split( s, " \t", parts );
		
		if ( res != 4 )
			TestFailed( "Wrong number of parts found %d expected 4", res );
		
		if ( parts[ 0 ] != "This" )
			TestFailed( "Part wrong" );
		if ( parts[ 1 ] != "is" )
			TestFailed( "Part wrong" );
		if ( parts[ 2 ] != "a" )
			TestFailed( "Part wrong" );
		if ( parts[ 3 ] != "test" )
			TestFailed( "Part wrong" );
	
		JHSTD::string s2( "12 1556 abc" );

		long r2 = JetHead::strtol( s2 );

		if ( r2 != 12 )
			TestFailed( "strtol failed" );
		
		unsigned end = 0;
		
		r2 = JetHead::strtol( 0, s2, 0, end );
		
		if ( r2 != 12 || end != 2 )
			TestFailed( "strtol failed %d", end );

		r2 = JetHead::strtol( 3, s2, 0, end );

		if ( r2 != 1556 || end != 7 )
			TestFailed( "strtol failed %d", end );

		r2 = JetHead::strtol( 5, s2, 0, end );
		
		if ( r2 != 56 || end != 7 )
			TestFailed( "strtol failed %d", end );

		r2 = JetHead::strtol( 7, s2, 0, end );
		
		if ( r2 != 0 || end != 7 )
			TestFailed( "strtol failed %d", end );

		JHSTD::string s3;
		
		JetHead::stl_sprintf( s3, "Hello World %d", 15 );
		
		if ( s3 != "Hello World 15" )
			TestFailed( "stl_sprintf failed" );
			
		TestPassed();
	}
};

static const int gNumTests = 12;

int main( int argc, char*argv[] )
{
	TestRunner runner( argv[ 0 ] );
	
	TestCase *test_set[ gNumTests ];

	for ( int i = 0; i < gNumTests; i++ )
		test_set[ i ] = jh_new StringTest( i );
	
	runner.RunAll( test_set, gNumTests );

	return 0;
}


