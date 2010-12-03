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

#include "URI.h"
#include <unistd.h>

#include "logging.h"
#include "jh_memory.h"

#include "jh_string.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

struct test_data {
	const char *string;
	const char *rev_string;
	const char *scheme;
	const char *authority;
	const char *host;
	int	port;
	const char *path;
	const char *query;
	const char *fragment;
	const char *param1;
	const char *p1_value;
	const char *param2;
	const char *p2_value;
	bool	relative;
};

struct test_data test_urls[] = {
	{ "http://www.jetheaddev.com/pages/index.html", NULL, "http", "www.jetheaddev.com", "www.jetheaddev.com", 0, "/pages/index.html", NULL, NULL, NULL, NULL, NULL, NULL, false },
	{ "http://h:80", "http://h:80/", "http", "h:80", "h", 80, "", "", NULL, NULL, NULL, NULL, NULL, false },
	{ "http://h:80/", NULL, "http", "h:80", "h", 80, "/", "", NULL, NULL, NULL, NULL, NULL, false },
	{ "http://h:80/a/b", NULL, "http", "h:80", "h", 80, "/a/b", "", NULL, NULL, NULL, NULL, NULL, false },
	{ "http://h:80/a/b?c=d", NULL, "http", "h:80", "h", 80, "/a/b", "c=d", NULL, "c", "d", NULL, NULL, false },
	{ "http://h:80/a/b?c=d&e=f", NULL, "http", "h:80", "h", 80, "/a/b", "c=d&e=f", NULL, "c", "d", "e", "f", false },
	{ "http://h:80/a/b?c=d&e=f&c=g", NULL, "http", "h:80", "h", 80, "/a/b", "c=d&e=f&c=g", NULL, "c", "d", "e", "f", false },
	{ "http://h:80/a/b?c=d&e=f#m", NULL, "http", "h:80", "h", 80, "/a/b", "c=d&e=f", "m", "c", "d", "e", "f", false },
	{ "a/b", NULL, "", "", "", 0, "a/b", NULL, NULL, NULL, NULL, NULL, NULL, true },
	{ "a/b/", NULL, "", "", "", 0, "a/b/", NULL, NULL, NULL, NULL, NULL, NULL, true },
	{ "a/b?c=d", NULL, "", "", "", 0, "a/b", "c=d", NULL, "c", "d", NULL, NULL, true },
	{ "a/b?c=d&e=f", NULL, "", "", "", 0, "a/b", "c=d&e=f", NULL, "c", "d", "e", "f", true },
	{ "/a/b", NULL, "", "", "", 0, "/a/b", NULL, NULL, NULL, NULL, NULL, NULL, false },
	{ "/a/b/", NULL, "", "", "", 0, "/a/b/", NULL, NULL, NULL, NULL, NULL, NULL, false },
	{ "/a/b?c=d", NULL, "", "", "", 0, "/a/b", "c=d", NULL, "c", "d", NULL, NULL, false },
	{ "/a/b?c=d&e=f", NULL, "", "", "", 0, "/a/b", "c=d&e=f", NULL, "c", "d", "e", "f", false },
	{ "mailto:bpayne@192.168.13.91:8080", NULL, "mailto", "bpayne@192.168.13.91:8080", "bpayne@192.168.13.91", 8080, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false },
};

	class URITest : public TestCase
{
public:
	URITest( int number ) : 
		TestCase( "URITest" ), mTestNum( number ) 
	{
		char name[ 32 ];
		snprintf( name, 32, "URITest%d", number + 1 );
		name[ 31 ] = '\0';
		SetTestName( name );	
	}

