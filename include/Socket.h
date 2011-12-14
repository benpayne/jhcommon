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

#ifndef JH_SOCKET_H_
#define JH_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "Selector.h"
#include "File.h"
#include "vector.h"

class Socket;
class ServerSocket;

//! A socket listener gets a little more information about IO events than most
class SocketListener
{
public:
	/**
	 * @brief Notify when a socket connected with connectAsync is connected
	 *
	 * This method is called when a socket connected with connectAsync has
	 * completed its connection.
	 * 
	 * @param       socket      [in]        Socket that has connected	 
	 * @param       success     [in]        Did the connection succeed?
	 */
	virtual void handleConnected(Socket *socket, bool success) {};

	/**
	 *	@brief Notify when socket has data available
	 *
	 *	This method is called when data arrives and needs to be processed
	 *	by the user for a Socket.
	 *
	 *	@param		socket		[in]		Socket that data arrived on
	 */
	virtual void handleData(Socket *socket) = 0;

	/**
	 *	@brief Notify when socket close occurs from other side
	 *
	 *	This method is called when a TCP socket detects that the other
	 *	side of the connection has closed.   The user has the choice of
	 *	returning false and letting the Socket close itself or returning
	 *	true and handling the close and cleanup of the Socket.
	 *
	 *	@param		socket		[in]		Socket that closed
	 *	@return		true		User will cleanup socket and can safely
	 *							delete the Socket class.
	 *	@return		false		Socket will close itself.
	 */
	virtual bool handleClose(Socket *socket) { return false; }
	
	
	/**
	 *	@brief Notify when a ServerSocket receives a new connection
	 *
	 *	This method is called to notify the listener that the ServerSocket
	 *	specified has accepted a new connection.   Returning true indicates
	 *	that you wish to accept the connection and returning false indicates
	 *	that you do NOT wish to accept it.   The caller will close and free
	 *	the new socket if you return false.
	 *
	 *	Since not all SocketListener's are listening to a ServerSocket we
	 *	provide a safe default that will deny the incoming connection
	 *
	 *	@param		server		[in]		ServerSocket that received new connect
	 *	@param		socket		[in]		Socket representing new connection
	 *	@return		true		User accepts connection and now owns socket
	 *	@return		false		User denies connection and socket will be
	 *							closed and destroyed
	 */
	virtual bool handleAccept(ServerSocket *server, Socket *socket)
	{
		return handleAccept(socket);
	}
	
	
	/**
	 *	@brief Notify when a ServerSocket recieves a new connection
	 *
	 *	This method is DEPRECATED and new code should implement the new
	 *	version of this method.
	 *
	 *	This method is called when a ServerSocket recieves a new connection
	 *	the socket specified is the NEW connection.   Returning true indicates
	 *	that you wish to accept the connection and returning false indicates
	 *	that you do NOT wish to accept it.   The caller will close and free
	 *	the socket if you return false.
	 *
	 *	Since not all SocketListener's are listening to a ServerSocket we
	 *	provide a safe default that will deny the incoming connection
	 *
	 *	@deprecated
	 *	@param		socket		[in]		Socket representing NEW connection
	 *	@return		true		User accepts connection
	 *	@return		false		User denies connection and the Socket
	 *							specified will be closed and destroyed
	 */
	virtual bool handleAccept(Socket *socket) { return false; }
	
protected:
	//! Virtual does-nothing destructor to avoid compiler warning
	virtual ~SocketListener() {}
};

class Socket : public SelectorListener, public IReaderWriter
{
public:
	//! A convenient representation of an (IP, port) pair
	class Address
	{
	public:		
		//! Default constructor (255.255.255.255, 0)
		Address()
		{
			mAddr.sin_family = AF_INET;
			mAddr.sin_port = 0;
			mAddr.sin_addr.s_addr = INADDR_ANY;
			mLen = sizeof( struct sockaddr_in );
		}
		
