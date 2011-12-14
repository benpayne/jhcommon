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

#include "Condition.h"

#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );


//! Initialize a new condition variable (see pthread_cond_init(3))
Condition::Condition()
{
	pthread_cond_init( &mCond, NULL );
}

//! Cleanup the condition variable we were wrapping
Condition::~Condition()
{
	int err = pthread_cond_destroy( &mCond );
   
	if (err != 0)
	{
		if (err == EINVAL)
			LOG_ERR_FATAL("pthread_cond_destroy() failed with EINVAL");
		else if (err == EBUSY)
			LOG_ERR_FATAL("pthread_cond_destroy() failed with EBUSY");
		else
			LOG_ERR_FATAL("pthread_cond_destroy() failed with %d", err);
	}
}

/**
 * This method will block the calling thread, waiting for the
 * condition to be signalled.
 *
 * @note The Mutex MUST be locked when this method is called, and
 * WILL be locked when this method returns, otherwise the results
 * are undefined (Deadlocks most likely)
 */
void Condition::Wait( Mutex &mutex )
{
	pthread_cond_wait( &mCond, &mutex.mMutex );
}

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
bool Condition::Wait( Mutex &mutex, uint32_t timeoutms )
{
	if ( timeoutms > 0 )
	{
		struct timespec timeout;
		TimeUtils::getCurTime( &timeout );
		TimeUtils::addOffset( &timeout, timeoutms );
		if ( pthread_cond_timedwait( &mCond, &mutex.mMutex, &timeout ) != 0 )
			return false;
	}
	else
		Wait( mutex );
	
	return true;
}

/**
 * This method signals the condition, waking up one and only one
 * waiting thread.  The thread to be awoken is not specified.  If
 * no threads are waiting on the Condition then this will do
 * nothing.
 */
void Condition::Signal()
{
	pthread_cond_signal( &mCond );
}

/**
 * This method signals the condition, waking up all waiting
 * threads.  If no threads are waiting on the Condition then this
 * will do nothing.
 */
void Condition::Broadcast()
{
	pthread_cond_broadcast( &mCond );
}
