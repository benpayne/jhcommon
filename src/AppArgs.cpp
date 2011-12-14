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

#include <getopt.h>
#include <string>

#include <jh_types.h>
#include "jh_memory.h"
#include "AppArgs.h"

/*class AppArgs
{
public:
	AppArgs( int num_long_options = 0 );
	
	void Parse( int argc, char *argv[] );

	void AddLongOption( const char *name, int number_params, int key );
	void AddOption( const char letter, bool takes_param );

	virtual void handleParam( int key, const char *param ) = 0;
	virtual void usage( const char *prog_name ) = 0;
	
private:
	int mMaxNumLongOptions;
	int mNumLongOptions;
	struct option *mLongOptions;
	const char mShortOtions[ 1024 ];
};
*/

AppArgs::AppArgs( int num_long_options ) : 
	mMaxNumLongOptions( num_long_options ), 
	mNumLongOptions( 0 ), mParamIndex( -1 )
{
	mLongOptions = jh_new struct option[ num_long_options + 1 ];
	mOptionData = jh_new OptionData[ num_long_options ];
}

AppArgs::~AppArgs()
{
	delete[] mLongOptions;
	delete[] mOptionData;
}

bool AppArgs::Parse( int argc, const char *argv[] )
{
	int opt, opt_index;
	char* end;
	const char *optstring;
	
	mLongOptions[ mNumLongOptions ].name = NULL;
	mLongOptions[ mNumLongOptions ].has_arg = 0;
	mLongOptions[ mNumLongOptions ].flag = NULL;
	mLongOptions[ mNumLongOptions ].val = 0;

	optstring = "";
	optind = 0;
	
	do
	{
		opt = getopt_long_only( argc, const_cast<char * const*>( argv ), optstring, mLongOptions, &opt_index );

		switch (opt)
		{
			case 0:
				if ( opt_index < mNumLongOptions && mOptionData[ opt_index ].numeric )
				{
					int val = strtol( optarg, &end, 0 );
					if ( *end != 0 )
					{
						usage( argv[ 0 ] );
						return false;
					}
					if ( handleParam( mKey, val ) == false )
					{
						usage( argv[ 0 ] );
						return false;
					}	
				}
				else
				{
					if ( handleParam( mKey, optarg ) == false )
					{
						usage( argv[ 0 ] );
						return false;
					}
				}
				break;
			
			case -1:
				mParamIndex = optind;
				break;
						
			case ':':
			case '?':
			default:
				usage( argv[ 0 ] );
				return false;				
		}
	} while( opt >= 0 );

	return true;
}

bool AppArgs::AddOption( const char *name, bool takes_param, bool param_numeric, int key )
{
	if ( mNumLongOptions < mMaxNumLongOptions )
	{
		mLongOptions[ mNumLongOptions ].name = name;
		mLongOptions[ mNumLongOptions ].has_arg = takes_param;
		mLongOptions[ mNumLongOptions ].flag = &mKey;
		mLongOptions[ mNumLongOptions ].val = key;
		mOptionData[ mNumLongOptions++ ].numeric = param_numeric;
		return true;
	}
	else
		return false;
}