		//! Do a DNS lookup on this name, set port (or use 0 as a default)
		Address( const char *name, int port = 0 )
		{
			mAddr.sin_family = AF_INET;
			mAddr.sin_port = htons( port );
			setAddress( name );
			mLen = sizeof( struct sockaddr_in );			
		}			

		//! Set from an in_addr
		Address( struct in_addr addr, int port = 0 )
		{
			mAddr.sin_family = AF_INET;
			mAddr.sin_port = htons( port );
			mAddr.sin_addr = addr;
			mLen = sizeof( struct sockaddr_in );
		}

		//! Set the port, but not the IP address
		explicit Address( int port )
		{
			mAddr.sin_family = AF_INET;
			mAddr.sin_port = htons( port );						
			mAddr.sin_addr.s_addr = INADDR_ANY;
			mLen = sizeof( struct sockaddr_in );			
		}			

		//! Does nothing virtual destructor, avoid compiler warnings
		~Address() {}
		
		//! Convert in_addr to dotted-quad string
		const char *getName() const
		{
			return inet_ntop( AF_INET, &mAddr.sin_addr, mName,
							  INET_ADDRSTRLEN );
		}

		//! Get the port we are storing
		uint16_t getPort() const { return ntohs( mAddr.sin_port ); }
		
		//! Set the host (doing a DNS lookup)
		void setAddress( const char *name );

		//! Set the port, performs endian conversion
		void setPort( uint16_t port )
		{
			mAddr.sin_port = htons( port );						
		}
		
		//! Be explicit about our assignment operator
		Address& operator=(const Address& rhs)
		{
			mAddr.sin_family = rhs.mAddr.sin_family;
			mAddr.sin_addr.s_addr = rhs.mAddr.sin_addr.s_addr;
			mAddr.sin_port = rhs.mAddr.sin_port;
			mLen = rhs.mLen;
			return *this;
		}
		
		//! Compare for equality
		bool operator == (const Address& rhs) const
		{
			return mAddr.sin_addr.s_addr == rhs.mAddr.sin_addr.s_addr and
				mAddr.sin_port == rhs.mAddr.sin_port and
				mLen == rhs.mLen;
		}
		
	private:
		//! Initialize from a sockaddr
		Address( const struct sockaddr_in *addr, int len )
		{
			setSockaddr( addr, len );
		}

		//! Copy out of a sockaddr
		void setSockaddr( const struct sockaddr_in *addr, int len )
		{
			memcpy( &mAddr, addr, len );
			mLen = len;
		}

		//! Convert to const sockaddr*
		const struct sockaddr *getAddr( int &len ) const 
		{ 
			len = mLen; 
			return (struct sockaddr*)&mAddr; 
		}
		
		//! Our host
		struct sockaddr_in	mAddr;

		//! The size of mAddr
		int					mLen;

		//! Used to hold the value returned by getName()
		mutable char		mName[INET_ADDRSTRLEN];
		
		friend class Socket;
		friend class ServerSocket;
		friend class MulticastSocket;
	};

	//! Default constructor, uses TCP unless told otherwise
	Socket( bool sock_stream = true );

	//! Cleanup and remove listeners
	virtual ~Socket();

	//! Connect to this host/port
	int connect( const Socket::Address &addr );

	//! Connect to this host/port, unless it takes too long
	int connect( const Socket::Address &addr, int timeout );
	
	//! Connect, will trigger a call to handleConnect if/when successful
	int connectAsync( const Socket::Address& addr );

	/**
	 * Called to add this socket to a selector, when data arrives you
	 * will be informed via the listener.  Call with NULL, NULL to
	 * remove a previously setSelector.  However normally a
	 * Selector/Listener should be used for the lifetime of this
	 * object.  But if your listener is destroyed before this class
	 * you should remove it so we don't call a destroyed object.
	 */
	void setSelector( SocketListener *listener, Selector *selector );
	
	/**
	 * Read up to 'len' bytes from the socket.  This will often return
	 * less than len bytes (unlike read(2)), it is important to check
	 * the return value all the time.
	 */
	virtual int read( void *buffer, int len );

	//! Write 'len' bytes to the socket.  Return the number actually written.
	int write( const void *buffer, int len );

