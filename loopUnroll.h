#pragma once

#include <unordered_map>
#include "../src/ast.h"
#include "symbolTable.h"
#include <iostream>

extern Node *root;
extern SymbolTable *globalSymTable;

void unrollWhile(void);