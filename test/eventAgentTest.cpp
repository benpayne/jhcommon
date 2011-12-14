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

#include <unistd.h>
#include <ctype.h>
#include "EventAgent.h"
#include "EventThread.h"
#include "jh_memory.h"
#include "logging.h"


SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_INFO);


class TestClass
{
public:
	TestClass() : mTheBool(false) {}
	~TestClass() {}

	void AsyncFunc0()
	{
		AsyncEventAgent0<TestClass> *agent = jh_new AsyncEventAgent0<TestClass>(this, &TestClass::handleAsyncFunc0);
		agent->send(&mEventThread);
	}
	void AsyncFunc1(uint32_t p1)
	{
		AsyncEventAgent1<TestClass, uint32_t> *agent =
			jh_new AsyncEventAgent1<TestClass, uint32_t>(this, &TestClass::handleAsyncFunc1,
				p1);
		agent->send(&mEventThread);
	}
	void AsyncFunc2(uint32_t p1, int16_t p2)
	{
		AsyncEventAgent2<TestClass, uint32_t, int16_t> *agent =
			jh_new AsyncEventAgent2<TestClass, uint32_t, int16_t>(this, &TestClass::handleAsyncFunc2,
				p1, p2);
		agent->send(&mEventThread);
	}
	void AsyncFunc3(uint32_t p1, int16_t p2, TestClass *p3)
	{
		AsyncEventAgent3<TestClass, uint32_t, int16_t, TestClass*> *agent =
			jh_new AsyncEventAgent3<TestClass, uint32_t, int16_t, TestClass*>(this, &TestClass::handleAsyncFunc3,
				p1, p2, p3);
		agent->send(&mEventThread);
	}
	void AsyncFunc4(uint32_t p1, int16_t p2, TestClass *p3, char *p4)
	{
		AsyncEventAgent4<TestClass, uint32_t, int16_t, TestClass*, char*> *agent =
			jh_new AsyncEventAgent4<TestClass, uint32_t, int16_t, TestClass*, char*>( this, &TestClass::handleAsyncFunc4,
				p1, p2, p3, p4);
		agent->send(&mEventThread);
	}
	void AsyncFunc5(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5)
	{
		AsyncEventAgent5<TestClass, uint32_t, int16_t, TestClass*, char*, int> *agent =
			jh_new AsyncEventAgent5<TestClass, uint32_t, int16_t, TestClass*, char*, int>( this, &TestClass::handleAsyncFunc5,
				p1, p2, p3, p4, p5);
		agent->send(&mEventThread);
	}
	void AsyncFunc6(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6)
	{
		AsyncEventAgent6<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool> *agent =
			jh_new AsyncEventAgent6<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool>( this, &TestClass::handleAsyncFunc6,
				p1, p2, p3, p4, p5, p6);
		agent->send(&mEventThread);
	}
	void AsyncFunc7(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7)
	{
		AsyncEventAgent7<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char> *agent =
			jh_new AsyncEventAgent7<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char>( this, &TestClass::handleAsyncFunc7,
				p1, p2, p3, p4, p5, p6, p7);
		agent->send(&mEventThread);
	}
	void AsyncFunc8(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8)
	{
		AsyncEventAgent8<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t> *agent =
			jh_new AsyncEventAgent8<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t>( this, &TestClass::handleAsyncFunc8,
				p1, p2, p3, p4, p5, p6, p7, p8);
		agent->send(&mEventThread);
	}
	void AsyncFunc9(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8, uint16_t p9)
	{
		AsyncEventAgent9<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t, uint16_t> *agent =
			jh_new AsyncEventAgent9<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t, uint16_t>( this, &TestClass::handleAsyncFunc9,
				p1, p2, p3, p4, p5, p6, p7, p8, p9);
		agent->send(&mEventThread);
	}
	void AsyncFunc10(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8, uint16_t p9, uint64_t p10)
	{
		AsyncEventAgent10<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t, uint16_t, uint64_t> *agent =
			jh_new AsyncEventAgent10<TestClass, uint32_t, int16_t, TestClass*, char*, int, bool, char, uint8_t, uint16_t, uint64_t>( this, &TestClass::handleAsyncFunc10,
				p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
		agent->send(&mEventThread);
	}
	
	
	void SyncFunc0()
	{
		SyncEventAgent0<TestClass> *agent =
			jh_new SyncEventAgent0<TestClass>(this, &TestClass::handleSyncFunc0);
		agent->send(&mEventThread);
	}
	
	void SyncFunc1(uint64_t &p1)
	{
		SyncEventAgent1<TestClass, uint64_t &> *agent =
			jh_new SyncEventAgent1<TestClass, uint64_t &>(this, &TestClass::handleSyncFunc1,
			p1);
		agent->send(&mEventThread);
		LOG_INFO("Sync1 result %llu", p1);
		
	}
	
	void SyncFunc2(uint64_t &p1, uint32_t &p2)
	{
		SyncEventAgent2<TestClass, uint64_t &, uint32_t &> *agent =
			jh_new SyncEventAgent2<TestClass, uint64_t &, uint32_t &>(this, &TestClass::handleSyncFunc2,
			p1, p2);
		agent->send(&mEventThread);
		LOG_INFO("Sync2 result %llu, %u", p1, p2);
	}
	
	void SyncFunc3(uint64_t &p1, uint32_t &p2, uint16_t &p3)
	{
		SyncEventAgent3<TestClass, uint64_t &, uint32_t &, uint16_t &> *agent =
			jh_new SyncEventAgent3<TestClass, uint64_t &, uint32_t &, uint16_t &>(this, &TestClass::handleSyncFunc3,
			p1, p2, p3);
		agent->send(&mEventThread);
		LOG_INFO("Sync3 result %llu, %u, %u", p1, p2, p3);
	}
	
	void SyncFunc4(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4)
	{
		SyncEventAgent4<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &> *agent =
			jh_new SyncEventAgent4<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &>(this, &TestClass::handleSyncFunc4,
			p1, p2, p3, p4);
		agent->send(&mEventThread);
		LOG_INFO("Sync4 result %llu, %u, %u, %u", p1, p2, p3, p4);
	}
	
	void SyncFunc5(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5)
	{
		SyncEventAgent5<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &> *agent =
			jh_new SyncEventAgent5<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &>(this, &TestClass::handleSyncFunc5,
			p1, p2, p3, p4, p5);
		agent->send(&mEventThread);
		LOG_INFO("Sync5 result %llu, %u, %u, %u, %lld", p1, p2, p3, p4, p5);
	}
	
	void SyncFunc6(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6)
	{
		SyncEventAgent6<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &> *agent =
			jh_new SyncEventAgent6<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &>(this, &TestClass::handleSyncFunc6,
			p1, p2, p3, p4, p5, p6);
		agent->send(&mEventThread);
		LOG_INFO("Sync6 result %llu, %u, %u, %u, %lld, %d", p1, p2, p3, p4, p5, p6);
	}
	
	void SyncFunc7(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7)
	{
		SyncEventAgent7<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &> *agent =
			jh_new SyncEventAgent7<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &>(this, &TestClass::handleSyncFunc7,
			p1, p2, p3, p4, p5, p6, p7);
		agent->send(&mEventThread);
		LOG_INFO("Sync7 result %llu, %u, %u, %u, %lld, %d, %d", p1, p2, p3, p4, p5, p6, p7);
	}
	
	void SyncFunc8(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8)
	{
		SyncEventAgent8<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &> *agent =
			jh_new SyncEventAgent8<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &>(this, &TestClass::handleSyncFunc8,
			p1, p2, p3, p4, p5, p6, p7, p8);
		agent->send(&mEventThread);
		LOG_INFO("Sync8 result %llu, %u, %u, %u, %lld, %d, %d, %d", p1, p2, p3, p4, p5, p6, p7, p8);
	}
	
	void SyncFunc9(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9)
	{
		SyncEventAgent9<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &, bool&> *agent =
			jh_new SyncEventAgent9<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &, bool&>(this, &TestClass::handleSyncFunc9,
			p1, p2, p3, p4, p5, p6, p7, p8, p9);
		agent->send(&mEventThread);
		LOG_INFO("Sync9 result %llu, %u, %u, %u, %lld, %d, %d, %d, %d", p1, p2, p3, p4, p5, p6, p7, p8, p9);
	}
	
	void SyncFunc10(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9, char &p10)
	{
		SyncEventAgent10<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &, bool&, char&> *agent =
			jh_new SyncEventAgent10<TestClass, uint64_t &, uint32_t &, uint16_t &, uint8_t &, int64_t &, int32_t &, int16_t &, int8_t &, bool&, char&>(this, &TestClass::handleSyncFunc10,
			p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
		agent->send(&mEventThread);
		LOG_INFO("Sync10 result %llu, %u, %u, %u, %lld, %d, %d, %d, %d, %c", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}
	
	bool SyncRetFunc0()
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result = false;
			SyncRetEventAgent0<TestClass, bool>* agent =
				jh_new SyncRetEventAgent0<TestClass, bool>(this, &TestClass::SyncRetFunc0);
			result = agent->send(&mEventThread);
			LOG_INFO("SyncRet0 result RetVal(%d)", result);
			return result;
		}
		LOG_INFO("Received");
		return true;
	}
	
	bool SyncRetFunc1(uint64_t &p1)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result = false;
			SyncRetEventAgent1<TestClass, bool, uint64_t &>* agent =
				jh_new SyncRetEventAgent1<TestClass, bool, uint64_t &>(this, &TestClass::SyncRetFunc1,
					p1);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu)", result, p1);
			return result;
		}
		
		LOG_INFO("Received p1(%llu)", p1);
		p1+=1024;
		return true;
	}
	
	bool SyncRetFunc2(uint64_t &p1, uint32_t p2)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result = false;
			SyncRetEventAgent2<TestClass, bool, uint64_t &, uint32_t>* agent =
				jh_new SyncRetEventAgent2<TestClass, bool, uint64_t &, uint32_t>(this, &TestClass::SyncRetFunc2,
					p1, p2);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u)", result, p1, p2);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u)", p1, p2);
		p1+=1024;
		p2+=512;
		return true;
	}
	
	bool SyncRetFunc3(uint64_t& p1, uint32_t p2, uint16_t& p3)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result = false;
			SyncRetEventAgent3<TestClass, bool, uint64_t &, uint32_t, uint16_t &>* agent =
				jh_new SyncRetEventAgent3<TestClass, bool, uint64_t &, uint32_t, uint16_t &>(this, &TestClass::SyncRetFunc3,
					p1, p2, p3);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u)", result, p1, p2, p3);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u)", p1, p2, p3);
		p1+=1024;
		p2+=512;
		p3+=256;
		return true;
	}
	
	class CopyCounterGuy
	{
	public:
		CopyCounterGuy() : mCount(0) {}
		CopyCounterGuy(const CopyCounterGuy& other)
		:	mCount(other.mCount+1) {}
		const CopyCounterGuy& operator=(const CopyCounterGuy& other)
		{
			mCount = other.mCount+1;
			return *this;
		}
		int getCount() { return mCount; }
	private:
		int mCount;
	};
	
	bool SyncRetFunc4(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result = false;
			SyncRetEventAgent4<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &>* agent =
				jh_new SyncRetEventAgent4<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &>(this, &TestClass::SyncRetFunc4,
					p1, p2, p3, ccg1);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d)", result, p1, p2, p3, ccg1.getCount());
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d)", p1, p2, p3, ccg1.getCount());
		p1+=1024;
		p2+=512;
		p3+=256;
		return true;
	}
	
	CopyCounterGuy SyncRetFunc5(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4)
	{
		if (not mEventThread.isThreadCurrent())
		{
			CopyCounterGuy result;
			SyncRetEventAgent5<TestClass, CopyCounterGuy, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &>* agent =
				jh_new SyncRetEventAgent5<TestClass, CopyCounterGuy, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&>(this, &TestClass::SyncRetFunc5,
				p1, p2, p3, ccg1, p4);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u)", result.getCount(), p1, p2, p3, ccg1.getCount(), p4);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u)", p1, p2, p3, ccg1.getCount(), p4);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		CopyCounterGuy newGuy;
		return newGuy;
	}
	
	bool SyncRetFunc6(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result;
			SyncRetEventAgent6<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &, int64_t&>* agent =
				jh_new SyncRetEventAgent6<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&>(this, &TestClass::SyncRetFunc6,
				p1, p2, p3, ccg1, p4, p5);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld)", result, p1, p2, p3, ccg1.getCount(), p4, p5);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld)", p1, p2, p3, ccg1.getCount(), p4, p5);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		return true;
	}
	
	bool SyncRetFunc7(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result;
			SyncRetEventAgent7<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &, int64_t&, int32_t&>* agent =
				jh_new SyncRetEventAgent7<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&, int32_t&>(this, &TestClass::SyncRetFunc7,
				p1, p2, p3, ccg1, p4, p5, p6);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d)", result, p1, p2, p3, ccg1.getCount(), p4, p5, p6);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d)", p1, p2, p3, ccg1.getCount(), p4, p5, p6);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		return true;
	}
	
	bool SyncRetFunc8(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6, int16_t &p7)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result;
			SyncRetEventAgent8<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&>* agent =
				jh_new SyncRetEventAgent8<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&, int32_t&, int16_t&>(this, &TestClass::SyncRetFunc8,
				p1, p2, p3, ccg1, p4, p5, p6, p7);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d)", result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d)", p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		return true;
	}
	
	bool SyncRetFunc9(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6, int16_t &p7, int8_t& p8)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result;
			SyncRetEventAgent9<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&, int8_t&>* agent =
				jh_new SyncRetEventAgent9<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&, int32_t&, int16_t&, int8_t&>(this, &TestClass::SyncRetFunc9,
				p1, p2, p3, ccg1, p4, p5, p6, p7, p8);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d) p8(%d)", result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d) p8(%d)", p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		p8-=16;
		return true;
	}
	
	bool SyncRetFunc10(uint64_t& p1, uint32_t p2, uint16_t& p3, CopyCounterGuy& ccg1, uint8_t& p4, int64_t& p5, int32_t &p6, int16_t &p7, int8_t& p8, bool& p9)
	{
		if (not mEventThread.isThreadCurrent())
		{
			bool result;
			SyncRetEventAgent10<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t &, int64_t&, int32_t&, int16_t&, int8_t&, bool&>* agent =
				jh_new SyncRetEventAgent10<TestClass, bool, uint64_t &, uint32_t, uint16_t &, CopyCounterGuy &, uint8_t&, int64_t&, int32_t&, int16_t&, int8_t&, bool&>(this, &TestClass::SyncRetFunc10,
				p1, p2, p3, ccg1, p4, p5, p6, p7, p8, p9);
			result = agent->send(&mEventThread);
			LOG_INFO("RetVal(%d) p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d) p8(%d) p9(%d)", result, p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8, p9);
			return result;
		}
		
		LOG_INFO("Received p1(%llu) p2(%u) p3(%u) ccg1(%d) p4(%u) p5(%lld) p6(%d) p7(%d) p8(%d) p9(%d)", p1, p2, p3, ccg1.getCount(), p4, p5, p6, p7, p8, p9);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		p8-=16;
		p9=!p9;
		return true;
	}
	
	
