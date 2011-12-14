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

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include "HttpHeader.h"

class HttpRequest : public HttpHeader
{
public:
	// This must match the listings in HttpRequest.cpp AND
	// jhutil/src/JHWebServer.cpp
	enum MethodType {
		kMethodOptions,
		kMethodGet,
		kMethodHead,
		kMethodPost,
		kMethodPut,
		kMethodDelete,
		kMethodTrace,
		kMethodConnect,
		kMethodUnknown,
	};

	HttpRequest();
	HttpRequest( const URI &uri, MethodType type = kMethodGet );
	virtual ~HttpRequest();

	MethodType getMethod() const { return mMethod; }
	const URI &getURI() const { return mUri; }
	void getVersion(int& major, int& minor);

	void setMethod( MethodType type ) { mMethod = type; }
	void setURI( const URI &uri ) { mUri = uri; }

	int parseFirstLine( const CircularBuffer &buf );
	
protected:
	virtual int buildFirstLine() const;

	void parseMethod( const char *method );
	void parseProtocol( const char *protocol );
											  
 private:
	MethodType mMethod;
	URI mUri;	

	int mMajor;
	int mMinor;
};

#endif // HTTPREQUEST_H_
