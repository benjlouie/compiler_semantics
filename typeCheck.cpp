#include "typeCheck.h"
#include <iostream>

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
		node->valType = globalSymTable->getVariable(node->value)->type;
		break;
	case NodeType::AST_CASE: {
		auto children = node->getChildren();
		Node * asttypechild = (Node *)children[2];

		node->valType = asttypechild->valType;
		break;
	}
	case NodeType::AST_TYPE:
		node->valType = node->value;
		break;
	case NodeType::AST_IF: {
		//Node(AST_IF,3,$2,$4,$6)
		auto children = node->getChildren();
		Node * iftest = (Node *)children[0];
		Node * thenchild = (Node *)children[1];
		Node * elsechild = (Node *)children[2];

		//if stuff
		if (iftest->valType != "Bool") {
			cerr << "ERROR IN IF TEST" << endl;
		}
		//TODO then else stuff 
		//forest will node->type = lub(vector<String> types);

		break;
	}
	case NodeType::AST_ISVOID:
		node->valType = "Bool";
		break;
	case NodeType::AST_NEW: {
		Node * child = (Node *)node->getChild();
		node->valType = child->valType;
		break;
	}
	case NodeType::AST_NOT: {
		//check for BOOL
		Node* child = (Node *)node->getChild();
		if (child->valType != "Bool") {
			cerr << "TYPE ERROR IN AST_TILDE" << endl;
		}
		else {
			child->valType = "Bool";
		}
		break;
	}
	case NodeType::AST_WHILE: {
		//Node(AST_WHILE,2,$2,$4)

		auto children = node->getChildren();
		Node * expressiontest = (Node *)children[0];
		Node * expression = (Node *)children[1];

		//if stuff
		if (expressiontest->valType != "Bool") {
			cerr << "ERROR IN EXPRESION TEST, SHOULD BE BOOL BUT ISNT" << endl;
		}
		node->valType = "Object";
		break;
	}
	case NodeType::AST_INTEGERLITERAL:
		node->valType = "Int";
		break;
	case NodeType::AST_STRING:
		node->valType = "String";
		break;
	case NodeType::AST_TRUE:
	case NodeType::AST_FALSE:
		node->valType = "Bool";
		break;
	case NodeType::AST_MINUS:
	case NodeType::AST_PLUS:
	case NodeType::AST_DIVIDE:
	case NodeType::AST_TIMES: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != "Int") {
			//TODO: Add Error Here
			cerr << "ERROR IN " << node->type << " WITH TYPES"<< endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right ->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else if (right->valType != "Int"){
			//TODO: Add Error Here
			cerr << "ERROR IN " << node->type << " WITH TYPES" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else {
			node->valType = "Int";
		}
		break;
	}
	case NodeType::AST_EQUALS: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType == "Bool" || left->valType == "Int" || left->valType == "String") {
			if (right->valType != left->valType) {
				//TODO: Add Error Here
				cerr << "ERROR WITH TYPES IN AST_EQUAL" << endl;
				cerr << "LEFT  TYPE IS " << left->valType << endl;
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				cerr << endl;
				node->valType = "Object";
			}
		}
		else if (right->valType == "Bool" || right->valType == "Int" || right->valType == "String") {
			if (right->valType != left->valType) {
				//TODO: Add Error Here
				cerr << "ERROR WITH TYPES IN AST_EQUAL" << endl;
				cerr << "LEFT  TYPE IS " << left->valType << endl;
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				cerr << endl;
				node->valType = "Object";
			}
		}
		else {
			node->valType = "Bool";
		}
		break;
	}
	case NodeType::AST_LE:
	case NodeType::AST_LT: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != "Int") {
			//TODO: Add Error Here
			cerr << "ERROR WITH TYPES IN AST_COMPARE" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else if (right->valType != "Int") {
			//TODO: Add Error Here
			cerr << "ERROR WITH TYPES IN AST_COMPARE" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else {
			node->valType = "Bool";
		}
		break;
	}
	case NodeType::AST_LARROW: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != right->valType) {
			//TODO: Add Error Here
			cerr << "ERROR WITH TYPES IN AST_LARROW" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else {
			node->valType = left->valType;
		}
		break;
	}
	case NodeType::AST_LET: {
		auto children = node->getChildren();
		Node * letexpression  = (Node *)children[1];

		node->valType = letexpression->valType;
		break;
	}
	case NodeType::AST_TILDE: {
		//check for INT
		Node* child = (Node *)node->getChildren()[0];
		if (child->valType != "Int") {
			cerr << "TYPE ERROR IN AST_TILDE" << endl;
			cerr << "Should be type Int but is type " << child->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else {
			node->valType = "Int";
		}
		break;
	}
	case NodeType::AST_DISPATCH:
		break;
	case NodeType::AST_EXPRSEMILIST: {
		auto children = node->getChildren();
		Node * lastchild = (Node *)children[children.size()-1];

		node->valType = lastchild->valType;

		break;
	}
	case NodeType::AST_IDTYPEEXPR: {
		Node *expr = (Node *)node->getChildren()[2];
		Node *type = (Node *)node->getChildren()[1];
		if (expr->type != NodeType::AST_NULL && expr->type != type->type) {
			cerr << "type mismatch in assignment of let statement" << endl;
		}
		break;
	}
	case NodeType::AST_CASESTATEMENT: //check children of AST_CASELIST, get lub(children)
		//TODO forest lub(Vector<String> types);
		break;
	case NodeType::AST_FEATURE_ATTRIBUTE: {
		Node *expr = (Node *)node->getChildren()[2];
		Node *type = (Node *)node->getChildren()[1];
		if (expr->type != NodeType::AST_NULL && expr->type != type->type) {
			cerr << "type mismatch in feature assignment" << endl;
		}
		break;
	}
	default:
		break;

	}

	return TypeErr::TYPE_OK;
}