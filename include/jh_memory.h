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

#ifndef __JH_MEMORY_H__
#define __JH_MEMORY_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
//#include <features.h> non-posix header

__BEGIN_DECLS

void *jh_alloc( size_t size, const char *file, int line );
void jh_free( void *ptr, const char *file, int line );
/**
 *  Set mTrace File
 * 
 *  Description:
 *  	This function turns off/on mTracing by passing in
 *  	a filename to start the mTracing, or passing it
 *  	NULL to stop tracing.  The file will go in your
 *  	current working directory unless you give a full
 *  	path with the filename.  If you pass the same
 *  	filename twice, your call will be basically ignored,
 *  	but I will flush buffers and make sure all trace
 *  	data is written to disk.
 *
 *  	@param lfilename	filename of mTrace File
 *
 *  	@author	Mat Mrosko
 *  	@date March 2006
 */
void setTraceFile( const char *lfilename );

__END_DECLS

#define jh_new new ( __FILE__, __LINE__ )

#define jhmalloc(x)		jh_alloc(x, __FILE__, __LINE__)
#define jhfree(x) 		jh_free(x, __FILE__, __LINE__ )

#ifdef __cplusplus

#include <new>
#include "List++.h"
#include <stdlib.h>
#include <assert.h>

class GCHeap
{
public:
	GCHeap();
	~GCHeap();

	struct ObjInfo : public List::Node
	{
		uint32_t magic;
		mutable int refCnt;
		void *ptr;
		uint32_t size;
		GCHeap *heap;
		const char *file;
		int line;
		//char thread_name[ Thread::kThreadNameLen ];
		
		void AddRef() const
		{
			++refCnt; 
		}

		void Release() const
		{
			--refCnt;		
			assert( refCnt >= 0 );
		}
	};
	
	ObjInfo *alloc( size_t size, const char *file, int line );
	void free( ObjInfo *info );

	/**
	 * Print a list of all current allocations.
	 */
	void dumpList();
	
	/** 
	 * Find the object for a pointer, we look for any objects that contains
	 *  this ptr.  Or ptr doesn't have to equal the pointer that was allocated.
	 *  This comes in handy when you have multiple inheritance and the pointer
	 *  to the object allocated and the pointer to the base class don't always
	 *  equal each other.
	 */
	ObjInfo *find( void *ptr );
	
	/**
	 * Simply convert the pointer into an ObjInfo assuming ptr is equal to the 
	 *  pointer allocated.
	 */
	static ObjInfo *quick_find( void *ptr );

	/**
	 * Ensure the default heap is initialized
	 */
	static inline void InitCheck() 
	{
		if (defaultHeap == NULL)
		{
			void *ptr = malloc( sizeof( GCHeap ) );
			defaultHeap = new (ptr) GCHeap;
		}		
	}
	
	static GCHeap *defaultHeap;
	
private:
	TypedList<ObjInfo> mObjects;
	int mCurrentSize;
	int mMaxSize;
	int mNumTotalAllocations;
	int mNumCurrentAllocations;
};

void *operator new( size_t size ) throw (std::bad_alloc);
void *operator new[]( size_t size ) throw (std::bad_alloc);
void *operator new( size_t size, const char *file, int line );
void *operator new[]( size_t size, const char *file, int line );
void operator delete( void *p ) throw ();
void operator delete[]( void *p ) throw ();
void operator delete( void *p, const char *file, int line );
void operator delete[]( void *p, const char *file, int line );

#endif // __cplusplus

#endif // __JH_MEMORY_H__
