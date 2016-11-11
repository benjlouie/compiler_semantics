/*********************************************
* Authors: Forest and Ben
* Sub-authors: Robert, Matt
*
* Description: Contains the function headers for
* creating the symbol table
**********************************************/

#ifndef __SYMBOLTABLE_H_
#define __SYMBOLTABLE_H_

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <queue>
#include <iostream>
#include <string>
using namespace std;

extern std::unordered_map<std::string, std::string> globalTypeList;
extern int numErrors;

class SymTableVariable
{
public:
	//Type
	std::string type;
};

class SymTableMethod
{
public:
	//Return type
	std::string returnType;
	//Functions within it?
	std::vector<std::string> argTypes;
};


class SymbolTable
{
public: 
    typedef struct SymNode {
        //Name of the node
        std::string name;
        //The variables in this node and their values
		std::unordered_map<std::string, SymTableVariable> variables;
        //The functions in this node and their expressions
		std::unordered_map<std::string, SymTableMethod> methods;
        //Where we came from 
        struct SymNode *parent;
        //
		//TODO(BEN): change this to unordered_map<string, struct SymNode *> so children can quickly be accessed/queried
		std::vector<struct SymNode *> children;
	} SymNode;
    SymNode *symRoot;
    SymNode *cur;

	SymbolTable(std::string scopeName);

	~SymbolTable();
        
    /*
        *Make a new scope (let, method, class) in the current scope
        */ 
	void addScope(std::string newScope);

	/*
		*Gets the name of the current class
		*/
	std::string getCurrentClass();
	/*
		*Checks to see if the sub string is a sub class of the super string
		*/
	bool isSubClass(std::string sub, std::string super);
        
    /*
        * Enter a scope, making sure it exists first
        */
	void enterScope(std::string scope);
        
    /*
        * Make a new scope in the current scope and enter it
        */
	void addAndEnterScope(std::string scope);

    /*
        * Get the name of the current scope
        */
	std::string getScope();

    /*
        *
        */
	void leaveScope();

	bool addVariable(std::string name, std::string type);

	bool addMethod(std::string name, std::vector<std::string> argTypes, std::string returnType);

	SymTableVariable *getVariable(std::string name);

	SymTableMethod *getMethod(std::string name);

	SymTableMethod *getMethodByClass(std::string method, std::string cls);

	void checkFunctions(void);

	void goToRoot(void);
};

#endif //__SYMBOLTABLE_H_