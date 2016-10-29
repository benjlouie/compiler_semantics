#pragma once
#include <unordered_map>

class SymbolTable
{
    public: 
        struct SymNode {
            //Name of the node
            string name;
            //The variables in this node and their values
            unordered_map<string,SymTableVariable> variables;
            //The functions in this node and their expressions
            unordered_map<string,SymTableMethod> methods;
            //Where we came from 
            SymNode *parent;
            //
            vector<SymNode *> children;
        }
        SymNode *symRoot;
        SymNode *cur;

        SymbolTable(string scopeName);
        
        /*
         *Make a new scope (let, method, class) in the current scope
         */ 
        void addScope(string newScope);
        
        /*
         * Enter a scope, making sure it exists first
         */
        void enterScope(string scope);
        
        /*
         * Make a new scope in the current scope and enter it
         */
        void addAndEnterScope(string scope);

        /*
         * Get the name of the current scope
         */
        string getScope();

        /*
         *
         */
        void leaveScope();

        void addVariable(string name, string type);

        void addMethod(string name, vector<string> argTypes, string returnType);

        SymTableVariable *getVariable(string name);
        SymTableMethod *getMethod(string name);

}

class SymTableVariable 
{
    //Type
    string type;
}

class SymTableMethod
{
    //Return type
    string returnType;
    //Functions within it?
    vector<string> argTypes;
}
