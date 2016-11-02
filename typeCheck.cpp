#include "typeCheck.h"

TypeErr typeCheck(void);
TypeErr typeCheck_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount);
TypeErr deSwitch(Node *node);

TypeErr typeCheck(void)
{
	while (globalSymTable->getScope() != "Object") {
		globalSymTable->leaveScope();
	}
	unsigned letCount = 0;
	unsigned caseCount = 0;
	return typeCheck_recursive(root, letCount, caseCount);
}

TypeErr typeCheck_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount)
{
	//process children
	auto children = ASTNode->getChildren();
	for (Tree *tchild : children) {
		Node *child = (Node *)tchild;

		bool enteredNewScope = false;
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
			if (ASTNode->type != NodeType::AST_LET) {
				globalSymTable->enterScope("let" + to_string(currentLetCount));
				enteredNewScope = true;
				currentLetCount++;
			}
			break;
		case NodeType::AST_FEATURE_METHOD:
		{
			string methodName = ((Node *)child->getChildren()[0])->value;
			globalSymTable->enterScope(methodName);
			enteredNewScope = true;
			break;
		}
		case NodeType::AST_CASE:
		{
			globalSymTable->enterScope("case" + to_string(currentCaseCount));
			enteredNewScope = true;
			currentCaseCount++;
			break;

		}
		}

		//recurse to children
		if (enteredNewScope) {

			unsigned newLetCount = 0;
			unsigned newCaseCount = 0;
			switch (typeCheck_recursive(child, newLetCount, newCaseCount)) {
			case TypeErr::TYPE_OK:
				break;
			}

			//exit scope
			globalSymTable->leaveScope();
		}
		else {
			switch (typeCheck_recursive(child, currentLetCount, currentCaseCount)) {
			case TypeErr::TYPE_OK:
				break;
			}
		}
	}
	//process self
	return deSwitch(ASTNode);
}

TypeErr deSwitch(Node *node)
{
	switch (node->type) {
	case NodeType::AST_IDENTIFIER:
		break;
	case NodeType::AST_CASE:
		break;
	case NodeType::AST_IF:
		break;
	case NodeType::AST_IFTEST:
		break;
	case NodeType::AST_IFTHEN:
		break;
	case NodeType::AST_IFELSE:
		break;
	case NodeType::AST_ISVOID:
		break;
	case NodeType::AST_NEW:
		break;
	case NodeType::AST_NOT:
		break;
	case NodeType::AST_WHILE:
		break;
	case NodeType::AST_WHILECOMPARE:
		break;
	case NodeType::AST_INTEGERLITERAL:
		break;
	case NodeType::AST_STRING:
		break;
	case NodeType::AST_TRUE:
	case NodeType::AST_FALSE:
		break;
	case NodeType::AST_MINUS:
	case NodeType::AST_PLUS:
	case NodeType::AST_DIVIDE:
	case NodeType::AST_TIMES:
		break;
	case NodeType::AST_EQUALS:
	case NodeType::AST_LE:
	case NodeType::AST_LT:
		break;
	case NodeType::AST_LARROW:
		break;
	case NodeType::AST_LET:
		break;
	case NodeType::AST_TILDE:
		break;
	case NodeType::AST_DISPATCH:
		break;
	case NodeType::AST_EXPRSEMILIST:
		break;
	case NodeType::AST_IDTYPEEXPR: //check var type == expr type
		break;
	case NodeType::AST_CASESTATEMENT: //check children of AST_CASELIST, get lub(children)
		break;
	case NodeType::AST_INTEGER:
		break;
	default:
		break;

	}

	return TypeErr::TYPE_OK;
}