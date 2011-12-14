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

#ifndef JH_EVENTAGENT_T_H_
#define JH_EVENTAGENT_T_H_

// Define helper templates for 0-10 parameter AsyncEventAgent
// and 0-10 parameter SyncEventAgent implementations

///////////////////////////////
//  Async 0 parameter agent
///////////////////////////////
template<class ClassType>
class AsyncEventAgent0 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(void);
	
	AsyncEventAgent0(ClassType *object, event_method_t method)
	:	mObject(object), mMethod(method)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)();
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
 protected:
	virtual ~AsyncEventAgent0() {}

};


///////////////////////////////
//  Async 1 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1>
class AsyncEventAgent1 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	
	AsyncEventAgent1(ClassType *object, event_method_t method,
					 ParamType1 param1)
	:	mObject(object), mMethod(method),
		mParam1(param1)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
 protected:
	virtual ~AsyncEventAgent1() {}

};


///////////////////////////////
//  Async 2 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2>
class AsyncEventAgent2 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	
	AsyncEventAgent2(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
 protected:
	virtual ~AsyncEventAgent2() {}
	
};


///////////////////////////////
//  Async 3 parameter agent
///////////////////////////////
template<typename ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3>
class AsyncEventAgent3 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, ParamType3);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	
	AsyncEventAgent3(ClassType *object, event_method_t method, ParamType1 param1, ParamType2 param2, ParamType3 param3)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
 protected:
	virtual ~AsyncEventAgent3() {}
	
};


///////////////////////////////
//  Async 4 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4>
class AsyncEventAgent4 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	
	AsyncEventAgent4(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
 protected:	
	virtual ~AsyncEventAgent4() {}

};


///////////////////////////////
//  Async 5 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5>
class AsyncEventAgent5 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	
	AsyncEventAgent5(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
 protected:
	virtual ~AsyncEventAgent5() {}	

};


///////////////////////////////
//  Async 6 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6>
class AsyncEventAgent6 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	typedef ParamType6& NoReferenceType6;
	
	AsyncEventAgent6(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
 protected:
	virtual ~AsyncEventAgent6() {}
	
};


///////////////////////////////
//  Async 7 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7>
class AsyncEventAgent7 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	typedef ParamType6& NoReferenceType6;
	typedef ParamType7& NoReferenceType7;
	
	AsyncEventAgent7(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
 protected:
	virtual ~AsyncEventAgent7() {}
	
};


///////////////////////////////
//  Async 8 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8>
class AsyncEventAgent8 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	typedef ParamType6& NoReferenceType6;
	typedef ParamType7& NoReferenceType7;
	typedef ParamType8& NoReferenceType8;
	
	AsyncEventAgent8(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
 protected:
	virtual ~AsyncEventAgent8() {}	

};


///////////////////////////////
//  Async 9 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9>
class AsyncEventAgent9 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	typedef ParamType6& NoReferenceType6;
	typedef ParamType7& NoReferenceType7;
	typedef ParamType8& NoReferenceType8;
	typedef ParamType9&	NoReferenceType9;
	
	AsyncEventAgent9(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8,
					 ParamType9 param9)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8,
						  mParam9);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
 protected:
	virtual ~AsyncEventAgent9() {}

};


///////////////////////////////
//  Async 10 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9,
		 typename ParamType10>
class AsyncEventAgent10 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9, ParamType10);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef ParamType1& NoReferenceType1;
	typedef ParamType2& NoReferenceType2;
	typedef ParamType3& NoReferenceType3;
	typedef ParamType4& NoReferenceType4;
	typedef ParamType5& NoReferenceType5;
	typedef ParamType6& NoReferenceType6;
	typedef ParamType7& NoReferenceType7;
	typedef ParamType8& NoReferenceType8;
	typedef ParamType9&	NoReferenceType9;
	typedef ParamType10& NoReferenceType10;
	
	AsyncEventAgent10(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8,
					 ParamType9 param9, ParamType10 param10)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9), mParam10(param10)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8,
						  mParam9, mParam10);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
	ParamType10		mParam10;
 protected:
	virtual ~AsyncEventAgent10() {}

};


///////////////////////////////
//  Sync 0 parameter agent
///////////////////////////////
template<class ClassType>
class SyncEventAgent0 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(void);
	
	SyncEventAgent0(ClassType *object, event_method_t method)
	:	mObject(object), mMethod(method)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)();
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
 protected:
	virtual ~SyncEventAgent0() {}
	
};


///////////////////////////////
//  Sync 1 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1>
class SyncEventAgent1 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1);
	
	SyncEventAgent1(ClassType *object, event_method_t method,
					 ParamType1 param1)
	:	mObject(object), mMethod(method),
		mParam1(param1)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
 protected:
	virtual ~SyncEventAgent1() {}

};


