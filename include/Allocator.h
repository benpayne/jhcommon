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

#ifndef JH_ALLOCATOR_H_
#define JH_ALLOCATOR_H_

/**
 * a simple memory allocator.  This can allocate chunks of arbitrary size.
 *  Beaware this algorithm will not do anything to prevent heap fragmentation.  
 *  for many application this should not be an issue but in some cases it may 
 *  be a problem.  Also note that this class is not thread safe.  It is 
 *  expected that thread safety would be handled by the user of this class. 
 */
class Allocator
{
public:
	Allocator( void *buffer, uint32_t size, bool doCoalesceOnFree=false );
	
	void *alloc( uint32_t size );
	void free( void *buf );
	
	uint32_t getFreeSpace();
	
private:
	int join_free_nodes();
	
	struct node_t {
		uint32_t size;
		node_t *next;
	};
	
	node_t *mFreeList;
	void *mBuffer;
	uint32_t mSize;
	//node_t *mAllocList;
	uint32_t mNumFreeNodes;
	bool mDoCoalesceOnFree;   // join nodes during free
};

void *operator new( size_t size, Allocator *alloc );
void *operator new[]( size_t size, Allocator *alloc );
void operator delete( void *p, Allocator *alloc );
void operator delete[]( void *p, Allocator *alloc );

#endif // JH_ALLOCATOR_H_
