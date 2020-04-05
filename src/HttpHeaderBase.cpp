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

#include "HttpHeaderBase.h"

#include "logging.h"
#include "jh_memory.h"
#include "jh_types.h"


SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);


using namespace JetHead;


HttpHeaderBase::HttpHeaderBase()
	:	mNumFields(0)
{
	mHeaderStr.reserve(kDefaultMessageSize);
}


HttpHeaderBase::~HttpHeaderBase()
{
	for (JetHead::list<FieldMap*>::iterator i = mFieldMappings.begin();
		 i != mFieldMappings.end(); ++i)
	{
		delete *i;
	}

	mFieldMappings.clear();
}


const char *HttpHeaderBase::getHeader()
{
	if (mHeaderStr.empty())
	{
		buildHeader();
	}
	return mHeaderStr.c_str();
}

void HttpHeaderBase::printHeader()
{
	if (mHeaderStr.empty())
	{
		buildHeader();
	}
	printf("%s\n", mHeaderStr.c_str());
}

int HttpHeaderBase::send(IReaderWriter *writer) const
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	if (mHeaderStr.empty())
	{
		buildHeader();
	}
	
	LOG("Writing header of length %lu", mHeaderStr.size());
	
	return writer->write(mHeaderStr.c_str(), mHeaderStr.size());
}

int HttpHeaderBase::send(IReaderWriter *writer, const uint8_t* data, int len) const
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	if (mHeaderStr.empty())
	{
		buildHeader();
	}
	
	if (not data) 
	{
		return send(writer);
	}
	
	uint8_t* temp = jh_new uint8_t[mHeaderStr.size() + len];
	memcpy(temp, mHeaderStr.c_str(), mHeaderStr.size());
	memcpy(temp + mHeaderStr.size(), data, len);
	
	int res = writer->write(temp, mHeaderStr.size() + len);
	delete[] temp;
	return res;
}


int HttpHeaderBase::sendto(Socket *writer, const Socket::Address& addr) const
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	if (mHeaderStr.empty())
	{
		buildHeader();
	}
	
	LOG("Writing header of length %lu", mHeaderStr.size());
	
	return writer->sendto(mHeaderStr.c_str(), mHeaderStr.size(), addr);
}


int HttpHeaderBase::addFieldMap(FieldMap *fieldMap)
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	if (fieldMap == NULL)
		return -1;
	
	// Currently the assumption is that people adding field maps are doing
	// it in order.  ie.   HTTP->DLNA->DTV and that they want the mappings
	// searched in that order.  So mappings are appended to the tail as they
	// are added.
	mFieldMappings.push_back(fieldMap);
	return 0;
}


FieldMap::FieldType HttpHeaderBase::getFieldType(const char *field) const
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	FieldMap::FieldType type = FieldMap::kInvalidFieldType;

	for (JetHead::list<FieldMap*>::iterator i = mFieldMappings.begin();
		 i != mFieldMappings.end(); ++i)
	{
		FieldMap *map = *i;
		type = map->getFieldType(field);
		if (type != FieldMap::kInvalidFieldType)
			break;
	}

	return type;
}


const char *HttpHeaderBase::getFieldTypeString(FieldMap::FieldType type) const
{
	TRACE_BEGIN(LOG_LVL_INFO);
	
	const char *fieldName = NULL;


	for (JetHead::list<FieldMap*>::iterator i = mFieldMappings.begin();
		 i != mFieldMappings.end(); ++i)
	{
		FieldMap *map = *i;
		fieldName = map->getFieldTypeString(type);
		if (fieldName != NULL)
			break;
	}
	return fieldName;
}