///////////////////////////////
//  Sync 2 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2>
class SyncEventAgent2 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2);
	
	SyncEventAgent2(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
 protected:
	virtual ~SyncEventAgent2() {}

};


///////////////////////////////
//  Sync 3 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3>
class SyncEventAgent3 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3);
	
	SyncEventAgent3(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
 protected:
	virtual ~SyncEventAgent3() {}

};


///////////////////////////////
//  Sync 4 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4>
class SyncEventAgent4 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4);
	
	SyncEventAgent4(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
 protected:
	virtual ~SyncEventAgent4() {}

};


///////////////////////////////
//  Sync 5 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5>
class SyncEventAgent5 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5);
	
	SyncEventAgent5(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
 protected:
	virtual ~SyncEventAgent5() {}

};


///////////////////////////////
//  Sync 6 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6>
class SyncEventAgent6 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6);
	
	SyncEventAgent6(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
 protected:
	virtual ~SyncEventAgent6() {}
	
};


///////////////////////////////
//  Sync 7 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7>
class SyncEventAgent7 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7);
	
	SyncEventAgent7(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
 protected:
	virtual ~SyncEventAgent7() {}

};


///////////////////////////////
//  Sync 8 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8>
class SyncEventAgent8 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8);
	
	SyncEventAgent8(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
 protected:
	virtual ~SyncEventAgent8() {}

};


///////////////////////////////
//  Sync 9 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9>
class SyncEventAgent9 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9);
	
	SyncEventAgent9(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8,
					 ParamType9 param9)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8,
						  mParam9);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
 protected:
	virtual ~SyncEventAgent9() {}

};


///////////////////////////////
//  Sync 10 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9,
		 typename ParamType10>
class SyncEventAgent10 : public SyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2,
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9, ParamType10);
	
	SyncEventAgent10(ClassType *object, event_method_t method,
					 ParamType1 param1, ParamType2 param2,
					 ParamType3 param3, ParamType4 param4,
					 ParamType5 param5, ParamType6 param6,
					 ParamType7 param7, ParamType8 param8,
					 ParamType9 param9, ParamType10 param10)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9), mParam10(param10)
	{
	}
		
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
						  mParam5, mParam6, mParam7, mParam8,
						  mParam9, mParam10);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
	ParamType10		mParam10;
 protected:
	virtual ~SyncEventAgent10() {}

};


///////////////////////////////
//  Async castable 1 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename StoreType1>
class AsyncCastableEventAgent1 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	
	AsyncCastableEventAgent1(ClassType *object, event_method_t method,
							 const StoreType1& param1)
	:	mObject(object), mMethod(method),
		mParam1(param1)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
 protected:
	virtual ~AsyncCastableEventAgent1() {}
	
};


///////////////////////////////
//  Async castable 2 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename StoreType1,
		 typename StoreType2>
class AsyncCastableEventAgent2 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	
	AsyncCastableEventAgent2(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
 protected:
	virtual ~AsyncCastableEventAgent2() {}
	
};

///////////////////////////////
//  Async castable 3 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3>
class AsyncCastableEventAgent3 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	
	AsyncCastableEventAgent3(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
 protected:
	virtual ~AsyncCastableEventAgent3() {}
	
};

///////////////////////////////
//  Async castable 4 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4>
class AsyncCastableEventAgent4 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	
	AsyncCastableEventAgent4(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
 protected:
	virtual ~AsyncCastableEventAgent4() {}
	
};

///////////////////////////////
//  Async castable 5 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5>
class AsyncCastableEventAgent5 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	
	AsyncCastableEventAgent5(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
 protected:
	virtual ~AsyncCastableEventAgent5() {}
	
};

///////////////////////////////
//  Async castable 6 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5,
		 typename StoreType6>
class AsyncCastableEventAgent6 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5, ParamType6);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	typedef StoreType6& NoReferenceType6;
	
	AsyncCastableEventAgent6(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5, const StoreType6& param6)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5,
							mParam6);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
	StoreType6		mParam6;
 protected:
	virtual ~AsyncCastableEventAgent6() {}
	
};

///////////////////////////////
//  Async castable 7 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5,
		 typename StoreType6,
		 typename StoreType7>
class AsyncCastableEventAgent7 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	typedef StoreType6& NoReferenceType6;
	typedef StoreType7& NoReferenceType7;
	
	AsyncCastableEventAgent7(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5, const StoreType6& param6,
							 const StoreType7& param7)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5,
							mParam6, mParam7);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
	StoreType6		mParam6;
	StoreType7		mParam7;
 protected:
	virtual ~AsyncCastableEventAgent7() {}
	
};

