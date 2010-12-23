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

#include "File.h"
#include "jh_memory.h"
#include "logging.h"
#include "Selector.h"
#include "jh_list.h"

using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class FileTest : public TestCase, public FileListener
{
public:
	FileTest( int test_id );
	virtual ~FileTest();

	void Run();
	
	static const int gNumberOfTests = 6;
	
protected:
	virtual void handleData( File *f, short events );

private:
	void Test1(); // Open/Close tests
	void Test2(); // read/write tests
	void Test3(); // Position tests
	void Test4(); // mmap tests
	void Test5(); // Pipe and Selector tests
	void Test6(); // JetHead::getErrorString testing.
	
	bool fillFile( File &f );
	bool validateFile( File &f );
	
	int mTest;
};

FileTest::FileTest( int test_id ) : TestCase( "FileTest" ), mTest( test_id )
{
	char name[ 32 ];
	snprintf( name, 32, "FileTest%d", test_id );
	name[ 31 ] = '\0';
	SetTestName( name );	
}

// Have the destructor do our clean up work. 
FileTest::~FileTest()
{
}

void FileTest::Run()
{
	LOG_NOTICE( "File Test Started" );

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
	}

	TestPassed();
}

bool FileTest::fillFile( File &f )
{
	uint8_t buffer[ 256 ];
	
	for ( int i = 0; i < 256; i++ )
		buffer[ i ] = i;
	
	for ( int i = 0; i < 6; i++ )
	{
		buffer[ 0 ] = i;

		int res = f.write( buffer, 256 );
		
		if ( res != 256 )
			return false;
	}
	return true;
}

bool FileTest::validateFile( File &f )
{
	uint8_t buffer[ 256 ];
	
	for ( int i = 0; i < 6; i++ )
	{
		int res = f.read( buffer, 256 );

		LOG_INFO( "read back failed %d, %d %s", i, res, getErrorString( f.getLastError() ) );

		if ( res != 256 )
			return false;

		print_buffer2( "File", buffer, 256 );
		
		if ( buffer[ 0 ] != i )
			return false;

		LOG_INFO( "byte one good" );
		
		for ( int j = 1; j < 256; j++ )
		{
			if ( buffer[ j ] != j )
			{
				LOG_INFO( "Failed at byte %d", j );
				return false;
			}
		}
	}
	return true;
}

// Open some files, see what happens when you play with the permissions.
// Do some writing then do some reading and all the while seek here and there
void FileTest::Test1()
{
	File f1;
	ErrCode err;
	
	// ensure that p does not exist.
	Path p( "tmp1" );
	p.remove();
	
	err = f1.open( "tmp1" );

	if ( err != kNotFound )
		TestFailed( "Open file that does not exist failed. \"%s\"", getErrorString( err ) );
	
	err = f1.open( "tmp1", File::OF_CREATE | File::OF_RDWR );

	if( err != kNoError )
		TestFailed( "Failed to create file. \"%s\"", getErrorString( err ) );

	if ( fillFile( f1 ) == false )
		TestFailed( "Failed to fill new file" );
	
	f1.setPos( 0 );
	
	if ( validateFile( f1 ) == false )
		TestFailed( "Failed to validate new file" );

	f1.close();
	
	err = f1.open( "tmp1" );

	if( err != kNoError )
		TestFailed( "Failed to create file. \"%s\"", getErrorString( err ) );

	if ( validateFile( f1 ) == false )
		TestFailed( "Failed to validate re-opened file" );
	
	if ( f1.getLength() != 6 * 256 )
		TestFailed( "Failed to validate length" );

	f1.close();

	err = f1.open( "tmp1", File::OF_TRUNC | File::OF_WRITE );

	if( err != kNoError )
		TestFailed( "Failed to create file. \"%s\"", getErrorString( err ) );

	if ( f1.getLength() != 0 )
		TestFailed( "Failed to validate length of truncated file" );

	if ( fillFile( f1 ) == false )
		TestFailed( "Failed to fill file" );
	
	f1.close();

	err = f1.open( "tmp1", File::OF_APPEND | File::OF_WRITE );

	if( err != kNoError )
		TestFailed( "Failed to create file. \"%s\"", getErrorString( err ) );

	if ( f1.getLength() != 6 * 256 )
		TestFailed( "Failed to validate length of truncated file" );

	if ( fillFile( f1 ) == false )
		TestFailed( "Failed to fill file" );

	if ( f1.getLength() != 2 * 6 * 256 )
		TestFailed( "Failed to validate length of truncated file" );
		
	f1.close();
}


