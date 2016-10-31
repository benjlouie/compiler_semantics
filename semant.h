#ifndef __SEMANT_H_
#define __SEMANT_H_

#include "ast.h"
#include "symbolTable.h"
#include "classInheritance.h"
#include <unordered_map>

extern SymbolTable *globalSymTable;

void semant();

#endif //__SEMANT_H_