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

/* From example of timeUtilsTest.cpp*/
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "CircularBuffer.h"
#include "jh_memory.h"
#include "logging.h"

using namespace std;
using namespace JetHead;

SET_LOG_CAT( LOG_CAT_DEFAULT );
SET_LOG_LEVEL( LOG_LVL_NOTICE );



// Stuff to keep make life easy
#include <algorithm>
#include <numeric>

#include <iostream>
#include <time.h>
#include <fstream>
#include <fcntl.h>
#include <File.h>
#include <Thread.h>
#include <Condition.h>
#include <Mutex.h>

#include "TestCase.h"


// Life will get ugly if this is not an even number.
// I may fix this later.
#define BUFFER_SIZE 128
#define HUGE_BUF_SIZE 254
#define TEST_FILE_NAME "testFile"

class CircBufTest : public TestCase
{
public:
	CircBufTest( int test_id);
	virtual ~CircBufTest() { ; }

private:
	void Run();

	// Byte interface and get... functions
	// If you don't pass a buffer it will create one of size HUGE_BUF_SIZE
	// If you do pass a buffer it should be of size HUGE_BUF_SIZE
	void test1(CircularBuffer* buf = NULL);
	// Test read / write of a buffer on the heap.
	// If you don't pass a buffer it will create one of size BUFFER_SIZE
	// If you do pass a buffer it should be of size BUFFER_SIZE
	void test2(CircularBuffer* buf = NULL);
	// Test copy / write_overflow of a buffer on the stack.
	void test3();
	// Test functions that interace with File.
	void test4();
	// Test locks and wait... functions
	void test5();
	// Tests two arg constructor,
	// Two arg constructor 'works' but uses an extra byte of memory
	// that it wsa not given.
	void test6();

	// Test for fill/wrap problem with getBytes returning 0 usable bytes
	void test7();

	// Test for fill/wrap problem with getBytes returning 0 usable
	// bytes + offsets
	void test8();

	// Test for get/peekLine
	void test9();

	int mTest;


	void thread1main();
	void thread2main();

	CircularBuffer gbuff;
	Condition gbuff_cond;
	Mutex gbuff_mutex;

};

CircBufTest::CircBufTest(int test_id)
		: TestCase("CircBuf"), mTest(test_id), gbuff(BUFFER_SIZE)
{

	char name[32];

	sprintf( name, "Circular Buffer Test %d", test_id);

	SetTestName( name );

}


void CircBufTest::Run()
{
	LOG_NOTICE( "Circular Buffer Test Started" );

	switch ( mTest )
	{
	case 0:
		test9();
		break;
	case 1:
		test1();
		break;
	case 2:
		test2();
		break;
	case 3:
		test3();
		break;
	case 4:
		test4();
		break;
	case 5:
		test5();
		break;
	case 6:
		test6();
		break;
	case 7:
		test7();
		break;
	case 8:
		test8();
		break;
	}

	TestPassed();
}



