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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "list.h"
#include "Thread.h"
#include <sys/syscall.h>

#include "jh_types.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_DEFAULT | LOG_CAT_TRACE );
SET_LOG_LEVEL( LOG_LVL_NOTICE );


#define LOGGING_MALLOC( size )	malloc( (size) )
#define LOGGING_FREE( ptr )		free( (ptr) )

int jh_log_indent = 0;
int jh_log_indent_size = 0;

static char *jh_log_names_buffer = NULL;

static FILE *logging_file = NULL;

static FILE* logging_copy_file = NULL;
static bool logging_sync_mode = false;
static int logging_mark_num = 0;

const char* jh_log_level_names[] =
{
	"\e[31mERROR\e[0m, ",
	"\e[31mERROR\e[0m, ",
	"\e[35mWARNING\e[0m, ",
	"\e[35mWARNING\e[0m, ",
	"",
	"",
	"",
};

struct file_data
{
	const char 			*filename;
	uint32_t			*cats;
	int					*level;
};

static bool first_register = true;
static JetHead::list<file_data>* file_data_list = NULL;

static const char *global_catagory_names[] = {
	"LOG_CAT_LOCAL1",
	"LOG_CAT_LOCAL2",
	"LOG_CAT_LOCAL3",
	"LOG_CAT_LOCAL4",
	"LOG_CAT_LOCAL5",
	"LOG_CAT_LOCAL6",
	"LOG_CAT_LOCAL7",
	"LOG_CAT_LOCAL8",
	"LOG_CAT_LOCAL9",
	"LOG_CAT_LOCAL10",
	"LOG_CAT_LOCAL11",
	"LOG_CAT_LOCAL12",
	"LOG_CAT_LOCAL13",
	"LOG_CAT_LOCAL14",
	"LOG_CAT_LOCAL15",
	"LOG_CAT_LOCAL16",
	"LOG_CAT_LOCAL17",
	"LOG_CAT_LOCAL18",
	"LOG_CAT_LOCAL19",
	"LOG_CAT_LOCAL20",
	"LOG_CAT_LOCAL21",
	"LOG_CAT_LOCAL22",
	"LOG_CAT_LOCAL23",
	"LOG_CAT_LOCAL24",
	"LOG_CAT_LOCAL25",
	"LOG_CAT_LOCAL26",
	"LOG_CAT_LOCAL27",
	"LOG_CAT_LOCAL28",
	"LOG_CAT_LOCAL29",
	"LOG_CAT_LOCAL30",
	"LOG_CAT_TRACE",
	"LOG_CAT_DEFAULT",
};

static const char *global_level_names[] = {
	"LOG_LVL_ERR",
	"LOG_LVL_ERR",
	"LOG_LVL_WARN",
	"LOG_LVL_WARN",
	"LOG_LVL_NOTICE",
	"LOG_LVL_INFO",
	"LOG_LVL_NOISE",
};

int logging_lookup_level( const char *str )
{
	int i;

	TRACE_BEGIN(LOG_LVL_INFO);

	LOG( "level is %s", str );

	// if a single digit number this is the level
	if ( str[ 0 ] >= '0' && str[ 0 ] <= '9' && str[ 1 ] == '\0' )
	{
		int res = str[ 0 ] - '0';
		if ( res < 0 || res > LOG_LVL_NOISE )
			return -1;
		else
			return res;
	}
	
	for ( i = 0; i < JH_ARRAY_SIZE( global_level_names ); i++ )
	{
		if ( strcmp( global_level_names[ i ], str ) == 0 )
		{
			LOG( "found level %d", i );
			TRACE_END();
			return i;
		}
	}

	TRACE_END();

	return -1;
}

uint32_t logging_lookup_cat( const char *str )
{
	int i;

	TRACE_BEGIN(LOG_LVL_INFO);

	LOG( "cat is %s", str );

	for ( i = 0; i < JH_ARRAY_SIZE( global_catagory_names ); i++ )
	{
		if ( strcmp( global_catagory_names[ i ], str ) == 0 )
		{
			LOG( "found cat %d", i );
			TRACE_END();
			return LOG_BIT_VALUE( i );
		}
	}

	TRACE_END();

	return 0;
}

