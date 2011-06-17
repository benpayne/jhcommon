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
 
#ifdef G_LOG_DOMAIN
#  undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN		"jhidl"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libIDL/IDL.h>
#include <stdbool.h>

#include "Reflect.h"
#include "DumpCPlusPlus.h"
#include "DumpXML.h"
#include "File.h"
#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

using namespace JHCOM;

struct WalkData 
{
	WalkData() : filename( NULL ), 
		mTypes( new TypeManager() ), 
		mCurrentClass( NULL ), 
		mCurrentStruct( NULL ),
		mCurrentMethod( NULL ), 
		mCurrentEnum( NULL ) 
	{
	}
	
	const char *filename;
	IDL_tree tree;
	IDL_ns ns;
	TypeManager *mTypes;
	ClassInfo	*mCurrentClass;
	ClassInfo	*mCurrentStruct;
	Method		*mCurrentMethod;
	Enumeration	*mCurrentEnum;
	SmartPtr<const TypeInfo>	mType;
};


#if 0
int printType( IDL_tree tree, void *data )
{
	WalkData *info = (WalkData*)data;

	if ( tree == NULL )
	{
		fprintf( info->output, "void" );
		return 0;
	}
	
	switch( IDL_NODE_TYPE( tree ) )
	{
	case IDLN_TYPE_INTEGER:
		if ( IDL_TYPE_INTEGER( tree ).f_signed == 0 )
			fprintf( info->output, "unsigned " );
		if ( IDL_TYPE_INTEGER( tree ).f_type == IDL_INTEGER_TYPE_SHORT )
			fprintf( info->output, "short" );
		else if ( IDL_TYPE_INTEGER( tree ).f_type == IDL_INTEGER_TYPE_LONG )
			fprintf( info->output, "int" );
		else if ( IDL_TYPE_INTEGER( tree ).f_type == IDL_INTEGER_TYPE_LONGLONG )
			fprintf( info->output, "long long" );
		break;
	case IDLN_TYPE_FLOAT:
		fprintf( info->output, "double" );
		break;
	case IDLN_TYPE_CHAR:
		fprintf( info->output, "char" );
		break;
	case IDLN_TYPE_STRING:
		fprintf( info->output, "std::string" );
		break;
	case IDLN_TYPE_BOOLEAN:
		fprintf( info->output, "bool" );
		break;

	case IDLN_IDENT:
		fprintf( info->output, "%s", IDL_IDENT( tree ).str );
		break;
		
	case IDLN_TYPE_FIXED:
	case IDLN_TYPE_WIDE_CHAR:
	case IDLN_TYPE_WIDE_STRING:
	case IDLN_TYPE_OCTET:
	case IDLN_TYPE_ANY:
	case IDLN_TYPE_OBJECT:
	case IDLN_TYPE_TYPECODE:
	case IDLN_TYPE_ENUM:
	case IDLN_TYPE_SEQUENCE:
	case IDLN_TYPE_ARRAY:
	case IDLN_TYPE_STRUCT:
	case IDLN_TYPE_UNION:
	default:
		printf( "Unsupported Type %s\n", IDL_NODE_TYPE_NAME( tree ) ); 
		break;
	}
	
	return 0;
}
#endif

