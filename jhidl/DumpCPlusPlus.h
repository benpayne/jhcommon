/*
 *  DumpCPlusPlus.h
 *  jhCommon
 *
 *  Created by Ben Payne on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DUMPCPLUSPLUS_H_
#define DUMPCPLUSPLUS_H_

#include "Reflect.h"
#include <string>
#include <sstream>

class DumpCPlusPlus
{
public:
	DumpCPlusPlus( const char *filename, JHCOM::TypeManager *types );
	
	void Dump( std::vector<std::string> includes );
	
protected:
	void dumpType( const JHCOM::TypeInfo *type );
	void dumpStruct( const JHCOM::TypeInfo *type );
	void dumpEnum( const JHCOM::TypeInfo *type );
	void dumpInterface( const JHCOM::TypeInfo *type );
	void dumpAlias( const JHCOM::TypeInfo *type );
	void dumpCpp( const JHCOM::TypeInfo *type );
	void dumpMethod( std::ostream &out, const JHCOM::Method *method, const JHCOM::TypeInfo *type );
	void dumpStubBody( std::ostream &out, const JHCOM::Method *method, int method_num, const JHCOM::TypeInfo *type );

	void dumpStubClass( std::ostream &out, const JHCOM::TypeInfo *type );
	void dumpInvokerClass( std::ostream &out, const JHCOM::TypeInfo *type );
	
	std::string getTypeName( const JHCOM::TypeInfo *type );
	std::string getConnectorName( const JHCOM::TypeInfo *type );
	std::string getVariantName( const JHCOM::TypeInfo *type );
	
	void pushIndent();
	void popIndent();
	
private:
	void findIncludes();
	
	std::string mFilename;
	std::stringstream mBody;
	FILE *output;
	JHCOM::TypeManager *mTypes;
	int mIndentLevel;
	std::string mIndent;
	bool usesString;
	bool usesInt;
	bool usesSmartPtr;
};

#endif // DUMPCPLUSPLUS_H_