///////////////////////////////
//  Async castable 8 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5,
		 typename StoreType6,
		 typename StoreType7,
		 typename StoreType8>
class AsyncCastableEventAgent8 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	typedef StoreType6& NoReferenceType6;
	typedef StoreType7& NoReferenceType7;
	typedef StoreType8& NoReferenceType8;
	
	AsyncCastableEventAgent8(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5, const StoreType6& param6,
							 const StoreType7& param7, const StoreType8& param8)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5,
							mParam6, mParam7, mParam8);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
	StoreType6		mParam6;
	StoreType7		mParam7;
	StoreType8		mParam8;
 protected:
	virtual ~AsyncCastableEventAgent8() {}
	
};

///////////////////////////////
//  Async castable 9 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5,
		 typename StoreType6,
		 typename StoreType7,
		 typename StoreType8,
		 typename StoreType9>
class AsyncCastableEventAgent9 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	typedef StoreType6& NoReferenceType6;
	typedef StoreType7& NoReferenceType7;
	typedef StoreType8& NoReferenceType8;
	typedef StoreType9& NoReferenceType9;
	
	AsyncCastableEventAgent9(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5, const StoreType6& param6,
							 const StoreType7& param7, const StoreType8& param8,
							 const StoreType9& param9)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9)
											  
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5,
							mParam6, mParam7, mParam8, mParam9);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
	StoreType6		mParam6;
	StoreType7		mParam7;
	StoreType8		mParam8;
	StoreType9		mParam9;
 protected:
	virtual ~AsyncCastableEventAgent9() {}
	
};

///////////////////////////////
//  Async castable 9 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9,
		 typename ParamType10,
		 typename StoreType1,
		 typename StoreType2,
		 typename StoreType3,
		 typename StoreType4,
		 typename StoreType5,
		 typename StoreType6,
		 typename StoreType7,
		 typename StoreType8,
		 typename StoreType9,
		 typename StoreType10>
class AsyncCastableEventAgent10 : public AsyncEventAgent
{
public:
	typedef void (ClassType::*event_method_t)(ParamType1, ParamType2, 
											  ParamType3, ParamType4,
											  ParamType5, ParamType6,
											  ParamType7, ParamType8,
											  ParamType9, ParamType10);
	// Cause compile errors if the user specifies a reference for
	// any of the types in the template.  See comments in EventAgent.h
	typedef StoreType1& NoReferenceType1;
	typedef StoreType2& NoReferenceType2;
	typedef StoreType3& NoReferenceType3;
	typedef StoreType4& NoReferenceType4;
	typedef StoreType5& NoReferenceType5;
	typedef StoreType6& NoReferenceType6;
	typedef StoreType7& NoReferenceType7;
	typedef StoreType8& NoReferenceType8;
	typedef StoreType9& NoReferenceType9;
	typedef StoreType10& NoReferenceType10;
	
	AsyncCastableEventAgent10(ClassType *object, event_method_t method,
							 const StoreType1& param1, const StoreType2& param2,
							 const StoreType3& param3, const StoreType4& param4,
							 const StoreType5& param5, const StoreType6& param6,
							 const StoreType7& param7, const StoreType8& param8,
							 const StoreType9& param9, const StoreType10& param10)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9), mParam10(param10)
	{
	}
	
	void deliver()
	{
		(mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4, mParam5,
							mParam6, mParam7, mParam8, mParam9, mParam10);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	StoreType1		mParam1;
	StoreType2		mParam2;
	StoreType3		mParam3;
	StoreType4		mParam4;
	StoreType5		mParam5;
	StoreType6		mParam6;
	StoreType7		mParam7;
	StoreType8		mParam8;
	StoreType9		mParam9;
	StoreType10		mParam10;
 protected:
	virtual ~AsyncCastableEventAgent10() {}
	
};


///////////////////////////////
//  Sync returnType 0 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType>
class SyncRetEventAgent0 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(void);
	
	SyncRetEventAgent0(ClassType *object, event_method_t method)
	:	mObject(object), mMethod(method)
	{
	}
	
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)();
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
 protected:
	virtual ~SyncRetEventAgent0() {}
	
};


///////////////////////////////
//  Sync returnType 1 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1>
class SyncRetEventAgent1 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1);
	
	SyncRetEventAgent1(ClassType *object, event_method_t method,
					   ParamType1 param1)
	:	mObject(object), mMethod(method),
		mParam1(param1)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
 protected:
	virtual ~SyncRetEventAgent1() {}

};


///////////////////////////////
//  Sync returnType 2 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2>
class SyncRetEventAgent2 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2);
	
	SyncRetEventAgent2(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
 protected:
	virtual ~SyncRetEventAgent2() {}

};