// Starting simple with the filling and unfilling.
void CircBufTest::test1(CircularBuffer* tbuff)
{
	CircularBuffer tinyBuf = CircularBuffer(1);
	// uint8_t tmpbuf[HUGE_BUF_SIZE];
	//CircularBuffer hugeBuf = CircularBuffer(tmpbuf,HUGE_BUF_SIZE);
	CircularBuffer hugeBuf = CircularBuffer(HUGE_BUF_SIZE);


	if(tbuff != NULL)
		hugeBuf = *tbuff;

	uint8_t buf[HUGE_BUF_SIZE];
	uint8_t dumpBuf[HUGE_BUF_SIZE];

	iota(buf, buf + HUGE_BUF_SIZE , (uint8_t) 0);


	// Checking getFreeSpace, getSize, getLength on a buffer of size 1.
	if (tinyBuf.getFreeSpace() != 1 && tinyBuf.getSize() != 1)
		TestFailed("getFreeSpace or getSize failure.");

	if (tinyBuf.getLength() != 0)
		TestFailed("getLength failure.");

	tinyBuf.writeByte(0);

	// Test that after writing a byte free space and length are updated correctly
	if (tinyBuf.getLength() != 1 && tinyBuf.getSize() != 1 && tinyBuf.getFreeSpace() != 0)
		TestFailed("update of buffer length after writeByte failed");

	tinyBuf.clear();

	// Test that after calling claer free space and length are updated correctly
	if (tinyBuf.getLength() != 0 &&  tinyBuf.getSize() != 1 && tinyBuf.getFreeSpace() != 1)
		TestFailed("update of buffer length after clear failed");


	// Test individual write
	hugeBuf.write(buf, HUGE_BUF_SIZE / 2);
	
	if (hugeBuf.getFreeSpace() != HUGE_BUF_SIZE / 2
		and hugeBuf.getLength() != HUGE_BUF_SIZE / 2)
	{
		TestFailed("update of buffer length after write failed");
	}
	
	// Test that a read can undo the write
	hugeBuf.read(dumpBuf, HUGE_BUF_SIZE);
	if (hugeBuf.getFreeSpace() != HUGE_BUF_SIZE
		and hugeBuf.getLength() != 0)
	{
		TestFailed("update of buffer length after read failed");
	}
	
	
	// Test individual write again
	hugeBuf.write(buf, HUGE_BUF_SIZE / 2);
	
	if (hugeBuf.getFreeSpace() != HUGE_BUF_SIZE / 2
		and hugeBuf.getLength() != HUGE_BUF_SIZE / 2)
	{
		TestFailed("update of buffer length after write failed");
	}
	
	int tmp = hugeBuf.byteAt(HUGE_BUF_SIZE / 2);
	if (tmp != -1)
	{
		TestFailed("byteAt failed");
	}
	
	int tmpsz = HUGE_BUF_SIZE;
	const uint8_t *tmpptr = hugeBuf.getBytes(0, tmpsz);
	if (tmpsz != HUGE_BUF_SIZE / 2)
	{
		TestFailed("getBytes failed");
	}
	tmpptr = NULL;
	
	hugeBuf.clear();
	if (hugeBuf.getFreeSpace() != HUGE_BUF_SIZE
		and hugeBuf.getLength() != 0)
	{
		TestFailed("update of buffer length after clear failed");
	}
	
	// More testing of getSize, getFreeSpace, getLength, but with a large buffer.
	// Also testing byte interface.

	// This code is semi-randomized, it initally fills a bit of the buffer and
	// empties it, with the goal to get the buffer out of its initial state.

	// Then in each iteration it adds some amount
	// (starting small and becoming large) of bytes to the buffer,
	//  and takes them out.
	for (int j = 1; j < HUGE_BUF_SIZE; j++)
	{
		// Warning: If clear could screw up something below, but I doubt it.
		if( j % 24 == 0)
		{
			hugeBuf.clear();

			if( hugeBuf.getSize() != HUGE_BUF_SIZE &&
					hugeBuf.getFreeSpace() != HUGE_BUF_SIZE &&
					hugeBuf.getLength() != 0)
				TestFailed("getLength, getSize, or getFreeSpace failed after a clear.");
		}
		int offset = rand() % j;

		// Just want to advance starting position.
		hugeBuf.write(buf, offset );
		hugeBuf.read(dumpBuf, offset);



		for( int k = 0; k < HUGE_BUF_SIZE; k++)
		{

			if( hugeBuf.getSize() != HUGE_BUF_SIZE &&
					hugeBuf.getFreeSpace() != HUGE_BUF_SIZE &&
					hugeBuf.getLength() != 0)
				TestFailed("getLength, getSize, or getFreeSpace failure.");

			for( int i = 0; i < k; i++)
				hugeBuf.writeByte((uint8_t)i);

			if( hugeBuf.getSize() != HUGE_BUF_SIZE &&
					hugeBuf.getFreeSpace() != HUGE_BUF_SIZE - k &&
					hugeBuf.getLength() != k)
				TestFailed("getLength, getSize, or getFreeSpace failure.");

			// Do it backwards for fun.
			for(int i = k - 1; i > -1; i--)
				if( hugeBuf.byteAt(i) != i)
					TestFailed("writeByte or byteAt failed");

			int size;
			for( int i = 0; i < k; i++)
			{
				const uint8_t *loc = hugeBuf.getBytes(i , size);
				if ( memcmp(loc, buf + i, size) )
					TestFailed("getBytes failed");
			}

			// byteAt doesn't take things out of the buffer, so do it manually.
			hugeBuf.read(dumpBuf,hugeBuf.getLength());
		}
	}
}


