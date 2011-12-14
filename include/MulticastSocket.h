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

#ifndef JH_MULTICAST_SOCKET_H_
#define JH_MULTICAST_SOCKET_H_

/**
 * @file 
 *
 * A multicast socket.  Note that multicast implies that this is a UDP
 * (datagram) socket, since it is impossible to have a 2-way
 * stream-oriented connection on a multicast group.
 */ 

#include "Socket.h"

class MulticastSocket : public ServerSocket
{
 public:
	//! Create a new multicast socket
	MulticastSocket();

	//! Destroy the socket, closes and leaves the group if needed
	~MulticastSocket();

	/**
	 * @brief Join the group (call this after bind)
	 *
	 * @note Returns the exit code of the setsockopt call
	 */
	int joinGroup(const Socket::Address& addr);

	/**
	 * @brief Join the group on this interface (call this after bind)
	 *
	 * @note Returns the exit code of the setsockopt call
	 */
	int joinGroup(const Socket::Address& addr, const Socket::Address& ifAddr);


	/**
	 * @brief Set the TTL on packets you send to this group
	 *
	 * @note Returns the exit code of the setsockopt call
	 */
	int setTTL(uint8_t ttl);

	/**
	 * @brief Set whether packets sent from here go to other sockets on this ip
	 *
	 * @note Returns the exit code of the setsockopt call
	 */
	int setLoopback(bool enable);

	/**
	 * @brief Set which interface is used for this socket
	 *
	 * @note Returns the exit code of the setsockopt call
	 * @note Packets will go out the interface corresponding to the
	 * default route unless you say otherwise with this call.
	 */
	int setInterface(const Socket::Address& addr);

	//! Set which interface is used for this socket, by name
	int setInterface(const char* ifName);

	//! Set broadcast
	int setBroadcast(bool enable);

	/**
	 * @brief Leave the group (call this after joinGroup)
	 *
	 * @note Returns the exit code of the setsockopt call
	 */
	int leaveGroup();
 private:
	//! My group address
	Socket::Address mAddr;

	//! Are we in the group?
	bool mInGroup;

};

#endif // JH_MULTICAST_SOCKET_H_
