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

#ifndef JH_APPARGS_H_
#define JH_APPARGS_H_

/** Class for parsing arguments from a command line.
 *
 * This parser will accept commands line options with a - or a -- to start with.
 *  to use you should derivce a class from this and impement the handleParam
 *  and usage virtual methods.  You class can then store the param info for
 *  others to query as needed.
 */
class AppArgs
{
public:
	AppArgs( int num_long_options = 32 );
	virtual ~AppArgs();
	
	/** 
	 * Parse the command line and call handleParam for any matches.  If any
	 *  parse error occure usage will be called.
	 */
	virtual bool Parse( int argc, const char *argv[] );

	/** 
	 * Add a option to the list.  The parse will match for eith -name or --name
	 *  also set if this option take a param and if that param is numeric.
	 *  key is used as a unique identified for the option so that you switch on
	 *  key in handleParam.
	 */
	bool AddOption( const char *name, bool takes_param, bool param_numeric, int key );

	/** 
	 * Get the index in argv of the first param that is not an option.
	 */
	int GetParamIndex() { return mParamIndex; }
	
	/**
	 * Called when an option that takes no params or a option that takes a 
	 *  string is found.  If you return false it's assumed there was a badly
	 *  formatted option and usage is called.
	 */
	virtual bool handleParam( int key, const char *param ) = 0;

	/**
	 * Called when an option that takes number is found.
	 *  If you return false it's assumed there was a badly
	 *  formatted option and usage is called.
	 */
	virtual bool handleParam( int key, int param ) = 0;

	/**
	 * Called to have you print out the usage string for your application.
	 *  This routine should probably exit the program, but that's up to you.
	 */
	virtual void usage( const char *prog_name ) = 0;
	
private:
	struct OptionData
	{
		bool numeric;
	};
	
	int mMaxNumLongOptions;
	int mNumLongOptions;
	struct option *mLongOptions;
	OptionData *mOptionData;
	int mKey;
	int mParamIndex;	
};

#endif // JH_APPARGS_H_
