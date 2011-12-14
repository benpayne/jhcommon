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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "Socket.h"
#include "jh_memory.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

#ifdef PLATFORM_DARWIN
#define MSG_NOSIGNAL	0
#endif

Socket::Socket( bool sock_stream )
	:	mConnectedAsync( false ),
		mListener( NULL ),
		mFd( -1 ),
		mSelector( NULL ),
		mReaderWriterSelector( NULL ), mReaderWriterListener( NULL ),
		mConnected( false ), mPrivateData( 0 ), mSockStream( sock_stream ),
		mParent(this), mReadTimeout( 0 )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	if ( sock_stream )
		mFd = socket( AF_INET, SOCK_STREAM, 0 );
	else
		mFd = socket( AF_INET, SOCK_DGRAM, 0 );
		
	int on = 1;
	setsockopt( mFd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) );
	LOG_NOISE( "fd %d", mFd );

	if ( mFd == -1 )
		LOG_ERR_FATAL( "Failed to create socket" );
}

Socket::Socket( int fd ) : 
	mConnectedAsync( false ),
	mListener( NULL ), 
	mFd( fd ),  
	mSelector( NULL ), 
	mReaderWriterSelector( NULL ), mReaderWriterListener( NULL ), 
	mConnected( false ), mPrivateData( 0 ), mSockStream( true ),
	mParent(this), mReadTimeout( 0 )
{
	TRACE_BEGIN( LOG_LVL_INFO );
}

Socket::~Socket()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	shutdown();
	close();
}
	
int Socket::connect( const Socket::Address &addr )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	int len;
	const struct sockaddr *saddr = addr.getAddr( len );
	int res = ::connect( mFd, saddr, len );

	LOG( "connect to %s on port %d", addr.getName(), addr.getPort() );

	if ( res == 0 )
		setConnected( true );

	return res;
}

int Socket::connect( const Socket::Address &addr, int timeout )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	socklen_t len = 0;
	int len_addr = 0;
	int res = 0;
	int saveflags, ret, back_err;
	const struct sockaddr *saddr = addr.getAddr( len_addr );
	fd_set fd_w;
	struct timeval t;

	t.tv_sec = timeout;
	t.tv_usec = 0;

	saveflags=fcntl(mFd,F_GETFL,0);

	if( saveflags < 0 ) {
		return -1;
	}

	/* Set non blocking */
	if(fcntl(mFd,F_SETFL,saveflags|O_NONBLOCK)<0) {
		return -1;
	}

	/* This will return immediately */
	res = ::connect( mFd, saddr, len_addr );
	back_err=errno;

	/* restore flags */
	if(fcntl(mFd,F_SETFL,saveflags)<0) {
		return -1;
	}

	/* return unless the connection was successful or the connect is
	still in progress. */
	if(res<0 && back_err!=EINPROGRESS) {
		return -1;
	}

	FD_ZERO(&fd_w);
	FD_SET(mFd,&fd_w);

	res=select(FD_SETSIZE,NULL,&fd_w,NULL,&t);

	if(res < 0) {
		return -1;
	}

	/* 0 means it timeout out & no fds changed */
	if( res == 0 ) {
		return -1;
	}

	/* Get the return code from the connect */
	len=sizeof(ret);
	res=getsockopt(mFd,SOL_SOCKET,SO_ERROR,&ret,&len);
	if(res<0) {
		return -1;
	}

	/* ret=0 means success, otherwise it contains the errno */
	if(ret) {
		return -1;
	}

	LOG_NOTICE( "connect to %s on port %d", addr.getName(), addr.getPort() );

	if ( res == 0 )
		setConnected( true );

	return res;
}

