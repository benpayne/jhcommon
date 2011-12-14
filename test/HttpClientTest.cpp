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

#include "jh_memory.h"
#include "logging.h"
#include "HttpAgent.h"
#include "File.h"
#include <fcntl.h>

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class HttpClientTest : public TestCase
{
public:
	HttpClientTest( const char *uri, const char *file, int start_range = 0,
					int end_range = 0 ) : TestCase( "HttpClientTest" ),
					mUri( uri ), mFilename( file )
	{
		SetTestName( "HTTP Client" );	
	}

private:
	void Run();	
	const char *mUri;
	const char *mFilename;
};

void HttpClientTest::Run()
{
	HttpAgent agent;
	HttpResponse res;
	File f;
	FileBodyHandler handler( f );
	URI uri( mUri );
	int ret = 0;
	
	f.open( mFilename, O_CREAT | O_TRUNC );

	// range optional!	
	ret = agent.get( uri, res, &handler );

	if  ( !(( res.getResponseCode() == 200 ) || ( res.getResponseCode() == 206 )) )
		TestFailed( "%s, Response was %d", getErrorString((JetHead::ErrCode)ret), res.getResponseCode() );
	else if ( ret != kNoError )
		TestFailed( "%s", getErrorString((JetHead::ErrCode)ret));
	
	f.close();
	
	TestPassed();
}

int main( int argc, char*argv[] )
{	
	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ 10 ];
	test_set[ 0 ] = jh_new HttpClientTest( "http://www.yahoo.com", "yahoo.html" );
	test_set[ 1 ] = jh_new HttpClientTest( "http://www.google.com", "google.html" );
	test_set[ 2 ] = jh_new HttpClientTest( "http://www.kernel.org/pub/linux/kernel/v2.6/patch-2.6.20.7.bz2", "patch-2.6.20.7.bz2" );
	test_set[ 3 ] = jh_new HttpClientTest( "http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.20.tar.bz2", "linux-2.6.20.tar.bz2" );
	
	runner.RunAll( test_set, 4 );

	return 0;
}
