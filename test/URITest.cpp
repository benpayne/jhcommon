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
#include "logging.h"
#include <unistd.h>
#include <iostream>

using namespace std;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

//#define DEBUG_PRINTS	1

const char *test_urls[] = {
	"http://h:80",
	"http://h:80/",
	"http://h:80/a/b",
	"http://h:80/a/b?c=d",
	"http://h:80/a/b?c=d&e=f",
	"http://h:80/a/b?c=d&e=f&c=g",
	"http://h:80/a/b?c=d&e=f#m",
	"a/b",
	"a/b/",
	"a/b?c=d",
	"a/b?c=d&e=f",
	"/a/b",
	"/a/b/",
	"/a/b?c=d",
	"/a/b?c=d&e=f",
	"mailto:bpayne@192.168.13.91:8080",	
};

int main( int argc, char*argv[] )
{	
	LOG_NOTICE( "Test Started" );

	URI u;
	
	u.setScheme( "http" );
	u.setAuthority( "www.jetheaddev.com" );
	u.setPath( "pages/index.html" );

	string ustr = u.getString();	
	LOG_NOTICE( "URI is: %s", ustr.c_str() );

	for ( uint32_t i = 0; i < JH_ARRAY_SIZE( test_urls ); i++ )
	{
		bool res = u.setString( test_urls[ i ] );

#ifdef DEBUG_PRINTS
		cout << "scheme: " << u.getScheme() << endl;
		cout << "authority: " << u.getAuthority() << endl;
		cout << "host: " << u.getHost() << endl;
		cout << "port: " << u.getPort() << endl;
		cout << "query: " << u.getQuery() << endl;
		cout << "path: " << u.getPath() << endl;
		cout << "fragment: " << u.getFragment() << endl;
		cout << "query param \"c\": " << u.getQueryParam( "c" ) << endl;
		cout << "query param \"e\": " << u.getQueryParam( "e" ) << endl;
		cout << "is relative: " << u.isRelative() << endl;
#endif

		if ( not res )
		{
			LOG_WARN( "parse uri %s: FAILED", test_urls[ i ] );
			exit( 1 );
		}
		else
		{
			LOG_NOTICE( "parse uri %s: PASSED", test_urls[ i ] );
		}
	}

	u.clear();
	u.setScheme( "http" );
	u.setAuthority( "www.jetheaddev.com" );
	u.setPath( "pages/index.html" );
	u.appendQueryParam( "a", "b" );
	u.appendQueryParam( "c", "d" );
	u.setFragment( "m" );

	URI copy = u;

	ustr = u.getString();	
	LOG_NOTICE( "URI is: %s", ustr.c_str() );
	ustr = copy.getString();	
	LOG_NOTICE( "Copy is: %s", ustr.c_str() );
	
#ifdef DEBUG_PRINTS
	cout << "scheme: " << u.getScheme() << endl;
	cout << "scheme: " << copy.getScheme() << endl;
	cout << "authority: " << u.getAuthority() << endl;
	cout << "authority: " << copy.getAuthority() << endl;
	cout << "host: " << u.getHost() << endl;
	cout << "host: " << copy.getHost() << endl;
	cout << "port: " << u.getPort() << endl;
	cout << "port: " << copy.getPort() << endl;
	cout << "query: " << u.getQuery() << endl;
	cout << "query: " << copy.getQuery() << endl;
	cout << "path: " << u.getPath() << endl;
	cout << "path: " << copy.getPath() << endl;
	cout << "fragment: " << u.getFragment() << endl;
	cout << "fragment: " << copy.getFragment() << endl;
	cout << "query param \"a\": " << u.getQueryParam( "a" ) << endl;
	cout << "query param \"a\": " << copy.getQueryParam( "a" ) << endl;
	cout << "query param \"c\": " << u.getQueryParam( "c" ) << endl;
	cout << "query param \"c\": " << copy.getQueryParam( "c" ) << endl;
	cout << "is relative: " << u.isRelative() << endl;
	cout << "is relative: " << copy.isRelative() << endl;
#endif

	if ( u.getScheme() != copy.getScheme() or 
		u.getAuthority() != copy.getAuthority() or 
		u.getQuery() != copy.getQuery() or 
		u.getPath() != copy.getPath() or 
		u.getFragment() != copy.getFragment() or 
		u.getQueryParam( "a" ) != copy.getQueryParam( "a" ) or 
		u.getQueryParam( "c" ) != copy.getQueryParam( "c" ) or 
		u.isRelative() != copy.isRelative() )
	{
		LOG_WARN( "copy of uri: FAILED" );
	}
	else
	{
		LOG_NOTICE( "copy of uri: PASSED" );
	}
	
	return 0;
}