int Socket::connectAsync( const Socket::Address &addr )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	int len_addr = 0;
	int res = 0;
	int saveflags, back_err;
	const struct sockaddr *saddr = addr.getAddr( len_addr );

	saveflags=fcntl(mFd,F_GETFL,0);

	if( saveflags < 0 ) {
		return -1;
	}

	/* Set non blocking */
	if(fcntl(mFd,F_SETFL,saveflags|O_NONBLOCK)<0) {
		return -1;
	}

	// Attempt it
	res = ::connect( mFd, saddr, len_addr );
	back_err=errno;

	/* return unless the connection was successful or the connect is
	still in progress. */
	if(res<0 && back_err!=EINPROGRESS) {
		return -1;
	}
	
	// Remember that we were connected in this fashion
	mConnectedAsync = true;

	// We need to hear POLLOUT once in order to inform us that the
	// socket has connected.  Once we get that once, we will want to
	// unregister for POLLOUT, otherwise we'll get it all the time.
	setConnected( true, POLLIN|POLLOUT );


	/* Turn off non-blocking */
	if(fcntl(mFd,F_SETFL,saveflags)<0) {
		return -1;
	}

	return 0;
}


int Socket::shutdown()
{
	if ( mFd != -1 )
	{
		return ::shutdown( mFd, SHUT_RDWR );
	}
	return -1;
}

void Socket::setSelector( SelectorListener *listener, Selector *selector )
{
	if ( mFd != -1 and mConnected )
	{
		if ( mReaderWriterSelector != NULL and mReaderWriterListener != NULL )
			mReaderWriterSelector->removeListener( mFd, mReaderWriterListener );
		mReaderWriterListener = listener;
		mReaderWriterSelector = selector;
		if ( mReaderWriterSelector != NULL and mReaderWriterListener != NULL )
			mReaderWriterSelector->addListener( mFd, POLLIN, mReaderWriterListener );
	}
	else
	{
		mReaderWriterListener = listener;
		mReaderWriterSelector = selector;
	}
}

void Socket::setSelector( SocketListener *listener, Selector *selector )
{
	mListener = listener;
	if ( mConnected )
	{
		if ( mSelector != NULL )
			mSelector->removeListener( mFd, this );
		mSelector = selector;
		if ( mSelector != NULL and mListener != NULL )
		{
			mSelector->addListener( mFd, POLLIN, this );
		}
	}
	else
		mSelector = selector;		
}

int Socket::read( void *buffer, int len )
{
	TRACE_BEGIN(LOG_LVL_NOISE);
	
	if (mReadTimeout)
	{
		int retCode = 0;
		fd_set readSet;
		struct timeval timeout;
		int numBytes = 0;

		if( mReadTimeout < 0 ) 
		{
			return -1; // invalid timeout
		}

		FD_ZERO( &readSet );
		FD_SET( ( unsigned )mFd, &readSet );

		timeout.tv_sec = mReadTimeout;
		timeout.tv_usec = 0;

		while( true ) {
			if( mReadTimeout == 0 ) 
			{
				retCode = select( FD_SETSIZE, &readSet, NULL, NULL, NULL );
			} 
			else 
			{
				retCode = select( FD_SETSIZE, &readSet, NULL, NULL, &timeout );
			}

			if( retCode == 0 ) 
			{
				return -1; // timed out
			}
			if( retCode == -1 ) 
			{
				if( errno == EINTR )
					continue;
				
				return -1; // error
			} 
			else 
			{
				break; // continue with read 
			}
		}

		// read data
		numBytes = recv( mFd, buffer, len, MSG_NOSIGNAL );

		if( numBytes < 0 ) 
		{
			return -1; // socket error;
		}

		return numBytes;
	}
	else
	{
		if (not mSockStream)
		{
			return recvfrom(buffer, len, mLastDatagramSender);
		} else {
			return ::read(mFd, buffer, len);
		}
	}
} 

int Socket::write( const void *buffer, int len )
{
	// Passing in the MSG_NOSIGNAL flag to tell the OS not to send us
	// SIGPIPE when the socket is closed during this call.  At some
	// point we might make this a member variable that can be tweaked
	// by the user, but that's interface clutter that we don't need
	// yet.
	return ::send(mFd, buffer, len, MSG_NOSIGNAL);
}

