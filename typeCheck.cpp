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
	case NodeType::AST_IDENTIFIER: {
		SymTableVariable *var = globalSymTable->getVariable(node->value);
		if (var == nullptr) {
			cerr << "Variable " << node->value << " Not found in current scope" << endl;
			node->valType = "Object";
			break;
		}
		node->valType = var->type;
		break;
	}
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
		
		//Checking to make sure we have the types before calling Lub

		if ((globalTypeList.count(types[0]) == 0 && types[0] != "SELF_TYPE" ) || (globalTypeList.count(types[1]) == 0 && types[1] != "SELF_TYPE")) {
			cerr << "Undeclared type in IF-Then-Else Block" << endl;
			node->valType = "Object";
		}
		else {
			node->valType = lub(types);
		}

		break;
	}
	case NodeType::AST_ISVOID:
		node->valType = "Bool";
		break;
	case NodeType::AST_NEW: {
		Node * child = (Node *)node->getChild();
		if (child->valType == "SELF_TYPE") {
			node->valType = globalSymTable->getCurrentClass();//TODO create this	
		}
		else {
			node->valType = child->valType;
		}
		break;
	}
	case NodeType::AST_NOT: {
		//check for BOOL
		Node* child = (Node *)node->getChild();
		if (child->valType != "Bool") {
			cerr << "TYPE ERROR IN AST_NOT" << endl;
			node->valType = "Bool";
		}
		else {
			node->valType = "Bool";
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
			cerr << "ERROR IN " << enum2string(node->type) << " WITH TYPES"<< endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right ->valType << endl;
			cerr << endl;
			numErrors++;
			node->valType = "Object";
		}
		else if (right->valType != "Int"){
			cerr << "ERROR IN " << enum2string(node->type) << " WITH TYPES" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			numErrors++;
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
				cerr << "ERROR WITH TYPES IN AST_EQUAL" << endl;
				cerr << "LEFT  TYPE IS " << left->valType << endl;
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				cerr << endl;
				numErrors++;
			}
		}
		else if (right->valType == "Bool" || right->valType == "Int" || right->valType == "String") {
			if (right->valType != left->valType) {
				cerr << "ERROR WITH TYPES IN AST_EQUAL" << endl;
				cerr << "LEFT  TYPE IS " << left->valType << endl;
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				cerr << endl;
				numErrors++;
			}
		}
		node->valType = "Bool";
		break;
	}
	case NodeType::AST_LE:
	case NodeType::AST_LT: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != "Int") {
			cerr << "ERROR WITH TYPES IN AST_COMPARE" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			numErrors++;
		}
		else if (right->valType != "Int") {
			cerr << "ERROR WITH TYPES IN AST_COMPARE" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			numErrors++;
		}
		node->valType = "Bool";
		break;
	}
	case NodeType::AST_LARROW: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		//if (left->valType != right->valType) {
		if(!globalSymTable->isSubClass(left->valType,right->valType)) {
			//TODO write this method 
			//TODO: Add Error Here
			cerr << "ERROR WITH TYPES IN AST_LARROW" << endl;
			cerr << "LEFT  TYPE IS " << left->valType << endl;
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			cerr << endl;
			numErrors++;
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

		if (letexpression->valType == "SELF_TYPE") {
			node->valType = globalSymTable->getCurrentClass(); //TODO forest write this
		}
		else {
			node->valType = letexpression->valType;
		}
		break;
	}
	case NodeType::AST_TILDE: {
		//check for INT
		Node* child = (Node *)node->getChildren()[0];
		if (child->valType != "Int") {
			cerr << "TYPE ERROR IN AST_TILDE" << endl;
			cerr << "Should be type Int but is type " << child->valType << endl;
			cerr << endl;
			numErrors++;
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
		int type;

		/* Case 1: calling a method within the class*/
		if (caller->type == NodeType::AST_NULL || (caller->valType == "SELF_TYPE" && atType->type == NodeType::AST_NULL)) {
			method = globalSymTable->getMethod(id->value);
			type = 0;
		}
		else if (atType->type == NodeType::AST_NULL) { //specify an object
			method = globalSymTable->getMethodByClass(id->value, caller->valType);
			type = 1;
		}
		else { //Specify which class to use
			if (atType->valType == "SELF_TYPE") {
				cerr << "Cannot have @SELF_TYPE on line: " << node->lineNumber << endl;
				node->valType = "Object";
				break;
			}
			method = globalSymTable->getMethodByClass(id->value, atType->valType);
			type = 2;
		}

		if (method == nullptr) {
			cerr << "Cannot find method '" << id->value << "' in current scope" << endl;
			numErrors++;
			node->valType = "Object";
			break;
		}

		/* build a vector with types of actual parameters*/
		vector<string> param_types;
		Node *actual;
		for (int i = params->getChildren().size() - 1; i >= 0; i--) { //they'll be backwards if we do i 0 to n
			actual = (Node *)params->getChildren()[i];
			if (actual->valType == "SELF_TYPE") {
				param_types.push_back(globalSymTable->getCurrentClass());//TODO forest write this
			}
			else {
				param_types.push_back(actual->valType);
			}
		}

		/* error check */
		if (param_types.size() != method->argTypes.size()) {
			cerr << "Mismatch in number of expected parameters vs given in function " << id->value << endl;
			numErrors++;
		}
		else {
			for (int i = 0; i < param_types.size(); i++) {
				//if (param_types[i] != method->argTypes[i]) {
				if (!globalSymTable->isSubClass(param_types[i], method->argTypes[i])) {//TODO write this method
					cerr << "Type of parameter given: " << param_types[i] << ", expected: " << method->argTypes[i] << " in method " << id->value << endl;
					//TODO: numErrors++ here?
				}
			}
		}
		
		if (method->returnType == "SELF_TYPE") {
			switch (type) {
			case 0:
				node->valType = "SELF_TYPE";
				break;
			case 1:
				node->valType = caller->valType;
				break;
			case 2:
				node->valType = atType->valType;
				break;
			default:
				cerr << "how? how did you get here?" << endl;
				node->valType = "Object";
			}
			//node->valType = globalSymTable->getCurrentClass();//TODO this might be weird
		}
		else {
			node->valType = method->returnType;
		}
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
		if (expr->type != NodeType::AST_NULL && expr->valType != type->valType) {
			cerr << "type mismatch in assignment of let statement" << endl;
			numErrors++;
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

		bool badType = false;
		//for-loop to check each type
		for (string type : types) {
			if (globalTypeList.count(type) == 0 && type != "SELF_TYPE") {
				badType = true;
				break;
			}
		}

		if (badType) {
			cerr << "Undeclared type in Case Statement" << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = lub(types);
		}
		break;
	}
	case NodeType::AST_FEATURE_METHOD: {
		Node *type = (Node *)node->getChildren()[2];
		Node *expr = (Node *)node->getChildren()[3];
		if (type->valType != expr->valType) {
			cerr << "Type mismtach between declared type of method and derived type" << endl;
			numErrors++;
		}
		break;
	}
	case NodeType::AST_FEATURE_ATTRIBUTE: {
		Node *expr = (Node *)node->getChildren()[2];
		Node *type = (Node *)node->getChildren()[1];
		if (expr->type != NodeType::AST_NULL && expr->type != type->type) {
			cerr << "type mismatch in feature assignment" << endl;
			numErrors++;
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
	/* check if classes contain self type*/
	int counter = 0;
	for (string s : classes) {
		if (s == "SELF_TYPE") {
			counter++;
		}
	}
	if (counter != 0 && counter == classes.size()) {
		return "SELF_TYPE";
	}
	else if (counter != 0) {
		return "Object";
	}

	/* gotta get the distances to Object first */
	vector<int> distances(classes.size()); //all init to 0
	string tmp;

	//TYPECHECK MOTHERFUCKER YOU SHOULD USE IT
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