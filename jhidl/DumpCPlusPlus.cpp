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
#include "Path.h"

#include "logging.h"

#include <string.h>
#include <stdio.h>
#include <fstream>

SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOISE );

using namespace JHCOM;
using namespace JetHead;
using namespace std;

DumpCPlusPlus::DumpCPlusPlus( const char *filename, TypeManager *types ) : 
	mFilename( filename ), mTypes( types ), mIndentLevel( 0 ),
	usesString( false ), usesInt( false ), usesSmartPtr( false )	
{
}

void DumpCPlusPlus::pushIndent()
{
	mIndentLevel += 1;
	mIndent.assign( mIndentLevel, '\t' );
}

void DumpCPlusPlus::popIndent()
{
	mIndentLevel -= 1;
	mIndent.assign( mIndentLevel, '\t' );
}

void DumpCPlusPlus::Dump( std::vector<std::string> includes )
{
	TRACE_BEGIN( LOG_LVL_INFO );

	Path p( mFilename );
	std::string new_name = p.fileBasename() + ".h";
	
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

	fprintf( output, "#include \"JHCOM_IDL.h\"\n" );
	
	for ( unsigned i = 0; i < includes.size(); i++ )
	{
		if ( includes[ i ] != "ISupports.idl" )
		{
			Path inc( includes[ i ] );
			std::string inc_str = inc.fileBasename() + ".h";
			fprintf( output, "#include \"%s\"\n", inc_str.c_str() );
		}
	}
	
	if ( usesInt )
		fprintf( output, "#include \"jh_types.h\"\n" );
	if ( usesSmartPtr )
		fprintf( output, "#include \"RefCount.h\"\n" );
	if ( usesString )
		fprintf( output, "#include <string>\n" );
		
	fprintf( output, "\n%s", mBody.str().c_str() );

	fclose( output );
	
	for ( unsigned i = 0; i < types.size(); i++ )
	{
		if ( types[ i ]->isInterface() && types[ i ]->getNamespace().length() == 0 )
		{	
			dumpCpp( types[ i ] );
		}
	}
}

void DumpCPlusPlus::dumpMethod( std::ostream &out, const Method *method, const TypeInfo *type )
{
	std::vector<const ParamInfo*> params;
	if ( method->getReturnType() == NULL )
		out << "void ";
	else
	{
		if ( method->getReturnType()->isInterface() )
			out << getTypeName( method->getReturnType() ) << " *";
		else
			out << getTypeName( method->getReturnType() ) << " ";
	}
	
	if ( type != NULL )
		out << type->getName() << "::" << method->getName() << "(";
	else
		out << method->getName() << "(";
		
	method->getParamTypes( params );
	
	for ( unsigned k = 0; k < params.size(); k++ )
	{
		if ( k != 0 )
			out << ", ";
		else
			out << " ";
			
		// TODO: need to deal with const properly and pass objects like string by ref
		if ( params[ k ]->getParamType() == ParamInfo::PARAM_IN )
		{	
			if ( params[ k ]->getType()->isInterface() || params[ k ]->getType()->isStruct() )
				out << getTypeName( params[ k ]->getType() ) << " *" << params[ k ]->getName();
			else if ( params[ k ]->getType()->isString() )
				out << "const " << getTypeName( params[ k ]->getType() ) << " &" << params[ k ]->getName();
			else
				out << getTypeName( params[ k ]->getType() ) << " " << params[ k ]->getName();
		}
		else
		{	
			if ( params[ k ]->getType()->isPrimitive() )
				out << getTypeName( params[ k ]->getType() ) << " &" << params[ k ]->getName();
			else
				out << getTypeName( params[ k ]->getType() ) << " *" << params[ k ]->getName();
		}
	}

	if ( params.size() > 0 )
		out << " ";

	out << ")";
}

void DumpCPlusPlus::dumpStubBody( std::ostream &out, const Method *method, int method_num, const TypeInfo *type )
{
	std::vector<const ParamInfo*> params;

	out << mIndent << "mConn->reset();" << endl;
	
	method->getParamTypes( params );	
	for ( unsigned k = 0; k < params.size(); k++ )
	{
		out << mIndent << "mConn->push" << getConnectorName( params[ k ]->getType() ) << "( ";
		out << params[ k ]->getName();
		out << " );" << endl;
	}

	out << mIndent << "mConn->call( " << method_num << " );" << endl;

	for ( unsigned k = 0; k < params.size(); k++ )
	{
		if ( params[ k ]->getParamType() & ParamInfo::PARAM_OUT )
		{
			out << mIndent << params[ k ]->getName() << " = mConn->getParam" << getConnectorName( params[ k ]->getType() ) << "( " << k << " );" << endl;
		}
	}

	if ( method->getReturnType() != NULL )
		out << mIndent << "return mConn->getParam" << getConnectorName( method->getReturnType() ) << "( -1 );" << endl;	
}