int Socket::close()
{
	int res = 0;
	if ( mFd != -1 )
	{
		setConnected( false );
		res = ::close( mFd );
		LOG_NOISE( "fd %d", mFd );
		mFd = -1;
	}
	return res;
}

// flags has a default value of POLLIN
void Socket::setConnected( bool state, int flags )
{
	LOG_INFO( "Connected %d sel %p", state, mSelector );
	
	// if we are already in this state do nothing.
	if ( mConnected == state )
		return;
	
	mConnected = state;
	if ( mSelector != NULL and mListener != NULL )
	{
		if ( mConnected )
			mSelector->addListener( mFd, flags, this );
		else
			mSelector->removeListener( mFd, this );
	}
	
	if ( mReaderWriterSelector != NULL and mReaderWriterListener != NULL )
	{
		if ( mConnected )
			mReaderWriterSelector->addListener( mFd, flags, mReaderWriterListener );
		else
			mReaderWriterSelector->removeListener( mFd, mReaderWriterListener );
	}
}

void Socket::processFileEvents( int fd, short events, uint32_t private_data )
{
	TRACE_BEGIN( LOG_LVL_NOISE );
	LOG( "events %x", events );
	bool closeDetected = false;
	bool userHandled = false;
	
	// Generally, we are only going to be listening for POLLOUT in the
	// case of async connection on a TCP socket (someone called
	// connectAsync).  In that case, setConnected would add POLLOUT to
	// our event list, and when it becomes writable (connected) we'll
	// trigger this.
	if (events & POLLOUT)
	{
		// If we WERE actually connected with connectAsync, AND
		// haven't called our listener for that yet.
		if (mConnectedAsync)
		{
			// Don't notify anyone with a handleConnected again.
			mConnectedAsync = false;

			// If we have a selector (which I guess we do since we are
			// here), then tweak the flags we are interested in.
			if (mSelector != NULL)
			{
				mSelector->removeListener(mFd, this);
				mSelector->addListener(mFd, POLLIN, this);
			}		

			if (mListener == NULL)
			{
				LOG_WARN("Listener doesn't exist");
				return;
			}

			// Figure out whether the connect succeeded or failed (not
			// strictly sure it can fail, the docs are a bit unclear).
			socklen_t len;
			int ret = 0;
			len = sizeof(ret);
			int res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &ret, &len);
			if (res < 0) {
				// Notify the listener and be done
				mListener->handleConnected(this, false);
				return;
			}

			// Notify the listener and be done
			mListener->handleConnected(this, true);
			return;
		}		
	}
	
	if (events & POLLIN)
	{
		int bytesAvail = getBytesAvailable();
		if (bytesAvail == 0)
		{
			closeDetected = true;
		}
		else
		{
			if (bytesAvail == -1)
				LOG_WARN_PERROR("Failed to get number of bytes available");
			
			if (mListener != NULL)
			{
				mListener->handleData(this);

				// We REALLY want to return here, in the case that the
				// user has called close on the socket.  Really, any
				// time that we hand off control to the listener, we
				// shouldn't assume that the socket is still valid or
				// registered with the selector.  

				// However, this isn't really a problem: if we give
				// them control and they close us, the return here is
				// great.  If they return control after consuming the
				// data, then the POLLHUP will still exist and the
				// next time through the poll we will come right back
				// here without POLLIN.  If they haven't consumed all
				// of the data, then we'll just loop until they do
				// (not our problem really.)
				return;
			}
		}
	}
	if (events & POLLHUP)
	{
		closeDetected = true;
	}
	
	
	// If we detected a close on the other end of a
	// connection based socket then inform the listener
	// and if needed close the socket
	if (closeDetected)
	{
		if (mListener != NULL)
			userHandled = mListener->handleClose(this);
		
		if (not userHandled)
			close();
	}
}


