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

#include <unistd.h>
#include <string>
#include <map>

#include "logging.h"
#include "jh_memory.h"
#include "Socket.h"
#include "Selector.h"
#include "Timer.h"

SET_LOG_CAT(LOG_CAT_ALL);
SET_LOG_LEVEL(LOG_LVL_NOTICE);

#define COMMAND_PROMT " > "

class ICommandHandler
{
public:
	virtual void handleCommand( IReaderWriter *rw, const char *cmd, const char *line ) = 0;
};

class TelnetServer : public SocketListener
{
public:
	TelnetServer();
	~TelnetServer();
	
	void setPromt( const char *str );
	void start( int port );

	void addCommand( const char *cmd, ICommandHandler *handler );

	void setDefaultHandler( ICommandHandler *handler )
	{
		mUnknownHandler = handler;
	}
	
private:
	void handleData( Socket *socket );
	bool handleAccept( ServerSocket *server, Socket *socket );
	bool handleClose( Socket *socket );
	
	struct CommandData
	{
		CommandData() : handler( NULL ), valid( false ) {}
		
		std::string command;
		std::string line;
		ICommandHandler *handler;
		bool		valid;
	};

	int fillCommandData( CommandData &data, const char *buf, int len );
	
	ServerSocket	mSocket;
	int 			mPort;
	Selector		mSelector;
	std::string		mPromt;
	ICommandHandler *mUnknownHandler;
	std::map<std::string,ICommandHandler*> mCommandMap;
};

class DefaultHandler : public ICommandHandler 
{
public:
	DefaultHandler() {}
	virtual ~DefaultHandler() {}
	
	void handleCommand( IReaderWriter *rw, const char *cmd, const char *line );
};

void DefaultHandler::handleCommand( IReaderWriter *rw, const char *cmd, const char *line )
{
	LOG_NOTICE( "command line \"%s\" %p", line, cmd );
	LOG_NOTICE( "command \"%s\"", cmd );

	if ( strcmp( cmd, "quit" ) == 0 || strcmp( cmd, "exit" ) == 0 )
	{
		rw->close();
	}
	else
	{
		char buffer[ 128 ];
		sprintf( buffer, "unknown command \"%s\"\r\n", cmd );
		rw->write( buffer, strlen( buffer ) );
	}
}

TelnetServer::TelnetServer() : mUnknownHandler( NULL )
{
}

TelnetServer::~TelnetServer()
{
}

void TelnetServer::start( int port )
{
	Socket::Address addr( port );
	mSocket.bind( addr );
	mSocket.listen( 5 );
	mSocket.setSelector( this, &mSelector );
}

void TelnetServer::setPromt( const char *str )
{
	mPromt = str;
}

void TelnetServer::addCommand( const char *cmd, ICommandHandler *handler )
{
	mCommandMap[ cmd ] = handler;
}

void TelnetServer::handleData( Socket *socket )
{
	char buffer[ 128 ];	
	int buf_len = 0;
	CommandData data;
	int buf_off = 0;
	
	do {
		buf_len = socket->read( buffer, 128 );
		
		while( buf_len > 0 )
		{
			int i = fillCommandData( data, buffer + buf_off, buf_len );
			buf_off += i;
			buf_len -= i;
			
			if ( data.valid )
			{
				if ( data.handler != NULL )
					data.handler->handleCommand( socket, data.command.c_str(), data.line.c_str() );
				else
					mUnknownHandler->handleCommand( socket, data.command.c_str(), data.line.c_str() );
			}
		}
	} while ( buf_len == 128 );
	
	socket->write( mPromt.c_str(), mPromt.length() );
}

int TelnetServer::fillCommandData( CommandData &data, const char *buf, int len )
{
	int i = 0;
	int term_cnt = 0;
	
	while( i < len && term_cnt < 2 )
	{
		switch( buf[ i ] )
		{
			case '\r':
				if ( term_cnt == 0 )
					term_cnt++;
				else
					term_cnt = 0;
				break;
			case '\n':
				if ( term_cnt == 1 )
					term_cnt++;
				else
					term_cnt = 0;
				break;
				
			default:
				term_cnt = 0;
				break;
		}
		i++;
	}
	
	// append line data to length
	data.line.append( buf, i - term_cnt );

	// if we found the end of line
	if ( term_cnt == 2 )
	{
		data.valid = true;
		int off = data.line.find( " " );
		if ( off == -1 )
			data.command = data.line;
		else
			data.command = data.line.substr( 0, off );
		
		std::map<std::string,ICommandHandler*>::iterator cur = mCommandMap.find(data.command);
		if ( cur != mCommandMap.end() )
		{
			data.handler = (*cur).second;
		}
	}

	return i;
}

bool TelnetServer::handleAccept( ServerSocket *server, Socket *socket )
{
	socket->setSelector( this, &mSelector );
	socket->write( mPromt.c_str(), mPromt.length() );
	
	LOG_NOTICE( "Starting session with client %p", socket );

	return true;
}

bool TelnetServer::handleClose( Socket *socket )
{
	LOG_NOTICE( "Session with client %p ended", socket );
	delete socket;
	return true;
}

int main( int argc, char *argv[] )
{
	TelnetServer server;
	DefaultHandler handler;
	
	server.start( 2000 );
	server.setPromt( "> " );
	server.setDefaultHandler( &handler );

	while( 1 )
	{
		sleep( 10 );
	}
	
	return 0;
}
