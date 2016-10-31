#ifndef __CLASSINHERITANCE_H_
#define __CLASSINHERITANCE_H_

#include "ast.h"
#include "symbolTable.h"

extern Node *root;
extern SymbolTable *globalSymTable;
extern std::unordered_map<std::string, std::string> globalTypeList;

enum ScopeErr {OK};

ScopeErr buildScope(void);

#endif //__CLASSINHERITANCE_H_