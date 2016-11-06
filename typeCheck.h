/*****************************************************************************************************
*	Main Authors: Matt karasz, Benjamin Cope, Ben Louie, Robert Blasi, Forest Thomas
*	Sub Authors:
*
*	Description: This file recursively walks the AST from the bottom up checking types as it goes
******************************************************************************************************/
#pragma once

#include "../src/ast.h"
#include "symbolTable.h"

extern Node *root;
extern SymbolTable *globalSymTable;
extern std::unordered_map<std::string, std::string> globalTypeList;
extern int numErrors;

enum TypeErr { TYPE_OK };

TypeErr typeCheck(void);
TypeErr typeCheck_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount);
TypeErr deSwitch(Node *node);