#include "symbolTable.h"
#include <stdexcept>
#include <queue>
using namespace std;

SymbolTable::SymbolTable(string scopeName) {
	symRoot = new SymNode;
	cur = symRoot;
	cur->name = scopeName;
	cur->parent = nullptr;
}

/**
* Go through every symnode in the table and delete them
*/
SymbolTable::~SymbolTable() {
	queue<SymNode *> q;
	q.push(symRoot);
	SymNode *tmp;
	while (!q.empty()) {
		tmp = q.front();
		q.pop();
		for (auto ch : tmp->children) {
			q.push(ch);
		}
		delete tmp;
	}
}

/*
*Make a new scope (let, method, class) in the current scope
*/
void SymbolTable::addScope(std::string newScope)
{
	for (auto s : cur->children) {
		if (s->name == newScope) {
			throw exception("Scope name already exists");
		}
	}
	SymNode *tmp = new SymNode;
	tmp->name = newScope;
	tmp->parent = cur;
	cur->children.push_back(tmp);
}

/*
* Enter a scope, making sure it exists first
*/
void SymbolTable::enterScope(std::string scope)
{
	for (auto s : cur->children) {
		if (s->name == scope) {
			cur = s;
			return;
		}
	}
	throw exception("Scope not found");
}

/*
* Make a new scope in the current scope and enter it
*/
void SymbolTable::addAndEnterScope(std::string scope)
{
	addScope(scope);
	enterScope(scope);
}

/*
* Get the name of the current scope
*/
string SymbolTable::getScope()
{
	return cur->name;
}

/*
*
*/
void SymbolTable::leaveScope()
{
	if (cur == symRoot) {
		throw exception("Cannot leave global scope");
	}
	cur = cur->parent;
}


bool SymbolTable::addVariable(string name, string type)
{
	/*if (cur->variables.find(name) == cur->variables.end()) {
		throw exception("Cannot add variable, already exists in current scope");
		return false;
	}

	SymTableVariable v;
	v.type = type;
	cur->variables[name] = v;
	return true;
	*/
	if (cur->variables.count(name) > 0) {
		return false; //already exists
	}
	cur->variables[name].type = type;
	return true;
}

bool SymbolTable::addMethod(string name, vector<string> argTypes, string returnType)
{
	/*if (cur->variables.find(name) == cur->variables.end()) {
		throw exception("Cannot add method, already exists in current scope");
		return false;
	}
	SymTableMethod m;
	m.returnType = returnType;
	m.argTypes = argTypes;
	cur->methods[name] = m;
	return true;
	*/
	if (cur->methods.count(name) > 0) {
		return false; //method already exists
	}
	SymTableMethod &method = cur->methods[name];
	method.argTypes = argTypes;
	method.returnType = returnType;
	return true;
}

SymTableVariable *SymbolTable::getVariable(string name)
{
	SymNode *scanner = cur;
	while (scanner != nullptr) {
		if (scanner->variables.find(name) != cur->variables.end()) {
			return &(scanner->variables[name]);
		}
		scanner = scanner->parent;
	}
	throw exception("Cannot find variable");
	return nullptr;
}
SymTableMethod *SymbolTable::getMethod(string name)
{
	SymNode *scanner = cur;
	while (scanner != nullptr) {
		if (scanner->methods.find(name) != cur->methods.end()) {
			return &(scanner->methods[name]);
		}
		scanner = scanner->parent;
	}
	throw exception("Cannot find method");
	return nullptr;
}