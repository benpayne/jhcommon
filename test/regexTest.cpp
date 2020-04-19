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

#include "jh_Regex.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

struct TestPair {
	const char *regex;
	const char *tree;
	bool		pass;
};

struct TestGroup {
	const char *regex;
	const char *tree;
	bool		pass;
	const char *groups[ 5 ];
};


TestPair gPrepareTests[] = {
	{ "abc", "[S][T:a][T:b][T:c]", true },
	{ ".bc", "[S][T:.][T:b][T:c]", true },
	{ ".*bc", "[S][T:.]{0,-1}[T:b][T:c]", true },
	{ "\\tbc", "[S][T:0x9][T:b][T:c]", true },
	{ "[abc]", "[S][C][T:a][T:b][T:c]", true },
	{ "[a-c]", "[S][C][T:a-c]", true },
	{ "[a-zA-Z0-9_]", "[S][C][T:a-z][T:A-Z][T:0-9][T:_]", true },
	{ "[a-zA-Z0-9_]+", "[S][C]{1,-1}[T:a-z][T:A-Z][T:0-9][T:_]", true },
	{ "[a-zA-Z0-9_]?", "[S][C]{0,1}[T:a-z][T:A-Z][T:0-9][T:_]", true },
	{ "[^_sA-Z0-9]", "[S][C^][T:_][T:s][T:A-Z][T:0-9]", true },
	{ "\\w+[0-9]*", "[S][C]{1,-1}[T:a-z][T:A-Z][T:0-9][T:_][C]{0,-1}[T:0-9]", true },
	{ "\\W+[0-9]*", "[S][C^]{1,-1}[T:a-z][T:A-Z][T:0-9][T:_][C]{0,-1}[T:0-9]", true },
	{ "\\s+[0-9]*", "[S][C]{1,-1}[T: ][T:0x9][T:0xa][T:0xd][T:0xc][C]{0,-1}[T:0-9]", true },
	{ "\\S+[0-9]*", "[S][C^]{1,-1}[T: ][T:0x9][T:0xa][T:0xd][T:0xc][C]{0,-1}[T:0-9]", true },
	{ "\\d+[A-Z]*", "[S][C]{1,-1}[T:0-9][C]{0,-1}[T:A-Z]", true },
	{ "\\D+[A-Z]*", "[S][C^]{1,-1}[T:0-9][C]{0,-1}[T:A-Z]", true },
	{ "[\\d\\w@#$!~`%^&]", "[S][C][T:0-9][T:a-z][T:A-Z][T:0-9][T:_][T:@][T:#][T:$][T:!][T:~][T:`][T:%][T:^][T:&]", true },
	{ "[0-9]{2}", "[S][C]{2,2}[T:0-9]", true },
	{ "[0-9]{2,10}", "[S][C]{2,10}[T:0-9]", true },
	{ "[0-9]{14,180}", "[S][C]{14,180}[T:0-9]", true },
	{ "(a|b|c)*", "[S][O]{0,-1}[S][T:a][S][T:b][S][T:c]", true },
	{ "(a|b+|c)*", "[S][O]{0,-1}[S][T:a][S][T:b]{1,-1}[S][T:c]", true },
	{ "(ben|fred|ava)*", "[S][O]{0,-1}[S][T:b][T:e][T:n][S][T:f][T:r][T:e][T:d][S][T:a][T:v][T:a]", true },
	{ "[*]*", "", false },
	{ "[\\*\\]]*", "[S][C]{0,-1}[T:*][T:]]", true },
	{ "(fred(ben|ava))", "[S][O][S][T:f][T:r][T:e][T:d][O][S][T:b][T:e][T:n][S][T:a][T:v][T:a]", true },
	{ "([a-z]([0-9]*|[^a-z]*))", "[S][O][S][C][T:a-z][O][S][C]{0,-1}[T:0-9][S][C^]{0,-1}[T:a-z]", true },
	{ "(([a-z]*)([0-9]*))", "[S][O][S][O][S][C]{0,-1}[T:a-z][O][S][C]{0,-1}[T:0-9]", true },
};