int HttpHeaderBase::parseLine(const CircularBuffer &buffer)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	enum {ParseField, ParseData, ParseCRLF, SkipWS } state = ParseField;
	
	std::string field;
	std::string data;
	
	bool appendSpace = false;
	int length = 1;
	int c;
	
	for (int i = 0; i < buffer.getLength(); ++i, ++length)
	{
		c = buffer.byteAt(i);
		
		// This shouldn't happen, but make sure...
		if (c == -1)
		{
			LOG_ERR("Index %d, Buffer length %d, got -1 from byteAt",
					i, buffer.getLength());
			return -1;
		}
		
		// Add characters to the field until we find a ":" at
		// which point we will transition to SkipWS state.
		if (state == ParseField)
		{
			// Check for ":"
			if (c == ':')
			{
				// Update state to SkipWS
				state = SkipWS;
				appendSpace = false;
			}
			else
			{
				// Append character to the field
				field += c;
			}
		}
		// Add characters to the data until we find a CRLF
		else if (state == ParseData)
		{
			// Check for CRLF
			if (c == '\r' and  buffer.byteAt(i + 1) == '\n')
			{
				state = ParseCRLF;
				
				// In the case that this might be a continuation we
				// want a single space appended before the next valid
				// character
				appendSpace = true;
			}
			else
			{
				data += c;
			}
		}
		// Someone detected CRLF and we have to parse it appropriately
		else if (state == ParseCRLF)
		{
			// First character when we have CRLF should be the
			// LF.   Determine whether this is the end of the field
			// data or a continuation
			if (c == '\n')
			{
				// Ah... a continuation... transition to SkipWS
				if (buffer.byteAt(i + 1) == ' ' or buffer.byteAt(i + 1) == '\t')
				{
					state = SkipWS;
				}
				// Otherwise... this is the end of the field
				else
				{
					FieldMap::FieldType type = getFieldType(field.c_str());
					
					if (type != FieldMap::kInvalidFieldType)
					{
						mFields[mNumFields].setType(type);
						mFields[mNumFields].setData(data.c_str());
						mNumFields++;
						LOG("Field %s, data %s", field.c_str(), data.c_str());
					}
					else
					{
						LOG("Unknown field %s, data %s", field.c_str(), data.c_str());
					}
					return length;
				}
			}
		}
		// Skip over white space ' ' and '\t', if we see a
		// CRLF then transition to HaveCRLF
		else if (state == SkipWS)
		{
			// Check for CRLF
			if (c == '\r' and buffer.byteAt(i + 1) == '\n')
			{
				state = ParseCRLF;
			}
			// Otherwise if this is not just normal whitespace (' ' or '\t')
			// then append it to the data
			else if (c != '\t' and c != ' ')
			{
				// If we are supposed to append a space when we
				// are done skipping white space then do so now
				if (appendSpace)
				{
					data += ' ';
					appendSpace = false;
				}
				data += c;
				state = ParseData;
			}
		}
		else
		{
			LOG_ERR("Parse error, invalid state");
			return -1;
		}
	}
	
	// Return an error as we parsed through the entire buffer but
	// were unable to determine the field/data
	LOG_ERR("Parse error, unable to parse");
	return -1;
}


const char *HttpHeaderBase::getField(FieldMap::FieldType type) const
{
	for (int i = 0; i < mNumFields; i++)
	{
		if (mFields[i].mType == type)
			return mFields[i].mData;
	}
	
	return NULL;
}


ErrCode HttpHeaderBase::getFieldInt(FieldMap::FieldType type,
									int32_t &value) const
{
	const char *str = getField(type);
	
	if (str != NULL)
	{
		value = strtol(str, NULL, 0);
		return kNoError;
	}
	
	value = 0;
	return kNotFound;
}


ErrCode HttpHeaderBase::getFieldInt64(FieldMap::FieldType type,
									  int64_t &value) const
{
	const char *str = getField(type);
	
	if (str != NULL)
	{
		value = strtoll(str, NULL, 0);
		return kNoError;
	}
	
	value = 0;
	return kNotFound;
}