// This code is semi-randomized, it initally fills a bit of the buffer and
// empties it, with the goal to get the buffer out of its initial state.
// It then has four tests in runs, small reads, big reads, small writes,
// and big writes.
void CircBufTest::test2(CircularBuffer *testBuf)
{
	CircularBuffer tinyBuf = CircularBuffer(1);
	CircularBuffer bigBuf = CircularBuffer(111);

	uint8_t buf[BUFFER_SIZE];
	uint8_t dumpBuf[BUFFER_SIZE];

	int count = 0;
	CircularBuffer *heapBuf;

	if( testBuf == NULL)
		heapBuf = (CircularBuffer*) (jh_new CircularBuffer(BUFFER_SIZE));
	else
		heapBuf = testBuf;

	iota(buf, buf + BUFFER_SIZE, (uint8_t) 0);

	// 1st Test: use one arg constructor to make a heapBuf, fill it
	// with a bunch of small writes, then make sure it worked.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		heapBuf->write(buf, offset );
		heapBuf->read(dumpBuf, offset);

		count = 0;
		while (count < BUFFER_SIZE)
		{

			int size = rand() % 5;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			heapBuf->write(buf + count, size);
			count += size;
		}


		heapBuf->read(dumpBuf, BUFFER_SIZE);


		for( int i = 0; i < BUFFER_SIZE; i++)
		{
			if ( buf[i] != dumpBuf[i] )
			{
				TestFailed("Random start smallish write test failed.") ;
			}
		}
	}

	// 2st Test: use one arg constructor to make a heapBuf, fill it
	// with a bunch of big writes, then make sure it worked.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		heapBuf->write(buf, offset );
		heapBuf->read(dumpBuf, offset);

		count = 0;
		while (count < BUFFER_SIZE)
		{

			int size = rand() % 50;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			heapBuf->write(buf + count, size);
			count += size;
		}


		heapBuf->read(dumpBuf, BUFFER_SIZE);


		for( int i = 0; i < BUFFER_SIZE; i++)
		{
			if ( buf[i] != dumpBuf[i] )
			{
				TestFailed(" Random start bigish write test failed.") ;
			}
		}
	}


	// 3nd Test: refill buffer, read small bits at a time.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		heapBuf->write(buf, offset );
		heapBuf->read(dumpBuf, offset);

		heapBuf->write(buf, BUFFER_SIZE );

		// Why use a for loop when you can do it the C++ way?
		fill(dumpBuf, dumpBuf + BUFFER_SIZE, 0);

		count = 0;
		while( count < BUFFER_SIZE)
		{

			int size = rand() % 5;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			heapBuf->read(dumpBuf + count , size);
			count += size;
		}

		for( int i = 0; i < BUFFER_SIZE; i++ )
		{
			if( dumpBuf[i] != i)
			{
				TestFailed("smallish read failed.");

			}
		}
	}


	// 4th Test: refill buffer, big bits at a time.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		heapBuf->write(buf, offset );
		heapBuf->read(dumpBuf, offset);


		heapBuf->write(buf, BUFFER_SIZE );

		// Why use a for loop when you can do it the C++ way?
		fill(dumpBuf, dumpBuf + BUFFER_SIZE, 0);


		count = 0;
		while( count < BUFFER_SIZE)
		{

			int size = rand() % 50;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			heapBuf->read(dumpBuf + count , size);
			count += size;

		}

		for( int i = 0; i < BUFFER_SIZE; i++ )
		{
			if( dumpBuf[i] != i )
			{
				TestFailed("big reads failed.");
			}

		}

	}
}

