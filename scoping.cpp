#include "../compiler_semantics/scoping.h"

ScopeErr buildScope_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount);

ScopeErr buildScope(void)
{
	unsigned letCount = 0;
	unsigned caseCount = 0;
	//go back to Object scope
	while (globalSymTable->getScope() != "Object") {
		globalSymTable->leaveScope();
	}

	return buildScope_recursive(root, letCount, caseCount);
}

//TODO: error check this like crazy
ScopeErr buildScope_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount)
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
				
			if ( ASTNode->type != NodeType::AST_LET) {
				globalSymTable->addAndEnterScope("let" + to_string(currentLetCount));
				enteredNewScope = true;
				currentLetCount++;
			}
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
			if (!globalSymTable->addMethod(methodName, formalTypes, returnType)) {
				cout << child->lineNumber << ": error adding method '" << methodName << "': method already exists" << endl;
				numErrors++;
				continue;
			}
			globalSymTable->addAndEnterScope(methodName);
			enteredNewScope = true;
			break;
		}
		case NodeType::AST_FEATURE_ATTRIBUTE:
		{
			string attrName = ((Node *)child->getChildren()[0])->value;
			string attrType = ((Node *)child->getChildren()[1])->value;
			if (attrName == "self") {
				cerr << child->lineNumber << ": variable 'self' cannot be redefinded" << "\n";
				numErrors++;
			}
			else if (!globalSymTable->addVariable(attrName, attrType)) {
				cout << child->lineNumber << ": error adding variable '" << attrName << "': variable already exists" << endl;
				numErrors++;
			}
			break;
		}
		case NodeType::AST_IDTYPEEXPR: //the variable declared by the a let expr
		{
			string varName = ((Node *)child->getChildren()[0])->value;
			string varType = ((Node *)child->getChildren()[1])->value;
			if (varName == "self") {
				cerr << child->lineNumber << ": variable 'self' cannot be redefinded" << "\n";
				numErrors++;
			}
			else
			if (!globalSymTable->addVariable(varName, varType)) {
				cout << child->lineNumber << ": error adding variable '" << varName << "': variable already exists" << endl;
				numErrors++;
			}
			break;
		}
		case NodeType::AST_FORMAL:
		{
			string formalName = ((Node *)child->getChildren()[0])->value;
			string formalType = ((Node *)child->getChildren()[1])->value;
			if (!globalSymTable->addVariable(formalName, formalType)) {
				cout << child->lineNumber << ": error adding variable '" << formalName << "': variable already exists" << endl;
				numErrors++;
			}
			break;
		}
		case NodeType::AST_CASE: 
		{
			globalSymTable->addAndEnterScope("case" + to_string(currentCaseCount));
			string caseIDName = ((Node *)child->getChildren()[0])->value;
			string caseIDType = ((Node *)child->getChildren()[1])->value;
			if (caseIDName == "self") {
				cerr << child->lineNumber << ": variable 'self' cannot be redefinded" << "\n";
				numErrors++;
			}
			else {
				globalSymTable->addVariable(caseIDName, caseIDType);
			}
			enteredNewScope = true;
			currentCaseCount++;
			break;
		}
		default:
			break;
		}

		//recurse to children
		if (enteredNewScope) {
			
			unsigned newLetCount = 0;
			unsigned newCaseCount = 0;
			switch (buildScope_recursive(child, newLetCount, newCaseCount)) {
			case ScopeErr::SCOPE_OK:
				break;
			}

			//exit scope
			globalSymTable->leaveScope();
		}
		else {
			switch (buildScope_recursive(child, currentLetCount, currentCaseCount)) {
			case ScopeErr::SCOPE_OK:
				break;
			}
		}
	}

	return ScopeErr::SCOPE_OK;
}