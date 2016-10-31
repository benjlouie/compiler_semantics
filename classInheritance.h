#ifndef __CLASSINHERITANCE_H_
#define __CLASSINHERITANCE_H_

#include "../src/ast.h"
#include "symbolTable.h"
#include <unordered_map>
#include <queue>

extern Node *root;
extern SymbolTable *globalSymTable;
extern std::unordered_map<std::string, std::string> globalTypeList;

enum ClassErr {OK, MULTI_DEF, CYCLIC_BAD_INHERITANCE};

ClassErr setupClasses(void);

#endif //__CLASSINHERITANCE_H_