const TypeInfo *getType( IDL_tree tree, void *data )
{
	WalkData *info = (WalkData*)data;
	const TypeInfo *type = NULL;
	
	if ( tree == NULL )
	{
		return NULL;
	}
	
	switch( IDL_NODE_TYPE( tree ) )
	{
		case IDLN_TYPE_INTEGER:
		{	
			switch( IDL_TYPE_INTEGER( tree ).f_type )
			{
				case IDL_INTEGER_TYPE_SHORT:
					if ( IDL_TYPE_INTEGER( tree ).f_signed == 1 )
						type = info->mTypes->findType( "int16_t" );
					else
						type = info->mTypes->findType( "uint16_t" );						
					break;
				case IDL_INTEGER_TYPE_LONG:
					if ( IDL_TYPE_INTEGER( tree ).f_signed == 1 )
						type = info->mTypes->findType( "int32_t" );
					else
						type = info->mTypes->findType( "uint32_t" );						
					break;
				case IDL_INTEGER_TYPE_LONGLONG:
					if ( IDL_TYPE_INTEGER( tree ).f_signed == 1 )
						type = info->mTypes->findType( "int64_t" );
					else
						type = info->mTypes->findType( "uint64_t" );						
					break;
			}
		} break;
			
		case IDLN_TYPE_FLOAT:
		{	
			switch( IDL_TYPE_FLOAT( tree ).f_type )
			{
				case IDL_FLOAT_TYPE_FLOAT:
					type = info->mTypes->findType( "float" );
					break;
				case IDL_FLOAT_TYPE_DOUBLE:
					type = info->mTypes->findType( "double" );
					break;
				case IDL_FLOAT_TYPE_LONGDOUBLE:
					type = info->mTypes->findType( "long double" );
					break;
			}
		} break;

		case IDLN_TYPE_CHAR:
			type = info->mTypes->findType( "char" );
			break;
		case IDLN_TYPE_STRING:
			type = info->mTypes->findType( "string" );
			break;
		case IDLN_TYPE_BOOLEAN:
			type = info->mTypes->findType( "bool" );
			break;
			
		case IDLN_IDENT:
			if ( info->mCurrentClass != NULL )
				type = info->mTypes->findType( IDL_IDENT( tree ).str, info->mCurrentClass->getName() );
			else
				type = info->mTypes->findType( IDL_IDENT( tree ).str );
				
			if ( type == NULL )
				fprintf( stderr, "ERROR: Type not found %s\n", IDL_IDENT( tree ).str ); 
			break;

		case IDLN_TYPE_ANY:
			type = info->mTypes->findType( "any" );
			break;
			
		case IDLN_TYPE_FIXED:
		case IDLN_TYPE_WIDE_CHAR:
		case IDLN_TYPE_WIDE_STRING:
		case IDLN_TYPE_OCTET:
		case IDLN_TYPE_OBJECT:
		case IDLN_TYPE_TYPECODE:
		case IDLN_TYPE_ENUM:
		case IDLN_TYPE_SEQUENCE:
		case IDLN_TYPE_ARRAY:
		case IDLN_TYPE_STRUCT:
		case IDLN_TYPE_UNION:
		default:
			fprintf( stderr, "ERROR: Unsupported Type %s\n", IDL_NODE_TYPE_NAME( tree ) ); 
			break;
	}
		
	return type;
}

typedef int		(*list_node_callback)(IDL_tree node, void *data );

void processList( IDL_tree tree, list_node_callback cb, void *data )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	if (IDL_NODE_TYPE(tree) == IDLN_LIST) 
	{
		int i;
		for ( i = 0; i < IDL_list_length(tree); i++ )
		{
			IDL_tree node = IDL_LIST( IDL_list_nth( tree, i ) ).data;
			cb( node, data );
		}
	}
}

int handleInheritance( IDL_tree node, void *data )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	WalkData *info = (WalkData*)data;
	const TypeInfo *type = info->mTypes->findType( IDL_IDENT( node ).str );

	if ( type == NULL )
	{	
		fprintf( stderr, "ERROR: Failed to find type %s\n", IDL_IDENT( node ).str );
		exit( 1 );
	}

	LOG( "%p", info->mCurrentClass );
	
	info->mCurrentClass->addParent( type->getClass() );
	
	return 0;
}

