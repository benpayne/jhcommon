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
#include<time.h>

using namespace std;
using namespace JetHead;

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

#include "TestCase.h"

#define BUF_SIZE 128

/* Things to know:
 * 
 * 1) If you write to a UDP ServerSocket before its been bound
 * it will never work.
 * 2) Calling write on a closed UDP socket, returns 0.
 *
 * 3) If you write to a TCP ServerSocket before its been bound
 * your program will exit with SIGPIPE.
 * 4) A TCP Socket will hang if you try and read from a closed
 * host
 *
 * 5) Because of the use of Listener scoping becomes a big issue.
 * For example, if you create a listener on the heap, you can't delete it
 * before its associated ServerSocket is either destroyed or unset. (Other,
 * wise your program will crash.)
 *
 * 6) Anything that causes the TCP tests to delay execution for more than a 
 * second (or while(true);) causes SIGALRM to be raised. Why?
 */

class UdpSocketTest : public TestCase, public SocketListener 
{
public:
    UdpSocketTest();
    virtual ~UdpSocketTest() {}
	
private:
    void Run();
    
    void test1(); // Test basic socket behavior.
    void test2(); // Test socket behavior when used incorrectly.
    void handleData( Socket *s );
    bool handleClose( Socket *s );
    bool handleAccept( Socket *s );
  
    int mTestState;
    bool handleData128;
};

// SIGALRM is thrown at the end. I believe this is because we are using
// usleep as well as the Listener.
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
    
    void test1(); // Lets play echo with the user. 
    void test2(); // Test socket behavior when used incorrectly
                  // as well as readTimeout
    void test3(); // Test basic socket behavior.
    
    bool no_echo;
    bool delay;
    int mTestState;
    Selector *mSelector;


};

UdpSocketTest::UdpSocketTest() 
: TestCase( "UdpSocketTest" ), handleData128(false)
{
	SetTestName( "UDP Socket" );
}


// Try using a socket that hasn't been created correctly
// If you write to a ServerSocket before its been bound, its worthless.
// If you write to a closed socket, the return val is 0
void UdpSocketTest::test2()
{
 
  ServerSocket server(false), server2(false);
  Socket::Address server_addr, server_addr2;
  int res;

  char buf[128];
  for (int i = 0; i < 128; i++)
    buf[i] = 'A';

  res = server.write(buf, 128);
  if( res != -1 )
    TestFailed("Writing to an unconnected socket should be an error: %d.", res);
  res = server.write(buf, 128);
  if( res != -1 )
    TestFailed("Writing to an unconnected socket should be an error: %d.", res);

  // This fails...
  res = server.bind(server_addr);
  //if ( res < 0 )
  //TestFailed( "Bind Failed" );


   res = server2.bind(server_addr2);
   if ( res < 0 )
     TestFailed( "Bind Failed" );

   for (int i = 0; i < 128; i++)
     buf[i] = 'B';

   res = server2.write(buf, 128);
   if( res != -1 )
     TestFailed("Writing to a socket before it has an address should be an \
		 erro: %d ", res);

  server2.getLocalAddress( server_addr2 );  
  server_addr2.setAddress( "127.0.0.1" );
  //cerr << "Server name " << server_addr2.getName() << " " << server_addr2.getPort()  << endl;

  for (int i = 0; i < 128; i++)
    buf[i] = 'C';

  res = server2.write(buf, 128);
  if ( res != -1 )
    TestFailed("Writing to a non-connected socket should be an error: %d.", res);



  // Create a socket: write before being connected, connect, write, read.
  Socket sock(false);
  Socket::Address sock_addr;
  for (int i = 0; i < 128; i++)
    buf[i] = 'D';


  res = sock.write(buf, 128);
  if ( res != -1 )
    TestFailed("Writing to an unconnected socket should be an error: %d.", res);

  Socket::Address test_addr("127.0.0.1", 6001);
  res = sock.connect(test_addr);
  if( res < 0 )
    TestFailed("Connect failed %d.", res);

  sock.getLocalAddress( sock_addr );
  //cerr << "Sock name " << sock_addr.getName() << " " << sock_addr.getPort()  << endl;
  
  res = sock.write(buf, 128);
  if( res < 0 )
    TestFailed("Write failed %d.", res);


  // This will fail unless there is an other socket connected to this one
  // nc -u -p 6001 127.0.0.1 <sock_addr.getPort()>
  res = sock.read(buf, 128);
  if( res != -1)
    TestFailed("reading from a socket that is connected to nothing should fail %d.", res);

  res = sock.close();
  if( res < 0 )
    TestFailed("Close failed: %d.", res);
  
  res = sock.write(buf, 128);
  if ( res != -1 )
    TestFailed("Writing to a closed socket should be an error: %d.", res);
  
  res = sock.write(buf, 128);
  if ( res != -1 )
    TestFailed("Writing to a closed socket should be an error: %d.", res);
 
}