void DumpCPlusPlus::dumpStubClass( std::ostream &out, const TypeInfo *type )
{
	std::vector<const Method*> methods;

	out << "class " << type->getClass()->getName() << "_Stubs : public " << type->getClass()->getName() << endl;
	out << "{" << endl;
	out << "public:" << endl;
	pushIndent();

	out << mIndent << type->getClass()->getName() << "_Stubs( IConnector *conn ) : mConn( conn ) {}" << endl;
	out << endl;
	
	type->getClass()->getMethods( methods );
	for ( unsigned j = 0; j < methods.size(); j++ )
	{
		
		out << mIndent;
		dumpMethod( out, methods[ j ], NULL );
		out << endl;
		out << mIndent << "{" << endl;
		pushIndent();
		dumpStubBody( out, methods[ j ], j, type );	
		popIndent();
		out << mIndent << "}" << endl;
		out << endl;
	}

	out << mIndent << "JHCOM_DECL_ISUPPORTS1( " << type->getClass()->getName() << " )" << endl;
	out << endl;
	out << "protected:" << endl;
	out << mIndent << "virtual ~" << type->getClass()->getName() << "_Stubs() { delete mConn; }" << endl;
	out << endl;
	out << "private:" << endl;
	out << mIndent << "IConnector *mConn;" << endl;
	
	popIndent();
	
	out << "};" << endl << endl;
	
	out << type->getName() << "* " << type->getName() << "::GetStubs( JHCOM::IConnector *conn )" << endl;
	out << "{" << endl;
	pushIndent();
	out << mIndent << "return jh_new " << type->getName() << "_Stubs( conn );" << endl;
	popIndent();
	out << "}" << endl << endl;
}

void DumpCPlusPlus::dumpInvokerClass( std::ostream &out, const TypeInfo *type )
{
	std::vector<const Method*> methods;
	
	out << "class " << type->getClass()->getName() << "_Invoker : public IInvoker" << endl;
	out << "{" << endl;
	out << "public:" << endl;
	pushIndent();

	out << mIndent << type->getClass()->getName() << "_Invoker( " << type->getClass()->getName() << " *obj ) : mObj( obj ) {}" << endl;
	out << endl;
	
	out << mIndent << "void call( int method_num, std::vector<Variant> &in_params, Variant *ret_value )" << endl;
	out << mIndent << "{" << endl;
	pushIndent();
	out << mIndent << "switch ( method_num )" << endl;
	out << mIndent << "{" << endl;

	type->getClass()->getMethods( methods );
	for ( unsigned j = 0; j < methods.size(); j++ )
	{		
		out << mIndent << "case " << j << ":" << endl;
		pushIndent();
		
		out << mIndent;
		if ( methods[ j ]->getReturnType() != NULL )
			out << "ret_value->set" << getVariantName( methods[ j ]->getReturnType() ) << "( ";

		out << "mObj->" << methods[ j ]->getName() << "(";
		
		std::vector<const ParamInfo*> params;
		methods[ j ]->getParamTypes( params );
	
		for ( unsigned k = 0; k < params.size(); k++ )
		{
			if ( k != 0 )
				out << ",";
			
			out << " in_params[ " << k << " ].get";
			if ( ( params[ k ]->getParamType() & ParamInfo::PARAM_OUT ) && params[ k ]->getType()->isNumber() )
				out << getVariantName( params[ k ]->getType() ) << "Ref()";
			else
				out << getVariantName( params[ k ]->getType() ) << "()";
		}

		if ( params.size() > 0 )
			out << " )";
		else
			out << ")";
		
		if ( methods[ j ]->getReturnType() != NULL )
			out << " )";

		out << ";" << endl;
		
		out << mIndent << "break;" << endl;
		popIndent();
	}

	out << mIndent << "}" << endl;
	popIndent();
	out << mIndent << "}" << endl << endl;
	
	out << "protected:" << endl;
	out << mIndent << "virtual ~" << type->getClass()->getName() << "_Invoker() {}" << endl;
	out << endl;
	out << "private:" << endl;
	out << mIndent << "SmartPtr<" << type->getClass()->getName() << "> mObj;" << endl;	
	popIndent();
	out << "};" << endl << endl;
	
	out << "IInvoker *" << type->getName() << "::GetInvoker( " << type->getClass()->getName() << " *obj )" << endl;
	out << "{" << endl;
	pushIndent();
	out << mIndent << "return jh_new " << type->getName() << "_Invoker( obj );" << endl;
	popIndent();
	out << "}" << endl << endl;
}

void DumpCPlusPlus::dumpCpp( const TypeInfo *type )
{
	Path p( mFilename );
	std::string inc_name = p.fileBasename() + ".h";
	std::string filename = type->getName() + ".cpp";
	std::fstream fs( filename.c_str(), ios_base::out | ios_base::trunc );

	mIndentLevel = 0;
		
	fs << "#include \"" << inc_name << "\"" << endl;
	fs << "#include \"jh_memory.h\"" << endl << endl;

	fs << "using namespace JHCOM;" << endl << endl;
	
	dumpStubClass( fs, type );
	dumpInvokerClass( fs, type );
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
	else if ( type->isAlias() )
	{
		dumpAlias( type );
	}
	else
		fprintf( output, "Type: %s\n", type->getName().c_str() );
}