	//! Call recvfrom (get some data, tell me who it is from)
	int recvfrom(void* buf, int len, Socket::Address& addr, int flags=0);

	//! Call sendto (send this data to this address, doesn't work well on TCP)
	int sendto(const void* buf, int len, const Socket::Address& addr, 
			   int flags=0);

	/**
	 * 	@brief Call recvmsg (scatter-gather reads)
	 *
	 *	@param buffers vector of tuples consisting of a memory address and the
	 *	requested amount of data to receive
	 *	@param addr address to receive data from.  This field is ignored when
	 *	using a connected (TCP) socket, and required when using an unconnected
	 *	(UDP) socket.
	 *	@param flags optional recv() flags
	 *
	 *	This method allows you to make multiple recv() calls using only one
	 *	system call.
	 *
	 * 	@return	Number of bytes received
	 */	
	int recvmsg(const JetHead::vector<iovec> &buffers,
				const Socket::Address *addr = NULL, int flags=0);


	/**
	 * 	@brief Call sendmsg (scatter-gather writes)
	 *
	 * 	@param buffers vector of tuples consisting of a memory address and the
	 * 	requested amount of data to send
	 * 	@param addr address to send data to.  This field is ignored when using
	 * 	a connected (TCP) socket, and required when using an unconnected (UDP)
	 * 	socket.
	 * 	@param flags optional send() flags
	 *
	 * 	This method allows you to make multiple send() calls using only one
	 * 	system call.
	 *
	 * 	@return Number of bytes sent
	 */
	int sendmsg(const JetHead::vector<iovec> &buffers,
				const Socket::Address *addr = NULL, int flags=0);

	//! Close the socket
	int close();

	//! Shutdown the socket (see shutdown(2))
	int shutdown();
	
	/**
	 *	@brief Get bytes available from the socket
	 *
	 *	@return		Number of bytes available on the socket or -1 on error
	 */
	virtual int getBytesAvailable();
	
	/**
	 *	@brief Alias to getBytesAvailable()
	 *	@deprecated
	 */
	int peekAvailable() { return getBytesAvailable(); }

	//! Get the address of the remote side of this socket
	int getRemoteAddress( Socket::Address &addr );	

	//! Get the address of this side of this socket
	int getLocalAddress( Socket::Address &addr );

	/**
	 *	@brief Get the parent of this socket
	 *
	 *	This method returns the parent of this socket.  The socket
	 *	will return itself if it was not created by processing the
	 *	accept() method of ServerSocket.
	 */
	Socket *getParent() { return mParent; }
	
	//! Set the opaque private data associated with this socket
	void setPrivateData( uint32_t pd ) { mPrivateData = pd; }
	
	//! Get back the private data associated with this socket
	uint32_t getPrivateData() { return mPrivateData; }
	
	//! Get an address associated with this interface (like "eth0")
	static int getInterfaceAddress( const char *if_name,
									Socket::Address& addr );

	//! Get the hardware (MAC) address of this interface (like "eth0")
	//! in MAC-format - i.e. '00:1E:26:5C:FD:13'. The output buffer 
	//! supplied should be at least 20 bytes.
	static int getHardwareAddress(	const char *if_name,
									char *out_addr );

	//! Set the amount of time we are willing to wait on a read
	void setReadTimeout(int t) { mReadTimeout = t; }
	
	//! Are we TCP?  
	bool isSockStream() const { return mSockStream; }

	/**
	 * @brief Set keepalive checking
	 *
	 * Causes the socket to close in (roughly) this many
	 * seconds if the remote host becomes unavailable.
	 * 
	 * Set this to a small number (10 is good) to find out if the
	 * remote host is gone.  Set this to a larger number to keep the
	 * socket open when passing through a NAT while the socket might
	 * go idle.
	 *
	 * @note The actual timeout will happen somewhere between min(5,
	 * timeout) seconds and timeout+5 seconds after the remote host
	 * goes silent.  Thus, it is not necessarily possible to get a
	 * sub-5-second accuracy in your timeouts.
	 *
	 * @return true if OK, false if an error occurred
	 */
	bool setKeepAlive(int timeout);

