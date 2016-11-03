#include "typeCheck.h"
#include <iostream>

TypeErr typeCheck(void);
TypeErr typeCheck_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount);
TypeErr deSwitch(Node *node);
string lub(vector<string> classes);

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
		vector<string> types;
		types.push_back(thenchild->valType);
		types.push_back(elsechild->valType);
		node->valType = lub(types);

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
			cerr << "ERROR IN " << enum2string(node->type) << " WITH TYPES"<< endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right ->valType << endl;
			cerr << endl;
			node->valType = "Object";
		}
		else if (right->valType != "Int"){
			//TODO: Add Error Here
			cerr << "ERROR IN " << enum2string(node->type) << " WITH TYPES" << endl;
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
	case NodeType::AST_DISPATCH: {
		Node *caller = (Node *)node->getChildren()[0];
		Node *atType = (Node *)node->getChildren()[1];
		Node *id = (Node *)node->getChildren()[2];
		Node *params = (Node *)node->getChildren()[3];
		SymTableMethod *method;

		/* Case 1: calling a method within the class*/
		if (caller->type == NodeType::AST_NULL) {
			method = globalSymTable->getMethod(id->value);
		}
		else if (atType->type == NodeType::AST_NULL) { //specify an object
			method = globalSymTable->getMethodByClass(id->value, caller->valType);
		}
		else { //Specify which class to use
			method = globalSymTable->getMethodByClass(id->value, atType->valType);
		}

		if (method == nullptr) {
			cerr << "Cannot find method '" << id->value << "' in current scope" << endl;
			node->valType = "Object";
			break;
		}

		/* build a vector with types of actual parameters*/
		vector<string> param_types;
		Node *actual;
		for (int i = params->getChildren().size() - 1; i >= 0; i--) { //they'll be backwards if we do i 0 to n
			actual = (Node *)params->getChildren()[i];
			param_types.push_back(actual->valType);
		}

		/* error check */
		if (param_types.size() != method->argTypes.size()) {
			cerr << "Mismatch in number of expected parameters vs given" << endl;
		}
		else {
			for (int i = 0; i < param_types.size(); i++) {
				if (param_types[i] != method->argTypes[i]) {
					cerr << "Type of parameter given: " << param_types[i] << ", expected: " << method->argTypes[i] << endl;
				}
			}
		}
		node->valType = method->returnType;
		
		break;
	}
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
	case NodeType::AST_CASESTATEMENT: {
		vector<string> types;
		Node *case_node;
		for (auto c : node->getChildren()[1]->getChildren()) {
			case_node = (Node *) c;
			types.push_back(case_node->valType);
		}
		node->valType = lub(types);
		break;
	}
	case NodeType::AST_FEATURE_METHOD: {
		Node *type = (Node *)node->getChildren()[2];
		Node *expr = (Node *)node->getChildren()[3];
		if (type->valType != expr->valType) {
			cerr << "Type mismtach between declared type of method and derived type" << endl;
		}
		break;
	}
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

/* This function does not perserve formal classes, so don't use it after
 * calling this
 */
string lub(vector<string> classes) {
	/* gotta get the distances to Object first */
	vector<int> distances(classes.size()); //all init to 0
	string tmp;
	int n = classes.size(); //we'll be using this a whole lot
	for (int i = 0; i < n; i++) {
		tmp = classes[i];
		while (tmp != "Object") {
			tmp = globalTypeList[tmp];
			distances[i]++;
		}
	}
	int min = distances[0];
	for (int i = 1; i < n; i++) {
		if (distances[i] < min)
			min = distances[i];
	}
	/* check if object is lub */
	if (min == 0)
		return "Object";

	/* bring everything to the same distance to object */
	for (int i = 0; i < n; i++) {
		while (distances[i] != min) {
			classes[i] = globalTypeList[classes[i]];
			distances[i]--;
		}
	}

	/* check if they're all equal, if not move everything up one in the hierarchy */
	bool check;
	while (true) { //eventually everything reaches object
		check = true;
		for (int i = 0; i < n - 1; i++) {
			check &= classes[i] == classes[i + 1];
		}
		if (check)
			return classes[0];
		for (int i = 0; i < n; i++)
			classes[i] = globalTypeList[classes[i]];
	}
	return ""; //this will never happen
}