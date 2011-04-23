/*
 *  DumpXML.cpp
 *  jhCommon
 *
 *  Created by Ben Payne on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "Reflect.h"
#include "DumpXML.h"
#include "File.h"
#include "Path.h"

#include "logging.h"

#include <string.h>
#include <stdio.h>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

using namespace JHCOM;
using namespace JetHead;

void DumpXML::pushIndent()
{
	mIndentLevel += 1;
	mIndent.assign( mIndentLevel, '\t' );
}

void DumpXML::popIndent()
{
	mIndentLevel -= 1;
	mIndent.assign( mIndentLevel, '\t' );
}

void DumpXML::Dump( std::vector<std::string> includes )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	
	Path p( mFilename );	
	std::string new_name = p.fileBasename() + ".jci";
	
	output = fopen( new_name.c_str(), "w+" );
		
	std::vector<SmartPtr<const TypeInfo> > types;
	mTypes->findTypesInFile( mFilename, types );
	
	LOG( "number of types %d", types.size() );
		
	for ( unsigned i = 0; i < types.size(); i++ )
	{
		if ( types[ i ]->getNamespace().length() == 0 )
		{	
			dumpType( types[ i ] );
		}
	}		
}

void DumpXML::dumpType( const TypeInfo *type )
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
	else if ( type->isAlias() )
	{
		dumpAlias( type );
	}
	else
		fprintf( output, "Type: %s\n", type->getName().c_str() );
}

void DumpXML::dumpStruct( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Field*> fields;
	fprintf( output, "%s<struct>\n", mIndent.c_str() );
	pushIndent();
	fprintf( output, "%s<name>%s</name>\n", mIndent.c_str(), type->getName().c_str() );

	type->getClass()->getFields( fields );
	for ( unsigned i = 0; i < fields.size(); i++ )
	{
		fprintf( output, "%s<field><type>%s</type><name>%s</name></field>\n", mIndent.c_str(), fields[ i ]->getType()->getName().c_str(), fields[ i ]->getName().c_str() );
	}
	popIndent();
	fprintf( output, "%s</struct>\n", mIndent.c_str() );	
}

void DumpXML::dumpEnum( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<std::string> values;
	fprintf( output, "%s<enum><name>%s</name>\n", mIndent.c_str(), type->getName().c_str() );
	type->getEnum()->getElements( values );
	pushIndent();
	for ( unsigned i = 0; i < values.size(); i++ )
	{
		fprintf( output, "%s<element>%s</element>\n", mIndent.c_str(), values[ i ].c_str() );
	}
	popIndent();
	fprintf( output, "%s</enum>\n", mIndent.c_str() );	
}

void DumpXML::dumpAlias( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	fprintf( output, "%s<alias><type>%s</type><name>%s</name>\n", mIndent.c_str(), 
		getTypeName( type->getAlias()->getType() ).c_str(), 
		type->getName().c_str() );
}

std::string DumpXML::getTypeName( const TypeInfo *type )
{
	std::string name;
	
	switch( type->getType() )
	{
	default:
		name = type->getName();
		break;
	}
	
	return name;
}

void DumpXML::dumpInterface( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Method*> methods;
	std::vector<const ClassInfo*> parents;
	
	fprintf( output, "%s<class>\n", mIndent.c_str() );
	pushIndent();
	fprintf( output, "%s<name>%s</name>\n", mIndent.c_str(), type->getName().c_str() );
	
	type->getClass()->getParents( parents );
	
	for ( unsigned j = 0; j < parents.size(); j++ )
	{
		fprintf( output, "%s<parent>%s</parent>\n", mIndent.c_str(), parents[ j ]->getName().c_str() );
	}

	if ( type->getClass()->getIID() != "" )
		fprintf( output, "%s<iid>%s</iid>\n", mIndent.c_str(), type->getClass()->getIID().c_str() );
	
	type->getClass()->getMethods( methods );
	for ( unsigned j = 0; j < methods.size(); j++ )
	{
		std::vector<const ParamInfo*> params;
		std::string method_type = "sync";
		if ( methods[ j ]->isAsync() )
			method_type = "async";

		fprintf( output, "%s<method type=\"%s\">\n", mIndent.c_str(), method_type.c_str() );
		pushIndent();
		fprintf( output, "%s<name>%s</name>\n", mIndent.c_str(), methods[ j ]->getName().c_str() );
		
		if ( methods[ j ]->getReturnType() != NULL )
			fprintf( output, "%s<returnType>%s</returnType>\n", mIndent.c_str(), getTypeName( methods[ j ]->getReturnType() ).c_str() );
		
		methods[ j ]->getParamTypes( params );
		
		for ( unsigned k = 0; k < params.size(); k++ )
		{
			switch( params[ k ]->getParamType() )
			{
			case ParamInfo::PARAM_IN:
				fprintf( output, "%s<param dir=\"in\">\n", mIndent.c_str() );
				break;
			case ParamInfo::PARAM_OUT:
				fprintf( output, "%s<param dir=\"out\">\n", mIndent.c_str() );
				break;
			case ParamInfo::PARAM_INOUT:
				fprintf( output, "%s<param dir=\"inout\">\n", mIndent.c_str() );
				break;
				break;
			}
			
			pushIndent();
			fprintf( output, "%s<type>%s</type>\n", 
				mIndent.c_str(), 
				getTypeName( params[ k ]->getType() ).c_str() );
			fprintf( output, "%s<name>%s</name>\n", 
				mIndent.c_str(), 
				params[ k ]->getName().c_str() );
			
			popIndent();
			fprintf( output, "%s</param>\n", mIndent.c_str() );			
		}
		
		popIndent();
		fprintf( output, "%s</method>\n", mIndent.c_str() );
	}
	
	std::vector<SmartPtr<const TypeInfo> > types;	
	mTypes->findTypesByNamespace( type->getName(), types );

	for ( unsigned i = 0; i < types.size(); i++ )
	{
		dumpType( types[ i ] );
	}

	popIndent();
	fprintf( output, "%s</class>\n", mIndent.c_str() );
}

