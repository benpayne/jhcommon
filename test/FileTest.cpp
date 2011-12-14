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
#include <iostream>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;
using namespace __gnu_cxx;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include <fcntl.h>

#include "TestCase.h"
#include <algorithm>
#include <ext/numeric>
#include <list>

#define FNAME1 "tmp1"
#define FNAME2 "tmp2"
#define FNAME3 "tmp3"
#define FNAME4 "tmp4"
#define FNAME5 "tmp5"
#define FNAME6 "tmp6"

// BUF_SIZE = 26
#define BUF_SIZE ('z' - 'a' + 1)

using namespace __gnu_cxx;


/* Things to know:
 * 
 * 1) Opening a File opens rdwr (ie. it wont nuke wants in the file but 
 * the offset is set to 0.
 *
 * 2) The SelectorListener is slow, it is very easy to overwhelm it.
 */
class FileTest : public TestCase, public SelectorListener
{
public:
	FileTest(int test_id);
	virtual ~FileTest();

protected:
	virtual void processFileEvents(int fd, short events, uint32_t);

private:

	void Run();
	void Test1(); // Basic tests
	void Test2(); // Error cases
	void Test3(); // Selector

	int mTest;
	list<char> l1, l2, l3;
	int dumpfile;
	

};

// We will have our constructor do a lot of the set-up work.
// The reason we have so many different files is that we create
// all instances of the class before any of the tests are run.
// So reusing files causes problems.
FileTest::FileTest(int test_id)
	: TestCase("FileTest"), mTest(test_id)
{
	
	// This 32 causes magic numbers everywhere :(
	char name[32];
	fill( name, name + 32, 0);
	sprintf(name, "File Test %d", test_id);
	SetTestName(name);
	int res;
	
	
	if (test_id == 1)
	{
		
		res = ::open("tmp1", O_CREAT | O_RDWR | O_TRUNC, 
						 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		
		if ( res < 0 )
			TestFailed("Error starting test");
		else
		{
			// Put some data in the file
			write(res, name, sizeof(name));
			l1.insert(l1.begin(), name, name + 32);
			close(res);
		}
		
		res = open(FNAME2, O_CREAT | O_RDWR | O_TRUNC,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if ( res < 0 )
			TestFailed("Error starting test");
		else
		{
			// Put some data in the file
			write(res, name, sizeof(name));
			l2.insert(l2.begin(), name, name + 32);
			close(res);
		}
		
		res = open(FNAME3, O_CREAT | O_RDWR | O_TRUNC,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if ( res < 0 )
			TestFailed("Error starting test");		
		else
		{
			// Put some data in the file.
			write(res, name, sizeof(name));
			l3.insert(l3.begin(), name, name + 32);
			close(res);
		}
	}
	else if(mTest == 2)
	{

		res = open(FNAME4, O_CREAT | O_RDWR | O_TRUNC,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if ( res < 0 )
			TestFailed("Error starting test");		
		else
			close(res);

	}
	else if(mTest == 3)
	{

		res = open(FNAME5, O_CREAT | O_RDWR | O_TRUNC,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if ( res < 0 )
			TestFailed("Error starting test");		
		else
			close(res);

		dumpfile = open(FNAME6, O_CREAT | O_RDWR | O_TRUNC | O_APPEND,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if ( dumpfile < 0 )
			TestFailed("Error starting test");		
		else
			; 
		// We'll close it on exit.
	}
	

}

// Have the destructor do our clean up work. 
FileTest::~FileTest()
{
	int res;

	if (mTest == 1)
	{
		
		res = remove(FNAME1);
		if ( res != 0 )
			;
		// File didn't get erased tell someone
		res = remove(FNAME2);
		if ( res != 0 )
		;
		// File didn't get erased tell someone
		res = remove(FNAME3);
		if ( res != 0 )
			;
		// File didn't get erased tell someone
	}
	else if (mTest == 2)
	{
		res = remove(FNAME4);
		if ( res != 0 )
			;
		// File didn't get erased tell someone
	}
	else if (mTest == 3)
	{
		res = remove(FNAME5);
		close(dumpfile);
		res += remove(FNAME6);
		if ( res != 0 )
			;
		// File didn't get erased tell someone
	}	

}

void FileTest::Run()
{

	LOG_NOTICE( "File Test Started" );

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
	}

	TestPassed();
}

// Open some files, see what happens when you play with the permissions.
// Do some writing then do some reading and all the while seek here and there
void FileTest::Test1()
{

	File f1, f2, f3;
	uint stat;
	
	stat = f1.open(FNAME1);
	//l1.clear();
	if ( stat < 0 )
	{
		TestFailed("Error opening %s.", FNAME1);
	}
	stat = f2.open(FNAME2, O_APPEND);
	if( stat < 0 )
	{
		TestFailed("Error opening %s.", FNAME2);
	}
	stat = f3.open(FNAME3, O_TRUNC);
	l3.clear();
	if ( stat < 0 )
	{
		TestFailed("Error opening %s.", FNAME3);
	}


	// Do some writing
	char *buf = jh_new char[BUF_SIZE];
	iota(buf, buf + BUF_SIZE, 'a');
	
	// Use copy instead of insert because the file hasn't been nuked
	// simply had its offset set to 0.
	copy(buf, buf + BUF_SIZE, l1.begin());
	l2.insert(l2.end(),buf, buf + BUF_SIZE);
	l3.insert(l3.end(),buf, buf + BUF_SIZE);
	
	stat = f1.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed");

	stat = f2.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed");
	
	stat = f3.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed");

	
	// Seek
	stat = f1.getPos();
	// != BUF_SIZE because we opened the file from the begining and 
	// only wrote BUF_SIZE data.
	if ( stat != BUF_SIZE )
		TestFailed("Open didn't obey our flags 1 %d %d.", stat, l1.size());

	// != BUF_SIZE because we opened the file O_APPEND and therefore 
	// everthing is relative to the end of the file when it was opened.
	stat = f3.getPos();
	if ( stat != BUF_SIZE )
		TestFailed("Open didn't obey our flags 2 %d %d.", stat, l2.size());

	stat = f3.getPos();
	if ( stat != l3.size() )
		TestFailed("Open didn't obey our flags 3 %d %d.", stat, l3.size());

	// Set these to somewhere interesting
	f1.setPos(BUF_SIZE / 2);
	// We put 32 chars at the start (see the constructor)
	f2.setPos(32);
	f3.setPos(0);


	// Write a bit more
	iota(buf, buf + BUF_SIZE, 'A');
	list<char>::iterator i1 = l1.begin();
	advance(i1, BUF_SIZE / 2);
	
	// Erase everyting after where we are because it will just
	// be copied over. (Idealy we would have some sort of copy and extend
	// function.)
	l1.erase(i1, l1.end());
	l1.insert(l1.end(), buf, buf + BUF_SIZE);
	
	// Everything is an append therefore offset is reset to the end of the
	// file before every write.
	l2.insert(l2.end(), buf, buf + BUF_SIZE);

	
	copy(buf, buf + BUF_SIZE, l3.begin());
	

	stat = f1.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed 1 %d", stat);

	stat = f2.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed 2 %d", stat);
	


	stat = f3.write(buf, BUF_SIZE);
	if (stat != BUF_SIZE)
		TestFailed("Write Failed 3 %d", stat);



	// Do some reading
	f1.setPos(0);
	f2.setPos(0);
	f3.setPos(0);
	int dumpbuf_size = (BUF_SIZE) * 2 + 32;
	char *dumpbuf = jh_new char[dumpbuf_size];
	fill(dumpbuf, dumpbuf + dumpbuf_size, 0);

	// The magic '32' everywhere is ugly but its here because
	// thats how long the buffer that we shoved the name of
	// the test is and we coppied that buffer into the front
	// of f2 and f3 before we started.

	stat = f1.read(dumpbuf, dumpbuf_size);

	if ( ! equal(l1.begin(), l1.end(), dumpbuf) )
		TestFailed("Write or Read failed 1.");

	stat = f2.read(dumpbuf, dumpbuf_size);
	if ( ! equal(l2.begin(), l2.end(), dumpbuf) )
		TestFailed("Write or Read failed 2.");

	
	stat = f3.read(dumpbuf, dumpbuf_size);
	if ( ! equal(l3.begin(), l3.end(), dumpbuf) )
		TestFailed("Write or Read failed 3.");

	if( !File::isFile(FNAME1) || File::isDir(FNAME1) || !File::exists(FNAME1))
		TestFailed("isFile, isDir, or exists failed");
	if( !File::isFile(FNAME2) || File::isDir(FNAME2) || !File::exists(FNAME2))
		TestFailed("isFile, isDir, or exists failed");
	if( !File::isFile(FNAME3) || File::isDir(FNAME3) || !File::exists(FNAME3))
		TestFailed("isFile, isDir, or exists failed");

	stat = f1.close();
	if ( stat < 0)
		TestFailed("Close Failed");
	
	stat = f2.close();
	if ( stat < 0 )
		TestFailed("Close Failed");
	
	stat = f3.close();
	if ( stat < 0)
		TestFailed("Close Failed");

	

}


// Test what happens when you do silly things.
void FileTest::Test2()
{
	File f1;
	int res;
	char buf[BUF_SIZE];
	fill(buf, buf + BUF_SIZE, 'A');

	res = f1.write(buf, BUF_SIZE);
	if( res > 0 )
		TestFailed("Write should fail on a File that is not open.");



	res = f1.read(buf, BUF_SIZE);
	if( res > 0 )
		TestFailed("Read should fail on a  File that is not open.");

	res = f1.getPos();
	if( res > 0 )
		TestFailed("getPos should fail on a  File that is not open.");

	res = f1.seekEnd();
	if (res > 0 )
		TestFailed("seekEnd should fail on a  File that is not open.");
	
	res = f1.setPos(BUF_SIZE);
	if ( res > 0 )
		TestFailed("seek should fail on a  File that is not open.");
	
	res = f1.getLength();
	if ( res > 0 )
		TestFailed("getLength should fail on a  File that is not open.");

	res = ::open(FNAME1, 0x000);
	close(res);

	res = f1.open2(FNAME1);
	if( res > 0 )
		TestFailed("open should fail on a file that is not read or"
				   "writeable.");

	res = f1.close();
	if( res > 0 )
		TestFailed("close should fail on a closed File.");
	   
	
	res = f1.open(FNAME4);
	if( res < 0 )
		TestFailed("Open failed %d.", res);

	res = f1.write(buf, BUF_SIZE);
	if( res < 1 )
		TestFailed("Write failed.");

	char dumpbuf[BUF_SIZE];
	fill(dumpbuf, dumpbuf + BUF_SIZE, 0);
	f1.setPos(0);
	res = f1.read(dumpbuf, BUF_SIZE);
	if( res < 1 && (! equal(buf, buf + BUF_SIZE, dumpbuf) ) )
		TestFailed("Read failed.");


}


// We have made FileTest into a Selector listener.  All it does
// is copy incoming data into a new file.
void FileTest::processFileEvents(int fd, short events, uint32_t pdata)
{
	char buf[BUF_SIZE + 1];
	fill(buf, buf + BUF_SIZE + 1, 0);
	static uint bytes_total = 0;

	if( POLLIN & events )
	{
		uint bytes = 0;
		if( ioctl(fd, FIONREAD, &bytes) == 0 )
		{
			// Is there data to read?
			if( bytes > bytes_total )
			{
				bytes_total += bytes;
				uint amntRead = 0;
				do {
					
					int temp = ::read(fd, buf, BUF_SIZE);
					amntRead += temp;
					int res = ::write(dumpfile, buf, temp);
					if (res != temp)
						cerr << buf << endl;
				} while( (amntRead > 0) && (amntRead < bytes)  );
			}
			

		}
		if( events & POLLHUP)
			;//	cerr <<" here" << endl;
		

	}

}

// Use setSelector
void FileTest::Test3()
{
	File f1;
	Selector s;
	
	char buf[BUF_SIZE];
	fill(buf, buf + BUF_SIZE, 'B');

	f1.open(FNAME5);
	f1.setSelector(this, &s);

	// For some reason this write isn't causing the SelectorLisener to
	// fire.
	//f1.write(buf, BUF_SIZE);

	// If you increase the sleep time, you can manually append to
	// FNAME5 and the sellector will append it to FNAME6
	// I tried to test this above but it didn't work when writting through
	// f1.
	
	int sleep_time = 1;
	do {
		sleep_time = sleep(sleep_time);
	} while(sleep_time);


	// Check to see if FNAME5 and FNAME6 are the same.

	struct stat64 fstatus;
	fstat64(dumpfile, &fstatus);
	int f6size = fstatus.st_size;
	int f5size = f1.getLength();
	
	if( f5size != f6size )
		TestFailed("Something went wrong wtih Selector.");
	

	char buf5[f5size];
	f1.read(buf5, f5size);
	char buf6[f6size];
	

	if(	! equal(buf5, buf5 + f5size, buf6) )
		TestFailed("Something went wrong with selector.");

	f1.close();

}

int main( int argc, char* argv[] )
{


	TestRunner runner( argv[ 0 ] );
	TestCase *test_set[ 10 ];
	test_set[ 0 ] = jh_new FileTest( 1 );
	test_set[ 1 ] = jh_new FileTest( 2 );
	test_set[ 2 ] = jh_new FileTest( 3 );

   
	runner.RunAll( test_set, 3 );

	
	return 0;
}