// read/write
void FileTest::Test2()
{
	File f;
	char buffer[ 100 ];
	
	// ensure that p does not exist.
	Path p( "tmp2" );
	p.touch();
	
	// check that write fails on a read only file
	ErrCode err = f.open( p, File::OF_READ );

	if( err != kNoError )
		TestFailed( "Failed to open file. \"%s\"", getErrorString( err ) );

	int res = f.write( buffer, 100 );
	
	if ( res != -1 )
		TestFailed( "write on read only succeeded %d", res );

	f.close();

	// check that read fails on a write only file
	err = f.open( p, File::OF_WRITE );

	if( err != kNoError )
		TestFailed( "Failed to open file. \"%s\"", getErrorString( err ) );

	res = f.read( buffer, 100 );
	
	if ( res != -1 )
		TestFailed( "read on write only succeeded %d", res );

	f.close();	

	// make some data to write/read
	for ( int i = 0; i < 100; i++ )
	{
		buffer[ i ] = 'a' + ( i % 26 );
	}
	
	err = f.open( p, File::OF_WRITE );

	if( err != kNoError )
		TestFailed( "Failed to open file. \"%s\"", getErrorString( err ) );

	// write the test data
	res = f.write( buffer, 100 );
	
	if ( res == -1 )
		TestFailed( "write on write only failed %d", res );

	f.close();	

	// clear the buffer 
	memset( buffer, 0, 100 );
	
	err = f.open( p, File::OF_READ );

	if( err != kNoError )
		TestFailed( "Failed to open file. \"%s\"", getErrorString( err ) );

	// read the a-z data
	res = f.read( buffer, 100 );
	
	if ( res == -1 )
		TestFailed( "read on read only failed %d", res );

	f.close();	

	// validate the read data.
	for ( int i = 0; i < 100; i++ )
	{
		if ( buffer[ i ] != 'a' + ( i % 26 ) )
			TestFailed( "Failed to valudate buffer from read" );
	}

	// this should fail since the file is closed. 
	res = f.read( buffer, 100 );

	if ( res != -1 )
		TestFailed( "read didn't fail on closed file" );

	res = f.write( buffer, 100 );

	if ( res != -1 )
		TestFailed( "write didn't fail on closed file" );
}


// position
void FileTest::Test3()
{
	char buffer[ 100 ];
	char buffer2[ 100 ];
	Path p( "tmp3" );
	p.touch();
	int start_pos[] = { 200, 400, 512, 634, 789 };
	int length[] = { 100, 76, 88, 100, 11 };
	
	// make a buffer for a-z over and over.
	for ( int i = 0; i < 100; i++ )
	{
		buffer[ i ] = 'a' + ( i % 26 );
	}

	File f;
	
	f.open( p, File::OF_WRITE );

	// fill the file with 1536 bytes of data.	
	fillFile( f );

	// add our a-z data at various location and lengths, validate the offset
	//  after writes also.
	for ( int i = 0; i < JH_ARRAY_SIZE( start_pos ); i++ )
	{
		f.setPos( start_pos[ i ] );

		int res = f.write( buffer, length[ i ] );
	
		if ( res != length[ i ] )
			TestFailed( "Failed to write" );
		
		if ( f.getPos() != start_pos[ i ] + length[ i ] )
			TestFailed( "GetPos does not match %d", i );
	}
	
	f.close();
	
	f.open( p, File::OF_READ );

	// now validate that with the re-opened file we can read the a-z data 
	//  at locations.
	for ( int i = 0; i < JH_ARRAY_SIZE( start_pos ); i++ )
	{
		f.setPos( start_pos[ i ] );

		memset( buffer2, 0, 100 );
		
		int res = f.read( buffer2, length[ i ] );
	
		if ( res != length[ i ] )
			TestFailed( "Failed to write" );
		
		if ( memcmp( buffer2, buffer, length[ i ] ) != 0 )
			TestFailed( "Buffer failed to match %d", i );

		if ( f.getPos() != start_pos[ i ] + length[ i ] )
			TestFailed( "GetPos does not match %d", i );
	}

	f.close();	

	File f2;
	
	f2.open( p, File::OF_READ );

	// some tests of seekEnd	
	if ( f2.seekEnd() != f2.getLength() )
		TestFailed( "SeekEnd not right" );
	
	if ( f2.getPos() != f2.getLength() )
		TestFailed( "GetPos after SeekEnd not right" );

	// just make sure this work with a new File object also.
	for ( int i = 0; i < JH_ARRAY_SIZE( start_pos ); i++ )
	{
		f2.setPos( start_pos[ i ] );

		memset( buffer2, 0, 100 );
		
		int res = f2.read( buffer2, length[ i ] );
	
		if ( res != length[ i ] )
			TestFailed( "Failed to write" );
		
		if ( memcmp( buffer2, buffer, length[ i ] ) != 0 )
			TestFailed( "Buffer failed to match %d", i );

		if ( f2.getPos() != start_pos[ i ] + length[ i ] )
			TestFailed( "GetPos does not match %d", i );
	}

	f2.close();	
	
	int res = f.getPos();

	if ( res != (jh_off64_t)-1 )
		TestFailed( "getPos didn't fail on closed file" );
}

