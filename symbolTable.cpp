#include "symboltable.h"


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
			throw "Scope name already exists";
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
	throw "Scope not found";
}

/*
* Make a new scope in the current scope and enter it
*/

void SymbolTable::addAndEnterScope(std::string scope)
{
	addScope(scope);
	enterScope(scope);
}

string SymbolTable::getCurrentClass()
{
	SymNode *currentSymNode = this->cur;

	while ((globalTypeList.count(currentSymNode->name)) == 0) {
		currentSymNode = currentSymNode->parent;
	}

	return currentSymNode->name;
}

bool SymbolTable::isSubClass(string sub, string super)
{
	while (sub != "Object" && sub != super) {
		sub = globalTypeList[sub];
	}

	return sub == super;
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
		throw "Cannot leave global scope";
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
		if (scanner->variables.count(name) > 0) {
			return &(scanner->variables[name]);
		}
		scanner = scanner->parent;
	}
	return nullptr;
}
SymTableMethod *SymbolTable::getMethod(string name)
{
	SymNode *scanner = cur;
	while (scanner != nullptr) {
		if (scanner->methods.count(name) > 0) {
			return &(scanner->methods[name]);
		}
		scanner = scanner->parent;
	}
	return nullptr;
}

SymTableMethod *SymbolTable::getMethodByClass(string method, string cls) {
	
	/* find the path to cls*/
	SymNode *searcher = symRoot;
	string tmp = cls;
	vector<string> path;
	while (tmp != "Object") {
		path.push_back(tmp);
		tmp = globalTypeList[tmp];
	}

	/* send searcher through path to the correct class */
	for (int i = path.size() - 1; i >= 0; i--) {
		for (auto c : searcher->children) {
			if (c->name == path[i]) {
				searcher = c;
				break;
			}
		}
	}
	if (searcher->name != cls) {
		cerr << "Error finding class in getMethod(string, string)" << endl;
		exit(-1);
	}

	/* search up hierarchy for method */
	while (searcher != nullptr) {
		if (searcher->methods.count(method) > 0) {
			return &(searcher->methods[method]);
		}
		searcher = searcher->parent;
	}
	return nullptr;
}