void CircBufTest::test3()
{

	// Like test2 but using bigBug instead of heapBuf,
	// write_overflow instead of write, and copy instead of read

	CircularBuffer tinyBuf = CircularBuffer(1);
	CircularBuffer bigBuf = CircularBuffer(BUFFER_SIZE / 2 );

	uint8_t buf[BUFFER_SIZE];
	uint8_t dumpBuf[BUFFER_SIZE];
	uint8_t trashBuf[BUFFER_SIZE];

	int count = 0;

	iota(buf, buf + (BUFFER_SIZE / 2), (uint8_t) 0);
	iota(buf + (BUFFER_SIZE / 2), buf + BUFFER_SIZE, (uint8_t) 0);


	// 1st Test: use one arg constructor to make a heapBuf, fill it
	// with a bunch of small writes, then make sure it worked.

	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write_overflow(buf, offset );
		bigBuf.copy(dumpBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));
		bigBuf.read(trashBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));

		count = 0;
		while (count < BUFFER_SIZE)
		{

			int size = rand() % 5;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			bigBuf.write_overflow(buf + count, size);
			count += size;
		}


		bigBuf.copy(dumpBuf, BUFFER_SIZE / 2 );
		bigBuf.read(trashBuf, BUFFER_SIZE / 2 );


		for( int i = 0; i < BUFFER_SIZE / 2; i++)
		{
			if ( buf[i] != dumpBuf[i] )
			{
				cerr << "dumpBuf: " << (int)dumpBuf[i] << endl;
				TestFailed("Random start smallish write_overflow test failed.") ;
			}
		}
	}

	// 2st Test: use one arg constructor to make a bigBuf, fill it
	// with a bunch of big writes, then make sure it worked.

	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write_overflow(buf, offset );
		bigBuf.copy(dumpBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));
		bigBuf.read(trashBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));

		count = 0;
		while (count < BUFFER_SIZE)
		{

			int size = rand() % 50;

			if ( (count + size) > BUFFER_SIZE)
				size = BUFFER_SIZE - count;

			bigBuf.write_overflow(buf + count, size);
			count += size;
		}


		bigBuf.copy(dumpBuf, BUFFER_SIZE / 2 );
		bigBuf.read(trashBuf, BUFFER_SIZE / 2 );


		for( int i = 0; i < BUFFER_SIZE / 2; i++)
		{
			if ( buf[i] != dumpBuf[i] )
			{
				TestFailed(" Random start bigish write_overflow test failed.") ;
			}
		}
	}


	// 3nd Test: refill buffer, read small bits at a time.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write_overflow(buf, offset );
		bigBuf.copy(dumpBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));
		bigBuf.read(trashBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));

		bigBuf.write_overflow(buf, BUFFER_SIZE );

		// Why use a for loop when you can do it the C++ way?
		fill(dumpBuf, dumpBuf + BUFFER_SIZE, 0);


		count = 0;
		while( count < BUFFER_SIZE / 2)
		{

			int size = rand() % 5;

			if ( (count + size) > BUFFER_SIZE / 2)
				size = ( BUFFER_SIZE / 2) - count;

			bigBuf.copy(dumpBuf + count , size);
			bigBuf.read(trashBuf + count , size);
			count += size;
		}

		for( int i = 0; i < BUFFER_SIZE / 2; i++ )
		{
			if( dumpBuf[i] != i)
				TestFailed("smallish read failed.");

		}
	}



	// 4th Test: refill buffer, big bits at a time.
	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write_overflow(buf, offset );
		bigBuf.copy(dumpBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));
		bigBuf.read(trashBuf,(offset > BUFFER_SIZE / 2 ? BUFFER_SIZE / 2 : offset));


		bigBuf.write_overflow(buf, BUFFER_SIZE );

		// Why use a for loop when you can do it the C++ way?
		fill(dumpBuf, dumpBuf + BUFFER_SIZE, 0);


		count = 0;
		while( count < BUFFER_SIZE / 2)
		{

			int size = rand() % 50;

			if ( (count + size) > BUFFER_SIZE / 2)
				size = (BUFFER_SIZE / 2) - count;

			bigBuf.copy(dumpBuf + count , size);
			bigBuf.read(trashBuf + count , size);
			count += size;

		}

		for( int i = 0; i < BUFFER_SIZE / 2; i++ )
		{
			if( dumpBuf[i] != i )
				TestFailed("big reads failed.");

		}

	}

}