const char* logging_get_names()
{
	uint32_t size = 1;
	char* temp;

	TRACE_BEGIN(LOG_LVL_INFO);
	
	if ( jh_log_names_buffer != NULL )
		return jh_log_names_buffer;
	
	// else figure out size, make buffer and copy data into it.

	LOG( "calculating storage size" );
	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		size += strlen(i->filename) + 1;
	}

	// if no file in list yet.
	if (size == 1) 
		return NULL;

	jh_log_names_buffer = (char*)LOGGING_MALLOC( size );	
	temp = jh_log_names_buffer;

	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		strcpy( temp, i->filename );
		temp += strlen( i->filename );
		*temp = ' ';
		temp++;
	}

	temp--;
	*temp = '\0';

	return jh_log_names_buffer;
}

static struct file_data *lookup_file( const char *filename )
{
	TRACE_BEGIN(LOG_LVL_INFO);

	LOG( "file is %s", filename );
	
	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		if ( strcmp( i->filename, filename ) == 0 )
		{
			TRACE_END();
			return &(*i);
		}
	}

	TRACE_END();

	return NULL;
}

int logging_set_cats( const char *filename, uint32_t cats )
{
	int all_files = ( strcmp( "all", filename ) == 0 );
	int result = 0;
	
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG( "changing logging properties on file %s\n", filename );

	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		if ( all_files == 1 || strcmp(i->filename, filename) == 0 )
		{
			*(i->cats) = cats;
			result = 1;
		}
	}

	// Seems like everything came out OK
	TRACE_END();
	return result;
}

int logging_set_level( const char *filename, int level )
{
	int all_files = ( strcmp( "all", filename ) == 0 );
	int result = 0;
	
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG( "changing logging properties on file %s\n", filename );

	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		if ( all_files == 1 || strcmp(i->filename, filename) == 0 )
		{
			*(i->level) = level;
			result = 1;
		}
	}

	// Seems like everything came out OK
	TRACE_END();
	return result;
}

uint32_t logging_get_cats( const char *filename )
{
	struct file_data *node;
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG( "looking up category on file %s", filename );

	node = lookup_file( filename );

	if ( node == NULL )
		return 0;

	TRACE_END();
	return *node->cats;
}

int logging_get_level( const char *filename )
{
	struct file_data *node;
	TRACE_BEGIN(LOG_LVL_INFO);
	LOG( "looking up level on file %s", filename );

	node = lookup_file( filename );

	if ( node == NULL )
		return -1;

	TRACE_END();
	return *node->level;
}

void register_logging( const char *filename, uint32_t *cats, int *level )
{
	struct file_data node;

	TRACE_BEGIN(LOG_LVL_INFO);

	if ( first_register )
		logging_init();

	node.filename = filename;
	node.cats = cats;
	node.level = level;

	bool found = false;
	for (JetHead::list<file_data>::iterator i = file_data_list->begin(); 
		 i != file_data_list->end(); ++i)
	{
		if ( strcasecmp( i->filename, filename) > 0 )
		{
			i.insertBefore(node);
			found = true;
			break;
		}
	}
	
	if ( not found )
	{
		file_data_list->push_back(node);
	}

	// if we have already built the name buffer then delete it since we just 
	//  changed it.
	if ( jh_log_names_buffer != NULL )
	{
		LOGGING_FREE( jh_log_names_buffer );
		jh_log_names_buffer = NULL;
	}
	
	TRACE_END();
}

void logging_init()
{
	TRACE_BEGIN(LOG_LVL_INFO);

	if ( first_register == false )
		return;
	else
		first_register = false;

	file_data_list = jh_new JetHead::list<file_data>();

	logging_file = stdout;
	
	TRACE_END();
}

void logging_cleanup()
{
	TRACE_BEGIN(LOG_LVL_INFO);

	file_data_list->clear();

	TRACE_END();
}

void logging_set_file( FILE *new_file )
{
	logging_file = new_file;
}

void logging_set_copy_file( FILE* copy_file )
{
	logging_copy_file = copy_file;
}

void logging_set_sync_mode( bool mode )
{
	logging_sync_mode = mode;
}

void logging_sync()
{
	if ( logging_file != NULL )
		fflush( logging_file );
	if ( logging_copy_file != NULL )
		fflush( logging_copy_file );
}

