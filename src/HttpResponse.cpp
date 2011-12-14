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

#include "HttpResponse.h"
#include "logging.h"

#include <string.h>

using namespace JetHead;

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);

struct ResponseCodeData
{
	int code;
	const char *description;
};

ResponseCodeData gResponseCodeList[] = {
	{ 100, "Continue" },
	{ 100, "Switching Protocols" },
	
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authorative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	
	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 305, "Temporary Redirect" },

	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Request Entity Too Large" },
	{ 414, "Request-URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Request Range Not Satisfiable" },
	{ 417, "Expectation Failed" },

	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Timeout" },
	{ 505, "HTTP Version Not Supported" },
};

HttpResponse::HttpResponse() : mResponseCode( 200 )
{
}

HttpResponse::~HttpResponse()
{
	mResponseCode = -1;
}
	
const char *HttpResponse::getResponseString() const
{
	if (mResponseCode == -1)
	{
		LOG_ERR_FATAL("called after destruction with mResponseCode (%d)",
					  mResponseCode);
	}
	else if (mResponseCode < 100 or mResponseCode > 599)
	{
		LOG_ERR("unexpected mResponseCode (%d)", mResponseCode);
	}

	return getResponseString( mResponseCode );
}

int HttpResponse::parseFirstLine( const CircularBuffer &buf )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	int i = 0;
	char c = 0;

	std::string statusCodeStr;

	enum { ParseProtocol,
		   SeekStatusCode,
		   ParseStatusCode,
		   SeekStatusText,
		   ParseStatusText,
		   ParsingError } state = ParseProtocol;

	// We should only be called on buffers that contain at least two
	// bytes of CRLF
	if (buf.getLength() <= 1)
	{
		LOG_ERR("called with only %d bytes in buffer", buf.getLength());
		return -1;
	}
	
	// In the case of having got a 100, we might now get a blank line
	// before the real header.
	if (buf.byteAt(0) == '\r' and buf.byteAt(1) == '\n')
	{
		return 2;
	}
	
	while(i < buf.getLength() and state != ParsingError)
	{
		c = buf.byteAt(i++);
		
		switch(state)
		{
		case ParseProtocol:
			if (c != ' ')
			{
				// append c to protocol string, unimplemented for now
				break;
			}
			state = SeekStatusCode;
			// found a space, so fall through to the next state to
			// handle the space
			
		case SeekStatusCode:
			if (c == ' ')
			{
				// skip over spaces to get to status code
				break;
			}
			state = ParseStatusCode;
			// found a non-space, so fall through to the next state to
			// handle the non-space char
			
		case ParseStatusCode:
			if (c != ' ')
			{
				// append c to status code string
				statusCodeStr += c;
				break;
			}
			// found a space, convert statusCodeStr to int
			else
			{
				const char* nptr = statusCodeStr.c_str();
				char* endptr = NULL;
				errno = 0;
				
				mResponseCode = strtol(nptr, &endptr, 10);

				if (nptr == endptr or errno != 0)
				{
					LOG_ERR_PERROR("problem converting %s to int", nptr);
					state = ParsingError;
					break;
				}
			}

			state = SeekStatusText;
			// found a space, so fall through to the next state to
			// handle the space

		case SeekStatusText:
			if (c == ' ')
			{
				// skip over spaces to get to status text
				break;
			}
			state = ParseStatusText;
			// found a non-space, so fall through to the next state to
			// handle the non-space char

		case ParseStatusText:
			if (c != '\r' and c != '\n')
			{
				// append c to status text string, unimplemented for now
				break;
			}

			// if CRLF move past LF
			if (i < buf.getLength() and buf.byteAt(i) == '\n')
			{
				++i;
			}

			// If we got a 100 and there is enough data in the buffer
			// for the (possible) additional \r\n, then consume those
			// now too.
			if ( mResponseCode == 100 and i + 1 < buf.getLength() and
				 buf.byteAt(i) == '\r' and buf.byteAt(i + 1) == '\n' )
			{
				i += 2;
			}

			// Parsing complete, return the number of bytes to be
			// removed from the buffer
			return i;

		default:
			LOG_ERR_FATAL("invalid state %d", state);
			state = ParsingError;
		}
	}

	// If we exited the parsing loop, then encountered an error while
	// parsing or ran out of data, so return an error
	LOG_ERR("problem at %d parsing buffer of length %d, state %d",
			i, buf.getLength(), state);

	return -1;
}

int HttpResponse::buildFirstLine() const
{
	char statusCodeBuf[kStatusCodeSize + 1];
	statusCodeBuf[kStatusCodeSize] = '\0'; // terminate in case of error

	int res = snprintf(statusCodeBuf, kStatusCodeSize, "%d", mResponseCode);
	
	if (res >= kStatusCodeSize)
	{
		LOG_ERR_FATAL("mResponseCode %d exceeds kStatusCodeSize %d",
					  mResponseCode, kStatusCodeSize);
	}

	mHeaderStr = "HTTP/1.1 ";
	mHeaderStr += statusCodeBuf;
	mHeaderStr += " ";
	mHeaderStr += getResponseString(mResponseCode);
	mHeaderStr += "\r\n";
	
	return mHeaderStr.size();
}

const char *HttpResponse::getResponseString( int code ) const
{
	for( uint32_t i = 0; i < JH_ARRAY_SIZE( gResponseCodeList ); i++ )
	{
		if ( gResponseCodeList[ i ].code == code )
			return gResponseCodeList[ i ].description;
	}

	return "Unknown HTTP error";
}
