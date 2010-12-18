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
#include "TimeUtils.h"
#include "FdReaderWriter.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

using namespace JetHead;

CircularBuffer::CircularBuffer( uint8_t *buffer, int buf_size )
	: mLock( true ), mFreeBuffer( false )
{
	mBuffer = buffer;
	mReadPtr = mBuffer;
	mWritePtr = mBuffer;
	mBufferSize = buf_size;
	mEnd = mBuffer + buf_size;
	mLen = 0;
}

CircularBuffer::CircularBuffer( int buf_size )
	: mLock( true ), mFreeBuffer( true )
{
	mBuffer = jh_new uint8_t[ buf_size ];
	mReadPtr = mBuffer;
	mWritePtr = mBuffer;
	mBufferSize = buf_size;
	mEnd = mBuffer + buf_size;
	mLen = 0;
}

CircularBuffer::~CircularBuffer()
{
	if ( mFreeBuffer )
		delete [] mBuffer;
}
	
int CircularBuffer::write_overflow( uint8_t *buffer, int size )
{
	TRACE_BEGIN( LOG_LVL_NOISE );

	int offset = 0;
	
	// If the size specified is larger than the total buffer size
	// then adjust the size we are going to write and where we're going
	// to write from
	if ( size > mBufferSize )
	{
		offset  = size - mBufferSize;
		size = mBufferSize;
	}
	
	AutoLock lock( mLock );
	
	// If the number of bytes we are trying to write is greater than
	// the amount of free space then clear the difference before writing
	// this is the "overflow" mechanism.
	int read_size = size - getFreeSpace();
	if ( read_size > 0 )
	{
		int __attribute__((unused)) res = read( NULL, read_size );
		LOG( "overflow reading %d bytes (read %d)", read_size, res );
	}
	
	return write_internal( buffer + offset, size ) + offset;
}

int CircularBuffer::write( const uint8_t *buffer, int size )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	AutoLock lock( mLock );
	return write_internal( buffer, size );
}

int CircularBuffer::write_internal( const uint8_t *buffer, int size )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	if ( buffer == NULL )
		return 0;
	
	// limit size to free space, we will return this for the size of
	// this write
	if ( size > getFreeSpace() )
		size = getFreeSpace();

	// if size is zero, return early
	if ( size == 0 )
		return 0;
	
	// first, copy from the write pointer up to the end of the buffer,
	// note that this truncates implicitly to no more than free space
	int tail_size = mEnd - mWritePtr;

	// truncate tail_size to the requested write size, note that size
	// has already been truncated to no more than free space
	if ( tail_size > size )
		tail_size = size;

	bufferCopy( mWritePtr, buffer, tail_size );
	mWritePtr += tail_size;

	// if the write pointer is now at the end of the buffer, then
	// reset it to the beginning of the buffer
	if ( mWritePtr == mEnd )
		mWritePtr = mBuffer;

	// if tail_size is less than size, then we haven't filled free
	// space up to the write request size, so continue at the begining
	// of the buffer
	if ( tail_size < size )
	{
		// compute the remainder of the size bytes to write, which we
		// have already truncated to available free space
		int head_size = size - tail_size;
		
		// copy head_size bytes into the begining of the circular
		// buffer from offset tail_size within our input buffer
		if (buffer != NULL)
		{
			bufferCopy( mWritePtr, buffer + tail_size, head_size );
		}
		mWritePtr += head_size;
	}
	
	// update the space now in use in the buffer
	add_length_internal( size );
	
	// signal all waiters for data.
	mDataCondition.Broadcast();

	LOG( "wp %p rd %p", mWritePtr, mReadPtr );

	return size;
}

int CircularBuffer::read( uint8_t *buffer, int size )
{
	TRACE_BEGIN( LOG_LVL_NOISE );

	AutoLock lock( mLock );

	// limit size to data available, we will return this for the size of
	// this read
	if ( size > length_internal() )
		size = length_internal();

	// if size is zero, return early
	if ( size == 0 )
		return 0;
	
	// first, prepare to copy data up to the end of the buffer
	int tail_size = mEnd - mReadPtr;

	// truncate tail_size to the requested read size, note that size
	// has already been truncated to no more than data available
	if ( tail_size > size )
		tail_size = size;
	
	bufferCopy( buffer, mReadPtr, tail_size );
	mReadPtr += tail_size;

	// if tail_size is less than size, then continue at the begining
	// of the buffer
	if ( tail_size < size )
	{
		// compute the remainder of the size bytes to read, which we
		// have already truncated to available data
		int head_size = size - tail_size;
		
		// copy head_size bytes to offset tail_size within our output
		// buffer from the begining of the circular buffer
		if (buffer != NULL)
		{
			bufferCopy( buffer + tail_size, mBuffer, head_size );
		}
		mReadPtr = mBuffer + head_size;
	}
	
	// update the space now in use in the buffer
	subtract_length_internal( size );

	// signal any waiters for free space.
	mFreeCondition.Broadcast();

	LOG( "wp %p rd %p", mWritePtr, mReadPtr );
	return size;
}

