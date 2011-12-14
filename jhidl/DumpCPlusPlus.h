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

class DumpCPlusPlus
{
public:
	DumpCPlusPlus( const char *filename, JHCOM::TypeManager *types ) : mFilename( filename ), mTypes( types ) {}
	
	void Dump();
	
protected:
	void dumpType( const JHCOM::TypeInfo *type );
	void dumpStruct( const JHCOM::TypeInfo *type );
	void dumpEnum( const JHCOM::TypeInfo *type );
	void dumpInterface( const JHCOM::TypeInfo *type );
	
private:
	void findIncludes();
	
	std::string mFilename;
	FILE *output;
	JHCOM::TypeManager *mTypes;
};

#endif // DUMPCPLUSPLUS_H_