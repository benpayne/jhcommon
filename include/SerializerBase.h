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

#ifndef JH_SERIALIZERBASE_H_
#define JH_SERIALIZERBASE_H_


#include "ISerializer.h"

/**
 *	@brief Simple base Serializer
 *
 *	This class implements some simple buffer management via the
 *	constructor and destructor to allow users to derive their
 *	specific ISerializer implementations without cluttering
 *	their code with the mechanics of allocating the working
 *	buffer and freeing it.  This also provides a means for
 *	users to specify a buffer to serialize from that would
 *	not be deallocated by this base class.
 *
 *	The perk of using this base implementation is that the
 *	buffer (mWorkingBuffer) is declared mutable.  So it is
 *	possible to delay building your working buffer until
 *	the write method is called(which is a const method) if
 *	you are serializing out
 */
class SerializerBase : public ISerializer
{
public:

	/**
	 *	@brief Basic constructor
	 *
	 *	The basic constructor allocates a buffer of the specified
	 *	size and stores the size as mSize and the buffer as
	 *	mWorkingBuffer.   The buffer is marked to be freed when
	 *	this object is deleted.
	 */
	SerializerBase(uint32_t bufferSize)
	:	mWorkingBuffer(NULL), mSize(bufferSize), mFreeBuffer(false)
	{
		if (bufferSize > 0)
		{
			mFreeBuffer = true;
			mWorkingBuffer = jh_new uint8_t[bufferSize];
		}
	}
	
	/**
	 *	@brief Pre-allocated constructor
	 *
	 *	The pre-allocated constructor allows the user to specify
	 *	the buffer and size.  The buffer is stored as mWorkingBuffer
	 *	and the size as mSize.   When the SerializerBase destructor
	 *	runs this buffer WILL NOT be deleted
	 */
	SerializerBase(uint8_t *buffer, uint32_t bufferSize)
	:	mWorkingBuffer(buffer), mSize(bufferSize),
		mFreeBuffer(false)
	{
	}
	
	/**
	 *	@brief Destructor
	 *
	 *	Delete mWorkingBuffer if we were created with the BasicConstructor
	 *	otherwise do nothing
	 */
	virtual ~SerializerBase()
	{
		if (mFreeBuffer == true and mWorkingBuffer != NULL)
		{
			delete [] mWorkingBuffer;
			mWorkingBuffer = NULL;
		}
	}
	
protected:
	mutable uint8_t *mWorkingBuffer;
	uint32_t mSize;
	bool mFreeBuffer;
};


#endif // JH_SERIALIZERBASE_H_

