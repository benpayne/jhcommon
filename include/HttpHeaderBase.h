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

#ifndef HTTPHEADER_BASE_H_
#define HTTPHEADER_BASE_H_

/**
 * \file
 *
 * Base class for HTTP-like headers (including things like RTSP)
 */

#include "jh_string.h"

#include "JetHead.h"
#include "URI.h"
#include "IReaderWriter.h"
#include "CircularBuffer.h"
#include "FieldMap.h"
#include "Socket.h"

/**
 *	@brief Base class for HTTP-style header generation/parsing
 *
 *	This class provides a generic means to parse and generate HTTP-style
 *	headers.  Users derive from this class and implement the
 *	parseFirstLine and buildFirstLine methods to generate and parse the
 *	appropriate first line of the HTTP-style header.
 *
 *	Users provide the HttpHeaderBase with field mappings from a
 *	FieldMap::FieldType to a string and the inverse and the HttpHeaderBase
 *	will parse and generate fields appropriately.  This can be further
 *	extended in derived classes, adding additional FieldMaps.
 *
 *	@see FieldMap
 *	@see HttpFieldMap
 */
class HttpHeaderBase
{
public:
	//! Allocate a new header 
	HttpHeaderBase();

	//! Free up allocated memory
	virtual ~HttpHeaderBase();

	/**
	 *	@brief Generate and transmit HTTP-style header
	 *
	 *	The HttpHeaderBase will generate an HTTP-style header from the
	 *	list of fields the user has specified (parsed) and write the
	 *	header to the IReaderWriter specified.
	 */
	int send(IReaderWriter *writer) const;

	/**
	 *	@brief Send header and body in one write
	 *
	 *	The HttpHeaderBase will generate an HTTP-style header from the
	 *	list of fields the user has specified (parsed) and write the
	 *	header to the IReaderWriter specified, along with the specified data
	 *  
	 *  @note This requires making a copy of the supplied data!
	 */
	int send(IReaderWriter *writer, const uint8_t* data, int len) const;


	/**
	 *	@brief Generate and transmit HTTP-style header
	 *
	 *	The HttpHeaderBase will generate an HTTP-style header from the
	 *	list of fields the user has specified (parsed) and write the
	 *	header to the IReaderWriter specified.
	 */
	int sendto(Socket *writer, const Socket::Address& addr) const;
	
	
	/**
	 *	@brief Retrieve string value of a field
	 *
	 *	This method retrives the string value associated with the
	 *	field specified.
	 *
	 *	@return		Errors:			NULL
	 */
	const char *getField( FieldMap::FieldType type ) const;
	
	/**
	 *	@brief Retrieve int value of a field
	 *
	 *	This method retrieves the int value associated with the
	 *	field specified.
	 *
	 *	@return		Errors:			JetHead::kNotFound
	 *	@return		Success:		JetHead::kNoError
	 */
	JetHead::ErrCode getFieldInt( FieldMap::FieldType type,
								  int32_t &value ) const;
	
	/**
	 *	@brief Retrieve int64 value of a field
	 *
	 *	This method retrieves the int value associated with the
	 *	field specified.
	 *
	 *	@return		Errors:			JetHead::kNotFound
	 *	@return		Success:		JetHead::kNoError
	 */
	JetHead::ErrCode getFieldInt64( FieldMap::FieldType type,
									int64_t &value ) const;
	
	/** 
	 *  @brief Remove a field from a fieldmap
	 *
	 *  This method removes the first instance of a given field from
	 *  the fieldmap.
	 *
	 *  @return     Errors:         JetHead::kNotFound
	 *  @return     Success:        JetHead::kNoError
	 */
	JetHead::ErrCode removeField(FieldMap::FieldType type);

	/**
	 *	@brief Add a field to the header (for output)
	 *
	 *	Given the field ID and some data to set as the value of that
	 *	header, add a new header field.
	 *
	 *	@return	true if successful, false if too many fields
	 */	
	bool addField( FieldMap::FieldType type, const char *data );

	/**
	 *	@brief Add a field to the header (for output)
	 *
	 *	Given the field ID and an int value for that field, add a new
	 *	header field.
	 *
	 *	@return	true if successful, false if too many fields
	 */	
	bool addFieldDecimal( FieldMap::FieldType type, int32_t data );

	/**
	 *	@brief Add a field to the header (for output)
	 *
	 *	Given the field ID and an int64 value for that field, add a new
	 *	header field.
	 *
	 *	@return	true if successful, false if too many fields
	 */	
	bool addFieldDecimal64(FieldMap::FieldType type, int64_t data);

