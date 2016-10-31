
#ifdef __linux__
#include "../src/semant.h"
#else
#include "../compiler_semantics/semant.h"
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
    //Build symbol table and scoping
	switch (buildScope()) {
	case ScopeErr::SCOPE_OK:
		break;
	}

    //TODO:Call stuff for type checking 
}