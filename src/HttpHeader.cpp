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

#include "HttpHeader.h"
#include "logging.h"

#include <time.h>
#include <string.h>

using namespace JetHead;

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);


//
// HttpFieldMap functions
//


// All HTTP/1.1 Header Fields
static const char *gFieldTypeList[] = {
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Accept-Ranges",
	"Age",
	"Allow",
	"Authorization",
	"Cache-Control",
	"Connection",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Date",
	"ETag",
	"Expect",
	"Expires",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Last-Modified",
	"Location",
	"Max-Forwards",
	"Pragma",
	"Proxy-Authenticate",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"Retry-After",
	"Server",
	"TE",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"User-Agent",
	"Vary",
	"Via",
	"Warning",
	"WWW-Authenticate"
};

FieldMap::FieldType HttpFieldMap::getFieldType(const char *field) const
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	for (int i = 0; i < kNumFields - FieldMap::kHttpFieldMapStart; ++i)
	{
		if (strcasecmp(field, gFieldTypeList[i]) == 0)
		{
			LOG("type %d, name %s",
				i+FieldMap::kHttpFieldMapStart, gFieldTypeList[i]);
			return (FieldMap::FieldType)i + FieldMap::kHttpFieldMapStart;
		}
	}
	return FieldMap::kInvalidFieldType;
}

const char * HttpFieldMap::getFieldTypeString(FieldType type) const
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	if (type >= FieldMap::kHttpFieldMapStart and type < kNumFields)
	{
		type -= FieldMap::kHttpFieldMapStart;
		
		LOG("type %d, name %s",
				   type, gFieldTypeList[type]);
		
		return gFieldTypeList[type];
	}
	return NULL;
}


//
//	HttpHeader functions
//


bool HttpHeader::addDateField()
{
	time_t t = time( NULL );
	struct tm tmp;
	localtime_r(&t, &tmp);

	char buf[64];
	strftime(buf, 64, "%a, %b %e %T %Y", &tmp);
	return addField(HttpFieldMap::kFieldDate, buf);
}



