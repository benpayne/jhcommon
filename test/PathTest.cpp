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
	
	static const int gNumberOfTests = 9;
	
private:
	void Test1(); // Construct and assign
	void PartsTest(); // Path info
	void NormalizeTest(); // normalize
	void AppendTest(); // append
	void Test5(); // isRelative/makeAbsolute
	void Test6(); // File info and helpers
	void Test7(); // File/Dir creation/deletion
	void Test8(); // Dir listing
	void Test9(); // Path <-> URI
	
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
		NormalizeTest();
		break;
	case 3:
		PartsTest();
		break;
	case 4:
		AppendTest();
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
	case 8:
		Test8();
		break;
	case 9:
		Test9();
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

struct PartsTestData {
	const char *path;
	const char *parent;
	const char *filename;
	const char *fileBasename;
	const char *filenameExtention;
} partsTestData[] = {
	{ "../../dir/file.so", 		"../../dir/", "file.so", "file", "so" },
	{ "/top/next/dir/file.so", 	"/top/next/dir/", "file.so", "file", "so" },
	{ "next/dir/file.so", 		"next/dir/", "file.so", "file", "so" },
	{ "file.so", 				".", "file.so", "file", "so" },
	{ "file", 					".", "file", "file", "" },
	{ "file/", 					".", "file", "file", "" },
	{ "next/file/", 					"next/", "file", "file", "" },
	{ "/next/file/", 					"/next/", "file", "file", "" },
	{ "/next/file", 					"/next/", "file", "file", "" },
	{ "//next//file//", 					"/next/", "file", "file", "" },
};

// Path parts
void PathTest::PartsTest()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( partsTestData ); i++ )
	{
		Path p( partsTestData[ i ].path );
		if ( p.parent() != partsTestData[ i ].parent )
			TestFailed( "parent failed %d \"%s\"", i, p.parent().getString().c_str() );
		if ( p.filename() != partsTestData[ i ].filename )
			TestFailed( "filename failed %d \"%s\"", i, p.filename().c_str() );
		if ( p.fileBasename() != partsTestData[ i ].fileBasename )
			TestFailed( "fileBasename failed %d \"%s\"", i, p.fileBasename().c_str() );
		if ( p.filenameExtention() != partsTestData[ i ].filenameExtention )
			TestFailed( "filenameExtention failed %d \"%s\"", i, p.filenameExtention().c_str() );
	}
}

struct NormalizeTestData {
	const char *path;
	const char *norm_path;
} normalizeTestData[] = {
	{ "/top/next/dir/file.so", 	"/top/next/dir/file.so" },
	{ "next/dir/file.so", 		"next/dir/file.so" },
	{ "file.so", 				"file.so" },
	{ "file", 					"file" },
	{ "file/", 					"file" },
	{ "/next/file/", 			"/next/file" },
	{ "/next/file", 			"/next/file" },
	{ "/next/file//", 			"/next/file" },
	{ "../../dir/file.so", 		"../../dir/file.so" },
	{ "/top/../next/dir/file.so", 	"/next/dir/file.so" },
	{ "/top/../next/../dir/file.so", 	"/dir/file.so" },
};

void PathTest::NormalizeTest()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( normalizeTestData ); i++ )
	{
		Path p = normalizeTestData[ i ].path;
		p.normalize();
		if ( p != normalizeTestData[ i ].norm_path )
			TestFailed( "Normalize of %s failed (%s)", normalizeTestData[ i ].path, p.getString().c_str() );
	}
}

struct AppendTestData {
	const char *path1;
	const char *path2;
	const char *result;
} appendTestData[] = {
	{ "/foo/bar", "help", "/foo/bar/help" },
	{ "/foo/bar/help", "../mydir/", "/foo/bar/help/../mydir/" },
	{ "/foo/bar/help/../mydir/", "p2", "/foo/bar/help/../mydir/p2" },
};


// append and relative vs absolute paths.
void PathTest::AppendTest()
{
	for ( int i = 0; i < JH_ARRAY_SIZE( appendTestData ); i++ )
	{
		Path p = appendTestData[ i ].path1;
		p.append( appendTestData[ i ].path2 );
		if ( p != appendTestData[ i ].result )
		{
			TestFailed( "Append of %s and %s failed (%s)", 
				appendTestData[ i ].path1, 
				appendTestData[ i ].path2, 
				p.getString().c_str() );
		}
	}
}