const char *getLiteral( IDL_tree node )
{
	static char value[ 120 ];
	value[ 0 ] = '\0';
	
	if ( IDL_NODE_IS_LITERAL( node ) )
	{
		switch( IDL_NODE_TYPE( node ) )
		{
		case IDLN_INTEGER:
			sprintf( value, "%lld", IDL_INTEGER( node ).value );
			break;
		case IDLN_STRING:
			sprintf( value, "%s", IDL_STRING( node ).value );
			break;
		case IDLN_CHAR:
			sprintf( value, "%c", *(IDL_CHAR( node ).value) );
			break;
		case IDLN_FIXED:
			sprintf( value, "%s", IDL_FIXED( node ).value );
			break;
		case IDLN_FLOAT:
			sprintf( value, "%f", IDL_FLOAT( node ).value );
			break;
		case IDLN_BOOLEAN:
			if ( IDL_BOOLEAN( node ).value == 0 )
				sprintf( value, "false" );
			else
				sprintf( value, "true" );
			break;
		case IDLN_WIDE_STRING:
		case IDLN_WIDE_CHAR:
			printf( "Wide Char/String not supported yet\n" );
			break;
		default:
			printf( "Uknown type %s\n", IDL_NODE_TYPE_NAME( node ) );
			break;
		}
	}
	
	return value;
}

int arrayElement( IDL_tree node, void *data )
{
	return 0;
}

int enumMember( IDL_tree node, void *data )
{
	WalkData *info = (WalkData*)data;
	info->mCurrentEnum->addElement( IDL_IDENT( node ).str );
	return 0;
}

int structMemberDcl( IDL_tree node, void *data )
{
	WalkData *info = (WalkData*)data;
	
	if ( IDL_NODE_TYPE( node ) == IDLN_IDENT )
	{
		info->mCurrentStruct->addField( IDL_IDENT( node ).str, info->mType );
	}
	else if ( IDL_NODE_TYPE( node ) == IDLN_TYPE_ARRAY )
	{
		processList( IDL_TYPE_ARRAY( node ).size_list, arrayElement, data );
	}

	return 0;
}

int typedefDcl( IDL_tree node, void *data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	WalkData *info = (WalkData*)data;
	
	if ( IDL_NODE_TYPE( node ) == IDLN_IDENT )
	{
		SmartPtr<TypeInfo> type = new TypeInfo( IDL_IDENT( node ).str, new Alias( info->mType ) );
		info->mTypes->addType( type );
		type->setFileInfo( node->_file, node->_line );

		LOG( "%s", IDL_IDENT( node ).str );
	}
	else if ( IDL_NODE_TYPE( node ) == IDLN_TYPE_ARRAY )
	{
		LOG( "%s[]\n", IDL_IDENT( IDL_TYPE_ARRAY( node ).ident ).str );
		//processList( IDL_TYPE_ARRAY( node ).size_list, arrayElement, data );
	}
	
	return 0;
}

int structMember( IDL_tree node, void *data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	WalkData *info = (WalkData*)data;
	LOG( "Struct Type %p %s", node, IDL_NODE_TYPE_NAME( node ) ); 
	
	if ( IDL_NODE_TYPE( node ) == IDLN_MEMBER )
	{
		info->mType = getType( IDL_MEMBER( node ).type_spec, data );
		if ( IDL_NODE_TYPE( IDL_MEMBER( node ).dcls ) == IDLN_LIST )
			processList( IDL_MEMBER( node ).dcls, structMemberDcl, data );
		else
			structMemberDcl( IDL_MEMBER( node ).dcls, data );
	
		info->mType = NULL;
	}
	
	return 0;
}