int Socket::getRemoteAddress( Socket::Address &addr )
{
	struct sockaddr_in sock_addr;
	socklen_t sock_len = sizeof( sock_addr );
	
	int res = getpeername( mFd, (struct sockaddr*)&sock_addr, &sock_len );
	
	if ( res == 0 )
	{
		addr = Socket::Address( &sock_addr, sock_len );
	}
	else
	{
		addr = Socket::Address();
		if (not mSockStream)
		{
			addr = mLastDatagramSender;
			return 0;
		}
	}
	
	return res;
}

int Socket::getLocalAddress( Socket::Address &addr )
{
	struct sockaddr_in sock_addr;
	socklen_t sock_len = sizeof( sock_addr );
	
	int res = getsockname( mFd, (struct sockaddr*)&sock_addr, &sock_len );
	
	if ( res == 0 )
	{
		addr = Socket::Address( &sock_addr, sock_len );
	}
	else
	{
		addr = Socket::Address();
	}
	
	return res;
}

int Socket::getBytesAvailable()
{
	int bytesAvail;
	
	int result = ioctl(mFd, FIONREAD, &bytesAvail);
	if (result != 0)
	{
		return -1;
	}
	return bytesAvail;
}

int Socket::recvfrom(void* buf, int len, Socket::Address& addr, int flags)
{
	return ::recvfrom(mFd, buf, len, flags, 
					  (sockaddr*)&addr.mAddr, (socklen_t*)&addr.mLen);	
}

int Socket::sendto(const void* buf, int len, const Socket::Address& addr, 
				   int flags)
{
	return ::sendto(mFd, buf, len, flags,
					(const sockaddr*)&addr.mAddr, 
					(socklen_t)addr.mLen);
}

int Socket::recvmsg(const JetHead::vector<iovec> &buffers,
					const Socket::Address *addr, int flags)
{
	msghdr msg;
	msg.msg_name = (addr) ? (void *)&addr->mAddr : NULL;
	msg.msg_namelen = (addr) ? addr->mLen : 0;
	msg.msg_iov = const_cast<iovec *>(&buffers[0]);
	msg.msg_iovlen = buffers.size();
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	return ::recvmsg(mFd, &msg, flags); 
}

int Socket::sendmsg(const JetHead::vector<iovec> &buffers,
					const Socket::Address *addr, int flags)
{
	msghdr msg;
	msg.msg_name = (addr) ? (void *)&addr->mAddr : NULL;
	msg.msg_namelen = (addr) ? addr->mLen : 0;
	msg.msg_iov = const_cast<iovec *>(&buffers[0]);
	msg.msg_iovlen = buffers.size();
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	return ::sendmsg(mFd, &msg, flags);
}

int Socket::getInterfaceAddress( const char *if_name, Socket::Address& addr )
{
	struct ifreq ifr;
	int sockfd;

	sockfd = socket( PF_INET, SOCK_DGRAM, 0 );

	if ( sockfd < 0 )
	{
		LOG_ERR_FATAL( "socket failed" );
		return -1;
	}
	
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	ifr.ifr_addr.sa_family = AF_INET;

	int res = ioctl(sockfd, SIOCGIFADDR, &ifr); 	

	if (res)
	{	
		LOG_WARN( "SIOCGIFADDR failed for %s", ifr.ifr_name );
		
		::close( sockfd );
		
		return res;
	}

	addr.setSockaddr( (struct sockaddr_in*) &(ifr.ifr_addr), 
					  sizeof(struct sockaddr_in) );
	
	LOG_INFO( "Lookup of %s got address %s", if_name, addr.getName() );
	
	::close( sockfd );
	
	return res;
}

