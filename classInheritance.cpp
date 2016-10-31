#ifdef __linux__
#include "../src/classInheritance.h"
#else
#include "classInheritance.h"
#endif

void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start);

ClassErr setupClasses(void)
{
	auto classNodes = root->getChildren();
	//<inherits, list of classes>
	unordered_map<string, vector<string>> classMap;

	//TODO: setup Int, Bool, String, IO classes
	//setup symboltable methods with them too

	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;
		string inherits = ((Node *)classDescNodes[1])->value;

		if (globalTypeList.count(className) > 0) {
			//TODO: report multi def errors
			return ClassErr::MULTI_DEF;
		}
		else {
			globalTypeList[className] = inherits;
		}
		classMap[inherits].push_back(className);
	}

	scopeClasses(classMap, string("Object"));
	/*
	queue<string> inheritsQueue;
	//setup queue
	for (string className : classMap["Object"]) {
		inheritsQueue.push(className);
	}
	classMap.erase("Object");

	while (!inheritsQueue.empty()) {
		size_t len = inheritsQueue.size();
		for (size_t i = 0; i < len; i++) {
			string currentClass = inheritsQueue.front();
			inheritsQueue.pop();
			if (classMap.count(currentClass) > 0) {
				for (string className : classMap[currentClass]) {
					inheritsQueue.push(className);
				}
				classMap.erase(currentClass);
			}
		}
	}
	*/
	//all classes should be checked now
	if (classMap.size() > 0) {
		//TODO: cyclic inheritance or undefined inheritance errors
		return ClassErr::CYCLIC_BAD_INHERITANCE;
	}
	return ClassErr::OK;
}

void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start) {
	if (classMap.count(start) == 0) {
		return;
	}
	//TODO: check this once SymbolTable is done
	for (string className : classMap[start]) {
		globalSymTable->addAndEnterScope(className);
		scopeClasses(classMap, className);
		globalSymTable->leaveScope();
	}
	classMap.erase(start);
}
