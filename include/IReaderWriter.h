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

#ifndef JH_IREADERWRITER_H_
#define JH_IREADERWRITER_H_

#include "Selector.h"
#include "JetHead.h"
#include "jh_types.h"

namespace JetHead
{
	/**
	 * @brief A Generic interface to reading and writing to file like things.  
	 * 
	 *
	 */
	class IReaderWriter
	{
	public:
		virtual ~IReaderWriter() {}
		
		virtual int read(void *buffer, int length) = 0;
		virtual int write(const void *buffer, int length) = 0;
		virtual ErrCode close() = 0;
		
		/**
		 *	@brief writeAll helper method
		 *
		 *	This helper method can be called to write all the bytes specified.
		 *	Unlike the normal write method which can return a positive value
		 *	less than length, this method will make sure that all of the bytes
		 *	are written, only returning when that is complete, or an error is
		 *	encountered.
		 */
		virtual int writeAll(const void *buffer, int length)
		{
			int bytesWritten = 0;
			
			while (bytesWritten < length)
			{
				int result = write((uint8_t*)buffer + bytesWritten,
								   length - bytesWritten);
				if (result < 0)
					return result;
	
				bytesWritten += result;
			}
			return bytesWritten;
		}
		
		/**
		 *	@brief readAll helper method
		 *
		 *	This helper method can be called to read all the bytes
		 *	specified.  Unlike the normal read method which can return a
		 *	postive value less than the length specified (without reaching
		 *	EOF), this method will make sure to read all of the bytes
		 *	requested (up to EOF), only returning when that is completed
		 *	or an error is encountered or EOF is reached.
		 *  
		 *  @return length on success, < 0 on failure, and between 0 and
		 *  length on EOF assuming that any bytes were read before EOF
		 */
		virtual int readAll(void *buffer, int length)
		{
			int bytesRead = 0;
			
			while (bytesRead < length)
			{
				int result = read((uint8_t*)buffer + bytesRead,
								  length - bytesRead);
				
				if (result < 0)
					return result;
				
				if (result == 0)
					return bytesRead;
				
				bytesRead += result;
			}
			return bytesRead;
		}
	};
}; // namespace JetHead

#endif // JH_IREADERWRITER_H_