// This tests basic socket behavior
void UdpSocketTest::test1()
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
    
    struct timespec t;
    t.tv_sec = 1;
    nanosleep( &t, NULL );
    
    // TestPassed();
}

void UdpSocketTest::Run()
{
  test1();
  test2();
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




TcpSocketTest::TcpSocketTest() 
  : TestCase( "TcpSocketTest" ), no_echo(false), delay(false)
{
	SetTestName( "TCP Socket" );
}

// If one increases the sleep time and uncomments the line
// that displays the server port they can uses
// 'nc localhost <server_addr.getPort()>' they can play echo.
// 
void TcpSocketTest::test1()
{

  no_echo = true;
  Selector s( "TcpSelector" );
  Socket sock;
  ServerSocket server;
  ServerSocket server2;
  Socket::Address server_addr, server_addr2, client_addr;
  int res;
  char buf[BUF_SIZE];
  for( int i = 0; i < BUF_SIZE; i++)
    buf[i] = 'A';
  
  
  
  // This crashes the program. 
  // res = server.write(buf, BUF_SIZE);
  //if( res != -1 )
  //TestFailed("Writing no where should fail");
  
  
  
  res = server.bind(server_addr);
  if( res == -1 )
    TestFailed("Bind Failed.");
  
  // res = server.write(buf, BUF_SIZE);
  //if( res != -1 )
  //TestFailed("Writing no where should fail");
  
  res = server.listen(1);
  if ( res < 0 )
    TestFailed("ListenFailed");
  
  //res = server.write(buf, BUF_SIZE);
  //if( res != -1 )
  //  TestFailed("Writing no where should fail");
  
  res = server.getLocalAddress( server_addr );
  server_addr.setAddress( "127.0.0.1" );
  
  // UNCOMMENT TO PALY ECHO cerr << "Server name " <<  server_addr.getName() << " "<<  server_addr.getPort() << endl;
  
  server.setSelector( this, mSelector );

  //res = sock.connect( server_addr );
  //cout << "In echo mode." << endl;
  
  struct timespec t,tt;
  t.tv_sec = tt.tv_sec = tt.tv_nsec = 0;
  t.tv_nsec = 10000000;
  res = nanosleep( &t, &tt );
 
  
  
  no_echo = false;
  //cout << "Out of echo mode." << endl;

    
}

// Tests readTimeout as well as other behavior when a socket is used
// incorrectly.
void TcpSocketTest::test2()

{
        delay = true;
        Socket sock, sock2, sock3, sock4;
	ServerSocket server;
	Socket::Address server_addr, client_addr;
	
	int res = server.bind( server_addr );

	if ( res < 0 )
		LOG_WARN_PERROR( "Bind Failed" );

	res = server.listen( 1 );

	if ( res < 0 )
		LOG_WARN_PERROR( "Listen Failed" );
	
	res = server.getLocalAddress( server_addr );
	server_addr.setAddress( "127.0.0.1" );
	
	if ( res < 0 )
		LOG_WARN_PERROR( "get local addr failed" );
	
	LOG_NOTICE( "Server name %s:%d", server_addr.getName(), server_addr.getPort() );
	

	server.setSelector( this, mSelector );
	res = sock.connect( server_addr );
	if (res < 0)
	  TestFailed("connect failed: %d.", res);

	res = sock2.connect( server_addr );
	if (res < 0)
	  TestFailed("connect failed: %d.", res);
	
	res = sock2.connect(server_addr);
	if (res != -1)
	  TestFailed("connect should only work once: %d.", res);

	res = sock4.connect(server_addr);
	if (res < 0)
	  TestFailed("connect failed: %d.", res);
	
	

	uint32_t number = 1000;
	uint32_t new_number;

	res = sock4.write( &number, 4 );
	
	if ( res < 0 )
	  TestFailed( "write failed" );
	
	//sock4.shutdown();
	sock2.close();

	
	
	sock.setReadTimeout(1);

	if ( res < 0 )
		LOG_WARN_PERROR( "connect failed" );

	res = sock.write( &number, 4 );
	
	if ( res < 0 )
		LOG_WARN_PERROR( "write failed" );


	// The goal was to test to make sure listen works
	// but it appears that tcp just waits it out, so this doesn't
	// work
	//	res = sock2.write( &number, 4 );
	//if ( res > 0 )
	//TestFailed("Server accepts too many connections: %d.", res);


	// the timeout is set smaller than the delay
	// therefore this fail.

	res = sock.read( &new_number, 4 );
	if ( res > 0 )
		LOG_WARN_PERROR( "timeout failed" );


	mTestState = 0;

	sock.close();
	
	
       	
	sock3.setReadTimeout(0);

	res = sock3.connect( server_addr );
	if (res < 0)
	  TestFailed("connect failed: %d.", res);
	number = 1002;
	res = sock3.write(&number, 4);
	//	cerr << res << endl;
	res = sock3.write(&number, 4);
	//cerr << res << endl;

	
	// The goal here was to test the timeour feature of
	// the socket, but since we can't sleep for a sufficent
	// legnth of time we are sol.
	//	res = sock3.read(&new_number, 4);
	//if ( res > 0 )
	//TestFailed("setReadTiemout ignored");



	
	delay = false;

	server.setSelector((SocketListener*)NULL, NULL);
}

// Test basic socket behavior.
void TcpSocketTest::test3()
{	
 
 
	LOG_NOTICE( "TCP Test Started" );


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
	
	//cerr << "Server name " <<  server_addr.getName() << " "<<  server_addr.getPort() << endl;
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

	
	// This call causses handleAccept to get called.
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

	struct timespec t, tt;
	t.tv_sec = tt.tv_sec = tt.tv_nsec = 0;
	t.tv_nsec = 2000000;
	
	nanosleep( &t, &tt );
	

	if ( mTestState != 1 )
	{
		TestFailed( "handleClose not recieved on server" );
	}

	// Need to unset selecot before we nuke it. Otherwise
	// it gets unset on destruction and thats causes a crash.
	server.setSelector((SocketListener*)NULL, NULL);

}

void TcpSocketTest::Run()
{

  mSelector = jh_new Selector( "TcpSelector" );
  test1();
  test2();
  test3();

  delete mSelector;
  TestPassed();

}

void TcpSocketTest::handleData( Socket *s )
{
  //cerr << "data" << endl;
	LOG_NOTICE( "called" );
	uint32_t num;
	
	// if no_echo is true, you can use nc to play
	// around.
	if(no_echo)
	{
	  char buf[BUF_SIZE+1];
	  for( int i = 0; i < BUF_SIZE+1; i++)
	    buf[i] = 0;
	  
	  int res = s->read(buf, BUF_SIZE);
	  //cerr << buf << endl;

	  res = s->write(buf, BUF_SIZE);
	  //cerr << "Bytes sent: " << res << endl;
       
	  return;
	}

	int res = s->read( &num, 4 );
	
	if ( res != 4 )
		TestFailed( "Read Failed %d", res );
	//cerr << num << endl;
	num += 1;

	// Set delay to ture, to test timeout stuff
	if (delay)
	{
	  struct timespec t;
	  t.tv_sec = 0;
	  t.tv_nsec = 200000;
	  nanosleep( &t, NULL );
	}

	res = s->write( &num, 4 );

	if ( res != 4 )
		TestFailed( "Write Failed %d", res );
}


bool TcpSocketTest::handleClose( Socket *s )
{
  //cerr << "close" << endl;
	LOG_NOTICE( "called" );
	
	mTestState++;
	return false;
}

bool TcpSocketTest::handleAccept( Socket *s ) 
{
  //cerr << "accept" <<endl;
  LOG_NOTICE( "called" );
  // Do this becaue setSelector has different behavior
  // depending on if we are connected or not.
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
