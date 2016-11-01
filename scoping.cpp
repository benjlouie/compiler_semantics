#ifdef __linux__
#include "../src/scoping.h"
#else
#include "scoping.h"
#endif

ScopeErr buildScope_recursive(Node *ASTNode, unsigned &currentLetCount);

ScopeErr buildScope(void)
{
	unsigned letCount = 0;
	return buildScope_recursive(root, letCount);
}

//TODO: error check this like crazy
ScopeErr buildScope_recursive(Node *ASTNode, unsigned &currentLetCount)
{
	if (ASTNode == nullptr) {
		return ScopeErr::SCOPE_OK;
	}

	auto classNodes = ASTNode->getChildren();

	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		bool enteredNewScope = false;

		//enter corect scope
		switch (child->type) {
		case NodeType::AST_CLASS:
		{
			//go to Object scope
			while (globalSymTable->getScope() != "Object") {
				globalSymTable->leaveScope();
			}
			//go to class scope
			string className = ((Node *)child->getChildren()[0])->value;
			string curClass = className;
			vector<string> inheritanceList;
			while (curClass != "Object") {
				inheritanceList.push_back(curClass);
				curClass = globalTypeList[curClass];
			}
			//traverse to class scope
			for (auto it = inheritanceList.rbegin(); it < inheritanceList.rend(); it++) {
				globalSymTable->enterScope(*it);
			}
			enteredNewScope = true;
			break;
		}
		case NodeType::AST_LET:
 			globalSymTable->addAndEnterScope("let" + to_string(currentLetCount));
			enteredNewScope = true;
			currentLetCount++;
			break;
		case NodeType::AST_FEATURE_METHOD:
		{
			string methodName = ((Node *)child->getChildren()[0])->value;
			string returnType = ((Node *)child->getChildren()[2])->value;
			vector<Tree *>formals = ((Node *)child->getChildren()[1])->getChildren();

			//get formal types for method
			vector<string> formalTypes;
			for (auto formal : formals) {
				string formalType = ((Node *)formal->getChildren()[1])->value;
				formalTypes.push_back(formalType);
			}

			//add the method to current scope, then enter it before processing children
			globalSymTable->addMethod(methodName, formalTypes, returnType); //TODO: check return (tells if mult def)
			globalSymTable->addAndEnterScope(methodName);
			enteredNewScope = true;
			break;
		}
		case NodeType::AST_FEATURE_ATTRIBUTE:
		{
			string attrName = ((Node *)child->getChildren()[0])->value;
			string attrType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(attrName, attrType); //TODO: check return (tells if mult def)
			break;
		}
		case NodeType::AST_IDTYPEEXPR: //the variable declared by the a let expr
		{
			string varName = ((Node *)child->getChildren()[0])->value;
			string varType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(varName, varType); //TODO: check return (tells if mult def)
			break;
		}
		case NodeType::AST_FORMAL:
		{
			string formalName = ((Node *)child->getChildren()[0])->value;
			string formalType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(formalName, formalType); //TODO: check return (tells if mult def)
			break;
		}
		}

		//recurse to children
		if (enteredNewScope) {
			
			unsigned newLetCount = 0;
			switch (buildScope_recursive(child, newLetCount)) {
			case ScopeErr::SCOPE_OK:
				break;
			}

			//exit scope
			globalSymTable->leaveScope();
		}
		else {
			switch (buildScope_recursive(child, currentLetCount)) {
			case ScopeErr::SCOPE_OK:
				break;
			}
		}
	}

	return ScopeErr::SCOPE_OK;
}