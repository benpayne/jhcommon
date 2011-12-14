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

#ifndef JH_ISERIALIZER_H_
#define JH_ISERIALIZER_H_

#include "IReaderWriter.h"

class ISerializer
{
public:
	virtual ~ISerializer() {}
	
	/**
	 *	@brief Send method
	 *
	 *	This method is called on an ISerializer interface to serialize
	 *	the contents of the object and write them to the IReaderWriter
	 *	specified.
	 *
	 *	@return		Success			Number of bytes written
	 *	@return		Errors			-1
	 */
	virtual int send(IReaderWriter *writer) const = 0;
	
	/**
	 *	@brief Recv method
	 *
	 *	This method is called on an ISerializer interface to read the
	 *	contents of the object from the IReaderWriter specified and
	 *	deserialize it, filling in the object fields as appropriate.
	 *
	 *	@return		Success			Number of bytes read
	 *	@return		Errors			-1
	 */
	virtual int recv(IReaderWriter *reader) = 0;
};


#endif // JH_ISERIALIZER_H_
