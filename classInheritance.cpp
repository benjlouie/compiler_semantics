/*********************************************
* Authors: Ben Loue, Matthew Karasz
* Sub-authors: Benji Cope
*
* This file handles the classes and their inheritance. It properly checks for improper inheritance and adds the classes to the symbol table.
**********************************************/

#include "../compiler_semantics/classInheritance.h"

void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start);
void setupBuiltinClasses(void);

/**
* setupClasses() is the main pass for all the classes contained in the program.
* First, it iterates through the classes and checks for class redefinitions while organizing the classes into a usable data structure.
* Second, it recurses throught the classes and builds the appropriate symbol table entries.
* Finally, it checks for bad/cyclic inheritance using the result of the second step.
*/
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

	//populates the inherits lists for the scoping and checking step
	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;
		string inherits = ((Node *)classDescNodes[1])->value;

		//check for class redefinition
		if (globalTypeList.count(className) > 0) {
			cerr << "Class \"" << className << "\" redefined" << "\n";
			return ClassErr::MULTI_DEF;
		}
		else {
			globalTypeList[className] = inherits;
		}
		classMap[inherits].push_back(className);
	}

	//add classes to scope and reveal any cyclic/bad inheritance
	string startingClass = "Object";
	scopeClasses(classMap, startingClass);

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

/**
* scopeClasses() is the recursive backbone of setupClasses().
* It recursively iterates throught the passes in classMap and adds the classes to the ssymbol table tree.
* It begins from the starting point ("Object") and expands any classes that inherit from that.
* Any classes leftover once it finishes must have bad or cyclic inheritance.
*/
void scopeClasses(unordered_map<string, vector<string>> &classMap, string &start) {
	if (classMap.count(start) == 0) {
		return;
	}
	//special case, cannot be inherited from
	if (start == "Int" || start == "Bool" || start == "String") {
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
		scopeClasses(classMap, className);
		globalSymTable->leaveScope();
	}
	classMap.erase(start);
}

/**
* setupBuiltinClasses initializes Cool's builtin classes.
* The default classes and their methods are added to the symbol table
* The default classes are also added to the default type list so they may be referenced.
*/
void setupBuiltinClasses(void)
{
	vector<string> argList;
	//get back to Object
	while (globalSymTable->getScope() != "Object") {
		globalSymTable->leaveScope();
	}
	//Object
	globalSymTable->addVariable("self", "SELF_TYPE");
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

	//Int
	//Bool
	//just add to type list
	globalSymTable->addScope("Int");
	globalSymTable->addScope("Bool");
	globalTypeList["Int"] = "Object";
	globalTypeList["Bool"] = "Object";

}