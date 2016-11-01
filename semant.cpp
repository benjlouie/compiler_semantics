
#ifdef __linux__
#include "../src/semant.h"
#else
#include "../compiler_semantics/semant.h"
#endif

void semant()
{
	//add Object to as starting class
	globalSymTable = new SymbolTable("Object");
    //setup the classes
	switch (setupClasses()) {
	case ClassErr::OK:
		break;
	case ClassErr::MULTI_DEF:
		printf("Error: Multiple class definitions\n");
		exit(-1);
		break;
	case ClassErr::CYCLIC_BAD_INHERITANCE:
		printf("Error: Bad or Cyclic class inheritance\n");
		exit(-1);
		break;
	}
    //Build symbol table and scoping
	switch (buildScope()) {
	case ScopeErr::SCOPE_OK:
		break;
	}

    //TODO:Call stuff for type checking 
}