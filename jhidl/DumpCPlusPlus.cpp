/*
 *  DumpCPlusPlus.cpp
 *  jhCommon
 *
 *  Created by Ben Payne on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "Reflect.h"
#include "DumpCPlusPlus.h"
#include "File.h"

#include "logging.h"

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

using namespace JHCOM;

static char *filenameTransform( const char *name, const char *ext )
{
	static char new_name[ 1024 ];
	
	if ( name[ 0 ] == '.' && name[ 1 ] == '/')
		name += 2;
	
	strcpy( new_name, name );
	int len = strlen( new_name );
	
	if ( new_name[ len - 4 ] == '.' &&
		new_name[ len - 3 ] == 'i' &&
		new_name[ len - 2 ] == 'd' &&
		new_name[ len - 1 ] == 'l' )
	{
		strcpy( new_name + len - 3, ext );
	}
	else
	{
		strcat( new_name, "." );
		strcat( new_name, ext );
	}
	
	return new_name;
}

void DumpCPlusPlus::findIncludes()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	File idl;
	int state = 1;  // start at new line so state is 1.
	
	LOG( "Checking includes in %s", mFilename.c_str() );
	
	int res = idl.open( mFilename.c_str() );
	ASSERT_ERR( res == 0, "Failed to open file %d", res );

	const char *data = (const char *)idl.mmap( 0, idl.getLength() );

	printf( "%s", data );
	
	for ( int i = 0; i < idl.getLength(); i++ )
	{
		switch( data[ i ] )
		{
			case '\n':
				state++;
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
						int start = line.find( "\"", 8 );
						int end = 0;
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
					}
					LOG( "Include file %s", filenameTransform( file.c_str(), "h" ) );
				}
				break;
				
			default:
				state = 0;
				break;
		}
	}
}

void DumpCPlusPlus::Dump()
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	output = fopen( filenameTransform( mFilename.c_str(), "h" ), "w+" );
	
	std::vector<SmartPtr<const TypeInfo> > types;
	mTypes->findTypesInFile( mFilename, types );
	
	LOG( "number of types %d", types.size() );
	
	findIncludes();
	
	for ( int i = 0; i < types.size(); i++ )
	{
		if ( types[ i ]->getNamespace().length() == 0 )
		{	
			dumpType( types[ i ] );
		}
	}		
}

void DumpCPlusPlus::dumpType( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	if ( type->isStruct() )
	{	
		dumpStruct( type );
	}
	else if ( type->isInterface() )
	{	
		dumpInterface( type );
	}
	else if ( type->isEnum() )
	{
		dumpEnum( type );
	}
	else
		fprintf( output, "Type: %s\n", type->getName().c_str() );
}
void DumpCPlusPlus::dumpStruct( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Field*> fields;
	fprintf( output, "struct %s {\n", type->getName().c_str() );
	type->getClass()->getFields( fields );
	for ( int i = 0; i < fields.size(); i++ )
	{
		if ( fields[ i ]->getType()->isInterface() )
			fprintf( output, "\tSmartPtr<%s> %s;\n", fields[ i ]->getType()->getName().c_str(), fields[ i ]->getName().c_str() );
		else
			fprintf( output, "\t%s %s;\n", fields[ i ]->getType()->getName().c_str(), fields[ i ]->getName().c_str() );					
	}
	fprintf( output, "};\n\n" );	
}

void DumpCPlusPlus::dumpEnum( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<std::string> values;
	fprintf( output, "enum %s {\n", type->getName().c_str() );
	type->getEnum()->getElements( values );
	for ( int i = 0; i < values.size(); i++ )
	{
		fprintf( output, "\t%s,\n", values[ i ].c_str() );
	}
	fprintf( output, "};\n\n" );	
}

void DumpCPlusPlus::dumpInterface( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Method*> methods;
	std::vector<const ClassInfo*> parents;
	
	fprintf( output, "class %s", type->getName().c_str() );
	
	type->getClass()->getParents( parents );
	
	if ( parents.size() > 0 )
		fprintf( output, " : public " );
	for ( int j = 0; j < parents.size(); j++ )
	{
		if ( j != 0 )
			fprintf( output, ", " );
		
		fprintf( output, "%s", parents[ j ]->getName().c_str() );
	}
	
	fprintf( output, "\n" );
	fprintf( output, "{\n" );
	fprintf( output, "public:\n" );
	fprintf( output, "\t%s() {}\n\n", type->getName().c_str() );
	type->getClass()->getMethods( methods );
	for ( int j = 0; j < methods.size(); j++ )
	{
		std::vector<const ParamInfo*> params;
		if ( methods[ j ]->getReturnType() == NULL )
			fprintf( output, "\tvirtual void %s( ", methods[ j ]->getName().c_str() );
		else
			fprintf( output, "\tvirtual %s %s( ", methods[ j ]->getReturnType()->getName().c_str(), methods[ j ]->getName().c_str() );
		
		methods[ j ]->getParamTypes( params );
		
		for ( int k = 0; k < params.size(); k++ )
		{
			if ( k != 0 )
				fprintf( output, ", " );
			
			if ( params[ k ]->getParamType() == ParamInfo::PARAM_IN )
			{	
				if ( params[ k ]->getType()->isInterface() )
					fprintf( output, "%s *%s", params[ k ]->getType()->getName().c_str(), params[ k ]->getName().c_str() );
				else
					fprintf( output, "%s %s", params[ k ]->getType()->getName().c_str(), params[ k ]->getName().c_str() );
			}
			else
			{	
				fprintf( output, "%s *%s", params[ k ]->getType()->getName().c_str(), params[ k ]->getName().c_str() );
			}
		}
		
		fprintf( output, " ) = 0;\n" );
	}
	
	std::vector<SmartPtr<const TypeInfo> > types;	
	mTypes->findTypesByNamespace( type->getName(), types );
	
	for ( int i = 0; i < types.size(); i++ )
	{
		dumpType( types[ i ] );
	}
	
	fprintf( output, "\nprotected:\n" );
	fprintf( output, "\t~%s() {}\n", type->getName().c_str() );
	fprintf( output, "};\n\n" );	
}

