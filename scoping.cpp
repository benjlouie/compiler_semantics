#include "../compiler_semantics/scoping.h"

ScopeErr buildScope_recursive(Node *ASTNode);

ScopeErr buildScope(void)
{
	return buildScope_recursive(root);
}

//TODO: error check this like crazy
ScopeErr buildScope_recursive(Node *ASTNode)
{
	if (ASTNode == nullptr) {
		return ScopeErr::SCOPE_OK;
	}

	auto classNodes = ASTNode->getChildren();
	int letCount = 0;

	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;

		//enter corect scope
		switch (ASTNode->type) {
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
			break;
		}
		case NodeType::AST_LET:
			globalSymTable->addAndEnterScope("let" + letCount);
			letCount++;
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

			//add the method to current scope, then ednter it before processing children
			globalSymTable->addMethod(methodName, formalTypes, returnType);
			globalSymTable->addAndEnterScope(methodName);
			break;
		}
		case NodeType::AST_FEATURE_ATTRIBUTE:
		{
			string attrName = ((Node *)child->getChildren()[0])->value;
			string attrType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(attrName, attrType);
			break;
		}
		case NodeType::AST_IDTYPEEXPR: //the variable declared by the a let expr
		{
			string varName = ((Node *)child->getChildren()[0])->value;
			string varType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(varName, varType);
			break;
		}
		case NodeType::AST_FORMAL:
		{
			string formalName = ((Node *)child->getChildren()[0])->value;
			string formalType = ((Node *)child->getChildren()[1])->value;
			globalSymTable->addVariable(formalName, formalType);
			break;
		}
		default:
			break;
		}

		//recurse to children
		switch (buildScope_recursive(child)) {
		case ScopeErr::SCOPE_OK:
			break;
		}

		//exit scope
		globalSymTable->leaveScope();
	}

	return ScopeErr::SCOPE_OK;
}