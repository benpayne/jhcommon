/*
 *
 * Some crude tests for libIDL.
 *
 * Usage: tstidl <filename> [flags]
 *
 * if given, flags is read as (output_flags << 24) | parse_flags
 *
 * gcc `libIDL-config --cflags --libs` tstidl.c -o tstidl
 *
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
			type = info->mTypes->findType( "int" );
			break;
		case IDLN_TYPE_FLOAT:
			type = info->mTypes->findType( "float" );
			break;
		case IDLN_TYPE_CHAR:
			type = info->mTypes->findType( "int" );
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
		SmartPtr<TypeInfo> type = new TypeInfo( new Alias( IDL_IDENT( node ).str, info->mType ) );
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
		SmartPtr<TypeInfo> type = new TypeInfo( TypeInfo::TYPE_INTERFACE, info->mCurrentClass );
		info->mTypes->addType( type );
		type->setFileInfo( tree->_file, tree->_line );
				
		if ( strcmp( tree->_file, info->filename ) == 0 )
		{
			if ( IDL_INTERFACE(tree).inheritance_spec != NULL )
			{
				processList( IDL_INTERFACE(tree).inheritance_spec, handleInheritance, data );
			}
		
			processTree( IDL_INTERFACE(tree).body, data );
			
			//const char *val = IDL_tree_property_get( IDL_INTERFACE(tree).ident, "IID" );
			//if ( val != NULL )
			//	fprintf( info->output, "\n\tJHCOM_DEFINE_IID( \"%s\" );\n", val );
						
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
		SmartPtr<TypeInfo> type = new TypeInfo( TypeInfo::TYPE_STRUCT, info->mCurrentStruct );
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
		info->mCurrentEnum = new Enumeration( IDL_IDENT( IDL_TYPE_ENUM(tree).ident ).str );
		SmartPtr<TypeInfo> type = new TypeInfo( info->mCurrentEnum );
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
	
	DumpCPlusPlus d( data.filename, data.mTypes );
	
	d.Dump();
	
	return 0;
}


