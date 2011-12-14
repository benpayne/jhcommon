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
#include "Allocator.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

// Align all memory allocations to a size of a pointer
#define PTR_SIZE (sizeof(void*))
#define ALIGN_SIZE( size ) ( ( (size) + PTR_SIZE - 1 ) / PTR_SIZE * PTR_SIZE )
#define NODE_SIZE (ALIGN_SIZE( sizeof( node_t ) ))
#define SMALLEST_BLOCK ALIGN_SIZE( 16 )

Allocator::Allocator( void *buffer, uint32_t size, bool doCoalesceOnFree ) : 
	mBuffer( buffer ), mSize( size ), mDoCoalesceOnFree(doCoalesceOnFree)
{
	TRACE_BEGIN( LOG_LVL_NOTICE );
	node_t *head = (node_t*)mBuffer;
	head->size = size;
	head->next = NULL;
	mFreeList = head;
	mNumFreeNodes = 1;
}
	
void *Allocator::alloc( uint32_t size )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	LOG( "size %d", size );
	node_t *n = mFreeList;
	node_t *prev = NULL;
	
	size = ALIGN_SIZE( size );
	size += NODE_SIZE;
	
	// I'm very much not a fan of goto's, but I'm using this despite
	//  my own rules.  
retry:
	while ( n != NULL && n->size < size )
	{	
		prev = n;
		n = n->next;
	}
	
	if ( n == NULL )
	{
		if ( !mDoCoalesceOnFree && (join_free_nodes() > 0) )
			goto retry;
		else
			return NULL;
	}

	// is node big enough to be split or do we just use it.
	if ( n->size - size >= NODE_SIZE + SMALLEST_BLOCK )
	{
		// split the node
		n->size -= size;	
		uint8_t *ptr = (uint8_t*)n;
		ptr += n->size;
		n = (node_t*)ptr;
		n->size = size;
		n->next = NULL;
	}
	else
	{
		mNumFreeNodes--;
		// remove from free list and use node.
		if ( prev != NULL )
			prev->next = n->next;
		else
			mFreeList = n->next;	

		// leave size as is.
		n->next = NULL;
	}

	return (void*)(n + 1);
}

void Allocator::free( void *buf )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	if ( buf == NULL )
	{
		LOG_NOTICE( "Attempt to delete NULL pointer" );
		return;
	}
	
	if ( buf < mBuffer || buf >= (uint8_t*)mBuffer + mSize )
	{
		LOG_ERR( "Attempt to delete pointer that is not managed by this allocator" );
		return;
	}
	
	node_t *cur_node = (node_t*)buf - 1;
	node_t *n = mFreeList;
	node_t *prev = NULL;
	
	// Add node to the free list in a sorted location.
	//  first find the first node after this location
	//  prev will be the node before this location.
	while( n != NULL && n < cur_node )
	{
		prev = n;
		n = n->next;
	}
	
	// Insert the free'd node between prev and n
	//  if prev == NULL then insert the node at the head.
	if ( prev != NULL )
	{
		// if we are to coalesce on free, then check now to see if we can combine with
		// the prev node
		if ( mDoCoalesceOnFree && (((int)prev + (int)prev->size) == (int) cur_node ))
		{
			prev->size += cur_node->size;
			cur_node = prev;
		}
		else
		{
			mNumFreeNodes++;
			cur_node->next = prev->next;
			prev->next = cur_node;
		}
	}
	else
	{
		cur_node->next = mFreeList;
		mFreeList = cur_node;
	}

	// if we are to coalesce on free, then check now to see if we can combine with
	// the next node
	if ( mDoCoalesceOnFree && (cur_node->next != NULL) && (((uint8_t*)cur_node->next - (uint8_t*)cur_node) == (int)cur_node->size ))
	{
		cur_node->size += cur_node->next->size;
		cur_node->next = cur_node->next->next;
		mNumFreeNodes--;
	}
	LOG_NOISE("NumAllocatorFreeNodes = %u",mNumFreeNodes);
	
}

uint32_t Allocator::getFreeSpace()
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	node_t *n = mFreeList;
	uint32_t size = 0;
	
	// join free nodes so the free space number is accurate.
	// skip if we have chosen to coalesce on free
	if (!mDoCoalesceOnFree)
	{
		join_free_nodes();
	}
	
	while( n != NULL )
	{
		size += n->size;
		n = n->next;
	}
	
	return size;
}

int Allocator::join_free_nodes()
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	
	// free list empty and therefore nothing to do.
	if ( mFreeList == NULL )
		return 0;
	
	node_t *n = mFreeList;
	node_t *next = n->next;
	int join_count = 0;

	while( n != NULL && n->next != NULL )
	{
		// if this node and next are adjacent then combined together
		//  leave n as the joined node so we can check if the new 
		//  node is adjacent to the node after the one just removed.
		// otherwise just move the the next node.
		if ( (uint8_t*)n->next - (uint8_t*)n == (int)n->size )
		{
			next = n->next;
			n->size += next->size;
			n->next = next->next;
			join_count++;
			mNumFreeNodes--;
		}
		else
		{
			n = n->next;
		}
	}
	
	return join_count;
}

void *operator new( size_t size, Allocator *alloc )
{
	if ( alloc == NULL )
		LOG_ERR_FATAL( "calling new on NULL allocator" );
	
	return alloc->alloc( size );
}

void *operator new[]( size_t size, Allocator *alloc )
{
	if ( alloc == NULL )
		LOG_ERR_FATAL( "calling new on NULL allocator" );
	
	return alloc->alloc( size );
}

void operator delete( void *p, Allocator *alloc )
{
	if ( alloc == NULL )
		LOG_ERR_FATAL( "calling delete on NULL allocator" );
	
	alloc->free( p );	
}

void operator delete[]( void *p, Allocator *alloc )
{
	if ( alloc == NULL )
		LOG_ERR_FATAL( "calling delete on NULL allocator" );
	
	alloc->free( p );	
}

