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

#include "MulticastSocket.h"
#include "netinet/in.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );

using namespace JetHead;

MulticastSocket::MulticastSocket()
	: ServerSocket(false),
	  mInGroup(false)
{

}

//! Destroy the socket, closes and leaves the group if needed
MulticastSocket::~MulticastSocket()
{
}

/**
 * @brief Join the group (call this after bind)
 *
 * @note Returns the exit code of the setsockopt call
 */
int MulticastSocket::joinGroup(const Socket::Address& addr)
{
	return joinGroup(addr, Socket::Address());
}

int MulticastSocket::joinGroup(const Socket::Address& addr,
							   const Socket::Address& ifAddr)
{
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = addr.mAddr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = ifAddr.mAddr.sin_addr.s_addr;

	int ret = setsockopt(getFd(),
						 IPPROTO_IP,
						 IP_ADD_MEMBERSHIP,
						 &mreq,
						 sizeof(struct ip_mreq));
	if (ret)
	{
		LOG_ERR_PERROR("Multicast join failed");
	} else {
		mInGroup = true;
	}
	return ret;
}

/**
 * @brief Set the TTL on packets sent to this multicast address
 *
 * @note Returns the exit code of the setsockopt call
 */
int MulticastSocket::setTTL(uint8_t ttl)
{
	int ret = setsockopt(getFd(),
						 IPPROTO_IP,
						 IP_MULTICAST_TTL,
						 &ttl,
						 sizeof(uint8_t));
	if (ret)
	{
		LOG_ERR_PERROR("Set Multicast TTL failed");
	}
	return ret;
}

/**
 * @brief Set whether packets sent from here go to other sockets on this ip
 *
 * @note Returns the exit code of the setsockopt call
 */
int MulticastSocket::setLoopback(bool enable)
{
	uint8_t val = 0;
	if (enable) val = 1;
	int ret = setsockopt(getFd(),
						 IPPROTO_IP,
						 IP_MULTICAST_LOOP,
						 &val,
						 sizeof(val));
	if (ret)
	{
		LOG_ERR_PERROR("Set multicast loopback");
	}
	return ret;
}


/**
 * @brief Set the outgoing interface for multicast packets on this socket
 *
 * @note Returns the exit code of the setsockopt call
 * @note By default, this will go out the interface with the default route
 */
int MulticastSocket::setInterface(const Socket::Address& addr)
{
	int ret = setsockopt(getFd(),
						 IPPROTO_IP,
						 IP_MULTICAST_IF,
						 &addr.mAddr.sin_addr,
						 sizeof(struct in_addr));
	if (ret)
	{
		LOG_ERR_PERROR("Set Multicast interface failed");
	}
	return ret;
}

int MulticastSocket::setInterface(const char* ifName)
{
	Socket::Address addr;

	int ret = getInterfaceAddress(ifName, addr);

	if (ret)
		return ret;
	
	return setInterface(addr);
}

int MulticastSocket::setBroadcast(bool enable)
{
	int option = enable ? 1 : 0;
	int ret = setsockopt(getFd(),
						 SOL_SOCKET,
						 SO_BROADCAST,
						 &option,
						 sizeof(option));
	if (ret)
	{
		LOG_ERR_PERROR("Set broadcast failed");
	}
	return ret;						 
}

/**
 * @brief Leave the group (call this after joinGroup)
 *
 * @note Returns the exit code of the setsockopt call
 */
int MulticastSocket::leaveGroup()
{
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = mAddr.mAddr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	int ret = setsockopt(getFd(),
						 IPPROTO_IP,
						 IP_DROP_MEMBERSHIP,
						 &mreq,
						 sizeof(struct ip_mreq));
	if (ret)
	{
		// This seems to ALWAYS trigger, and the extraneous warning is
		// bugging people.  I'm taking out the destructor call to
		// leaveGroup() to quiet it down. - TDW
		LOG_ERR_PERROR("Multicast leave failed");
	} else {
		mInGroup = false;
	}
	return ret;
}