ErrCode HttpHeaderBase::removeField(FieldMap::FieldType type)
{
	TRACE_BEGIN(LOG_LVL_NOISE);

	for (int i = 0; i < mNumFields; i++)
	{
		if (mFields[i].mType == type)
		{
			for (int j = i; j < mNumFields - 1; j++)
			{
				mFields[j] = mFields[j + 1];
			}
			mFields[mNumFields - 1].mData = NULL;
			mNumFields--;
			return JetHead::kNoError;
		}
	}
	return JetHead::kNotFound;
}

bool HttpHeaderBase::addField(FieldMap::FieldType type, const char *data)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	if (mNumFields >= kMaxFields)
		return false;
	
	mFields[mNumFields].mType = type;
	mFields[mNumFields].setData(data);
	mNumFields++;

	// Make sure we re-build the header if it has already been built.
	mHeaderStr.clear();
	
	return true;
}


bool HttpHeaderBase::addFieldDecimal(FieldMap::FieldType type, int32_t data)
{
	char buffer[32];
	
	sprintf(buffer, "%d", data);
	
	return addField(type, buffer);
}

bool HttpHeaderBase::addFieldHex(FieldMap::FieldType type, uint32_t data)
{
	char buffer[32];
	
	sprintf(buffer, "%x", data);
	
	return addField(type, buffer);
}


bool HttpHeaderBase::addFieldDecimal64(FieldMap::FieldType type, int64_t data)
{
	char buffer[32];
	
	sprintf(buffer, "%lld", data);
	
	return addField(type, buffer);
}


void HttpHeaderBase::buildHeader() const
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	if (buildFirstLine() <= 0)
	{
		mHeaderStr.clear();
		return;
	}
	
	for (int i = 0; i < mNumFields; i++)
	{
		mHeaderStr += getFieldTypeString(mFields[i].mType);
		mHeaderStr += ": ";
		mHeaderStr += mFields[i].mData;
		mHeaderStr += "\r\n";
	}
		mHeaderStr += "\r\n";	
}


int HttpHeaderBase::searchForLine(const CircularBuffer &buffer)
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	enum { SearchCR, HaveCR } state = SearchCR;
	
	// Initial length of 1
	int length = 1;
	int c;
	
	// Continue until we reach the end of the buffer.  On each
	// iteration of the loop increment the index and the length
	for (int i = 0; i < buffer.getLength(); ++i, ++length)
	{
		// Fetch the byte and update the parse length
		c = buffer.byteAt(i);
		
		// This shouldn't happen, but make sure...
		if (c == -1)
		{
			LOG_ERR("Index %d, Buffer length %d, got -1 from byteAt",
					i, buffer.getLength());
			return -1;
		}
		
		if (state == SearchCR)
		{
			// Check for CR
			if (c == '\r')
			{
				// Update state to check for LF
				state = HaveCR;
			}
		}
		else if (state == HaveCR)
		{
			// Check for LF
			if (c == '\n')
			{
				// Fetch the next byte from the buffer so we
				// can test for continuation.
				c = buffer.byteAt(i + 1);
				
				// If the current length is 2 then this
				// is the end of the header
				if (length == 2)
				{
					return HttpHeaderBase::kEOH;
				}
				// Otherwise fetch the next byte and check
				// to see if it is a continuation.  If it is
				// then reset state to 0
				else if (c == ' ' or c == '\t')
				{
					state = SearchCR;
				}
				// Finally this wasn't a continuation so we have
				// found the end of the header line, return the length
				else
				{
					return length;
				}
			}
			// If no LF then search for another CR
			else
			{
				state = SearchCR;
			}
		}
	}

	// Return an error as we searched the entire buffer and did not
	// find a CRLF yet.
	LOG("Unable to find header field in buffer");
	return -1;
}


//
//	HttpHeaderBase::HeaderField functions
//


/**
 *	@brief Copy data specified into the HeaderField
 */
void HttpHeaderBase::HeaderField::setData(const char *data)
{
	if (mData)
	{
		delete[] mData;		
	}
	mData = jh_new char[strlen(data) + 1];
	strcpy(mData, data);
}


