/*
 *  DumpXML.h
 *  jhCommon
 *
 *  Created by Ben Payne on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DUMPXML_H_
#define DUMPXML_H_

#include "Reflect.h"

class DumpXML
{
public:
	DumpXML( const char *filename, JHCOM::TypeManager *types ) : 
		mFilename( filename ), mTypes( types ), mIndentLevel( 0 )
	{}
	
	void Dump( std::vector<std::string> includes );
	
protected:
	void dumpType( const JHCOM::TypeInfo *type );
	void dumpStruct( const JHCOM::TypeInfo *type );
	void dumpEnum( const JHCOM::TypeInfo *type );
	void dumpInterface( const JHCOM::TypeInfo *type );
	void dumpAlias( const JHCOM::TypeInfo *type );

	std::string getTypeName( const JHCOM::TypeInfo *type );
	
	void pushIndent();
	void popIndent();
	
private:
	void findIncludes();
	
	std::string mFilename;
	FILE *output;
	JHCOM::TypeManager *mTypes;
	int mIndentLevel;
	std::string mIndent;
};

#endif // DUMPXML_H_