int processTree( IDL_tree tree, void *data )
{
	TRACE_BEGIN( LOG_LVL_INFO );
		
	WalkData *info = (WalkData*)data;
	if ( tree == NULL )
		return 0;
	
	LOG_INFO( "Node Type %p %s", tree, IDL_NODE_TYPE_NAME( tree ) ); 
	
	if (IDL_NODE_TYPE(tree) == IDLN_LIST) 
	{
		processList( tree, processTree, data );
	}
	else if (IDL_NODE_TYPE(tree) == IDLN_INTERFACE )
	{
		if ( info->mCurrentClass != NULL )
			fprintf( stderr, "ERROR, can't scope interface inside another interface\n" );
		
		info->mCurrentClass = new ClassInfo( IDL_IDENT( IDL_INTERFACE(tree).ident ).str );
		SmartPtr<TypeInfo> type = new TypeInfo( 
			IDL_IDENT( IDL_INTERFACE(tree).ident ).str,
			TypeInfo::TYPE_INTERFACE, info->mCurrentClass );
		info->mTypes->addType( type );
		type->setFileInfo( tree->_file, tree->_line );
				
		if ( strcmp( tree->_file, info->filename ) == 0 )
		{
			if ( IDL_INTERFACE(tree).inheritance_spec != NULL )
			{
				processList( IDL_INTERFACE(tree).inheritance_spec, handleInheritance, data );
			}
		
			processTree( IDL_INTERFACE(tree).body, data );
			
			const char *val = IDL_tree_property_get( IDL_INTERFACE(tree).ident, "IID" );
			if ( val != NULL )
			{
				std::string v = val;
				info->mCurrentClass->setIID( v );
			}
			
			info->mCurrentClass->setConcrete();
		}

		info->mCurrentClass = NULL;
	}
	else if (IDL_NODE_TYPE(tree) == IDLN_OP_DCL )
	{
		info->mCurrentMethod = new Method( IDL_IDENT( IDL_OP_DCL(tree).ident ).str, getType( IDL_OP_DCL(tree).op_type_spec, data ) );
		
		processTree( IDL_OP_DCL(tree).parameter_dcls, data );
		
		const char *val = IDL_tree_property_get( IDL_OP_DCL(tree).ident, "async" );
		if ( val != NULL )
		{
			info->mCurrentMethod->setMethodAsync();
			LOG( "%s is async", IDL_IDENT( IDL_OP_DCL(tree).ident ).str );
		}
		
		info->mCurrentClass->addMethod( *info->mCurrentMethod );
		delete info->mCurrentMethod;
		info->mCurrentMethod = NULL;
	}
	else if ( IDL_NODE_TYPE(tree) == IDLN_PARAM_DCL )
	{
		LOG( "Param %d", IDL_PARAM_DCL(tree).attr );
		
		ParamInfo::ParamType type;
		
		switch ( IDL_PARAM_DCL(tree).attr )
		{
			case IDL_PARAM_IN:
				type = ParamInfo::PARAM_IN;
				break;
			
			case IDL_PARAM_OUT:
				type = ParamInfo::PARAM_OUT;
				break;
				
			case IDL_PARAM_INOUT:
				type = ParamInfo::PARAM_INOUT;
				break;
		}
		
		ParamInfo param( IDL_IDENT( IDL_PARAM_DCL(tree).simple_declarator ).str, type, getType( IDL_PARAM_DCL(tree).param_type_spec, data ) );
		info->mCurrentMethod->addParam( param );
	}
	else if ( IDL_NODE_TYPE(tree) == IDLN_TYPE_STRUCT )
	{
		info->mCurrentStruct = new ClassInfo( IDL_IDENT( IDL_TYPE_STRUCT(tree).ident ).str );
		SmartPtr<TypeInfo> type = new TypeInfo( 
			 IDL_IDENT( IDL_TYPE_STRUCT(tree).ident ).str,
			 TypeInfo::TYPE_STRUCT, info->mCurrentStruct );
		info->mTypes->addType( type );
		type->setFileInfo( tree->_file, tree->_line );
		if ( info->mCurrentClass != NULL )
			type->setNamespace( info->mCurrentClass->getName() );

		processList( IDL_TYPE_STRUCT( tree ).member_list, structMember, data );
	
		info->mCurrentStruct->setConcrete();
		info->mCurrentStruct = NULL;
	}
	else if ( IDL_NODE_TYPE(tree) == IDLN_TYPE_ENUM )
	{
		info->mCurrentEnum = new Enumeration();
		SmartPtr<TypeInfo> type = new TypeInfo( 
			IDL_IDENT( IDL_TYPE_ENUM(tree).ident ).str, info->mCurrentEnum );
		info->mTypes->addType( type );
		type->setFileInfo( tree->_file, tree->_line );
		if ( info->mCurrentClass != NULL )
			type->setNamespace( info->mCurrentClass->getName() );
		
		processList( IDL_TYPE_ENUM( tree ).enumerator_list, enumMember, data );

		info->mCurrentEnum = NULL;
	}
	else if ( IDL_NODE_TYPE(tree) == IDLN_TYPE_DCL )
	{
		info->mType = getType( IDL_TYPE_DCL( tree ).type_spec, data );
		LOG( "typedef %s", info->mType->getName().c_str() );
		if ( IDL_NODE_TYPE( IDL_TYPE_DCL( tree ).dcls ) == IDLN_LIST )
			processList( IDL_TYPE_DCL( tree ).dcls, typedefDcl, data );
		else
			typedefDcl( IDL_TYPE_DCL( tree ).dcls, data );
	}
	else
	{
		LOG_WARN( "Unhandled Node\n" );
	}
	
	return 0;
}