void logging_mark( int num )
{
	if ( num >= 0 )
		logging_mark_num = num;

	fprintf( logging_file, "====================\n" );
	fprintf( logging_file, "====================\n" );
	fprintf( logging_file, "==== MARK %05d ====\n", logging_mark_num++ );	
	fprintf( logging_file, "====================\n" );
	fprintf( logging_file, "====================\n" );	
}

void logging_show_files( FILE *file )
{
	int j;
	int first;

	TRACE_BEGIN(LOG_LVL_INFO);

	for (JetHead::list<file_data>::iterator i = file_data_list->begin();
		 i != file_data_list->end(); ++i)
	{
		//LOG_NOISE( "node %p", node );
		fprintf( file, "%s:%s:", i->filename, global_level_names[ *i->level ] );
		first = 1;

		if ( *i->cats == 0xFFFFFFFF )
		{
			fprintf( file, "LOG_CAT_ALL" );
		}
		else
		{
			for( j = 0; j < 32; j++ )
			{
				if ( *i->cats & LOG_BIT_VALUE( j ) )
				{
					if ( first )
					{
						fprintf( file, "%s", global_catagory_names[ j ] );
						first = 0;
					}
					else
					{
						fprintf( file, ", %s", global_catagory_names[ j ] );
					}
				}
			}
		}

		fprintf( file, "\n" );
	}

	TRACE_END();
}

int function_name_fixup( const char *pretty_name, char *name, int len )
{
	const char *end = pretty_name + strlen( pretty_name ) - 1;
	const char *pos = end;
	int i = 0;
	
	// Keep track of the number of <'s and >'s we've encountered.
	int numTemplates = 0;

	while ( pos >= pretty_name )
	{
		if (*pos == ')') 
			numTemplates++;
		else if (*pos == '(') 
			numTemplates--;
		
		if ( numTemplates == 0 )
			break;
		
		pos--;		
	}
	
	end = pos;
	
	// if parse error return orig string
	if ( pos < pretty_name )
	{
		strncpy( name, pretty_name, len );
		name[ len - 1 ] = 0;
		return strlen(name);
	}
	
	// Read backward until we get to the beginning of the string, but
	// don't bail in the middle of a <> pair (from c++ templates).
	while ( pos >= pretty_name )
	{
		if (*pos == '>') 
			numTemplates++;
		else if (*pos == '<') 
			numTemplates--;
		else if ( (isspace( *pos ) && numTemplates == 0) )
			break;

		pos--;		
	}

	// Now print the string between pos and end, but skip any part that is 
	//  of a template.
	pos++;
	numTemplates = 0;
	while( i < ( len - 1 ) && pos < end )
	{
		if (*pos == '>') 
			numTemplates--;
		
		if (numTemplates == 0)
			name[ i++ ] = *pos;
		
		if (*pos == '<') 
			numTemplates++;
		
		pos++;
	}

	name[ i ] = 0;
	return i;
}

// the last to arguments in the var arg list will be the file and line number
void jh_log_print( int level, const char *function, const char *file, int line, const char *fmt, ... )
{
	int len = 0;
	char output_fmt[ JH_LOG_OUTPUT_BUF_SIZE ];
	char output_buf[ JH_LOG_OUTPUT_BUF_SIZE ];
	va_list params;

	va_start(params, fmt);

	// now, for non-syslog style output
	function_name_fixup( function, output_fmt, JH_LOG_OUTPUT_BUF_SIZE );

	len = snprintf(output_buf, JH_LOG_OUTPUT_BUF_SIZE, 
				   "[%s:%ld]%s: %s", GetThreadName(), syscall(SYS_gettid),
				   /* libc does not provide a wrapper for gettid, so it must
					* be called directly.  This should eventually be
					* abstracted out with perhaps a GetThreadId() in
					* Thread.cpp.
					*/
				   output_fmt, jh_log_level_names[ level ]);

	if (len < JH_LOG_OUTPUT_BUF_SIZE)
		len += vsnprintf(output_buf + len, JH_LOG_OUTPUT_BUF_SIZE - len,
						 fmt, params);

	if (errno && len < JH_LOG_OUTPUT_BUF_SIZE &&
		(level == LOG_LVL_ERR_PERROR || level == LOG_LVL_WARN_PERROR) )
		len += snprintf(output_buf + len, JH_LOG_OUTPUT_BUF_SIZE - len,
						": \"%s\"", strerror(errno));

	if(len < JH_LOG_OUTPUT_BUF_SIZE)
		len += snprintf(output_buf + len, JH_LOG_OUTPUT_BUF_SIZE - len,
						" %s:%d\n", file, line);

	// if we were truncated, insert a new line at the end, and double
	// check that the buffer is still null terminated
	if(len >= JH_LOG_OUTPUT_BUF_SIZE)
	{
		output_buf[JH_LOG_OUTPUT_BUF_SIZE - 2] = '\n';
		output_buf[JH_LOG_OUTPUT_BUF_SIZE - 1] = '\0';
	}

	// copy log to the copy file, if set
	if ( logging_copy_file != NULL )
	{
		fprintf( logging_copy_file, "%s", output_buf );
		if ( logging_sync_mode )
			fflush( logging_copy_file );
	}
	
	// if not using syslog, log to the primary file
	if ( logging_file != NULL )
	{
		fprintf( logging_file, "%s", output_buf );
		if ( logging_sync_mode )
			fflush( logging_file );
	}
	
	va_end(params);
}