int CircularBuffer::copy( uint8_t *buffer, int size ) const
{
	TRACE_BEGIN( LOG_LVL_NOISE );

	AutoLock lock( mLock );

	// limit size to data available, we will return this for the size
	// of this copy
	if ( size > length_internal() )
		size = length_internal();
	
	int start_size = size;
	const uint8_t *src = getBytesInternal( 0, start_size );
	
	// truncate start_size to the requested read size, note that size
	// has already been truncated to no more than data available
	if ( start_size > size )
		start_size = size;
	
	memcpy( buffer, src, start_size );
	
	if ( start_size < size )
	{
		int end_size = size - start_size;
		int temp_size;
		src = getBytesInternal( start_size, temp_size );
		memcpy( buffer + start_size, src, end_size );
	}
	
	return size;
}


int CircularBuffer::fillFromFile( IReaderWriter *reader, int size )
{
	int freespace = getFreeSpace();
	int res = 0;
	
	AutoLock lock( mLock );

	if ( size > freespace )
		size = freespace;

	if ( size == 0 )
		return 0;
	
	LOG_NOISE( "reading %d", size );
	
	// if buffer wraps around.
	if ( mReadPtr > mWritePtr )
	{
		res = reader->read( mWritePtr, size );
		if (res < 0)
			return res;
		
		mWritePtr += res;
	}
	// else buffer doesn't wrap
	else
	{
		int read_size = size;
		
		if ( size > mEnd - mWritePtr )
			read_size = mEnd - mWritePtr;
		
		res = reader->read( mWritePtr, read_size );
		if (res < 0)
			return res;
		
		mWritePtr += res;
		
		if ( mWritePtr == mEnd )
			mWritePtr = mBuffer;
	}

	// update the space now in use in the buffer	
	add_length_internal(res);

	LOG_NOISE( "read %d", res );
	mDataCondition.Broadcast();

	return res;
}

int CircularBuffer::fillFromFile( int fd, int size )
{
	FdReaderWriter readerWriter( fd );
	
	return fillFromFile( &readerWriter, size );
}

int CircularBuffer::waitForData( int threshold, uint32_t msecs )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	uint32_t timeout = msecs;	
	struct timespec start, cur;
	TimeUtils::getCurTime( &start );
	
	LOG_NOISE( "Waiting for data %d cur size %d timeout %d", threshold, length_internal(), msecs );

	AutoLock lock( mLock );
	
	if ( length_internal() < threshold )
		LOG_INFO( "buffer underflow" );
	
	while ( length_internal() < threshold && mDataCondition.Wait( mLock, timeout ) )
	{
		TimeUtils::getCurTime( &cur );
		timeout = msecs - TimeUtils::getDifference( &cur, &start );
		
		LOG_INFO( "Wating for data %d cur size %d msecs remaining %d", threshold, length_internal(), timeout );
	}

	if ( length_internal() >= threshold )
		return length_internal();
	else
		return 0;
}

int CircularBuffer::waitForFreeSpace( int threshold, uint32_t msecs )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	uint32_t timeout = msecs;	
	struct timespec start, cur;
	TimeUtils::getCurTime( &start );
	
	LOG( "Wating for data %d cur size %d timeout %d", threshold, length_internal(), msecs );

	AutoLock lock( mLock );
	
	if ( mBufferSize - length_internal() < threshold )
		LOG_INFO( "buffer underflow" );
	
	while ( mBufferSize - length_internal() < threshold && mFreeCondition.Wait( mLock, timeout ) )
	{
		TimeUtils::getCurTime( &cur );
		timeout = msecs - TimeUtils::getDifference( &cur, &start );
		
		LOG( "Wating for data %d cur size %d msecs remaining %d", threshold, length_internal(), timeout );
	}

	if ( mBufferSize - length_internal() >= threshold )
		return mBufferSize - length_internal();
	else
		return 0;
}

