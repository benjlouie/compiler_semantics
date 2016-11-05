#include "../compiler_semantics/classInheritance.h"

void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start);
void setupBuiltinClasses(void);

ClassErr setupClasses(void)
{
	auto classNodes = root->getChildren();
	//<inherits, list of classes>
	unordered_map<string, vector<string>> classMap;

	//setup symboltable methods builtin Classes
	setupBuiltinClasses();
	//setup ClassMap too
	classMap["Object"].push_back("IO");
	classMap["Object"].push_back("String");


	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;
		string inherits = ((Node *)classDescNodes[1])->value;

		if (globalTypeList.count(className) > 0) {
			cerr << "Class \"" << className << "\" redefined" << "\n";
			return ClassErr::MULTI_DEF;
		}
		else {
			globalTypeList[className] = inherits;
		}
		classMap[inherits].push_back(className);
	}

	string startingClass = "Object";
	scopeClasses(classMap, startingClass);
	//all classes should be checked now

	//check for bad/cyclic class inheritance
	if (classMap.size() > 0) {
		cerr << "Classes: ";
		for (auto it = classMap.begin(); it != classMap.end(); it++) {
			for (string cl : it->second) {
				cerr << cl << ", ";
			}
		}
		cerr << "have improper inheritance" << "\n";
		return ClassErr::CYCLIC_BAD_INHERITANCE;
	}

	return ClassErr::OK;
}

void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start) {
	if (classMap.count(start) == 0) {
		return;
	}

	auto inheritsList = classMap[start];
	//Go through each class, add to sym table the correct variables/classes
	for (string className : inheritsList) {
		if (className == "IO" || className == "String") { 
			globalSymTable->enterScope(className);
		}
		else {
			globalSymTable->addAndEnterScope(className);
		}
		globalSymTable->addVariable("self", "SELF_TYPE");
		scopeClasses(classMap, className);
		globalSymTable->leaveScope();
	}
	classMap.erase(start);
}

void setupBuiltinClasses(void)
{
	vector<string> argList;
	//get back to Object
	while (globalSymTable->getScope() != "Object") {
		globalSymTable->leaveScope();
	}
	//Object
	//abort()
	globalSymTable->addMethod("abort", argList, "Object");
	globalSymTable->addScope("abort");
	//type_name()
	globalSymTable->addMethod("type_name", argList, "String");
	globalSymTable->addScope("type_name");
	//copy()
	globalSymTable->addMethod("copy", argList, "SELF_TYPE");
	globalSymTable->addScope("copy");
	globalTypeList["Object"] = ""; //Object doesn't inherit from anything

	//IO
	globalSymTable->addAndEnterScope("IO");
	//out_string()
	argList.push_back("String");
	globalSymTable->addMethod("out_string", argList, "SELF_TYPE");
	argList.pop_back();
	globalSymTable->addAndEnterScope("out_string");
	globalSymTable->addVariable("x", "String");
	globalSymTable->leaveScope();
	//out_int()
	argList.push_back("Int");
	globalSymTable->addMethod("out_int", argList, "SELF_TYPE");
	argList.pop_back();
	globalSymTable->addAndEnterScope("out_int");
	globalSymTable->addVariable("x", "Int");
	globalSymTable->leaveScope();
	//in_string()
	globalSymTable->addMethod("in_string", argList, "String");
	globalSymTable->addScope("in_string");
	//in_int()
	globalSymTable->addMethod("in_int", argList, "Int");
	globalSymTable->addScope("in_int");
	globalTypeList["IO"] = "Object";
	globalSymTable->leaveScope();

	//String
	globalSymTable->addAndEnterScope("String");
	//length()
	globalSymTable->addMethod("length", argList, "Int");
	globalSymTable->addScope("length");
	//concat()
	argList.push_back("String");
	globalSymTable->addMethod("concat", argList, "String");
	argList.pop_back();
	globalSymTable->addAndEnterScope("concat");
	globalSymTable->addVariable("s", "String");
	globalSymTable->leaveScope();
	//substr()
	argList.push_back("Int");
	argList.push_back("Int");
	globalSymTable->addMethod("substr", argList, "String");
	argList.pop_back();
	argList.pop_back();
	globalSymTable->addAndEnterScope("substr");
	globalSymTable->addVariable("i", "Int");
	globalSymTable->addVariable("l", "Int");
	globalSymTable->leaveScope();
	globalTypeList["String"] = "Object";
	globalSymTable->leaveScope();

	//TODO: do we add Int and Bool to scope? they can't be inherited from so maybe not?
	//Int
	//Bool
	//just add to type list
	globalTypeList["Int"] = "Object";
	globalTypeList["Bool"] = "Object";

}