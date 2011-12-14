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

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "CircularBuffer.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_DEFAULT );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

#include <assert.h>

#define BUFFER_SIZE		16
#define RW_SIZE			8

int main( int argc, char *argv[] )
{
	CircularBuffer *buf = jh_new CircularBuffer( BUFFER_SIZE );
	uint8_t buffer[ RW_SIZE ];
	
	for ( int i = 0; i < RW_SIZE; i++ )
	{
		buffer[ i ] = i;
	}

	buf->write( buffer, RW_SIZE );
	for( int i = 0; i < 40; i++ )
	{
		buf->write( buffer, RW_SIZE );
		buf->copy( buffer, RW_SIZE );
		for( int j = 0; j < RW_SIZE; j++ )
		{
			assert( buffer[ j ] == j );
		}
		buf->read( buffer, RW_SIZE );
		for( int j = 0; j < RW_SIZE; j++ )
		{
			assert( buffer[ j ] == j );
		}
	}
	
#if 1
	buf->copy( buffer, RW_SIZE );
	for( int j = 0; j < RW_SIZE; j++ )
	{
		assert( buffer[ j ] == j );
	}
	buf->read( buffer, RW_SIZE );
	for( int j = 0; j < RW_SIZE; j++ )
	{
		assert( buffer[ j ] == j );
	}
#endif

	return 0;
}