// fillFromFile only reads amount in buffer without having to wrap.
// Tests functions that interact with file I/O
void CircBufTest::test4()
{
	CircularBuffer bigBuf = CircularBuffer(BUFFER_SIZE);

	uint8_t buf[BUFFER_SIZE];
	uint8_t dumpBuf[BUFFER_SIZE];

	iota(buf, buf + BUFFER_SIZE , (uint8_t) 0);

	ofstream testFile;
	testFile.open(TEST_FILE_NAME, ios::out);
	for( char i = 'a'; i < 'z'+ 1; i++)
		testFile.put(i);


	testFile.flush();
	testFile.close();

	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write(buf, offset );
		bigBuf.read(dumpBuf, offset);


		int testFd = ::open(TEST_FILE_NAME,O_RDONLY  );
		lseek(testFd, 0, SEEK_SET);

		assert((bigBuf.getLength() == 0));
		assert((bigBuf.getSize() > ('z' - 'a' + 1)));
		assert((bigBuf.getFreeSpace() > ('z' - 'a' + 1)));

		int amntRead =  bigBuf.fillFromFile(testFd, 'z' - 'a' + 1);
		if (amntRead != 'z' - 'a' + 1)
			bigBuf.fillFromFile(testFd, ('z' - 'a' + 1) - amntRead);
		assert(bigBuf.getLength() == ('z' - 'a' + 1));

		::close(testFd);


		for( char i = 'a'; i < 'z'+1; i++) {
			//cerr << j <<" " << (int)i << " " << (int)bigBuf.byteAt(i - 'a') << endl;
			if( (char)bigBuf.byteAt(i - 'a') != i)
				TestFailed("fillFromFile (fd) failed.");
			;}

		// byteAt doesn't take things out of the buffer, so do it manually.
		bigBuf.read(dumpBuf,bigBuf.getLength());
		assert(bigBuf.getLength() == 0 );
	}


	// emptyToFile is not implemented so can't really test it.
	/*::truncate(TEST_FILE_NAME, 0);

	testFd = ::open(TEST_FILE_NAME,O_RDWR);
	 bigBuf.emptyToFile(testFd, BUFFER_SIZE);
	::close(testFd);


	testFile.open(TEST_FILE_NAME, ios::in);
	for( int i = 0; i < BUFFER_SIZE; i++)
{
	  char check;
	  testFile.read(&check,1);
	  if (check != (char)i)
		TestFailed(" fillFromFile or emptyToFile failed.");    
}

	*/

	File tfile;

	tfile.open(TEST_FILE_NAME);



	for (int j = 1; j < BUFFER_SIZE; j++)
	{
		int offset = rand() % j;

		// Just want to advance starting position.
		bigBuf.write(buf, offset );
		bigBuf.read(dumpBuf, offset);

		tfile.setPos(0);

		assert((bigBuf.getLength() == 0));
		assert((bigBuf.getSize() > ('z' - 'a' + 1)) );
		assert((bigBuf.getFreeSpace() > ('z' - 'a' + 1)));

		int amntRead = bigBuf.fillFromFile(&tfile, 'z' - 'a' + 1);
		if (amntRead != 'z' - 'a' + 1)
			bigBuf.fillFromFile(&tfile, ('z' - 'a' + 1) - amntRead);
		assert(bigBuf.getLength() == ('z' - 'a' + 1));

		for( char i = 'a'; i < 'z' +1; i++)
		{ //cerr << j <<" " << (int)i << " " << (int)bigBuf.byteAt(i - 'a') << endl;

			if( (char)(bigBuf.byteAt(i - 'a')) != i)
				TestFailed("fillFromFile (File) failed.");
		}
		// byteAt doesn't take things out of the buffer, so do it manually.
		bigBuf.read(dumpBuf,bigBuf.getLength());
		assert(bigBuf.getLength() == 0 );
	}


}


