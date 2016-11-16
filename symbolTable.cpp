/*********************************************
* Authors: Forest and Ben
* Sub-authors: Robert, Matt
*
* Description: Contains the functions used by
* symbol table in order to accurately generate the table
**********************************************/

#include "symbolTable.h"


/**
* This is the structure for the symbol table
* This takes in the scope name and created a new node
*/
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

/**
* Get the current class being evaluated
* returns the name of the Node
*/
string SymbolTable::getCurrentClass()
{
	SymNode *currentSymNode = this->cur;

	while ((globalTypeList.count(currentSymNode->name)) == 0) {
		currentSymNode = currentSymNode->parent;
	}

	return currentSymNode->name;
}

/**
* Checks if the current class is a super class
*/
bool SymbolTable::isSubClass(string sub, string super)
{
	if (super == "Object") {
		return true;
	}
	if (sub == super) {
		return true;
	}
	if (sub == "SELF_TYPE") {
		sub = getCurrentClass();
	}
	if (super == "SELF_TYPE") {
		super = getCurrentClass();
	}
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
* Function to leave the current scope worked in
*/
void SymbolTable::leaveScope()
{
	if (cur == symRoot) {
		throw "Cannot leave global scope";
	}
	cur = cur->parent;
}

/**
*
*/
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

/**
* Checks if the method has the correct argtypes and return type
* returns true if the method was correctly added
*/
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

/**
* Gets the variable with input name
*/
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

/**
* Gets the method with the input name
*/
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

/**
* Gets the name of methods based on the class name passed
*/
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
	for (int i = (int)path.size() - 1; i >= 0; i--) {
		for (auto c : searcher->children) {
			if (c->name == path[i]) {
				searcher = c;
				break;
			}
		}
	}
	if (searcher->name != cls) {
		//cerr << "Error finding class in getMethod(string, string)" << endl;
		return nullptr;
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

/**
* Checks to make sure that if a child has a function with the same name as a parent
* then the signatures of those methods match. increments error count if so
*/
void SymbolTable::checkFunctions(void) {
	/* add pointers of each class's Symnode to a vector */
	queue<SymNode *> q;
	vector<SymNode *> classes;
	q.push(symRoot);
	SymNode *tmp;
	while (!q.empty()) {
		tmp = q.front();
		q.pop();
		for (auto chld : tmp->children) {
			if (globalTypeList.count(chld->name) != 0) {
				q.push(chld);
				classes.push_back(chld);
			}
		}
	}
	/* For each class's methods, check that if it is defined in an ancestor
	   then the ancestor's definition is consistent with it's own*/
	//TODO: This should be cleaned up and made more efficient
	SymNode *ancestor;
	vector<pair<string, SymTableMethod>> methods;
	SymTableMethod check;
	vector<string> params1, params2;
	for (auto cl : classes) {
		if (cl->name == "Int" || cl->name == "Bool" || cl->name == "String" || cl->name == "Object" || cl->name == "IO")
			continue;
		for (auto method : cl->methods) {
			methods.push_back(method);
		}
		ancestor = cl->parent;
		while (ancestor != nullptr) {
			for (auto parent_method : ancestor->methods) {
				for (auto child_method : methods) {
					if (parent_method.first == child_method.first) { //names match
						if (parent_method.second.returnType != child_method.second.returnType) { //check return types matching
							numErrors++;
							cerr << "In class " << cl->name << " mismatch in return type of method " << parent_method.first << " previously declared in " << ancestor->name << endl;
						}
						else { //check rest of parameters match
							params1 = parent_method.second.argTypes;
							params2 = child_method.second.argTypes;
							if (params1.size() != params2.size()) {
								numErrors++;
								cerr << "In class " << cl->name << " mismatch in number of parameters to method " << parent_method.first << " previously declared in " << ancestor->name << endl;
							}
							else {
								for (int i = 0; i < params1.size(); i++) {
									if (params1[i] != params2[i]) {
										numErrors++;
										cerr << "In class " << cl->name << " mismatch in types of parameters to method " << parent_method.first << " previously declared in " << ancestor->name << endl;
									}
								}
							}
						}
					}
				}
			}
			ancestor = ancestor->parent;
		}
		while (methods.size() != 0) //empty methods list for next class
			methods.pop_back();
		
	}
}

/**
 * authors: Forest, Benji
 */
vector<string> SymbolTable::getChildrenNames() {
	vector<string> names;
	for (auto chld : cur->children) {
		names.push_back(chld->name);
	}
	return names;
}

/**
 * authors: Forest, Benji
 */
vector<string> SymbolTable::getMethodNames() {
	vector<string> methods;
	for (auto meth : cur->methods) {
		methods.push_back(meth.first); //meth first kids
	}
	return methods;
}

void SymbolTable::goToRoot(void) {
	cur = symRoot;
}

void SymbolTable::generateOffsets()
{
	generateOffsets_recursive(symRoot, 0, -8);
}

void SymbolTable::generateOffsets_recursive(SymbolTable::SymNode *cur, size_t depth, size_t curOffset)
{
	//go through variables
	size_t localOffset = 0;
	for (auto it = cur->variables.begin(); it != cur->variables.end(); it++) {
		SymTableVariable &var = cur->variables[it->first];
		var.depth = depth;
		var.offset = curOffset + localOffset;
		localOffset += 8; //8 bytes for everything
	}

	//go through children
	for (auto child : cur->children) {
		generateOffsets_recursive(child, depth + 1, curOffset + localOffset);
	}
}

void SymbolTable::countLocals()
{
	countLocals_recursive(symRoot);
}

int SymbolTable::countLocals_recursive(SymbolTable::SymNode *cur)
{
	int count = 0;
	for (auto childScope : cur->children) {
		if (childScope->name.substr(0, 3) == "let") {
			count += (int)childScope->variables.size();
		}
		else if (childScope->name.substr(0, 4) == "case") {
			count += (int)childScope->variables.size();
		}
		count += countLocals_recursive(childScope);
	}
	cur->numLocals = count;

	return count;
}