void PathTest::Test5()
{
}

#define IfError( str... ) \
		if ( err != kNoError ) \
		TestFailed( str )

// File/Dir creation/deletion. (touch, remove, rename, mkdir, mkdirs )
void PathTest::Test6()
{
	Path p1( "bar" );
	Path p2( "foo" );
	
	// remove p1 and p2 incase they exist from a previous test.
	ErrCode err = p1.remove();

	if ( err != kNoError && err != kNotFound )
		TestFailed( "unlink failed on \"bar\"" );

	err = p2.remove();

	if ( err != kNoError && err != kNotFound )
		TestFailed( "unlink failed on \"foo\"" );

	// touch p1 and p2
	err = p1.touch();
	IfError( "touch failed on \"bar\"" );

	err = p2.touch();
	IfError( "touch failed on \"foo\"" );

	// test the existance of p1, but succefully removing it.
	err = p1.remove();	
	IfError( "remove failed on \"bar\"" );

	// now rename p2 to p1
	err = p2.rename( p1 );
	IfError( "rename failed on \"bar\"" );

	// again remove to test existance.
	err = p1.remove();	
	IfError( "remove failed on \"bar\"" );

	// lets create a dir and remove it.
	Path d1( "test" );
	Path p3( "test/foo" );

	err = p3.remove();	
	if ( err != kNoError && err != kNotFound )
		TestFailed( "remove of \"test/foo\" failed \"%s\"", JetHead::getErrorString( err ) );
	err = d1.remove();	
	if ( err != kNoError && err != kNotFound )
		TestFailed( "remove of \"test\" failed \"%s\"", JetHead::getErrorString( err ) );
	
	err = d1.mkdir();
	IfError( "mkdir failed on \"test\" \"%s\"", JetHead::getErrorString( err ) );
	
	err = d1.remove();	
	IfError( "remove failed on \"test\" \"%s\"", JetHead::getErrorString( err ) );

	// now create the dir put a file in it and test that remove of the dir
	//  fails with NotEmpty.
	err = d1.mkdir();
	IfError( "mkdir failed on \"test\"" );

	err = p3.touch();
	IfError( "touch failed on \"%s", p3.getString().c_str() );

	err = d1.remove();
	if ( err != kNotEmpty )
		TestFailed( "remove of \"test\" should have failed \"%s\"", JetHead::getErrorString( err ) );
	
	err = p3.remove();
	IfError( "remove of \"test/foo\" failed \"%s\"", JetHead::getErrorString( err ) );

	err = d1.remove();
	IfError( "remove of \"test\" failed \"%s\"", JetHead::getErrorString( err ) );

	// lets create multiple dirs in one call
	Path d2( "test/path/to/" );
	Path p4( "test/path/to/file" );
	
	err = d2.mkdir();	
	if ( err != kNotFound )
		TestFailed( "creating dir with mkdir should fail \"%s\"", JetHead::getErrorString( err ) );

	err = d2.mkdirs();	
	IfError( "creating with mkdirs failed \"%s\"", JetHead::getErrorString( err ) );
	
	err = p4.touch();
	IfError( "creating file in dirs failed \"%s\"", JetHead::getErrorString( err ) );

#if 0
	err = p4.remove();
	IfError( "cleanup failed \"%s\"", JetHead::getErrorString( err ) );
	err = d2.remove();
	IfError( "cleanup failed \"%s\"", JetHead::getErrorString( err ) );
	d2 = "test/path";
	err = d2.remove();
	IfError( "cleanup failed \"%s\"", JetHead::getErrorString( err ) );
	d2 = "test";
	err = d2.remove();
	IfError( "cleanup failed \"%s\"", JetHead::getErrorString( err ) );
#endif

}

// File Info and helpers (length, getModifiedTime, isFile, isDir, exists
void PathTest::Test7()
{
	Path p1( "pathTest" );
		
	if ( p1.length() == 0 )
		TestFailed( "length zero for executable file." );
}

// Dir Listing ( list )
void PathTest::Test8()
{
}

// Path <-> URI (URI assign, construct and toURI )
void PathTest::Test9()
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

