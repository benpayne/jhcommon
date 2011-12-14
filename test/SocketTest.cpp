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

#include "Socket.h"
#include "jh_memory.h"
#include "logging.h"
#include <unistd.h>
#include <iostream>

using namespace std;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

class UdpSocketTest : public TestCase, public SocketListener 
{
public:
	UdpSocketTest();
	virtual ~UdpSocketTest() {}
	
private:
	void Run();

	void handleData( Socket *s );
	bool handleClose( Socket *s );
	bool handleAccept( Socket *s );
	
	int mTestState;
};

class TcpSocketTest : public TestCase, public SocketListener 
{
public:
	TcpSocketTest();
	virtual ~TcpSocketTest() {}
	
private:
	void Run();

	void handleData( Socket *s );
	bool handleClose( Socket *s );
	bool handleAccept( Socket *s );
	
	int mTestState;
	Selector *mSelector;
};

UdpSocketTest::UdpSocketTest() : TestCase( "UdpSocketTest" )
{
	SetTestName( "UDP Socket" );
}

void UdpSocketTest::Run()
{	
	LOG_NOTICE( "UDP Test Started" );

	Selector s;

	Socket sock( false );
	ServerSocket server( false );
	Socket::Address server_addr, client_addr;
	
	int res = server.bind( server_addr );

	if ( res < 0 )
		TestFailed( "Bind Failed" );
		
	res = server.getLocalAddress( server_addr );
	server_addr.setAddress( "127.0.0.1" );
	
	if ( res < 0 )
		TestFailed( "get local addr failed" );
	
	LOG_NOTICE( "Server name %s:%d", server_addr.getName(), server_addr.getPort() );
	
	server.setSelector( this, &s );
	res = sock.connect( server_addr );

	if ( res < 0 )
		TestFailed( "connect failed" );
	
	res = sock.getLocalAddress( client_addr );

	if ( res < 0 )
		TestFailed( "get local addr failed" );

	res = server.connect( client_addr );

	if ( res < 0 )
		TestFailed( "server connet failed" );
	
	uint32_t number = 1000;
	uint32_t new_number;
	
	if ( res < 0 )
		TestFailed( "connect failed" );
	
	res = sock.write( &number, 4 );

	if ( res < 0 )
		TestFailed( "write failed" );

	res = sock.read( &new_number, 4 );

	if ( res < 0 )
		TestFailed( "read failed" );

	if ( new_number != number + 1 )
		TestFailed( "Data Read failed" );
	
	number = new_number + 1;
	
	res = sock.write( &number, 4 );

	if ( res < 0 )
		TestFailed( "write failed" );

	res = sock.read( &new_number, 4 );

	if ( new_number != number + 1 )
		TestFailed( "FAILED" );
	
	if ( res < 0 )
		TestFailed( "write failed" );
	
	LOG_NOTICE( "Data sent" );
	
	mTestState = 0;
	
	sock.close();
	usleep( 100000 );
	
	TestPassed();
}

void UdpSocketTest::handleData( Socket *s )
{
	LOG_NOTICE( "called" );
	uint32_t num;
	
	int res = s->read( &num, 4 );
	
	if ( res != 4 )
		TestFailed( "Read Failed %d", res );

	num += 1;
	res = s->write( &num, 4 );

	if ( res != 4 )
		TestFailed( "Write Failed %d", res );
}

bool UdpSocketTest::handleClose( Socket *s )
{
	LOG_NOTICE( "called" );
	TestFailed( "handle close not expected on UdpSocket" );
	return false;
}

bool UdpSocketTest::handleAccept( Socket *s ) 
{
	LOG_NOTICE( "called" );
	TestFailed( "handle close not expected on UdpSocket" );
	return true;
}

TcpSocketTest::TcpSocketTest() : TestCase( "TcpSocketTest" )
{
	SetTestName( "TCP Socket" );
}

void TcpSocketTest::Run()
{	
	LOG_NOTICE( "TCP Test Started" );

	mSelector = jh_new Selector( "TcpSelector" );

	Socket sock;
	ServerSocket server;
	Socket::Address server_addr, client_addr;
	
	int res = server.bind( server_addr );

	if ( res < 0 )
		LOG_WARN_PERROR( "Bind Failed" );

	res = server.listen( 5 );

	if ( res < 0 )
		LOG_WARN_PERROR( "Listen Failed" );
	
	res = server.getLocalAddress( server_addr );
	server_addr.setAddress( "127.0.0.1" );
	
	if ( res < 0 )
		LOG_WARN_PERROR( "get local addr failed" );
	
	LOG_NOTICE( "Server name %s:%d", server_addr.getName(), server_addr.getPort() );
	
	server.setSelector( this, mSelector );
	res = sock.connect( server_addr );

	uint32_t number = 1000;
	uint32_t new_number;
	
	if ( res < 0 )
		LOG_WARN_PERROR( "connect failed" );
	
	res = sock.write( &number, 4 );

	if ( res < 0 )
		LOG_WARN_PERROR( "write failed" );

	res = sock.read( &new_number, 4 );

	if ( new_number != number + 1 )
		LOG_WARN( "FAILED" );
	
	if ( res < 0 )
		LOG_WARN_PERROR( "write failed" );

	number = new_number + 1;
	
	res = sock.write( &number, 4 );

	if ( res < 0 )
		LOG_WARN_PERROR( "write failed" );

	res = sock.read( &new_number, 4 );

	if ( new_number != number + 1 )
		LOG_WARN( "FAILED" );
	
	if ( res < 0 )
		LOG_WARN_PERROR( "write failed" );
	
	LOG_NOTICE( "Data sent" );
	
	mTestState = 0;
	
	sock.close();
	usleep( 100000 );
	
	if ( mTestState != 1 )
	{
		TestFailed( "handleClose not recieved on server" );
	}
	
	delete mSelector;
	
	TestPassed();
}

void TcpSocketTest::handleData( Socket *s )
{
	LOG_NOTICE( "called" );
	uint32_t num;
	
	int res = s->read( &num, 4 );
	
	if ( res != 4 )
		TestFailed( "Read Failed %d", res );

	num += 1;
	res = s->write( &num, 4 );

	if ( res != 4 )
		TestFailed( "Write Failed %d", res );
}

bool TcpSocketTest::handleClose( Socket *s )
{
	LOG_NOTICE( "called" );
	mTestState++;
	return false;
}

bool TcpSocketTest::handleAccept( Socket *s ) 
{
	LOG_NOTICE( "called" );
	s->setSelector( this, mSelector );
	return true;
}

int main( int argc, char*argv[] )
{	
	TestRunner runner( argv[ 0 ] );

	TestCase *test_set[ 10 ];
	
	test_set[ 0 ] = jh_new UdpSocketTest();
	test_set[ 1 ] = jh_new TcpSocketTest();
	
	runner.RunAll( test_set, 2 );

	return 0;
}
