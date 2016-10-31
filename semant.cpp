
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
    //checkClassInheritance();
    //TODO:Build symbol table and scoping
    
    //TODO:Call stuff for type checking 
}