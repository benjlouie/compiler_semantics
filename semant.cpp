/*********************************************
* Authors: Everyone
* Sub-authors:
*
* Description: Sets up the globalSymTable and
* checks for cyclic inheritance and multiple
* class definitions
**********************************************/

#include "../compiler_semantics/semant.h"

/**
* The semant function checks for multiple class definitions and
* cyclic inheritance. This function also sets up the main and object
* scopes.
*/
void semant()
{
	//add Object to as starting class
	globalSymTable = new SymbolTable("Object");
    //setup the classes
	switch (setupClasses()) {
	case ClassErr::OK:
		break;
	case ClassErr::MULTI_DEF:
		cerr << "Class Error: Multiple class definitions\n";
		numErrors++;
		exit(-1);
		break;
	case ClassErr::CYCLIC_BAD_INHERITANCE:
		cerr << "Class Error: Bad or Cyclic class inheritance\n";
		numErrors++;
		exit(-1);
		break;
	}
    //Build symbol table and scoping
	switch (buildScope()) {
	case ScopeErr::SCOPE_OK:
		break;
	}

	//check for Main and main()
	if (globalTypeList.count("Main") == 0) {
		cerr << "Class Error: No Main class\n";
	}
	else {
		//goto main class
		string cur = "Main";
		vector<string> mainPath;
		//Get path to main
		while (cur != "Object") {
			mainPath.push_back(cur);
			cur = globalTypeList[cur];
		}

		//Go to object scope
		while (globalSymTable->getScope() != "Object") {
			globalSymTable->leaveScope();
		}

		//Go to main scope
		for (auto rit = mainPath.rbegin(); rit != mainPath.rend(); rit++) {
			globalSymTable->enterScope(*rit);
		}

		//Check for main method
		if (globalSymTable->getMethod("main") == nullptr) {
			cerr << "Class Error: No main method\n";
		}
	}

	/* check symbol table for multiple definitions of functions */
	globalSymTable->checkFunctions();

	typeCheck();
	
	//OPTIMIZATION - Loop Unswitching
	unswitchLoops();
	//must rebuild symbol table
	//if a let is unswitched the symbolTable will be incorrect
	delete globalSymTable;
	globalSymTable = new SymbolTable("Object");
	globalTypeList.clear(); //needed so setupClasses works correctly
	setupClasses();
	buildScope();

}

void rebuildSymTable()
{
    delete globalSymTable;
    globalSymTable = new SymbolTable("Object");
    globalTypeList.clear();
    setupClasses();
    buildScope();
}
