#pragma once
#include "../src/ast.h"
#include <unordered_map>
#include <queue>

extern Node *root;
extern std::unordered_map<std::string, std::string> globalTypeList;

void eliminateUnreachable();