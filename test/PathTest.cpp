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

#include "Path.h"
#include "jh_memory.h"
#include "logging.h"
#include "Selector.h"
#include "jh_list.h"

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class PathTest : public TestCase
{
public:
	PathTest( int test_id );
	virtual ~PathTest();

	void Run();
	
	static const int gNumberOfTests = 7;
	
private:
	void Test1(); // Construct and assign
	void Test2(); // Path info
	void Test3(); // append
	void Test4(); // File info and helpers
	void Test5(); // File/Dir creation/deletion
	void Test6(); // Dir listing
	void Test7(); // Path <-> URI
	
	int mTest;
};

PathTest::PathTest( int test_id ) : TestCase( "PathTest" ), mTest( test_id )
{
	char name[ 32 ];
	snprintf( name, 32, "PathTest%d", test_id );
	name[ 31 ] = '\0';
	SetTestName( name );	
}

// Have the destructor do our clean up work. 
PathTest::~PathTest()
{
}

void PathTest::Run()
{
	LOG_NOTICE( "Path Test Started" );

	if ( mTest > gNumberOfTests )
		TestFailed( "Bad Test Number" );
	
	switch(mTest)
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
	case 4:
		Test4();
		break;
	case 5:
		Test5();
		break;
	case 6:
		Test6();
		break;
	case 7:
		Test7();
		break;
	}

	TestPassed();
}

void PathTest::Test1()
{
	Path p1( "/foo/bar" );
	JHSTD::string s = "../mydir";
	Path p2( s );
	Path p3( p2 );
	
	if ( p1.getString() != "/foo/bar" )	
		TestFailed( "const char * constructor failed" );
	if ( p2.getString() != "../mydir" )	
		TestFailed( "JHSTD::string constructor failed" );
	if ( p3.getString() != "../mydir" )	
		TestFailed( "Path & constructor failed" );

	p2 = "/bar/foo";
	if ( p2.getString() != "/bar/foo" )	
		TestFailed( "const char * assignment failed" );

	p1 = s;
	if ( p1.getString() != "../mydir" )	
		TestFailed( "const char * assignment failed" );

	p1 = p2;
	if ( p1.getString() != "/bar/foo" )	
		TestFailed( "const char * assignment failed" );
}

struct Test2Data {
	const char *path;
	const char *parent;
	const char *filename;
	const char *fileBasename;
	const char *filenameExtention;
} test2Data[] = {
	{ "../../dir/file.so", 		"../../dir/", "file.so", "file", "so" },
	{ "/top/next/dir/file.so", 	"/top/next/dir/", "file.so", "file", "so" },
	{ "next/dir/file.so", 		"next/dir/", "file.so", "file", "so" },
	{ "file.so", 				".", "file.so", "file", "so" },
	{ "file", 					".", "file", "file", "" },
};

// Path parts
void PathTest::Test2()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( test2Data ); i++ )
	{
		Path p( test2Data[ i ].path );
		if ( p.parent() != test2Data[ i ].parent )
			TestFailed( "parent failed %d \"%s\"", i, p.parent().c_str() );
		if ( p.filename() != test2Data[ i ].filename )
			TestFailed( "filename failed %d \"%s\"", i, p.filename().c_str() );
		if ( p.fileBasename() != test2Data[ i ].fileBasename )
			TestFailed( "fileBasename failed %d \"%s\"", i, p.fileBasename().c_str() );
		if ( p.filenameExtention() != test2Data[ i ].filenameExtention )
			TestFailed( "filenameExtention failed %d \"%s\"", i, p.filenameExtention().c_str() );
	}
}

// append and relative vs absolute paths.
void PathTest::Test3()
{
	Path p1( "/foo/bar" );
	Path p2( "p2" );
	JHSTD::string s = "../mydir/";
	Path p3( s );
	Path cwd = Path::getCWD();
	
	p1.append( "help" );
	
	if ( p1.getString() != "/foo/bar/help" )
		TestFailed( "append failed %s", p1.getString().c_str() );

	p1.append( s );

	if ( p1.getString() != "/foo/bar/help/../mydir/" )
		TestFailed( "append failed %s", p1.getString().c_str() );

	p1.append( p2 );

	if ( p1.getString() != "/foo/bar/help/../mydir/p2" )
		TestFailed( "append failed %s", p1.getString().c_str() );
	
	if ( p1.isRelative() == true )
		TestFailed( "isRelative" );
	if ( p2.isRelative() == false )
		TestFailed( "isRelative" );
	if ( p3.isRelative() == false )
		TestFailed( "isRelative" );
	
	p1.normalize();
	p2.makeAbsolute();
	p3.makeAbsolute();
	
	if ( p1.getString() != "/foo/bar/mydir/p2" )
		TestFailed( "normalize failed %s", p1.getString().c_str() );
	
	JHSTD::string scwd = cwd.getString();
	
	LOG_NOTICE( "CWD = %s", scwd.c_str() );
	
	if ( p2.getString() != scwd + "/p2" )
		TestFailed( "makeAbsolute failed %s", p2.getString().c_str() );

	if ( p3.getString() != scwd + "/../mydir/" )
		TestFailed( "makeAbsolute failed %s", p3.getString().c_str() );	
}

// File Info and helpers (length, getModifiedTime, isFile, isDir, exists
void PathTest::Test4()
{
}

// File/Dir creation/deletion. (touch, remove, rename, mkdir, mkdirs )
void PathTest::Test5()
{
}

// Dir Listing ( list )
void PathTest::Test6()
{
}

// Path <-> URI (URI assign, construct and toURI )
void PathTest::Test7()
{
}

int main( int argc, char* argv[] )
{
	TestRunner runner( argv[ 0 ] );
	TestSuite ts;
	
	for ( int i = 0; i < PathTest::gNumberOfTests; i++ )
	{
		ts.AddTestCase( jh_new PathTest( i + 1 ) );
	}
	
	runner.RunAll( ts );
	
	return 0;
}