int Socket::getHardwareAddress(const char *if_name, char *out_addr)
{
#ifdef PLATFORM_DARWIN
	return 0;
#else
	if(not if_name or not out_addr)
	{
		LOG_WARN("NULL parameters supplied");
		return -1;
	}

	// open a socket
	struct ifreq ifr;
	int sockfd;
	
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		LOG_ERR_FATAL("socket failed");
		return -1;
	}

	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ -1] = 0;

	ifr.ifr_addr.sa_family = AF_INET;

	int res = ioctl(sockfd, SIOCGIFHWADDR, &ifr);

	if(res)
	{
		LOG_WARN("failed to get hardware address");
	
		::close(sockfd);
		
		return res;
	}

	// copy the hardware address into a temporary buffer (unsigned)
	unsigned char MAC[14];
	memcpy(MAC, &ifr.ifr_ifru.ifru_hwaddr.sa_data, 14);

	// print in MAC-address format
	sprintf(out_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
				MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);

	::close(sockfd);

	return res;
#endif
}

bool Socket::setKeepAlive(int timeout)
{
#ifdef PLATFORM_DARWIN
	return false;
#else
	// How long must the connection be idle before we start the
	// keepalive checking?
	int aliveTime = 5;

	// At what interval (in seconds) do we send out addtional probes?
	int interval = 5;
	
	// The number of probes is the timeout / interval, rounded up.
	int probes = (timeout + (interval - 1)) / interval;

	// setsockopt requires pointers to values, thus lame
	int one = 1;

	// First we have to set SO_KEEPALIVE so that the socket layer
	// knows that there are keepalive options.  (The details of those
	// options are protocol specific.)
	if ( setsockopt(mFd, SOL_SOCKET, SO_KEEPALIVE, &one, 
					sizeof(one)) != 0 )
	{
		LOG_WARN_PERROR("Failed to set SO_KEEPALIVE");
		return false;
	}

	// Set the idle time
	if ( setsockopt(mFd, IPPROTO_TCP, TCP_KEEPIDLE, &aliveTime, 
					sizeof(aliveTime)) != 0 )
	{
		LOG_WARN_PERROR("Failed keepidle");
		return false;
	}

	// Set the probe interval
	if (setsockopt( mFd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, 
					sizeof(interval)) != 0 )
	{
		LOG_WARN("Failed keepintvl");
		return false;
	}

	// Set the number of probes
	if (setsockopt( mFd, IPPROTO_TCP, TCP_KEEPCNT, &probes,     
					sizeof(probes)) != 0 )
	{
		LOG_WARN("Failed keepcnt");
		return false;
	}
	
	// We win
	return true;
#endif
}

int Socket::checkSocketHealth(uint32_t thresholdSeconds)
{
#ifdef PLATFORM_DARWIN
	return 0;
#else
	// Compute the log of the thresholdSeconds for use below
	uint8_t thresholdMagnitude = 0;
	for (uint32_t i = thresholdSeconds; i > 1; i /= 2, ++thresholdMagnitude);

	struct tcp_info info;
	socklen_t info_len = sizeof(info);

	if ( getsockopt(mFd, IPPROTO_TCP, TCP_INFO, &info, &info_len) )
	{
		LOG_WARN_PERROR("failed to getsockopt TCP_INFO");
		return -1;
	}

	// For the "sending" state: see if the number of outstanding
	// retransmits currently exceeds one more than the magnitude of
	// the threshold and if the last ack we have received from the
	// client is older than the threshold
	if (info.tcpi_retransmits > thresholdMagnitude + 1 &&
		info.tcpi_last_ack_recv > thresholdSeconds * 1000)
	{
		LOG_ERR("failed for retransmits %d", info.tcpi_retransmits);
		LOG_WARN("last_ack_recv %d, backoff %d",
				 info.tcpi_last_ack_recv, info.tcpi_backoff);
		return info.tcpi_last_ack_recv / 1000;
	}
	// For the "zero-window probe" state: See if we have missed any
	// probes, and if the total of missed probes and the backoff
	// exceeds twice the magnitude of the of the threshold, and if the
	// last ack we have received from the client is older than the
	// threshold
	if (info.tcpi_probes > 0 &&
		info.tcpi_probes + info.tcpi_backoff > thresholdMagnitude * 2 &&
		info.tcpi_last_ack_recv > thresholdSeconds * 1000)
	{
		LOG_ERR("failed for probes %d", info.tcpi_probes);
		LOG_WARN("last_ack_recv %d, backoff %d",
				 info.tcpi_last_ack_recv, info.tcpi_backoff);
		return info.tcpi_last_ack_recv / 1000;
	}
	
	// Socket health looks good *or* anything bad is still under threshold
	return 0;
#endif
}

