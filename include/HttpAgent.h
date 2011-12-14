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

#ifndef HTTPAGENT_H_
#define HTTPAGENT_H_

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "CircularBuffer.h"
#include "Socket.h"

#define TEST_BUFFER_SIZE 64*1024

class BodyHandler
{
public:
	BodyHandler() : mStop( false ) {} 
	virtual ~BodyHandler() {}
	virtual void handleData( const char *buf, int len ) = 0;
	virtual int handleSocket( Socket &sock, int len ) = 0;
	virtual void setStop( bool stop ) { mStop = stop; }
	
protected:
	// Derived classes may implement handleData and/or handleSocket
	// to prematurely terminate handling of data.
	bool mStop;
};

class FileBodyHandler : public BodyHandler
{
public:
	FileBodyHandler( File &f ) : mFile( f ) {}
	virtual ~FileBodyHandler() {}
	
	void handleData( const char *buf, int len )
	{
		mFile.write( buf, len );
	}
	
	int handleSocket( Socket &sock, int len )
	{
		char buf[TEST_BUFFER_SIZE];
		int total_read = 0;
		
		while (len)
		{
			int read = 0;
			int attempt_read = TEST_BUFFER_SIZE;

			if (len < TEST_BUFFER_SIZE)
				attempt_read = len;

			read = sock.read(buf, attempt_read);

			if (read < 0)
			{
				printf("ERROR READING = %d\n", read);
				break;
			}

			mFile.write( buf, read );

			len -= read;
			total_read += read;
		}
		
		return total_read;
	}
	
private:
	File &mFile;
};

class HttpAgent
{
public:
	HttpAgent();
	~HttpAgent() {}
	
	int get( const URI &uri, HttpResponse &res, BodyHandler *handler = NULL );
	int get( HttpRequest &req, HttpResponse &res, BodyHandler *handler = NULL );
	int sendAndGet( HttpRequest &req, HttpResponse &res, 
					const std::string& toSend,
					BodyHandler* handler = NULL );
	
protected:
	static const int kMaxMessageSize = 2048;
	
private:
	CircularBuffer	mBuffer;
	Socket			mSock;
};


#endif // HTTPAGENT_H_