	virtual ~URITest() {}
	
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
		default:
			break;
		}
	}
	
	void Test1()
	{
		// Build a URI from a string and check that the params are correct.
		//  also check that the string returned by getString is matching the 
		//  original string.  
		for ( int i = 0; i < JH_ARRAY_SIZE( test_urls ); i++ )
		{
			URI uri( test_urls[ i ].string );
			
			if ( test_urls[ i ].scheme != NULL &&
				 uri.getScheme() != test_urls[ i ].scheme )
			{
				TestFailed( "Failed to validate scheme for uri %d", i );
			}

			if ( test_urls[ i ].authority != NULL &&
				 uri.getAuthority() != test_urls[ i ].authority )
			{
				TestFailed( "Failed to validate authority for uri %d", i );
			}
			if ( test_urls[ i ].host != NULL &&
				 uri.getHost() != test_urls[ i ].host )
			{
				TestFailed( "Failed to validate host for uri %d", i );
			}
			if ( test_urls[ i ].port != 0 &&
				 uri.getPort() != test_urls[ i ].port )
			{
				TestFailed( "Failed to validate port for uri %d", i );
			}
			if ( test_urls[ i ].path != NULL &&
				 uri.getPath() != test_urls[ i ].path )
			{
				TestFailed( "Failed to validate path for uri %d, (%s)", i, uri.getPath().c_str() );
			}
			
			if ( test_urls[ i ].query != NULL &&
				 uri.getQuery() != test_urls[ i ].query )
			{
				TestFailed( "Failed to validate query for uri %d", i );
			}

			if ( test_urls[ i ].path != NULL && test_urls[ i ].query != NULL )
			{
				JHSTD::string path( test_urls[ i ].path );
				JHSTD::string query( test_urls[ i ].query );
				
				if ( not query.empty() )
				{
					path.append( "?" );
					path.append( query );
				}
				
				if ( uri.getPathAndQuery() != path )
				{
					TestFailed( "Failed to validate path and query for uri %d, (%s != %s)", i, uri.getPathAndQuery().c_str(), path.c_str() );
				}
			}

			if ( test_urls[ i ].query != NULL &&
				 uri.getQuery() != test_urls[ i ].query )
			
			if ( test_urls[ i ].fragment != NULL &&
				 uri.getFragment() != test_urls[ i ].fragment )
			{
				TestFailed( "Failed to validate fragment for uri %d", i );
			}

			if ( test_urls[ i ].param1 != NULL &&
				 uri.getQueryParam( test_urls[ i ].param1 ) != test_urls[ i ].p1_value )
			{
				TestFailed( "Failed to validate param1 for uri %d", i );
			}
			if ( test_urls[ i ].param2 != NULL &&
				 uri.getQueryParam( test_urls[ i ].param2 ) != test_urls[ i ].p2_value )
			{
				TestFailed( "Failed to validate param2 for uri %d", i );
			}
			
			if ( uri.isRelative() !=  test_urls[ i ].relative )
				TestFailed( "failed to validate relative for uri %d", i );
		}
		
		TestPassed();
	}

	void Test2()
	{
		// Build URI from parts and make sure the string is correct.
		for ( int i = 0; i < JH_ARRAY_SIZE( test_urls ); i++ )
		{
			URI uri;
			
			if ( test_urls[ i ].scheme != NULL )
				uri.setScheme(  test_urls[ i ].scheme );

			if ( test_urls[ i ].authority != NULL )
				uri.setAuthority(  test_urls[ i ].authority );

			if ( test_urls[ i ].path != NULL )
				uri.setPath(  test_urls[ i ].path, test_urls[ i ].relative );

			if ( test_urls[ i ].query != NULL )
				uri.setQuery(  test_urls[ i ].query );

			if ( test_urls[ i ].fragment != NULL )
				uri.setFragment(  test_urls[ i ].fragment );
			
			if ( test_urls[ i ].rev_string != NULL )
			{
				if ( uri.getString() != test_urls[ i ].rev_string )
					TestFailed( "Failed to match string %d (%s)", i, uri.getString().c_str() );
			}
			else
			{
				if ( uri.getString() != test_urls[ i ].string )
					TestFailed( "Failed to match string %d (%s)", i, uri.getString().c_str() );
			}
		}
		
		TestPassed();
	}

	void Test3()
	{
		// Build URI from parts and make sure the string is correct.  This time
		//  use the individual params, not the query string.  Skip test 6 due
		//  to it having 3 params and the test struct only describes 2 of them.
		for ( int i = 0; i < JH_ARRAY_SIZE( test_urls ); i++ )
		{
			if ( i == 6 )
				continue;
			
			URI uri;
			
			if ( test_urls[ i ].scheme != NULL )
				uri.setScheme(  test_urls[ i ].scheme );

			if ( test_urls[ i ].authority != NULL )
				uri.setAuthority(  test_urls[ i ].authority );

			if ( test_urls[ i ].path != NULL )
				uri.setPath(  test_urls[ i ].path, test_urls[ i ].relative );

			if ( test_urls[ i ].param1 != NULL )
				uri.appendQueryParam(  test_urls[ i ].param1, test_urls[ i ].p1_value );

			if ( test_urls[ i ].param2 != NULL )
				uri.appendQueryParam(  test_urls[ i ].param2, test_urls[ i ].p2_value );

			if ( test_urls[ i ].fragment != NULL )
				uri.setFragment(  test_urls[ i ].fragment );
			
			if ( test_urls[ i ].rev_string != NULL )
			{
				if ( uri.getString() != test_urls[ i ].rev_string )
					TestFailed( "Failed to match string %d (%s)", i, uri.getString().c_str() );
			}
			else
			{
				if ( uri.getString() != test_urls[ i ].string )
					TestFailed( "Failed to match string %d (%s)", i, uri.getString().c_str() );
			}
		}
		
		TestPassed();
	}

	void Test4()
	{
		JHSTD::string foo = test_urls[ 2 ].string;
		URI u( test_urls[ 0 ].string );
		URI copy( u );
		URI copy2( foo );
		
		if ( u.getString() != test_urls[ 0 ].string )
			TestFailed( "constructed char* not correct" );

		if ( copy.getString() != test_urls[ 0 ].string )
			TestFailed( "copy constructed uri not correct" );

		if ( copy2.getString() != test_urls[ 2 ].string )
			TestFailed( "constructed string not correct" );
		
		u = test_urls[ 5 ].string;
		copy = u;
		
		if ( u.getString() != test_urls[ 5 ].string )
			TestFailed( "assigned char* not correct" );

		if ( copy.getString() != test_urls[ 5 ].string )
			TestFailed( "assigned uri not correct" );
		
		bool res = ( u == copy );
		
		if ( res == false )
			TestFailed( "equality of uri not correct" );

		res = ( u != copy );
		
		if ( res == true )
			TestFailed( "non-equality of uri not correct" );
		
		if ( u.getScheme() != copy.getScheme() or 
			u.getAuthority() != copy.getAuthority() or 
			u.getQuery() != copy.getQuery() or 
			u.getPath() != copy.getPath() or 
			u.getFragment() != copy.getFragment() or 
			u.getQueryParam( "a" ) != copy.getQueryParam( "a" ) or 
			u.getQueryParam( "c" ) != copy.getQueryParam( "c" ) or 
			u.isRelative() != copy.isRelative() )
		{
			TestFailed( "params not correct" );
		}
		
		u.clear();
		
		if ( u.getString() != "" )
			TestFailed( "clear failed" );
			
		TestPassed();
	}
};

static const int gNumTests = 4;

int main( int argc, char*argv[] )
{
	TestRunner runner( argv[ 0 ] );
	
	TestCase *test_set[ gNumTests ];

	for ( int i = 0; i < gNumTests; i++ )
		test_set[ i ] = jh_new URITest( i );
	
	runner.RunAll( test_set, gNumTests );

	return 0;
}