protected:
	bool mTheBool;
	void handleAsyncFunc0()
	{
		LOG_INFO("Received Async0");
	}
	void handleAsyncFunc1(uint32_t p1)
	{
		LOG_INFO("Received Async1 %u", p1);
	}
	void handleAsyncFunc2(uint32_t p1, int16_t p2)
	{
		LOG_INFO("Received Async2 %u, %d", p1, p2);
	}
	void handleAsyncFunc3(uint32_t p1, int16_t p2, TestClass *p3)
	{
		LOG_INFO("Received Async3 %u, %d, %p", p1, p2, p3);
	}
	void handleAsyncFunc4(uint32_t p1, int16_t p2, TestClass *p3, char *p4)
	{
		LOG_INFO("Received Async4 %u, %d, %p, %s", p1, p2, p3, p4);
	}
	void handleAsyncFunc5(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5)
	{
		LOG_INFO("Received Async5 %u, %d, %p, %s, %d",
				  p1, p2, p3, p4, p5);
	}
	void handleAsyncFunc6(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6)
	{
		LOG_INFO("Received Async6 %u, %d, %p, %s, %d, %d",
				  p1, p2, p3, p4, p5, p6);
	}
	void handleAsyncFunc7(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7)
	{
		LOG_INFO("Received Async7 %u, %d, %p, %s, %d, %d, %c",
				  p1, p2, p3, p4, p5, p6, p7);
	}
	void handleAsyncFunc8(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8)
	{
		LOG_INFO("Received Async8 %u, %d, %p, %s, %d, %d, %c, %d",
				  p1, p2, p3, p4, p5, p6, p7, p8);
	}
	void handleAsyncFunc9(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8, uint16_t p9)
	{
		LOG_INFO("Received Async9 %u, %d, %p, %s, %d, %d, %c, %d, %d",
				  p1, p2, p3, p4, p5, p6, p7, p8, p9);
	}
	void handleAsyncFunc10(uint32_t p1, int16_t p2, TestClass *p3, char *p4, int p5, bool p6, char p7, uint8_t p8, uint16_t p9, uint64_t p10)
	{
		LOG_INFO("Received Async10 %u, %d, %p, %s, %d, %d, %c, %d, %d, %llu",
				  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	}
	
	
	void handleSyncFunc0()
	{
		LOG_INFO("Received Sync0");
	}
	
	void handleSyncFunc1(uint64_t &p1)
	{
		LOG_INFO("Received Sync1, %llu", p1);
		p1+=1024;
	}
	
	void handleSyncFunc2(uint64_t &p1, uint32_t &p2)
	{
		LOG_INFO("Received Sync2, %llu, %u", p1, p2);
		p1+=1024;
		p2+=512;
	}
	
	void handleSyncFunc3(uint64_t &p1, uint32_t &p2, uint16_t &p3)
	{
		LOG_INFO("Received Sync3, %llu, %u, %u", p1, p2, p3);
		p1+=1024;
		p2+=512;
		p3+=256;
	}
	
	void handleSyncFunc4(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4)
	{
		LOG_INFO("Received Sync4, %llu, %u, %u, %u", p1, p2, p3, p4);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
	}
	
	void handleSyncFunc5(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5)
	{
		LOG_INFO("Received Sync5, %llu, %u, %u, %u, %lld", p1, p2, p3, p4, p5);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
	}
	
	void handleSyncFunc6(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6)
	{
		LOG_INFO("Received Sync6, %llu, %u, %u, %u, %lld, %d", p1, p2, p3, p4, p5, p6);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
	}
	
	void handleSyncFunc7(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7)
	{
		LOG_INFO("Received Sync7, %llu, %u, %u, %u, %lld, %d, %d", p1, p2, p3, p4, p5, p6, p7);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
	}
	
	void handleSyncFunc8(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8)
	{
		LOG_INFO("Received Sync8, %llu, %u, %u, %u, %lld, %d, %d, %d", p1, p2, p3, p4, p5, p6, p7, p8);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		p8-=16;
	}
	
	void handleSyncFunc9(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9)
	{
		LOG_INFO("Received Sync9, %llu, %u, %u, %u, %lld, %d, %d, %d, %d", p1, p2, p3, p4, p5, p6, p7, p8, p9);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		p8-=16;
		p9=!p9;
	}
	
	void handleSyncFunc10(uint64_t &p1, uint32_t &p2, uint16_t &p3, uint8_t &p4,
				   int64_t &p5, int32_t &p6, int16_t &p7, int8_t &p8, bool &p9, char &p10)
	{
		LOG_INFO("Received Sync10, %llu, %u, %u, %u, %lld, %d, %d, %d, %d, %c", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
		p1+=1024;
		p2+=512;
		p3+=256;
		p4+=16;
		p5-=1024;
		p6-=512;
		p7-=256;
		p8-=16;
		p9=!p9;
		p10=toupper(p10);
	}
	
	EventThread mEventThread;
	
};


void runAsyncTests()
{
	TestClass app;
	
	uint32_t p1 = 102910;
	int16_t p2 = -42;
	TestClass *p3 = &app;
	char *p4 = "Hello World";
	int p5 = -2910;
	bool p6 = true;
	char p7 = 'a';
	uint8_t p8 = 255;
	uint16_t p9 = (uint16_t)-1;
	uint64_t p10 = (uint64_t)-1;
	
	app.AsyncFunc0();
	app.AsyncFunc1(p1);
	app.AsyncFunc2(p1, p2);
	app.AsyncFunc3(p1, p2, p3);
	app.AsyncFunc4(p1, p2, p3, p4);
	app.AsyncFunc5(p1, p2, p3, p4, p5);
	app.AsyncFunc6(p1, p2, p3, p4, p5, p6);
	app.AsyncFunc7(p1, p2, p3, p4, p5, p6, p7);
	app.AsyncFunc8(p1, p2, p3, p4, p5, p6, p7, p8);
	app.AsyncFunc9(p1, p2, p3, p4, p5, p6, p7, p8, p9);
	app.AsyncFunc10(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	sleep(2);
	LOG_INFO("Async Tests complete!");
}


void runSyncTests()
{
	TestClass app;

	uint64_t p1 = 1024;
	uint32_t p2 = 512;
	uint16_t p3 = 256;
	uint8_t p4 = 16;
	int64_t p5 = -1024;
	int32_t p6 = -512;
	int16_t p7 = -256;
	int8_t p8 = -16;
	bool p9 = true;
	char p10 = 'a';
	
	app.SyncFunc0();
	app.SyncFunc1(p1);
	app.SyncFunc2(p1, p2);
	app.SyncFunc3(p1, p2, p3);
	app.SyncFunc4(p1, p2, p3, p4);
	app.SyncFunc5(p1, p2, p3, p4, p5);
	app.SyncFunc6(p1, p2, p3, p4, p5, p6);
	app.SyncFunc7(p1, p2, p3, p4, p5, p6, p7);
	app.SyncFunc8(p1, p2, p3, p4, p5, p6, p7, p8);
	app.SyncFunc9(p1, p2, p3, p4, p5, p6, p7, p8, p9);
	app.SyncFunc10(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
	LOG_INFO("Sync Tests complete!");
}


void runSyncRetTests()
{
	TestClass app;
	
	TestClass::CopyCounterGuy returnCCG;
	
	uint64_t p1 = 1024;
	uint32_t p2 = 512;
	uint16_t p3 = 256;
	TestClass::CopyCounterGuy ccg1;
	uint8_t p4 = 16;
	int64_t p5 = -1024;
	int32_t p6 = -512;
	int16_t p7 = -256;
	int8_t p8 = -16;
	bool p9 = true;
	
	app.SyncRetFunc0();
	app.SyncRetFunc1(p1);
	app.SyncRetFunc2(p1, p2);
	app.SyncRetFunc3(p1, p2, p3);
	app.SyncRetFunc4(p1, p2, p3, ccg1);
	returnCCG = app.SyncRetFunc5(p1, p2, p3, ccg1, p4);
	LOG_INFO("Total copies after SyncRetFunc5(%d)", returnCCG.getCount());
	app.SyncRetFunc6(p1, p2, p3, ccg1, p4, p5);
	app.SyncRetFunc7(p1, p2, p3, ccg1, p4, p5, p6);
	app.SyncRetFunc8(p1, p2, p3, ccg1, p4, p5, p6, p7);
	app.SyncRetFunc9(p1, p2, p3, ccg1, p4, p5, p6, p7, p8);
	app.SyncRetFunc10(p1, p2, p3, ccg1, p4, p5, p6, p7, p8, p9);
	
	LOG_INFO("SyncRet tests complete!");
}


int main(int argc, char *argv[])
{
	runAsyncTests();
	runSyncTests();
	runSyncRetTests();
}

