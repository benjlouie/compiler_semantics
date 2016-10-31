#ifndef __SYMBOLTABLE_H_
#define __SYMBOLTABLE_H_

#include <unordered_map>
#include <vector>


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
		std::vector<struct SymNode *> children;
	} SymNode;
    SymNode *symRoot;
    SymNode *cur;

	SymbolTable(std::string scopeName)
	{
		return;
	}
        
    /*
        *Make a new scope (let, method, class) in the current scope
        */ 
	void addScope(std::string newScope)
	{
		return;
	}
        
    /*
        * Enter a scope, making sure it exists first
        */
	void enterScope(std::string scope)
	{
		return;
	}
        
    /*
        * Make a new scope in the current scope and enter it
        */
	void addAndEnterScope(std::string scope)
	{
		return;
	}

    /*
        * Get the name of the current scope
        */
	std::string getScope()
	{
		return "";
	}

    /*
        *
        */
	void leaveScope()
	{
		return;
	}

	void addVariable(std::string name, std::string type)
	{
		return;
	}

	void addMethod(std::string name, std::vector<std::string> argTypes, std::string returnType)
	{
		return;
	}

	SymTableVariable *getVariable(std::string name)
	{
		return nullptr;
	}
	SymTableMethod *getMethod(std::string name)
	{
		return nullptr;
	}

};

#endif //__SYMBOLTABLE_H_