// Someone grab a lock and wait for mroe data to come in: (it will fail)
// Someone wait for data and someone else puts in data
// same as above except taking data out

void CircBufTest::thread1main()
{
	uint8_t buf[BUFFER_SIZE];
	uint8_t dumpBuf[BUFFER_SIZE];

	fill(buf, buf + BUFFER_SIZE, 1);
	fill(dumpBuf, dumpBuf + BUFFER_SIZE, 20);

	gbuff.Lock();

	int offset = gbuff.getLength();


	gbuff.write(buf, BUFFER_SIZE / 2);

	gbuff.copy(dumpBuf, gbuff.getLength());

	for( int i = 0 + offset; i < ( offset + (BUFFER_SIZE / 2)); i++)
		if( buf[i - offset] != dumpBuf[i] )
			TestFailed("Lock doesn't work.");


	gbuff.Unlock();

	// Wait till the other guy fills it up, or if I fill it up keep going.
	if( !gbuff.waitForData(BUFFER_SIZE, 1000))
		TestFailed("waitForData might be broken.");

	gbuff.clear();

}

void CircBufTest::thread2main()
{

	uint8_t buf[BUFFER_SIZE];
	uint8_t dumpBuf[BUFFER_SIZE];

	fill(buf, buf + BUFFER_SIZE, -1);
	fill(dumpBuf, dumpBuf + BUFFER_SIZE, 10);

	gbuff.Lock();
	int offset = gbuff.getLength();

	gbuff.write(buf, BUFFER_SIZE / 2);

	gbuff.copy(dumpBuf, gbuff.getLength());

	for( int i = 0 + offset; i < offset + (BUFFER_SIZE / 2); ++i)
		if( buf[i - offset] != dumpBuf[i] )
			TestFailed("Lock doesn't work.");


	gbuff.Unlock();

	// Wait till the other guy clears the buffer.
	if(!gbuff.waitForFreeSpace(BUFFER_SIZE, 1000))
		TestFailed("waitForFreeSpace might be broken.");


}

// Tests functions that use Locks
void CircBufTest::test5()
{
	Runnable<CircBufTest> thread1("Thread 1", this, &CircBufTest::thread1main);
	Runnable<CircBufTest> thread2("Thread 2", this, &CircBufTest::thread2main);

	thread2.Start();
	thread1.Start();

	thread1.Join();
	thread2.Join();
}

// Test two arg constructor (ie, turing a normal buffer into a cir buffer)

void CircBufTest::test6()
{

	// Use a struct so that you can set a watchpoint in gdb on overflow
	// to see when circbuf attempts to use a byte of memeory it wasn't
	// given.
	struct {
		uint8_t testBuf[HUGE_BUF_SIZE];
		uint8_t overflow;
	} buf_t1;
	
	struct {
		uint8_t testBuf[BUFFER_SIZE];
		uint8_t overflow;
	} buf_t2 ;

	fill(buf_t1.testBuf, buf_t1.testBuf + HUGE_BUF_SIZE, -1);
	fill(buf_t2.testBuf, buf_t2.testBuf + BUFFER_SIZE, -1);
	
	buf_t1.overflow = (uint8_t)-1;
	buf_t2.overflow = (uint8_t)-1;
	
	CircularBuffer t1Buf(buf_t1.testBuf, HUGE_BUF_SIZE);
	CircularBuffer t2Buf(buf_t2.testBuf, BUFFER_SIZE);
	
	// Try test 1 again and check overflow
	test1(&t1Buf);
	if (buf_t1.overflow != (uint8_t)-1)
	{
		TestFailed("Wrapped circular buffer has one past the end problem");
	}
	
	test2(&t2Buf);

	if( buf_t2.overflow != (uint8_t)-1)
	{
		TestFailed("Wrapped circular buffer has one past the end problem");
	}
}

