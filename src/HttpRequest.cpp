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

#include "HttpRequest.h"
#include "logging.h"

#include <string.h>

using namespace JetHead;

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);

static const unsigned MAX_METHOD_SIZE = 128;
static const unsigned MAX_URI_SIZE = 2048;
static const unsigned MAX_PROTO_SIZE = 64;

// This must match the enumeration in HttpRequest.h
static const char *gMethodTypeList[] = {
	"OPTIONS",
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"CONNECT",
};

HttpRequest::HttpRequest() 
	: mMethod( kMethodUnknown ),
	  mMajor( -1 ),
	  mMinor( -1 )
{
}

HttpRequest::HttpRequest( const URI &uri, MethodType type ) : mMethod( type ), mUri( uri )
{
	addField( HttpFieldMap::kFieldHost, uri.getHost().c_str() );
	addDateField();
}

HttpRequest::~HttpRequest()
{
}

int HttpRequest::parseFirstLine( const CircularBuffer &buf )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	std::string method;
	method.reserve(MAX_METHOD_SIZE);
	std::string uri;
	uri.reserve(MAX_URI_SIZE);
	std::string protocol;
	protocol.reserve(MAX_PROTO_SIZE);
	int i = 0;
	int j = 0;
	int c;
	
	while ( 1 )
	{
		c = buf.byteAt( i++ );	
		
		// If we requested a byte beyond the line size . . .
		if ( c == -1 or i > buf.getLength() )
		{
			return -1;
		}

		if ( c == ' ' )
		{
			break;
		}
		
		method += c;
		if (method.size() >= MAX_METHOD_SIZE) return -1;
	}

	while ( (c = buf.byteAt( i ) == ' ') )
	{
		i++;

		// If we requested a byte beyond the line size . . .
		if ( c == -1 or i > buf.getLength() )
		{
			return -1;
		}
	}	

	j = 0;
	while ( 1 )
	{
		c = buf.byteAt( i++ );	

		// If we requested a byte beyond the line size . . .
		if ( c == -1 or i > buf.getLength() )
		{
			return -1;
		}

		if ( c == ' ' )
			break;

		uri += c;
		if (uri.size() >= MAX_URI_SIZE) return -1;
	}


	j = 0;
	while ( 1 )
	{
		c = buf.byteAt( i++ );	

		// If we requested a byte beyond the line size . . .
		if ( c == -1 or i > buf.getLength() )
		{
			return -1;
		}

		if ( c == '\r' )
			break;

		protocol += c;
		if (protocol.size() >= MAX_PROTO_SIZE) return -1;
	}
	
	// if CRLF move past LF
	if ( buf.byteAt( i ) == '\n' )
		i++;
	
	LOG( "Method %s, URI %s, Proto %s", 
		 method.c_str(), uri.c_str(), protocol.c_str() );

	parseProtocol( protocol.c_str() );
	parseMethod( method.c_str() );
	mUri = uri.c_str();

	return i;
}

int HttpRequest::buildFirstLine() const
{
	mHeaderStr = gMethodTypeList[ mMethod ]; 

	mHeaderStr += " ";
	mHeaderStr += mUri.getPath();

	if (not mUri.getQuery().empty())
	{
		mHeaderStr += "?";
		mHeaderStr += mUri.getQuery();
	}

	mHeaderStr += " HTTP/1.1\r\n";

	return mHeaderStr.size();
}

void HttpRequest::parseMethod( const char *method )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	for ( uint32_t i = 0; i < JH_ARRAY_SIZE( gMethodTypeList ); i++ )
	{
		//LOG_NOISE( "matching %s", gMethodTypeList[ i ] );
		if ( strcmp( method, gMethodTypeList[ i ] ) == 0 )
		{
			mMethod = (MethodType)i;
			LOG( "Found method %s", gMethodTypeList[ mMethod ] );
			return;
		}
	}
	
	LOG( "Failed to match method \"%s\"", method );
	mMethod = kMethodUnknown;
}

void HttpRequest::parseProtocol( const char *protocol )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	sscanf(protocol, "HTTP/%d.%d", &mMajor, &mMinor);
	LOG("Major proto %d minor proto %d", mMajor, mMinor);
}

void HttpRequest::getVersion(int& major, int& minor)
{
	major = mMajor;
	minor = mMinor;
}
