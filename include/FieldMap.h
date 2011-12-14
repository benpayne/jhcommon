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

#ifndef FIELDMAP_H_
#define FIELDMAP_H_

#include "list.h"

/**
 *	@brief Field mapping base interface
 *
 *	The FieldMap interface is used by the HttpHeaderBase class to parse
 *	and generate HTTP-style header fields.   Many open standards use HTTP
 *	as their basis and define custom fields to communicate information
 *	between the client and server.  Parsing and generation of these
 *	custom header fields is all general and can be handled in a single
 *	place (HttpHeaderBase), the FieldMap interface allows users to
 *	provide the HttpHeaderBase with knowledge about the fields it cares
 *	to parse and/or generate.
 *
 *	@see HttpHeaderBase
 *	@see HttpFieldMap
 *
 */
class FieldMap 
{
public:
	FieldMap() {}
	virtual ~FieldMap() {}
	
	typedef int FieldType;
	
	// Error value if the field in question is not part of the FieldMap
	static const int kInvalidFieldType = -1;
	
	// Define in the base FieldMap the start values of the number spaces
	// for various FieldMap namespaces.  Unless we map straight string to
	// string for Field:  Value in the HttpHeaderBase we need a shared
	// numberspace for all implementations of the FieldMap.
	// NOTE:   One of the compelling reasons to keep a mapping of number
	// value to string value is to allow compile time checking of header
	// field values.   It does have the side effect of forcing us to make
	// new FieldMaps when it comes time to add or retrieve a field that
	// hasn't already been defined.
	static const int kDlnaFieldMapStart = 100;
	static const int kDtvFieldMapStart = 200;
	static const int kRtspFieldMapStart = 300;
	static const int kHmcFieldMapStart = 400;
	static const int kSsdpFieldMapStart = 420;
	static const int kSoapFieldMapStart = 440;
	static const int kGenaFieldMapStart = 450;
	static const int kDLNAFieldMapStart = 460;
	static const int kDLNASessionFieldMapStart = 465;
	static const int kDLNARtspFieldMapStart =  475;
	static const int kHttpFieldMapStart = 1000;
	static const int kHttpTsFieldMapStart = 1100;

	virtual FieldType getFieldType(const char *field) const = 0;
	virtual const char *getFieldTypeString(FieldType type) const = 0;
};


#endif // FIELDMAP_H_
