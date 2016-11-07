/*********************************************
* Authors: Everyone
* Sub-authors:
*
* Description: Header file for semant.cpp and
* contains the reference to the globalSymTable
**********************************************/

#ifndef __SEMANT_H_
#define __SEMANT_H_

#include "../src/ast.h"
#include "symbolTable.h"
#include "classInheritance.h"
#include "scoping.h"
#include <unordered_map>

extern SymbolTable *globalSymTable;

void semant();

#endif //__SEMANT_H_