int	msg_callback(int level,
				 int num,
				 int line,
				 const char *filename,
				 const char *message)
{
	if ( level == IDL_ERROR )
	{
		printf( "ERROR (%d): %s, %s:%d\n", num, message, filename, line );
	}
	else
	{
		printf( "Warning (%d): %s, %s:%d\n", num, message, filename, line );
	}

	return 0;
}

static void findIncludes( const char *filename, std::vector<std::string> &include )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	JetHead::File idl;
	int state = 1;  // start at new line so state is 1.
	
	LOG( "Checking includes in %s", filename );
	
	int res = idl.open( filename );
	ASSERT_ERR( res == 0, "Failed to open file %d", res );

	const char *data = (const char *)idl.mmap();
	
	for ( int i = 0; i < idl.getLength(); i++ )
	{
		switch( data[ i ] )
		{
			case '\n':
				state = 1;
				break;
			
			case '#':
				if ( state == 1 )
				{
					int j = i + 8;
					while( j < idl.getLength() && data[ j ] != '\n' )
						j++;
					
					std::string line( data + i, j - i );
					std::string file;
					
					if ( line.find( "#include" ) == 0 )
					{
						unsigned start = line.find( "\"", 8 );
						unsigned end = 0;
						if ( start == std::string::npos )
						{	
							start = line.find( "<", 8 );
							ASSERT_ERR( start != std::string::npos, "Failed to parse start of include" );
							end = line.find( ">", start + 1 );
						}
						else
						{
							end = line.find( "\"", start + 1 );
						}
						
						ASSERT_ERR(  end != std::string::npos, "Failed to parse end of include" );
						
						LOG( "start = %d, end = %d", start, end );
						
						file.assign( line, start + 1, end - start - 1 );

						LOG( "Include file %s", file.c_str() );
						include.push_back( file );
					}
				}
				break;
				
			default:
				state = 0;
				break;
		}
	}
}

int main (int argc, char *argv[])
{
	TRACE_BEGIN( LOG_LVL_INFO );
	int rv;
	WalkData data;
	//unsigned long parse_flags = 0;

	IDL_check_cast_enable( TRUE );

	LOG("libIDL version %s", IDL_get_libver_string ());

	if (argc < 2) {
		fprintf (stderr, "usage: jhidl <filename> [parse flags, hex]\n");
		exit (1);
	}

	data.filename = argv[1];
	
	LOG("IDL_parse_filename");
	rv = IDL_parse_filename(
		data.filename, NULL, msg_callback, &data.tree, &data.ns,
		IDLF_VERBOSE | IDLF_PROPERTIES, IDL_WARNINGMAX);

	if (rv == IDL_ERROR) 
	{
		g_message ("IDL_ERROR");
		exit (1);	
	}
	else if (rv < 0) 
	{
		perror (data.filename);
		exit (1);
	}

	LOG( "Parse done" );
	
	/* rv == IDL_SUCCESS */

	processTree( data.tree, &data );
	
	IDL_ns_free( data.ns );
	IDL_tree_free( data.tree );

	data.mTypes->dumpTypes();

	std::vector<std::string> includes;
	
	findIncludes( data.filename, includes );
	
	DumpCPlusPlus dcpp( data.filename, data.mTypes );
	DumpXML dxml( data.filename, data.mTypes );
	
	dcpp.Dump( includes );
	dxml.Dump( includes );
	
	return 0;
}


