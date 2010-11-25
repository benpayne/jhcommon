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

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

/**
 * \file
 *
 * A circular buffer data structure.  Can be used to manage an
 * existing buffer, or allocate one on its own.  
 */

#include "jh_types.h"

#include "Mutex.h"
#include "Condition.h"
#include "File.h"

class CircularBuffer
{
public:
	//! Manage reads/writes to the given buffer
	CircularBuffer( uint8_t *buffer, int buf_size );

	//! Allocate a new buffer of the given size
	CircularBuffer( int buf_size );

	//! Clean up allocated space, close up shop
	~CircularBuffer();
	
	/**
	 * write data to the CircularBuffer, we write more data than there is free
	 *  space then we will discard the oldest data in the buffer and write all
	 *  bytes from the input buffer.  If the input buffer is bigger than the 
	 *  CircularBuffer the results are not garenteed.
	 */ 
	int write_overflow( uint8_t *buffer, int size );

	/**
	 * write data to the CircularBuffer, we write more data than there is free
	 *  space then the write will not transfer all bytes from the input buffer.
	 */ 
	int write( const uint8_t *buffer, int size );

	/**
	 * Read bytes from the CircularBuffer and remove them from the
	 * CircularBuffer.  If buffer is NULL, consume the data without
	 * copying it anywhere.
	 */ 
	int read( uint8_t *buffer, int size );

	/**
	 * Copy bytes from the CircularBuffer leaving them in the buffer.
	 */
	int copy( uint8_t *buffer, int size ) const;
	
	/**
	 * Read data from a file into the CircularBuffer.  This will not overflow
	 *  the CircularBuffer.
	 */
	int fillFromFile( int fd, int size );

	/**
	 * Read data from a IReaderWriter into the CircularBuffer.  This will not 
	 *  overflow the CircularBuffer.
	 */
	int fillFromFile( IReaderWriter *reader, int size );

	/**
	 * Drop all data from the buffer.
	 */
	void clear();

	/**
	 * Wait for a specified amount of data to arrive into the buffer.  Will
	 *  timeout after a specified time.  Time may be set to -1 to wait forever.
	 */
	int waitForData( int threshold, uint32_t msecs );

	/**
	 * Wait for a specificed amount of free space in the buffer.  Will timeout
	 *  after a specified time.  Time may be set to 0 to wait forever.
	 */
	int waitForFreeSpace( int threshold, uint32_t msecs );
	
	/**
	 * Get the byte at a specified offset into the buffer.
	 */
	int	byteAt( int i ) const;

	/**
	 * Get a pointer to the data at an offset into the buffer.  Upon returning
	 *  you will be told how many bytes following this ptr are valid.  Due to 
	 *  buffer wrapping this may be less that the length of the remaining data
	 *  in the buffer.  This does not move the read pointer.  Calling
	 *  read( NULL, size ) will move the read pointer.
	 */
	const uint8_t *getBytes( int i, int &size ) const;
	
	/**
	 * Write a byte to the end of the buffer.
	 */
	int writeByte( uint8_t byte );
	
	/**
	 * Get the amount of free space in the buffer.
	 */
	int getFreeSpace() const;

	/**
	 * Get the amount of data in the buffer.
	 */
	int getLength() const;

	/**
	 * Get the total size of the buffer.
	 */
	int getSize() const;
	
	/**
	 * Lock the buffer from beign modified.  Needed when the buffer is used in
	 *  multiple threads.
	 */
	void Lock() { mLock.Lock(); }

	/**
	 * Unlock the buffer from beign modified.  Needed when the buffer
	 * is used in multiple threads.
	 */
	void Unlock() { mLock.Unlock(); }
	
private:
	//! Error-checked memcpy
	void bufferCopy( uint8_t *dest, const uint8_t *src, int size );

	/**
	 * Return a pointer to the i'th byte, or null.  Set size to the
	 * number of bytes available from that point in the buffer.
	 * (Which might be the wrap point, or the total number of bytes.)
	 */
	const uint8_t *getBytesInternal( int i, int &size ) const;

	//! How many bytes are available?
	int length_internal() const;

	//! Add to the internal length, checking bounds.
	void add_length_internal( unsigned int size );

	//! Subtract from internal length, checking bounds.
	void subtract_length_internal( unsigned int size );
	
	//! Write some bytes into the buffer
	int write_internal( const uint8_t *buffer, int size );
	
	//! Pointer to the buffer
	uint8_t	*mBuffer;

	//! Pointer to the oldest byte in the buffer
	uint8_t	*mReadPtr;

	//! Pointer to the first unused byte in the buffer
	uint8_t	*mWritePtr;

	//! Pointer to the end of the data in the buffer
	uint8_t	*mEnd;

	//! The number of bytes used in the buffer
	uint32_t mLen;
	
	//! The size of mBuffer
	int mBufferSize;

	//! The lock we use for protecting access and making this thread safe
	mutable Mutex mLock;

	//! Readers may block on this if data is not available
	Condition mDataCondition;

	//! Writers may block on this if space is not available
	Condition mFreeCondition;

	//! Do we need to call delete[] on this buffer in the destructor?
	bool mFreeBuffer;
};

#endif // CIRCULAR_BUFFER_H_

