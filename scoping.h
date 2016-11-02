#ifndef __SCOPING_H_
#define __SCOPING_H_

#include "../src/ast.h"
#include "symbolTable.h"
#include "typeCheck.h"

extern Node *root;
extern SymbolTable *globalSymTable;
extern std::unordered_map<std::string, std::string> globalTypeList;

enum ScopeErr {SCOPE_OK};

ScopeErr buildScope(void);

#endif //__CLASSINHERITANCE_H_