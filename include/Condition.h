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

#ifndef _JH_CONDITION_H_
#define _JH_CONDITION_H_

#include <pthread.h>
#include <assert.h>
#include <sys/time.h>
#include "jh_types.h"
#include "Mutex.h"
#include "TimeUtils.h"

class Condition
{
public:
	//! Initialize a new condition variable (see pthread_cond_init(3))
	Condition();
	
	//! Cleanup the condition variable we were wrapping
	~Condition();
	
	/**
	 * This method will block the calling thread, waiting for the
	 * condition to be signalled.
	 *
	 * @note The Mutex MUST be locked when this method is called, and
	 * WILL be locked when this method returns, otherwise the results
	 * are undefined (Deadlocks most likely)
	 */
	void Wait( Mutex &mutex );
	
	/**
	 * This method will block the calling thread, waiting for the
	 * condition to be signalled or for a specified timeout to occur.
	 *
	 * @note The Mutex MUST be locked when this method is called, and
	 * WILL be locked when this method returns, otherwise the results
	 * are undefined (Deadlocks most likely)  
	 *
	 * @return true if the condition was signaled, and false if the 
	 * timeout fired.
	 */
	bool Wait( Mutex &mutex, uint32_t timeoutms );
	
	/**
	 * This method signals the condition, waking up one and only one
	 * waiting thread.  The thread to be awoken is not specified.  If
	 * no threads are waiting on the Condition then this will do
	 * nothing.
	 */
	void Signal();
	
	/**
	 * This method signals the condition, waking up all waiting
	 * threads.  If no threads are waiting on the Condition then this
	 * will do nothing.
	 */
	void Broadcast();
	
private:
	//! The condition variable we are wrapping
	pthread_cond_t		mCond;
};

#endif // _JH_CONDITION_H_
