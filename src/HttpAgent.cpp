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

#include "HttpAgent.h"

#include "logging.h"

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOISE);

using namespace JetHead;

HttpAgent::HttpAgent() : mBuffer( kMaxMessageSize )
{
}

int parseChunkLine( const CircularBuffer &buf, int &len )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	char length_str[ 40 ];
	int i = 0;
	int j = 0;
	int c;

	if (!buf.getLength())
		return -1;
				
	while ( 1 )
	{
		c = buf.byteAt( i++ );	

		if ( c < 0 )
			return c;

		if ( c == ';' || c == '\r')
			break;
		length_str[ j++ ] = c;
	}

	length_str[ j ] = 0;

	while ( c != '\r' )
	{
		c = buf.byteAt( i++ );	

		if ( c < 0 )
			return c;
	}
	
	// if CRLF move past LF
	c = buf.byteAt(i);
	if ( c == '\n' )
		i++;
	else
		return -1;
		
	len = strtol( length_str, NULL, 16 );

	LOG( "string %s", length_str );
	return i;
}

int HttpAgent::get( const URI &uri, HttpResponse &res, BodyHandler *handler )
{
	HttpRequest req( uri );

	return get(req, res, handler);
}

int HttpAgent::get( HttpRequest &req, HttpResponse &res, BodyHandler *handler )
{
	return sendAndGet(req, res, "", handler);
}

int HttpAgent::sendAndGet( HttpRequest &req, HttpResponse &res, 
						   const std::string& toSend, BodyHandler* handler )
{
	URI uri = req.getURI();

	Socket::Address addr( uri.getHost().c_str(), uri.getPort() );
	bool first_line = true;
	int len;
	int ret;
	ErrCode err = kNoError;

	mSock.setReadTimeout( 5 );
	ret = mSock.connect( addr, 10 );

	if (ret != 0)
	{
		LOG_NOTICE("Socket connect failed...");
		res.setResponseCode(-1);
		return kConnectionFailed;
	}

	ret = req.send( &mSock );
	if (not toSend.empty())
	{
		if (mSock.write(toSend.c_str(), toSend.size()) != (int)toSend.size())
		{
			return kWriteFailed;
		}
	}

	mBuffer.clear();
	do
	{
		ret = mBuffer.fillFromFile(&mSock, mBuffer.getFreeSpace());

		if (ret <= 0)
		{
			LOG_NOTICE("Socket read failed...");
			return kReadFailed;
		}

		while (HttpHeaderBase::searchForLine(mBuffer) > 0)
		{
			if (first_line)
			{
				len = res.parseFirstLine(mBuffer);
				first_line = false;
			}
			else
			{
				len = res.parseLine(mBuffer);
			}

			if (len < 0)
			{
				LOG_NOTICE("Problem parsing response...");
			}
			else
			{
				mBuffer.read(NULL, len);
			}
		}
		
	} while (HttpHeaderBase::searchForLine(mBuffer) != HttpHeaderBase::kEOH or
			 mBuffer.getLength() < 2);

	ret = res.getResponseCode();

	// read \r\n from buffer
	mBuffer.read( NULL, 2 );			
	
	// Get Body of Response.

	int read_len, length = 0;	
	err = res.getFieldInt( HttpFieldMap::kFieldContentLength, length );
	if ( err == kNoError )
	{
		const char *buffer;
		int buffer_len;
		
		while ( length > 0 )
		{
			buffer = (const char *)mBuffer.getBytes( 0, buffer_len );
			
			if ( buffer_len > length )
				buffer_len = length;
			
			if ( handler != NULL )
				handler->handleData( buffer, buffer_len );
			
			mBuffer.read( NULL, buffer_len );
			length -= buffer_len;

			if (length)
			{
				ret = handler->handleSocket( mSock, length );	

				if (ret < 0)
					return kReadFailed;
				length = 0;
			}
		}
	}
	else
	{
		const char *encoding = res.getField( HttpFieldMap::kFieldTransferEncoding );

		if ( encoding != NULL && strcmp( encoding, "chunked" ) == 0 )
		{			
			while ((read_len = parseChunkLine( mBuffer, length )) <= 0)
			{
				ret = mBuffer.fillFromFile( &mSock, mBuffer.getFreeSpace() );

				if (ret < 0)
				{
					LOG_ERR("Socket read failed...");
					return kReadFailed;
				}	

				if ( mBuffer.byteAt( 0 ) == '\r' )
					mBuffer.read( NULL, 1 );
				if ( mBuffer.byteAt( 0 ) == '\n' )
					mBuffer.read( NULL, 1 );
				
			}

			mBuffer.read( NULL, read_len );
			
			while ( length > 0 )
			{
				const char *buffer;
				int buffer_len;
			
				while ( length > 0 )
				{
					buffer = (const char *)mBuffer.getBytes( 0, buffer_len );
					
					if ( buffer_len > length )
						buffer_len = length;

					if ( (handler != NULL ) && (buffer_len) )
					{
						handler->handleData( buffer, buffer_len );
					
						mBuffer.read( NULL, buffer_len );
						length -= buffer_len;
					}

					if (length)
					{
						ret = handler->handleSocket( mSock, length );	

						if (ret <= 0)
						{
							if (ret == 0)
								return kNoError;
							else
								return kReadFailed;
						}
						length = 0;
					}

				}

				ret = mBuffer.fillFromFile( &mSock, mBuffer.getFreeSpace() );

				if (ret < 0)
				{
					LOG_ERR("Socket read failed...");
					return kReadFailed;
				}

				if ( mBuffer.byteAt( 0 ) == '\r' )
					mBuffer.read( NULL, 1 );
				if ( mBuffer.byteAt( 0 ) == '\n' )
					mBuffer.read( NULL, 1 );

				while ((read_len = parseChunkLine( mBuffer, length )) <= 0)
				{
					ret = mBuffer.fillFromFile( &mSock, mBuffer.getFreeSpace() );
					if (ret < 0)
					{
						LOG_ERR("Socket read failed...");
						return kReadFailed;
					}

					if ( mBuffer.byteAt( 0 ) == '\r' )
						mBuffer.read( NULL, 1 );
					if ( mBuffer.byteAt( 0 ) == '\n' )
						mBuffer.read( NULL, 1 );
				}

				mBuffer.read( NULL, read_len );
			}
			
			err = kNoError;
		}
	}

	if ( handler != NULL )
		handler->setStop(true);

	if ( !(( ret == 200 ) || ( ret == 206)) )
	{
		LOG_NOTICE("File not found... response code = %d", ret);
		return kNotFound;
	}

	return err;
}

