#pragma once

#include <unordered_map>
#include <stack>
#include "../src/ast.h"
#include "symbolTable.h"


extern Node *root;
extern SymbolTable *globalSymTable;
//extern std::unordered_map<std::string, std::string> globalTypeList;


void unswitchLoops(void);