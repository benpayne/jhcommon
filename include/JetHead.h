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

#ifndef JETHEAD_H_
#define JETHEAD_H_

namespace JetHead
{
	// This is intended for generic error message.  Please try and reuse
	//  these error codes as much as possible.  If you feel a new error code
	//  is needed please make a request Ben Payne before adding. 
	enum ErrCode {
		kNoError = 0,
		kNotFound,			// object not found
		kConnectionFailed,	// connection attempt failed
		kPermissionDenied,	// not sufficent persmission for operation
		kBusy,				// resource busy, action needed to free.
		kTemporarilyBusy,	// resource busy, but will soon be free.
		kInProgress,		// request in progress
		kAlreadyRequested,	// resource already added/requested.
		kTimedOut,			// operation timed out.
		kNotImplemented,	// operation not implemented.
		kOpenFailed,		// open of a file or device failed.
		kInvalidRequest,	// the parameters specificed in the request are not valid.
		kNotInitialized, 	// the device or resource has not been initialized yet
		kReadFailed, 		// an in progress read failed
		kWriteFailed,       // a write failed
		
		// This constant immediately follows the last string-decodable
		// JetHead ErrCode.  It is used to protect
		// JetHead::getErrorString from an out-of-bounds lookup.
		// Keep this enum in sync with gErrorStrings in JetHead.cpp.
		kMaxErrorString,		

		// If you have a application specific error that just doesn't fit with
		//  a generic error then you can add that to your class and use this as
		// the first ID.
		kFirstUserErrorCode = 1000 
	};

	/**
	 * Get a easy to read message associate with an error code.
	 */
	const char *getErrorString( ErrCode );
};


#endif // JETHEAD_H_

