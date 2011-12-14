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

#ifndef JH_TIMEUTILS_H_
#define JH_TIMEUTILS_H_

#include <stdint.h>
#include <sys/time.h>

namespace TimeUtils
{
	inline int getDifference( struct timeval const *t1, struct timeval const *t2 )
	{
		int ms = ( t1->tv_sec - t2->tv_sec ) * 1000;
		ms += ( t1->tv_usec - t2->tv_usec ) / 1000;
		return ms;
	}
	
	inline int getDifference( struct timespec const *t1, struct timespec const *t2 )
	{
		int ms = ( t1->tv_sec - t2->tv_sec ) * 1000;
		ms += ( t1->tv_nsec - t2->tv_nsec ) / 1000000;
		return ms;
	}

	inline int getDifferenceMicroSecs( struct timeval *t1, struct timeval *t2)
	{
		int us = ( t1->tv_sec - t2->tv_sec ) * 1000000;
		us += (t1->tv_usec - t2->tv_usec);
		return us;
	}

	inline int getDifferenceMicroSecs( struct timespec *t1, struct timespec *t2)
	{
		int us = ( t1->tv_sec - t2->tv_sec ) * 1000000;
		us += ( t1->tv_nsec - t2->tv_nsec ) / 1000;
		return us;
	}

	inline void getCurTime( struct timeval *t )
	{
		gettimeofday( t, NULL );
	}

	inline void getCurTime( struct timespec *t )
	{
#ifdef HAS_CLOCK_GETTIME
		clock_gettime(CLOCK_REALTIME, t);
#else
		struct timeval tv;
		gettimeofday( &tv, NULL );
		t->tv_sec = tv.tv_sec;
		t->tv_nsec = tv.tv_usec * 1000;
#endif		
	}

	inline void setTimeStruct( struct timespec *t, uint32_t msecs )
	{
		t->tv_sec = msecs / 1000;
		t->tv_nsec = (msecs % 1000) * 1000000;
	}
	
	inline void setTimeStruct( struct timeval *t, uint32_t msecs )
	{
		t->tv_sec = msecs / 1000;
		t->tv_usec = (msecs % 1000) * 1000;
	}
	
	inline void addOffset( struct timespec *t, int msecs )
	{
		t->tv_sec += msecs / 1000;
		t->tv_nsec += (msecs % 1000) * 1000000;

		if (t->tv_nsec >= 1000000000)
		{
			t->tv_sec++;
			t->tv_nsec -= 1000000000;
		}
		else if (t->tv_nsec < 0)
		{
			t->tv_sec--;
			t->tv_nsec += 1000000000;
		}
	}
	
	inline void addOffset( struct timeval *t, int msecs )
	{
		t->tv_sec += msecs / 1000;
		t->tv_usec += (msecs % 1000) * 1000;

		if (t->tv_usec >= 1000000)
		{
			t->tv_sec++;
			t->tv_usec -= 1000000;
		}
		else if (t->tv_usec < 0)
		{
			t->tv_sec--;
			t->tv_usec += 1000000;
		}
	}
};

#endif // JH_TIMEUTILS_H_