// mmap
void FileTest::Test4()
{
	Path p( "tmp4" );
	p.touch();
	File f;
	
	f.open( p, File::OF_WRITE );

	// fill the file with 1536 bytes of data.	
	fillFile( f );

	f.close();
	
	f.open( p, File::OF_RDWR );

	uint8_t *file_data = f.mmap();
	
	if ( file_data == NULL )
		TestFailed( "Failed to map file" );
		
	if ( f.getLength() != 1536 )
		TestFailed( "file incorrect length" );
		
	for ( int i = 0; i < 6; i++ )
	{
		if ( file_data[ i * 256 ] != i )
			TestFailed( "Failed to validate file data with mmap" );

		LOG_INFO( "byte one good" );
		
		for ( int j = 1; j < 256; j++ )
		{
			if ( file_data[ ( i * 256 ) + j ] != j )
			{
				LOG_INFO( "Failed at byte %d", j );
				TestFailed( "Failed to validate file data with mmap" );
			}
		}
	}
	
	file_data[ 100 ] = 5;
	
	f.msync();

	File f2;
	f2.open( p );
	
	if ( validateFile( f2 ) )
		TestFailed( "mmap modified file not changed on disk" );
	
	f2.close();
	
	f.munmap();
	f.close();

	file_data = f.mmap();
	
	if ( file_data != NULL )
		TestFailed( "Calling mmap on closed file worked..." );
}

// pipe/selector
void FileTest::Test5()
{
	File *pipe_files[ 2 ];
	Selector s;
	const char buffer[] = "Hello World";
	
	File::pipe( pipe_files );
	
	pipe_files[ File::PIPE_READER ]->setSelector( (FileListener*)this, &s );

	int length = strlen( buffer );
	
	int res = pipe_files[ File::PIPE_WRITER ]->write( buffer, length );
	
	if ( res != length )
		TestFailed( "Failed to write pipe %d", res );
	
}

void FileTest::handleData( File *f, short events )
{
	char buffer[ 32 ];
	
	int res = f->read( buffer, 32 );
	
	if ( res != 11 )
		TestFailed( "Failed to read correct amount of data" );
	
	buffer[ res ] = '\0';
	
	if ( strcmp( buffer, "Hello World" ) != 0 )
		TestFailed( "String read not correct \"%s\"", buffer );
}

// Test JetHead Error Codes, this should be somewhere more generic...
void FileTest::Test6()
{
	const char *str = NULL;
	
	for ( int i = 0; i < kMaxErrorString; i++ )
	{
		str = getErrorString( (ErrCode)i );
		if ( str == NULL )
			TestFailed( "Failed to get error string for error code %d", i );
	}
	
	if ( getErrorString( kMaxErrorString ) != NULL )
		TestFailed( "Failed to get NULL for out of range error code %d", kMaxErrorString );
		
	if ( getErrorString( (ErrCode)-1 ) != NULL )
		TestFailed( "Failed to get NULL for out of range error code %d", -1 );
}

int main( int argc, char* argv[] )
{
	TestRunner runner( argv[ 0 ] );
	TestSuite ts;
	
	for ( int i = 0; i < FileTest::gNumberOfTests; i++ )
	{
		ts.AddTestCase( jh_new FileTest( i + 1 ) );
	}
	
	runner.RunAll( ts );
	
	return 0;
}