TestGroup gParseTests[] = {
	{ "[a-z]", "a", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z]", "0", false, { NULL, NULL, NULL, NULL, NULL } },
	{ "[^a-z]", "0", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "[^a-z]", "a", false, { NULL, NULL, NULL, NULL, NULL } },
	{ "bet[a-z]", "beta", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "bet[a-z]s", "betas", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "bet[a-z]s", "foo", false, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z]*", "foo", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z]*[0-9]", "foo7", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z]*[0-9]*", "foo", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z][0-9]", "foo7", false, { NULL, NULL, NULL, NULL, NULL } },
	{ "[a-z]*[0-9]+", "foo75", true, { NULL, NULL, NULL, NULL, NULL } },
	{ "\\w*[0-9]+", "foo75", true, { NULL, NULL, NULL, NULL, NULL } },		// backtrack testing
	{ "\\w*[0-9]{2}", "foo75", true, { NULL, NULL, NULL, NULL, NULL } },		//  same
	{ "\\w*[0-9]{2}", "foo753", true, { NULL, NULL, NULL, NULL, NULL } },		//  same
	{ "\\w*[0-9]{2}", "foo7", false, { NULL, NULL, NULL, NULL, NULL } },		//  same
	{ "\\w*abc", "fooabc", true, { NULL, NULL, NULL, NULL, NULL } },		//  same
	{ "(([a-z]*)([0-9]*))", "foo7", true, { "foo7", "foo", "7", NULL, NULL } },			// group testing
	{ "(\\w*)abc", "fooabc", true, { "foo", NULL, NULL, NULL, NULL } },		//  same
	{ "(([a-z]*)a([0-9]*))", "fooa7", true, { "fooa7", "foo", "7", NULL, NULL } },			// group testing - with backtracking
	{ "(aa|bb|cc)", "aa", true, { "aa", NULL, NULL, NULL, NULL } },			// group testing - with backtracking
	{ "(ab|cd|efg)", "efg", true, { "efg", NULL, NULL, NULL, NULL } },			// group testing - with backtracking
	{ "(ab|cd|efg)", "fg", false, { NULL, NULL, NULL, NULL, NULL } },			// group testing - with backtracking
	{ "(ab|cd|efg)+", "abcd", true, { "ab", "cd", NULL, NULL, NULL } },			// group testing - with backtracking
};

class RegexPrepareTest : public TestCase
{
public:
	RegexPrepareTest( int number ) : 
		TestCase( "RegexPrepareTest" ), mTestNum( number ) 
	{
		char name[ 32 ];
		snprintf( name, 32, "RegexPrepareTest%d", number );
		name[ 31 ] = '\0';
		SetTestName( name );	
	}

	virtual ~RegexPrepareTest() {}
	
private:
	int mTestNum;
	
	void Run()
	{	
		Regex r;
		std::string dump;
		
		JetHead::ErrCode err = r.prepare( gPrepareTests[ mTestNum ].regex );

		if ( gPrepareTests[ mTestNum ].pass && err != JetHead::kNoError )
			TestFailed( "Prepare returned error code" );
		else if ( gPrepareTests[ mTestNum ].pass == false && err == JetHead::kNoError )
			TestFailed( "Prepare did not return error code" );
			
		r.dumpTree( dump );
		if ( dump == gPrepareTests[ mTestNum ].tree )
			TestPassed();
		else
		{
			LOG_NOTICE( "dump is: %s", dump.c_str() );
			LOG_NOTICE( "expected: %s", gPrepareTests[ mTestNum ].tree );
			LOG_NOTICE( "on regex: %s", gPrepareTests[ mTestNum ].regex );
			TestFailed( "Tree dump miss-match" );
		}
	}
};

class RegexParseTest : public TestCase
{
public:
	RegexParseTest( int number ) : 
		TestCase( "RegexParseTest" ), mTestNum( number ) 
	{
		char name[ 32 ];
		snprintf( name, 32, "RegexParseTest%d", number );
		name[ 31 ] = '\0';
		SetTestName( name );	
	}

	virtual ~RegexParseTest() {}
	
private:
	int mTestNum;
	
	void Run()
	{	
		Regex r;
		std::string dump;
		
		JetHead::ErrCode err = r.prepare( gParseTests[ mTestNum ].regex );

		if ( err != JetHead::kNoError )
			TestFailed( "Prepare returned error code" );

		bool res = r.parse( gParseTests[ mTestNum ].tree );
		
		if ( gParseTests[ mTestNum ].pass != res )
		{
			LOG_NOTICE( "Parse %s", res ? "succeeded" : "failed" );
			LOG_NOTICE( "string: %s", gParseTests[ mTestNum ].tree );
			LOG_NOTICE( "regex: %s", gParseTests[ mTestNum ].regex );
			TestFailed( "Parse failed to match expected result" );
		}			

		for ( int i = 0; i < 5; i++ )
		{
			if ( gParseTests[ mTestNum ].groups[ i ] != NULL )
			{
				std::string s = r.getData( i );
				if ( s != gParseTests[ mTestNum ].groups[ i ] )
				{
					LOG_NOTICE( "Got %s for group %d", s.c_str(), i );
					LOG_NOTICE( "Expected %s", gParseTests[ mTestNum ].groups[ i ] );
					TestFailed( "Group does not match" );
				}
			}
		}
		
		TestPassed();
	}
};

int main( int argc, char*argv[] )
{
	TestRunner runner( argv[ 0 ] );
	int i = 0;
	
	TestCase *test_set[ JH_ARRAY_SIZE( gPrepareTests ) + JH_ARRAY_SIZE( gParseTests ) ];

	for ( ; i < JH_ARRAY_SIZE( gPrepareTests ); i++ )
		test_set[ i ] = jh_new RegexPrepareTest( i );

	for ( int j = 0; j < JH_ARRAY_SIZE( gParseTests ); j++, i++ )
		test_set[ i ] = jh_new RegexParseTest( j );
	
	runner.RunAll( test_set, JH_ARRAY_SIZE( gPrepareTests ) + JH_ARRAY_SIZE( gParseTests ) );

	return 0;
}


