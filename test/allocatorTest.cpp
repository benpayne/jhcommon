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

#include "Allocator.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class AllocatorTest : public TestCase
{
public:
	AllocatorTest( int number ) : 
		TestCase( "AllocatorTest" ), mTestNum( number ) 
	{
		SetTestName( "AllocatorTest" );	
	}

	virtual ~AllocatorTest() {}
	
private:
	int mTestNum;
	Allocator *mAlloc;
	
	void Run()
	{	
		void *data = malloc( 4096 );
		mAlloc = jh_new Allocator( data, 4096 );
		void *ptrs[ 1024 ];
		memset( ptrs, 0, sizeof( void* ) * 1024 );
		int i = 0;
		uint32_t freespace = mAlloc->getFreeSpace();
		
		LOG_NOTICE( "Free space is %d", freespace );
		
		// set specific seed so test is reproducable.
		srand( 0 );

		uint32_t sizes[] = { 6, 8, 12, 16, 22, 32, 102, 252 };
		
		for ( int k = 0; k < 2; k++ )
		{
			i = 0;
			while ( ( ptrs[ i ] = mAlloc->alloc( sizes[ rand() % 8 ] ) ) != NULL && i < 1024 )
			{
				i++;
			}
	
			if ( i == 1024 )
			{
				TestFailed( "Too many allocations for freespace" );
			}
	
			for ( int j = 0; j < i; j++ )
			{
				int offset = rand() % i;
				while( ptrs[ offset ] == NULL )
				{
					offset = ( offset + 1 ) % i;
					//LOG_NOTICE( "already free %d", offset );
				}
				mAlloc->free( ptrs[ offset ] );
				ptrs[ offset ] = NULL;
			}
	
			if ( freespace != mAlloc->getFreeSpace() )
			{
				TestFailed( "Not all items freed (%d %d)", 
					freespace, mAlloc->getFreeSpace() );					
			}
		}
		
		LOG_NOTICE( "freespace %d", mAlloc->getFreeSpace() );		
		i = 0;
		
		while ( ( ptrs[ i ] = mAlloc->alloc( 32 ) ) != NULL && i < 1024 )
		{
			i++;
		}

		if ( i == 1024 )
		{
			TestFailed( "Second too many allocations for freespace" );
		}

		for ( int j = 0; j < i; j++ )
		{
			mAlloc->free( ptrs[ j ] );
			ptrs[ j ] = NULL;
		}

		if ( freespace != mAlloc->getFreeSpace() )
		{
			TestFailed( "Not all items freed (%d %d)", 
				freespace, mAlloc->getFreeSpace() );					
		}
		
		LOG_NOTICE( "freespace %d", mAlloc->getFreeSpace() );		
		i = 0;
		
		while ( ( ptrs[ i ] = mAlloc->alloc( 32 ) ) != NULL && i < 1024 )
		{
			i++;
		}

		if ( i == 1024 )
		{
			TestFailed( "Second too many allocations for freespace" );
		}

		for ( int j = i - 1; j >= 0; j-- )
		{
			mAlloc->free( ptrs[ j ] );
			ptrs[ j ] = NULL;
		}

		if ( freespace != mAlloc->getFreeSpace() )
		{
			TestFailed( "Not all items freed (%d %d)", 
				freespace, mAlloc->getFreeSpace() );					
		}

		LOG_NOTICE( "freespace %d", mAlloc->getFreeSpace() );		
		i = 0;
		
		delete mAlloc;
		free( data );
		
		TestPassed();
	}	
};

int main( int argc, char*argv[] )
{
	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ 10 ];
	
	test_set[ 0 ] = jh_new AllocatorTest( 1 );
	
	runner.RunAll( test_set, 1 );

	return 0;
}


