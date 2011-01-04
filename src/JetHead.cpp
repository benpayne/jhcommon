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

#include "JetHead.h"
#include "logging.h"	

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_INFO );

using namespace JetHead;

static const char *gErrorStrings[] = {
	"No Error",
	"Not Found",
	"Connection Failed",
	"Permission Denied",
	"Resource Busy",
	"Resource Temporarily Busy, Try Again",
	"Operation In Progress",
	"Resource Already Requested",
	"Operation Timed Out",
	"Operation Not Implemented",
	"Open of resource failed",
	"Invalid Parameter in request",
	"Resource not initialized",
	"Read Failed",
	"Write Failed",
	"Resource Full",
	"Unknown Error",
	"Connection Closed",
	"Resource Not Empty",
	
	// keep in sync with JetHead.h
	"Max Error String",
};

const char *JetHead::getErrorString( ErrCode err )
{
	// protect against enum bounds
	if ( (int)err < 0 || (int)err >= kMaxErrorString )
		return NULL;

	return gErrorStrings[ (int)err ];
}


JetHead::ErrCode	JetHead::getErrorCode( int _errno )
{
	JetHead::ErrCode err = JetHead::kNoError;
	
	switch( errno )
	{
	case EACCES:
	case EPERM:
	case EROFS:
		err = kPermissionDenied;
		break;
	case EEXIST:
		err = kAlreadyRequested;
		break;
	case EISDIR:
	case ENOTDIR:
	case EFAULT:
	case EBADF:
	case EINVAL:
		err = kInvalidRequest;
		break;
	case EFBIG:
	case EOVERFLOW:
		err = kNotImplemented;
		break;
	case ENFILE:
	case EMFILE:
	case ETXTBSY:
	case EWOULDBLOCK:
	//case EAGAIN: same value as EWOULDBLOCK
		err = kBusy;
		break;
	case ENOMEM:
		err = kTemporarilyBusy;
		break;
	case ENOENT:
	case ENODEV:
		err = kNotFound;
		break;
	case ENXIO:
		err = kNotInitialized;
		break;
	case ENOSPC:
		err = kFull;
		break;
	case EPIPE:
		err = kConnectionClosed;
		break;
	case ENOTEMPTY:
		err = kNotEmpty;
		break;
	case EIO:
	default:
		err = kUnknownError;
		break;
	}
	
	return err;
}

