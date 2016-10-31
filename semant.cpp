
#ifdef __linux__
#include "../src/semant.h"
#else
#include "semant.h"
#endif

void semant()
{
    //setup the classes
	globalSymTable = new SymbolTable("Object");
	switch (setupClasses()) {
	case ClassErr::OK:
		break;
	case ClassErr::MULTI_DEF:
		break;
	case ClassErr::CYCLIC_BAD_INHERITANCE:
		break;
	}
    //TODO:Build symbol table and scoping
	switch (buildScope()) {
	case ScopeErr::OK:
		break;
	}

    //TODO:Call stuff for type checking 
}