	/**
	 *	@brief Add a field to the header (for output)
	 *
	 *	Given the field ID and an int value for that field, add a new
	 *	header field in hex
	 *
	 *	@return	true if successful, false if too many fields
	 */	
	bool addFieldHex( FieldMap::FieldType type, uint32_t data );
	
	
	/**
	 *	@brief Parse first line of an HTTP-style header
	 *
	 *	This method should be implemented to do something useful by
	 *	specialized HttpHeaderBase children.   When an Agent receives
	 *	an HTTP-style request it would call this method to parse the
	 *	first line of the header and then make additional calls to
	 *	parseLine() until the end of the header was detected.
	 *
	 *	NOTE:  Circular buffer data is not consumed by this call,
	 *	the caller is responsible for updating the circular buffer
	 *	after completing a call to this method by the number of bytes
	 *	returned.
	 *
	 *	@return Success: Number of bytes parsed to retrieve first line
	 */
	virtual int parseFirstLine( const CircularBuffer &buf ) { return 0; }
	
	/**
	 *	@brief Parse field from an HTTP-style header
	 *
	 *	This method is called to parse and retrieve a field from an
	 *	HTTP-style header.   It will parse a single field and, using the
	 *	set of FieldMaps that the HttpHeaderBase has available to make
	 *	sense of it, any unknown fields are dropped on the floor.
	 *
	 *	NOTE:  Circular buffer data is not consumed by this call,
	 *	the caller is responsible for updating the circular buffer
	 *	after completing a call to this method by the number of bytes
	 *	returned.
	 *
	 *	@return		Success:		Number of bytes parsed to retrieve field
	 */
	virtual int parseLine( const CircularBuffer &buf );
	
	/**
	 * the return value for the end of header condition returned
	 * by the searchForLine method
	 */
	static const int kEOH = 0;
	
	/**
	 * the expected number of characters need to represent HTTP status
	 * code values
	 */
	static const int kStatusCodeSize = 16;
	
	/**
	 *	@brief Search for a line in the circular buffer
	 *
	 *	This helper method will search for the end of an HTTP style
	 *	header line in the circular buffer and return the length of the
	 *	line if one is found.
	 *
	 *	@return			-1		Indicates no field lines found in buffer
	 *	@return			kEOH	If a field line of 0 length is found
	 *	@return			length	Number of bytes parsed in the field line
	 */
	static int searchForLine(const CircularBuffer &buffer);
	
	//! Print out this header (to stdout)
	void printHeader();

	//! Get back a copy of the assembled header
	const char *getHeader();
protected:

	/**
	 *	@brief Add field mapping
	 *
	 *	Add a new field mapping to the HttpHeaderBase.  The new mapping will
	 *	be used during parse/generation of the HTTP-style header and cleaned
	 *	up when the HttpHeaderBase is destroyed
	 *
	 *	@see HttpHeader
	 */
	int addFieldMap(FieldMap *fieldMap);

	//! Return the length of the header
	int getLength() const { return mHeaderStr.size(); }

	/**
	 *	@brief Generate first line of an HTTP-style header
	 *
	 *	This method should be implemented to do something useful by
	 *	specialized HttpHeaderBase children.   Data should be affixed
	 *	to the beginning of the mBuffer member and the number of
	 *	bytes consumed returned (this includes CR+LF)
	 *
	 */
	virtual int buildFirstLine() const { return 0; }

	/**
	 * The buffer where the HTTP-style header is generated to before
	 * being written to the IReaderWriter in the send method.
	 */
	mutable JHSTD::string mHeaderStr;

private:

	/**
	 * Structure to store mappings of FieldMap::FieldType to string
	 * data values
	 */
	struct HeaderField 
	{	
		//! Default constructor, doesn't have to do much
		HeaderField() : mType(0), mData( NULL ) {}

		//! Free up the space we've allocated
		~HeaderField() { delete [] mData; }
		
		//! Set the data for this field
		void setData( const char *data );

		//! Set the field type for this field 
		void setType( FieldMap::FieldType field ) { mType = field; }
		
		//! The field type
		FieldMap::FieldType mType;

		//! The value for this field
		char *mData;
	};
	
	//! Look up a field ID by name
	FieldMap::FieldType getFieldType(const char *field) const;

	//! Look up a field string by ID
	const char *getFieldTypeString(FieldMap::FieldType type) const;
	
	//! Build the header into mHeaderStr
	void buildHeader() const;

	//! The initial reserved size for the message
	static const int kDefaultMessageSize = 2048;
	
	//! The maximum number of fields in this header
	static const int kMaxFields = 32;
	
	//! The fields
	HeaderField	mFields[ kMaxFields ];

	//! How many fields have we used
	int 		mNumFields;

	//! The mapping from field names to FieldType 
	JetHead::list<FieldMap*> mFieldMappings;
};

#endif // HTTPHEADER_BASE_H_