void k_inc( char *file, int line, char *function ) {}
void k_dec( char *file, int line, char *function ) {}

void jh_null_print( int level, const char *fmt, ... )
{
	// dummy function to keep from getting warnings about params not
	// being used.
}

#define GENERIC_PRINT( fmt, args... )		\
do {									\
	if ( logging_file != NULL )			\
		fprintf( logging_file, fmt, ## args ); \
} while( 0 )

void print_buffer( const char *str, const void *data, int len )
{
#ifdef JH_VERBOSE_LOGGING
	const uint8_t *buffer = (uint8_t*)data;
	int i, buffer_i = 0;

	while ( len > 0 )
	{
		int cnt = len;
		GENERIC_PRINT( "%s: ", str );

		if ( cnt > 8 )
			cnt = 8;

		for ( i = 0; i < cnt; i++ )
		{
			GENERIC_PRINT( "0x%02X ", buffer[ buffer_i++ ] );
		}

		GENERIC_PRINT( "\n" );
		len -= cnt;
	}

	if ( logging_file != NULL && logging_sync_mode )
		fflush( logging_file );

#endif//JH_VERBOSE_LOGGING
}

void print_buffer2( const char *str, const void *data, int len )
{
#ifdef JH_VERBOSE_LOGGING
	const uint8_t *buffer = (uint8_t*)data;
	int i, buffer_i = 0;
	char line[ 100 ];
	int reset_offset = 0;
	
	int line_offset = strlen( str );
	
	if ( line_offset > 9 )
		line_offset = 9;
	
	memcpy( line, str, line_offset );
	
	line_offset += sprintf( line + line_offset, ": " );
	
	reset_offset = line_offset;

	while ( len > 0 )
	{
		int cnt = len;
	
		if ( cnt > 16 )
			cnt = 16;

		for ( i = 0; i < 16; i++ )
		{
			if ( i == 8 )
				line_offset += sprintf( line + line_offset, " " );

			if ( i < cnt )
				line_offset += sprintf( line + line_offset, "%02X ", buffer[ buffer_i++ ] );
			else
				line_offset += sprintf( line + line_offset, "   " );			
		}

		line_offset += sprintf( line + line_offset, ": " );			

		buffer_i -= cnt;
		
		for ( i = 0; i < cnt; i++ )
		{
			if ( i == 8 )
				line_offset += sprintf( line + line_offset, " " );

			if ( buffer[ buffer_i ] >= 32 && buffer[ buffer_i ] < 127 )
			{
				line_offset += sprintf( line + line_offset, "%c", buffer[ buffer_i ] );
				// if this is a % char then print a second on so that it is escaped.
				if ( buffer[ buffer_i ] == '%' )
					line_offset += sprintf( line + line_offset, "%c", buffer[ buffer_i ] );					
			}
			else
				line_offset += sprintf( line + line_offset, "." );
			
			buffer_i++;
		}

		line_offset += sprintf( line + line_offset, "\n" );			
		
		len -= cnt;		
	
		GENERIC_PRINT( "%s", line );
		line_offset = reset_offset;
	}

	if ( logging_file != NULL && logging_sync_mode )
		fflush( logging_file );
	
#endif//JH_VERBOSE_LOGGING
}