void Socket::Address::setAddress( const char *name )
{
	// Set address to default.
	mAddr.sin_addr.s_addr = INADDR_ANY;

	if ( name != NULL )
	{
		// Returns non-zero if name is a valid dotted-quad
		if (inet_aton(name, &mAddr.sin_addr))
		{
			return;
		}

		// protect calls to gethostbyname until we are certain that it
		// is threadsafe
		Mutex::EnterCriticalSection();

		struct hostent *host = gethostbyname( name );
		
		if ( host != NULL && host->h_addrtype == AF_INET )
		{
			mAddr.sin_addr.s_addr = *((uint32_t*)host->h_addr_list[ 0 ]);
		}
		
		Mutex::ExitCriticalSection();
	}
}

ServerSocket::ServerSocket( bool sock_stream ) : Socket( sock_stream )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	struct linger l = { true, 5 };

	int res = setsockopt( getFd(), SOL_SOCKET, SO_LINGER, &l, sizeof( l ) );

	if ( res != 0 )
		LOG_WARN( "Failed to set linger" );
}

ServerSocket::~ServerSocket()
{
	TRACE_BEGIN( LOG_LVL_INFO );
}

int ServerSocket::bind( const Socket::Address &addr )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	int len;
	const struct sockaddr *saddr = addr.getAddr( len );
	int res = ::bind( getFd(), saddr, len );

	// for UDP sockets set connected on bind, since listen will not be called.
	if ( res == 0 )
	{
		if ( isSockStream() == false )
			setConnected( true );
	}
	else
		LOG_WARN_PERROR( "Bind Failed on %s:%d", 
						 addr.getName(), addr.getPort() );
		
	LOG_INFO( "bind to %s on port %d", addr.getName(), addr.getPort() );
	
	return res;
}

int ServerSocket::listen( int num )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	int res = ::listen( getFd(), num );

	if ( res == 0 )
		setConnected( true );
	else
		LOG_WARN_PERROR( "Listen Failed" );
	
	return res;
}

Socket *ServerSocket::accept()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	Socket *new_sock = NULL;
	
	int res = ::accept( getFd(), NULL, NULL );

	if ( res != -1 )
	{
		new_sock = jh_new Socket( res );
		new_sock->setConnected( true );
		new_sock->setParent(this);
	}
	else
	{
		LOG_WARN_PERROR("Accept failed");
	}
	
	return new_sock;
}

void ServerSocket::processFileEvents( int fd, short events, uint32_t private_data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	if ( events & POLLIN )
	{
		if ( isSockStream() )
		{
			bool accepted = false;
			
			// Call accept to initialize the new socket
			Socket *newSock = accept();
			
			if (newSock != NULL)
			{
				// Check if the listener wishes to accept this connection
				if (mListener != NULL)
					accepted = mListener->handleAccept(this, newSock);
				
				// If the listener does not accept the new connection then
				// close and destroy it.
				if (not accepted)
				{
					newSock->close();
					delete newSock;
				}
			}
		}
		else
		{
			if ( mListener != NULL )
			{
				mListener->handleData( this );
			}
		}
	}
}