void CircBufTest::test7()
{
	CircularBuffer buf(BUFFER_SIZE);
	uint8_t* temp = jh_new uint8_t[BUFFER_SIZE];
	
	buf.write(temp, BUFFER_SIZE);
	buf.read(temp, BUFFER_SIZE);

	int len;
	const uint8_t* p = buf.getBytes(0, len);

	if (len != 0)
	{
		TestFailed("Empty circular buffer returning usable bytes");
	}
	
	buf.write(temp, BUFFER_SIZE);
	p = buf.getBytes(0, len);

	if (len != BUFFER_SIZE)
	{
		TestFailed("Full circular buffer returning bogus byte count");
	}
}	

void CircBufTest::test8()
{
	CircularBuffer buf(BUFFER_SIZE);
	uint8_t* temp = jh_new uint8_t[BUFFER_SIZE];
	
	buf.write(temp, BUFFER_SIZE);
	buf.read(temp, BUFFER_SIZE);

	int len;
	const uint8_t* p = buf.getBytes(0, len);

	if (len != 0)
	{
		TestFailed("Empty circular buffer returning usable bytes");
	}
	
	buf.write(temp, BUFFER_SIZE);
	p = buf.getBytes(5, len);

	if (len != BUFFER_SIZE - 5)
	{
		TestFailed("Full circular buffer returning bogus byte count");
	}
}	

void CircBufTest::test9()
{
	CircularBuffer buf(120);
	uint8_t* temp = jh_new uint8_t[120];
	const char *test_string = "This is a string of 30 bytes\r\n";
	JHSTD::string tss( test_string, 28 );
	JHSTD::string ss;
	
	int res = buf.getLine( ss, "\r\n" );

	if ( res != 0 )
		TestFailed( "empty buffer test" );

	res = buf.peekLine( ss, "\r\n" );

	if ( res != 0 )
		TestFailed( "empty buffer test" );
	
	// fill buffer with alligned strings.
	for ( int i = 0; i < 4; i++ )
	{
		res = buf.write( (uint8_t*)test_string, 30 );
	
		if ( res != 30 )
			TestFailed( "Failed to write 30 bytes" );
	}
	
	for ( int i = 0; i < 4; i++ )
	{
		res = buf.peekLine( ss, "\r\n" );
		if ( res != 30 || ss != tss )
		{
			TestFailed( "peek does not match %d res %d str \"%s\"", i, res, ss.c_str() );
		}

		res = buf.getLine( ss, "\r\n" );
		if ( res != 30 || ss != tss )
		{
			TestFailed( "get does not match %d res %d str \"%s\"", i, res, ss.c_str() );
		}
	}
	
	buf.write( temp, 15 );
	buf.read( temp, 15 );

	// fill buffer with not alligned.
	for ( int i = 0; i < 4; i++ )
	{
		res = buf.write( (uint8_t*)test_string, 30 );
	
		if ( res != 30 )
			TestFailed( "Failed to write 30 bytes" );
	}
	
	for ( int i = 0; i < 4; i++ )
	{
		res = buf.peekLine( ss, "\r\n" );
		if ( res != 30 || ss != tss )
		{
			TestFailed( "peek does not match %d res %d str \"%s\"", i, res, ss.c_str() );
		}

		res = buf.getLine( ss, "\r\n" );	
		if ( res != 30 || ss != tss )
		{
			TestFailed( "get does not match %d res %d str \"%s\"", i, res, ss.c_str() );
		}
	}

	res = buf.getLine( ss, "\r\n" );

	if ( res != 0 )
		TestFailed( "empty buffer test" );

	res = buf.peekLine( ss, "\r\n" );

	if ( res != 0 )
		TestFailed( "empty buffer test" );	
}	

static const int gNumberTests = 9;

int main( int argc, char *argv[] )
{
	srand(time(NULL));

	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ gNumberTests ];

	for ( int i = 0; i < gNumberTests; i++ )
		test_set[ i ] = jh_new CircBufTest( i );

	runner.RunAll( test_set, gNumberTests );

	return 0;


}