void CircularBuffer::clear()
{
	AutoLock lock( mLock );

	mWritePtr = mReadPtr = mBuffer;
	mLen = 0;
	
	mFreeCondition.Broadcast();
}

const uint8_t *CircularBuffer::getBytes( int i, int &size ) const
{
	AutoLock lock( mLock );

	return getBytesInternal( i, size );
}

const uint8_t *CircularBuffer::getBytesInternal( int i, int &size ) const
{
	uint8_t *res = NULL;

	if ( i < length_internal() )
	{
		res = mReadPtr + i;
		size = mWritePtr - res;

		// if buffer wraps around.
		if ( mReadPtr >= mWritePtr )
		{
			if ( res >= mEnd )
			{
				res = mBuffer + (res - mEnd);
				size = mWritePtr - res;
			}
			else
				size = mEnd - res;
		}
	}
	else
		size = 0;

	assert( size >= 0 );
	
	return res;
}

int	CircularBuffer::byteAt( int i ) const
{
	int size;
	const uint8_t *res = getBytes( i, size );

	if ( res == NULL )
		return -1;
	else
		return *res;
}

int CircularBuffer::writeByte( uint8_t byte )
{
	AutoLock lock( mLock );

	if (getFreeSpace() == 0)
		return -1;

	*mWritePtr = byte;
	mWritePtr++;
	add_length_internal(1);

	if ( mWritePtr == mEnd )
		mWritePtr = mBuffer;
	
	mDataCondition.Broadcast();

	return 0;
}

int CircularBuffer::getLine( JHSTD::string &line, const char *terminal )
{
	int res = 0;
	Lock();
	res = read( NULL, peekLine( line, terminal ) );
	Unlock();
	return res;
}

int CircularBuffer::peekLine( JHSTD::string &line, const char *terminal )
{
	AutoLock lock( mLock );
	int term_len = strlen( terminal );
	int state = 0;
	uint8_t *ptr = mReadPtr;
	bool found = false;
	
	if ( mLen == 0 ) 
		return 0;
	
	do {
		if ( *ptr == terminal[ state ] )
			state += 1;
		else
			state = 0;
	
		if ( state == term_len )
		{
			found = true;
			break;
		}
		
		ptr += 1;
		if ( ptr == mEnd )
			ptr = mBuffer;
	} while( ptr != mWritePtr );
	
	if ( found )
	{
		// move passed the terminals last char, don't care about wrapping here.
		ptr += 1;

		// if wrap
		if ( ptr < mReadPtr )
		{
			line.assign( (char*)mReadPtr, mEnd - mReadPtr );
			line.append( (char*)mBuffer, ptr - mBuffer - term_len );
		}
		else
			line.assign( (char*)mReadPtr, ptr - mReadPtr - term_len );

		return line.length() + term_len;
	}

	return 0;
}

void CircularBuffer::bufferCopy( uint8_t *dest, const uint8_t *src, int size )
{
	if ( src == NULL || size <= 0 || dest == NULL || src == dest )
		return;
	else
		memcpy( dest, src, size );
}

int CircularBuffer::getFreeSpace() const
{
	AutoLock lock( mLock );
	
	return mBufferSize - length_internal();
}

int CircularBuffer::getLength() const
{	
	AutoLock lock( mLock );
	
	return length_internal();
}

int CircularBuffer::getSize() const
{	
	AutoLock lock( mLock );
	
	return mBufferSize;
}

int CircularBuffer::length_internal() const
{
	return mLen;
}

void CircularBuffer::add_length_internal( unsigned int size )
{
	if ( size > mBufferSize - mLen )
	{
		LOG_ERR_FATAL("adding size: %d to mLen: %d "
					  "would exceed mBufferSize: %d",
					  size, mLen, mBufferSize );
	}
	
	mLen += size;

	// If we were in the empty condition, update the read pointer to
	// its new beginning-of-buffer position.
	if (mReadPtr == mEnd)
	{
		mReadPtr = mBuffer;
	}
}

void CircularBuffer::subtract_length_internal( unsigned int size )
{
	if ( size > mLen )
	{
		LOG_ERR_FATAL("subtracting size: %d from mLen: %d would be negative",
					  size, mLen);
	}
	
	mLen -= size;

	// If we were in the full condition, update the write pointer to
	// its new beginning-of-buffer position.
	if (mWritePtr == mEnd)
	{
		mWritePtr = mBuffer;
	}
}