void DumpCPlusPlus::dumpStruct( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Field*> fields;
	mBody << mIndent << "struct " << type->getName() << " {" << endl;
	type->getClass()->getFields( fields );
	pushIndent();
	for ( unsigned i = 0; i < fields.size(); i++ )
	{
		if ( fields[ i ]->getType()->isInterface() )
		{
			mBody << mIndent << "SmartPtr<" << getTypeName( fields[ i ]->getType() ) << "> " << fields[ i ]->getName() << ";" << endl;
			usesSmartPtr = true;
		}
		else
			mBody << mIndent << getTypeName( fields[ i ]->getType() ) << " " <<  fields[ i ]->getName() << ";" << endl;					
	}
	popIndent();
	mBody << mIndent << "};" << endl << endl;	
}

void DumpCPlusPlus::dumpEnum( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<std::string> values;
	mBody << mIndent << "enum " << type->getName() << " {" << endl;
	type->getEnum()->getElements( values );
	pushIndent();
	for ( unsigned i = 0; i < values.size(); i++ )
	{
		mBody << mIndent << values[ i ].c_str() << "," << endl;
	}
	popIndent();
	mBody << mIndent << "};" << endl << endl;
}

void DumpCPlusPlus::dumpAlias( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	mBody << mIndent << "typedef " << getTypeName( type->getAlias()->getType() ) << " " << type->getName() << ";" << endl;
}

std::string DumpCPlusPlus::getVariantName( const TypeInfo *type )
{
	std::stringstream name;
	
	switch( type->getType() )
	{
	case TypeInfo::TYPE_STRING:
		name << "String";
		break;

	case TypeInfo::TYPE_INTEGER:
		if ( type->getBitWidth() == 8 )
			name << "Char";
		else if ( type->isSigned() )
			name << "Int" << type->getBitWidth();
		else
			name << "UInt" << type->getBitWidth();
		break;

	case TypeInfo::TYPE_FLOAT:
		if ( type->getBitWidth() == 64 )
			name << "Double";
		else
			name << "Float";
		break;
	
	case TypeInfo::TYPE_INTERFACE:
		name << "Object";
		break;
		
	default:
		name << type->getName();
		break;
	}
	
	return name.str();
}

std::string DumpCPlusPlus::getConnectorName( const TypeInfo *type )
{
	return getVariantName( type );
}

std::string DumpCPlusPlus::getTypeName( const TypeInfo *type )
{
	std::string name;
	
	switch( type->getType() )
	{
	case TypeInfo::TYPE_ANY:
		name = "void *";
		break;
		
	case TypeInfo::TYPE_STRING:
		name = "std::string";
		usesString = true;
		break;

	case TypeInfo::TYPE_INTEGER:
		usesInt = true;	
		name = type->getName();
		break;

	default:
		name = type->getName();
		break;
	}
	
	return name;
}

void DumpCPlusPlus::dumpInterface( const TypeInfo *type )
{
	TRACE_BEGIN( LOG_LVL_INFO );
	std::vector<const Method*> methods;
	std::vector<const ClassInfo*> parents;
	
	mBody << mIndent << "class " <<  type->getName();
	
	type->getClass()->getParents( parents );
	
	if ( parents.size() > 0 )
		mBody << " : public ";
	for ( unsigned j = 0; j < parents.size(); j++ )
	{
		if ( j != 0 )
			mBody << ", ";
		
		if ( parents[ j ]->getName() == "ISupports" )
			mBody << "JHCOM::ISupports";
		else
			mBody << parents[ j ]->getName();
	}
	
	
	mBody << endl;
	mBody << mIndent << "{" << endl;
	mBody << mIndent << "public:" << endl;
	pushIndent();
	// contructor
	mBody << mIndent << type->getName().c_str() << "() {}" << endl << endl;

	type->getClass()->getMethods( methods );
	for ( unsigned j = 0; j < methods.size(); j++ )
	{
		mBody << mIndent << "virtual ";
		dumpMethod( mBody, methods[ j ], NULL );

		if ( methods[ j ]->isAsync() )
			mBody << " = 0; // Async method" << endl;
		else
			mBody << " = 0;" << endl;
	}
	
	std::vector<SmartPtr<const TypeInfo> > types;	
	mTypes->findTypesByNamespace( type->getName(), types );

	if ( types.size() > 0 )
		mBody << endl;

	for ( unsigned i = 0; i < types.size(); i++ )
	{
		dumpType( types[ i ] );
	}

	if ( types.size() == 0 )
		mBody << endl;
	
	if ( type->getClass()->getIID() != "" )
		mBody << mIndent << "JHCOM_DEFINE_IID( \"" << type->getClass()->getIID() << "\" );" << endl << endl;

	mBody << mIndent << "static " << type->getName() << "* GetStubs( JHCOM::IConnector *conn );" << endl;
	mBody << mIndent << "static JHCOM::IInvoker *GetInvoker( " << type->getName() << " *obj );" << endl << endl;
	
	popIndent();	
	mBody << mIndent << "protected:" << endl;
	pushIndent();
	mBody << mIndent << "virtual ~" << type->getName() << "() {}" << endl;
	popIndent();
	mBody << mIndent << "};" << endl << endl;
}

