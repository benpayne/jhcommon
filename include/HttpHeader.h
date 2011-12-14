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

#ifndef HTTPHEADER_H_
#define HTTPHEADER_H_

/**
 * \file
 *
 * Basic interface shared between HttpRequest and HttpResponse
 */

#include "JetHead.h"
#include "HttpHeaderBase.h"
#include "FieldMap.h"
#include "jh_memory.h"

/**
 *	This class provides field mappings for standard HTTP 1.1 header fields
 */
class HttpFieldMap : public FieldMap
{
public:
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
	enum HttpFieldType {
		kFieldAccept = FieldMap::kHttpFieldMapStart,
		kFieldAcceptCharset,
		kFieldAcceptEncoding,
		kFieldAcceptLanguage,
		kFieldAcceptRanges,
		kFieldAge,
		kFieldAllow,
		kFieldAuthorization,
		kFieldCacheControl,
		kFieldConnection,
		kFieldContentEncoding,
		kFieldContentLanguage,
		kFieldContentLength,
		kFieldContentLocation,
		kFieldContentMD5,
		kFieldContentRange,
		kFieldContentType,
		kFieldDate,
		kFieldETag,
		kFieldExpect,
		kFieldExpires,
		kFieldFrom,
		kFieldHost,
		kFieldIfMatch,
		kFieldIfModifiedSince,
		kFieldIfNoneMatch,
		kFieldIfRange,
		kFieldIfUnmodifiedSince,
		kFieldLastModified,
		kFieldLocation,
		kFieldMaxForwards,
		kFieldPragma,
		kFieldProxyAuthenticate,
		kFieldProxyAuthorization,
		kFieldRange,
		kFieldReferer,
		kFieldRetryAfter,
		kFieldServer,
		kFieldTE,
		kFieldTrailer,
		kFieldTransferEncoding,
		kFieldUpgrade,
		kFieldUserAgent,
		kFieldVary,
		kFieldVia,
		kFieldWarning,
		kFieldWWWAuthenticate,
		
		kNumFields
	};
#endif
	
	// Virtual destructor
	virtual ~HttpFieldMap() {}
private:
	
	//
	// FieldMap interface
	//

	//! Return the field ID given the header string
	FieldType getFieldType(const char *field) const;

	//! Return the header string given the field ID
	const char *getFieldTypeString(FieldType fieldType) const;
};


/**
 *	@brief Base HTTP 1.1 header
 *
 *	This base class initializes with HTTP 1.1 Field mappings and
 *	provides a convenient method to invoke to generate an HTTP 1.1
 *	compliant Date field.
 */
class HttpHeader : public HttpHeaderBase
{
public:
	//! Extend HttpHeaderBase with the HttpFieldMap, above
	HttpHeader()
	{
		addFieldMap(jh_new HttpFieldMap());
	}
	
	//! Virtual do-nothing destructor
	virtual ~HttpHeader() {}

	//! Put the date value in the header
	bool addDateField();
};

#endif // HTTPHEADER_H_

