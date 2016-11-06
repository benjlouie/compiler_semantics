/*********************************************
* Authors: Ben Loue, Matthew Karasz
* Sub-authors: Benji Cope
*
* This file handles the classes and their inheritance. It properly checks for improper inheritance and adds the classes to the symbol table.
**********************************************/

#ifndef __CLASSINHERITANCE_H_
#define __CLASSINHERITANCE_H_

#include "../src/ast.h"
#include "symbolTable.h"
#include <unordered_map>
#include <queue>
#include <iostream>

extern Node *root;
extern SymbolTable *globalSymTable;
extern std::unordered_map<std::string, std::string> globalTypeList;

enum ClassErr {OK, MULTI_DEF, CYCLIC_BAD_INHERITANCE};

/**
* setupClasses() is the main pass for all the classes contained in the program.
* First, it iterates through the classes and checks for class redefinitions while organizing the classes into a usable data structure.
* Second, it recurses throught the classes and builds the appropriate symbol table entries.
* Finally, it checks for bad/cyclic inheritance using the result of the second step.
*/
ClassErr setupClasses(void);

#endif //__CLASSINHERITANCE_H_