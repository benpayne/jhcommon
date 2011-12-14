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

#ifndef JH_CODETIMER_H_
#define JH_CODETIMER_H_

#include "jh_types.h"
/**
 *	@brief Provide simple code timing mechanism
 *
 *	This header file defines code timing mechanisms for C++, and C.
 *	In C the library supports either kernel or non-kernel timing.
 *
 *	The general usage is as followed:
 *		- initialize timer with a number of iterations between prints
 *		- start call made before the piece of code you are timing
 *		- stop call made after the piece of code you are timing
 *	When stop is called the time difference is accumulated.  If
 *	the number of iterations is hit then the avearge is printed out
 */

#ifndef __KERNEL__

// For non-kernel timing we support either using gettimeofday or
// clock_gettime.   In this case we need to declare some appropriate
// macros
#define CT_ONE_MILLION (1000000)

#ifdef JH_CODETIMER_USE_GETTIMEOFDAY

#include <sys/time.h>
typedef struct timeval	ct_timer_t;
#define GET_TIME(x)		gettimeofday((x), NULL)
#define CALC_USECS(x)	(((x).tv_sec * CT_ONE_MILLION) + (x).tv_usec)

#else // !JH_CODETIMER_USE_GETTIMEOFDAY

#include <time.h>
typedef struct timespec ct_timer_t;
#define GET_TIME(x)		clock_gettime(CLOCK_REALTIME, (x))
#define CALC_USECS(x)	(((x).tv_sec * CT_ONE_MILLION) + ((x).tv_nsec / 1000))

#endif // JH_CODTIMER_USE_GETTIMEOFDAY

#endif // !__KERNEL__


// Define C++ CodeTimer class
#ifdef __cplusplus

class CodeTimer
{
public:
	CodeTimer( const char *name, int interval ) : mName( name ), 
		mInterval( interval ), mCount( 0 ), mTotal( 0 ), mAverage( 0 )
	{
		if (mInterval == 0)
			mInterval = 1;
	}
	~CodeTimer() {}

	void Start()
	{
		GET_TIME(  &mStart );
	}

	uint32_t End()
	{
		ct_timer_t et;
		uint64_t micro_s, micro_e;
	
		mCount++;
		GET_TIME( &et );
		micro_s = CALC_USECS( mStart );
		micro_e = CALC_USECS( et );
		mTotal+=(micro_e-micro_s);
	
		if (  mCount % mInterval == 0 )
		{
			mAverage = mTotal / mInterval;
			if ( mName != NULL )
				printf( "CodeTimer( %s ) average %d usecs or %d.%06d secs\n",
					mName, mAverage,
					(mAverage / CT_ONE_MILLION),
					(mAverage % CT_ONE_MILLION) );
			mTotal = 0;
		}
		return mAverage;
	}

	uint32_t getElapsedTime()
	{
		ct_timer_t et;
		uint64_t micro_s, micro_e;
	
		GET_TIME( &et );
		micro_s = CALC_USECS( mStart );
		micro_e = CALC_USECS( et );
		return (micro_e-micro_s);
	}

	void reset() { mCount = 0; mTotal = 0; mAverage = 0; }
	
	uint32_t getAverage() { return mAverage; }
	
private:
	const char *mName;
	int mInterval;
	int mCount;
	uint64_t mTotal;
	uint32_t mAverage;
	ct_timer_t mStart;
};


class ScopeTimer
{
public:
	ScopeTimer(const char *name)
	:	mTimer(name, 1)
	{
		mTimer.Start();
	}
	~ScopeTimer()
	{
		mTimer.End();
	}
	
private:
	CodeTimer mTimer;
};


#else // !__cplusplus

// Define code_timer structure for use in kernel development
#ifdef __KERNEL__

#include <asm/timex.h>

typedef struct
{
	cycles_t start_time;
	int count;
	int sum;
	int print_count;
	int max;
	int min;
	const char *name;
} code_timer;

static inline void code_timer_init( code_timer *this, const char *name, int print_count )
{
	this->print_count = print_count;
	this->count = 0;
	this->sum = 0;
	this->name = name;
	this->max = 0;
	this->min = 0;
}

static inline void code_timer_start( code_timer *this )
{
	this->start_time = get_cycles();
}

static inline void code_timer_stop( code_timer *this )
{
	int diff = get_cycles() - this->start_time;
	if (this->max == 0)
		this->max = diff;
	else if (this->max < diff)
		this->max = diff;
	
	if (this->min == 0)
		this->min = diff;
	else if (this->min > diff)
		this->min = diff;
	
	this->sum += diff;
//	this->sum += get_cycles() - this->start_time;
	this->count++;

	if ( this->count >= this->print_count )
	{
		printk( "CodeTimer( %s ), average cycles %d, max %d, min %d\n",
				this->name, this->sum / this->count, this->max, this->min );

		this->count = 0;
		this->sum = 0;
		this->max = 0;
		this->min = 0;
	}
}

#else // !__KERNEL__

// Define code_timer structure for C development in user space
typedef struct
{
	const char *name;
	int interval;
	int count;
	uint64_t total;
	uint32_t average;
	int32_t  min;
	int32_t  max;
	ct_timer_t start;
} code_timer;

static inline void code_timer_init(code_timer *this,
								   const char *name,
								   int interval)
{
	this->name = name;
	if (interval == 0)
		this->interval = 1;
	else
		this->interval = interval;
	this->count = 0;
	this->total = 0;
	this->average = 0;
	this->max = 0;
	this->min = 0;
}

static inline void code_timer_start(code_timer *this)
{
	GET_TIME(&this->start);
}

static inline void code_timer_reset(code_timer *this)
{
	this->count = 0;
	this->total = 0;
	this->average = 0;
	this->min = 0;
	this->max = 0;
}

static inline void code_timer_end(code_timer *this)
{
	ct_timer_t end;
	uint64_t micro_s, micro_e;
	int32_t diff;
	
	GET_TIME(&end);
	micro_s = CALC_USECS(this->start);
	micro_e = CALC_USECS(end);
	diff = micro_e - micro_s;
	this->total += diff;

	if (this->max == 0)
		this->max = diff;
	else if (this->max < diff)
		this->max = diff;
	
	if (this->min == 0)
		this->min = diff;
	else if (this->min > diff)
		this->min = diff;
	
	++this->count;
	if (this->count >= this->interval)
	{
		this->average = this->total / this->interval;
		if (this->name != NULL)
		{
			printf("CodeTimer(%s) average %d usecs(%d.%06d secs) min %d usec, max %d usec\n",
				   this->name,
				   this->average,
				   this->average / CT_ONE_MILLION,
				   this->average % CT_ONE_MILLION,
				   this->min,
				   this->max);
		}
		code_timer_reset(this);
	}
}

#endif // !__KERNEL__

#endif // !__cplusplus


#endif // JH_CODETIMER_H_