///////////////////////////////
//  Sync returnType 3 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3>
class SyncRetEventAgent3 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3);
	
	SyncRetEventAgent3(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
 protected:
	virtual ~SyncRetEventAgent3() {}

};


///////////////////////////////
//  Sync returnType 4 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4>
class SyncRetEventAgent4 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4);
	
	SyncRetEventAgent4(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
 protected:
	virtual ~SyncRetEventAgent4() {}

};


///////////////////////////////
//  Sync returnType 5 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5>
class SyncRetEventAgent5 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5);
	
	SyncRetEventAgent5(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4,
					   ParamType5 param5)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
 protected:
	virtual ~SyncRetEventAgent5() {}

};


///////////////////////////////
//  Sync returnType 6 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6>
class SyncRetEventAgent6 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5, ParamType6);
	
	SyncRetEventAgent6(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4,
					   ParamType5 param5, ParamType6 param6)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6)
	{
	}
	
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5, mParam6);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
 protected:
	virtual ~SyncRetEventAgent6() {}
	
};


///////////////////////////////
//  Sync returnType 7 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7>
class SyncRetEventAgent7 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5, ParamType6,
													ParamType7);
	
	SyncRetEventAgent7(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4,
					   ParamType5 param5, ParamType6 param6,
					   ParamType7 param7)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5, mParam6, mParam7);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
 protected:
	virtual ~SyncRetEventAgent7() {}

};


///////////////////////////////
//  Sync returnType 8 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8>
class SyncRetEventAgent8 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5, ParamType6,
													ParamType7, ParamType8);
	
	SyncRetEventAgent8(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4,
					   ParamType5 param5, ParamType6 param6,
					   ParamType7 param7, ParamType8 param8)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5, mParam6, mParam7, mParam8);
	}
	
	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
 protected:
	virtual ~SyncRetEventAgent8() {}
};


///////////////////////////////
//  Sync returnType 9 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9>
class SyncRetEventAgent9 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5, ParamType6,
													ParamType7, ParamType8,
													ParamType9);
	
	SyncRetEventAgent9(ClassType *object, event_method_t method,
					   ParamType1 param1, ParamType2 param2,
					   ParamType3 param3, ParamType4 param4,
					   ParamType5 param5, ParamType6 param6,
					   ParamType7 param7, ParamType8 param8,
					   ParamType9 param9)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5, mParam6, mParam7, mParam8,
											  mParam9);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}
	
	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
 protected:
	virtual ~SyncRetEventAgent9() {}

};


///////////////////////////////
//  SyncRet returnType 10 parameter agent
///////////////////////////////
template<class ClassType,
		 typename ReturnType,
		 typename ParamType1,
		 typename ParamType2,
		 typename ParamType3,
		 typename ParamType4,
		 typename ParamType5,
		 typename ParamType6,
		 typename ParamType7,
		 typename ParamType8,
		 typename ParamType9,
		 typename ParamType10>
class SyncRetEventAgent10 : public SyncRetEventAgent<ReturnType>
{
public:
	typedef ReturnType (ClassType::*event_method_t)(ParamType1, ParamType2,
													ParamType3, ParamType4,
													ParamType5, ParamType6,
													ParamType7, ParamType8,
													ParamType9, ParamType10);
	
	SyncRetEventAgent10(ClassType *object, event_method_t method,
						ParamType1 param1, ParamType2 param2,
						ParamType3 param3, ParamType4 param4,
						ParamType5 param5, ParamType6 param6,
						ParamType7 param7, ParamType8 param8,
						ParamType9 param9, ParamType10 param10)
	:	mObject(object), mMethod(method),
		mParam1(param1), mParam2(param2),
		mParam3(param3), mParam4(param4),
		mParam5(param5), mParam6(param6),
		mParam7(param7), mParam8(param8),
		mParam9(param9), mParam10(param10)
	{
	}
		
	void deliver()
	{
		this->mRetValue = (mObject->*mMethod)(mParam1, mParam2, mParam3, mParam4,
											  mParam5, mParam6, mParam7, mParam8,
											  mParam9, mParam10);
	}

	void* getDeliveryTarget()
	{
		return (void*)mObject;
	}

	ClassType		*mObject;
	event_method_t	mMethod;
	ParamType1		mParam1;
	ParamType2		mParam2;
	ParamType3		mParam3;
	ParamType4		mParam4;
	ParamType5		mParam5;
	ParamType6		mParam6;
	ParamType7		mParam7;
	ParamType8		mParam8;
	ParamType9		mParam9;
	ParamType10		mParam10;
 protected:
	virtual ~SyncRetEventAgent10() {}

};


#endif // JH_EVENTAGENT_T_H_

