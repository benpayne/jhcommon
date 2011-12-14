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

#include "Thread.h"
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include "Mutex.h"
#include "Condition.h"

#include "jh_memory.h"
#include "logging.h"

#include <unistd.h>
SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

class TestClass
{
public:
	TestClass();
	virtual ~TestClass();
	
	void startFaker();
	void stopFaker();
	void waitFaker();
	
	void startTicker();
	void stopTicker();
	
	void waitTime();
private:
	void ticker();
	void faker();


	sigset_t			mSigSet;
	sighandler_t		mOldHandler;
	Runnable<TestClass> *mTicker;
	Runnable<TestClass> *mFaker;
	
	Mutex mMutex;
	Condition mWait;
};


TestClass *testClass = NULL;

TestClass::TestClass()
:	mTicker(NULL), mFaker(NULL)
{
	TRACE_BEGIN(LOG_LVL_NOTICE);

	sigemptyset(&mSigSet);
	sigaddset(&mSigSet, SIGALRM);
//	pthread_sigmask(SIG_BLOCK, &mSigSet, 0);
}

TestClass::~TestClass()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	stopTicker();
}

void TestClass::startFaker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	if (mFaker == NULL)
	{
		mFaker = jh_new Runnable<TestClass>("faker", this, &TestClass::faker);
		mFaker->Start();
	}
}

void TestClass::stopFaker()
{
	if (mFaker != NULL)
	{
		mFaker->Stop();
		mWait.Signal();
	}
}

void TestClass::waitFaker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	if (mFaker != NULL)
	{
		mFaker->Join();
		delete mFaker;
		mFaker = NULL;
	}
}

void TestClass::faker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	while (!mFaker->CheckStop())
	{
		mMutex.Lock();
		mWait.Wait(mMutex);
		mMutex.Unlock();
	}
}

void TestClass::startTicker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	if (mTicker == NULL)
	{
		mTicker = jh_new Runnable<TestClass>("ticker", this, &TestClass::ticker);
		mTicker->Start();
	}
}

void TestClass::stopTicker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);

	if (mTicker != NULL)
	{
		mTicker->Stop();
		mTicker->Join();
		delete mTicker;
		mTicker = NULL;
	}
}

void TestClass::ticker()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	int signal;
	int tick = 0;
	
	struct itimerval timeout;
	timeout.it_interval.tv_sec = 1;
	timeout.it_interval.tv_usec = 0;//100*1000;
	timeout.it_value.tv_sec = 1;
	timeout.it_value.tv_usec = 0;//100*1000;
	
	setitimer(ITIMER_REAL, &timeout, NULL);
	while (!mTicker->CheckStop())
	{
		printf("Waiting...\n");
		if (sigwait(&mSigSet, &signal) != 0)
		{
			LOG_ERR_FATAL("sigwait failed");
		}
		else
		{
			tick++;
			LOG("Tick %d", tick);
		}
	}
}

sighandler_t oldHandler;

void gotTimeout(int sig)
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	LOG_WARN("???? DID I GET HERE????");
	
	signal(SIGALRM, oldHandler);
	testClass->stopFaker();
}

void TestClass::waitTime()
{
	TRACE_BEGIN(LOG_LVL_NOTICE);
	
	struct itimerval timeout;
	
	timeout.it_interval.tv_sec = 0;
	timeout.it_interval.tv_usec = 0;
	timeout.it_value.tv_sec = 3;
	timeout.it_value.tv_usec = 0;
	
//	siginterrupt(SIGALRM, 1);
	oldHandler = signal(SIGALRM, gotTimeout);
	setitimer(ITIMER_REAL, &timeout, NULL);
	
	if (oldHandler == SIG_DFL)
		LOG_NOTICE("Old handler is SIG_DFL");
	else if (oldHandler == SIG_ERR)
		LOG_NOTICE("Old handler is SIG_ERR");
	else if (oldHandler == SIG_IGN)
		LOG_NOTICE("Old handler is SIG_IGN");
	else
		LOG_NOTICE("Old handler is %p", oldHandler);
	
}

int main( int argc, char*argv[] )
{	
	LOG_NOTICE( "Test Started" );
	testClass = jh_new TestClass;
	
	
	LOG_NOTICE("Starting ticker...");
	testClass->startTicker();
	
	LOG_NOTICE("Setting timeout using SIGALRM for 5 seconds");
	// Start faker... faker is a bad name, but basically I am using
	// a thread as my "condition" for waiting.  I wait for faker to
	// stop then move on.  stopFaker is called when the gotTimeout
	// signal handler runs.
	testClass->startFaker();
	testClass->waitTime();

	testClass->waitFaker();

	
	LOG_NOTICE("Stopping ticker...");
//	testClass->stopTicker();
	
	LOG_NOTICE("Setting timeout using SIGALRM for 5 seconds");
	testClass->startFaker();
	
	sleep(5);
	testClass->waitTime();
	
	
	LOG_NOTICE("Starting ticker...");
//	testClass->startTicker();

	testClass->waitFaker();
	
	LOG_NOTICE("DONE");
	delete testClass;

	return 0;
}