	/**
	 * @brief Check the health of the socket based on the given
	 * liveness threshold
	 * 
	 * Check to see if the socket has shown healthy signs of activity
	 * within the preceeding liveness threshold (in seconds).  
	 *
	 * 1. If the socket appears healthy, or if any signs of ill health
	 * are still within the threshold, the return value will be 0.
	 *
	 * 2. If the socket shows signs of ill health, and has not shown
	 * positive signs within the threshold time, then the return value
	 * will be the positive number of seconds since the last positive
	 * sign.
	 *
	 * 3. If the attempt to get socket info fails for any reason, the
	 * return value will be -1.
	 *
	 * @note Only for use with SOCK_STREAM sockets.
	 *
	 * @return 0 for good health, greater than or equal to the
	 * threshold for observed poor health, and -1 for internal error
	 */
	int checkSocketHealth(uint32_t thresholdSeconds = 15);
	
protected:
	//! Wrap the supplied fd, which better be a socket
	Socket( int fd );

	//! Set parent socket (set by ServerSocket during accept)
	void setParent(Socket *parentSock) { mParent = parentSock; }
	
	//! Set whether or not we are connected (have called connect(2))
	void setConnected( bool state, int flags=POLLIN );

	//! Set the selector that we will be using
	void setSelector( SelectorListener *listener, Selector *selector );
	
	//! For IReaderWriter
	virtual void processFileEvents( int fd, short events, 
									uint32_t private_data );
	
	//! Find my selector
	Selector *getSelector() { return mSelector; }

	//! Find my socket listener
	SocketListener *getListener() { return mListener; }

	//! Give back the socket FD
	int getFd() { return mFd; }
		
	//! Set to true if we are a TCP socket connected with connectAsync
	bool mConnectedAsync;

	//! The listener that we will inform of IO events
	SocketListener *mListener;

private:
	//! The socket FD
	int mFd;

	/**
	 * The selector we are associated with.  Most of our processing
	 * will be done on that selector's thread.
	 */
	Selector *mSelector;

	/**
	 * If we are being used not as a Socket but as an IReaderWriter,
	 * then our behavior is a little different because of the contract
	 * on IReaderWriter.  This is the selector that gets used in that case.
	 */
	Selector *mReaderWriterSelector;

	/**
	 * If we are being used not as a Socket but as an IReaderWriter,
	 * then our behavior is a little different because of the contract
	 * on IReaderWriter.  This is the listener that gets used in that case.
	 */	
	SelectorListener *mReaderWriterListener;

	//! Have we called connect on this socket yet?
	bool mConnected;

	//! The opaque data associated with this socket
	uint32_t	mPrivateData;

	//! Are we a TCP socket?
	bool mSockStream;

	//! Pointer to parent socket (set by ServerSocket, default self)
	Socket		*mParent;
	
	/**
	 * Do we have a read timeout?  If 0, no biggie, if non-zero, wait
	 * at most that many seconds for a read.
	 */
	int mReadTimeout;

	//! The last address we read from (ONLY for UDP sockets)
	Address mLastDatagramSender;

	// This is needed so that accept can call the protected constructor 
	//  and setConnected on the new socket that it creates.  Looks like a 
	//  derived class can call protected methods only on itself.  (BP)
	friend class ServerSocket;
};

class ServerSocket : public Socket
{
public:
	//! Default to accepting TCP connections
	ServerSocket( bool sock_stream = true );

	//! Shut us down, release the port
	virtual ~ServerSocket();

	//! Bind to this address.  You probably want Socket::Address(int port)
	int bind( const Socket::Address &addr );

	//! Listen for at most this many UNACCEPTED connections
	int listen( int backlog );

	//! Block, waiting for the next incoming connection, and return that to me.
	Socket *accept();
	
protected:
	//! Handle IO from the selector
	void processFileEvents( int fd, short events, uint32_t private_data );

private:
};

#endif // JH_SOCKET_H_
