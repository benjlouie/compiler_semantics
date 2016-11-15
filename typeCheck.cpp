/*****************************************************************************************************
*	Main Authors: Matt karasz, Benjamin Cope, Ben Louie, Robert Blasi, Forest Thomas
*	Sub Authors: 
*
*	Description: This file recursively walks the AST from the bottom up checking types as it goes
******************************************************************************************************/
#include "typeCheck.h"
#include <iostream>

TypeErr typeCheck(void);
TypeErr typeCheck_recursive(Node *ASTNode, unsigned &currentLetCount, unsigned &currentCaseCount);
TypeErr deSwitch(Node *node);
string lub(vector<string> classes);

/*	This function starts the main typechecking pass.
*/
TypeErr typeCheck(void)
{
	while (globalSymTable->getScope() != "Object") {
		globalSymTable->leaveScope();
	}
	unsigned letCount = 0;
	unsigned caseCount = 0;
	return typeCheck_recursive(root, letCount, caseCount);
}

/*	This function recursively descends through the AST.It sets up the scope for the calls to
	our main switch statement to typecheck each node type.
*/
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

		/*	When encountering an AST_LET node enter a new scope, name it "let" followed
		by the value of currentLetCount, and increment currentLetCount.	
		*/
		case NodeType::AST_LET:
			if (ASTNode->type != NodeType::AST_LET) {
				globalSymTable->enterScope("let" + to_string(currentLetCount));
				enteredNewScope = true;
				currentLetCount++;
			}
			break;


		/*	When encountering an AST_FEATURE_METHOD node enter a new scope and name it
			the same name as the method
		*/
		case NodeType::AST_FEATURE_METHOD:
		{
			string methodName = ((Node *)child->getChildren()[0])->value;
			globalSymTable->enterScope(methodName);
			enteredNewScope = true;
			break;
		}

		/*	When encountering an AST_CASE node enter a new scope, name it "case" followed
			by the value of currentCaseCount, and increment currentCaseCount for future
			AST_CASE nodes.
		*/
		case NodeType::AST_CASE:
		{
			globalSymTable->enterScope("case" + to_string(currentCaseCount));
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

/*	This function is a large switch statement which decides how to type check each node type
*/
TypeErr deSwitch(Node *node)
{
	auto children = node->getChildren();
	for (auto tchild : children) {
		Node *child = (Node *)tchild;
		if (globalTypeList.count(child->valType) == 0 && child->valType != "SELF_TYPE") {
			if (child->valType != "" && child->valType != "SELF_TYPE") {
				cerr << child->lineNumber << ": Undefined type '" << child->valType << "'" << endl;
			}
			if (child->valType != "SELF_TYPE") {
				child->valType = "Object";
			}
		}
	}

	switch (node->type) {
	/*	This case handles AST_IDENTIFIER nodes. The case checks to make sure that the
		type listed for the id is reachable from the current scope. If the id is not
		reachable then an error message is printed, the error counter is incremented, and
		the node is assigned type Object to allow semantic analysis to continue. If there
		are no errors then the node is assigned the value in its type field. In this case
		the string in the type field is the type listed for the id. EX X:Int, Int would be
		in the type field.
	*/
	case NodeType::AST_IDENTIFIER: {
		SymTableVariable *var = globalSymTable->getVariable(node->value);
		if (var == nullptr) {
			cerr << node->lineNumber << ": Variable " << node->value << " Not found in current scope" << endl;
			node->valType = "Object";
			numErrors++;
			break;
		}
		node->valType = var->type;
		break;
	}
	
	/*	This case handles AST_CASE nodes. The case first checks to make sure that the defined
		type of one specific case usable in the current scope. If that type is not usable
		then an error message is printed, the error counter is incremented, and the node is
		assigned type Object to allow semantic analysis to continue. If there are no errors
		then the node is assigned the type of its third child. In this case the third child
		is the expression of the specific case. 
	*/
	case NodeType::AST_CASE: {
		auto children = node->getChildren();
		Node * type = (Node *)children[1];
		Node * asttypechild = (Node *)children[2];
		if (globalTypeList.count(type->valType) == 0 && type->valType != "SELF_TYPE") {
			cerr << node->lineNumber << ": type '" << type->valType << "' not defined" << endl;
			numErrors++;
			node->valType = "Object";
			break;
		}
		node->valType = asttypechild->valType;
		break;
	}

	/*	This case handles AST_TYPE nodes. The case simply assigns the type of the node
		to be the value of the node. In this case value is the string listed as the type
		for an assignment or declaration.
	*/
	case NodeType::AST_TYPE:
		node->valType = node->value;
		break;

	/*	This case handles AST_IF nodes. The case firsts checks to make sure the first child
		is type Bool. In this case the first child is the condition of the if statement. Next
		the case checks to see if the types of the second and third child are reachable from
		the current scope. Then the case gets the types of the second and third child and finds
		the least upper bound of the two of them. In this case the second child is the 
		expression following the THEN keyword and the third child is the expression following
		the ELSE keyword. If there are any errors then an error message is printed, the error
		counter is incremented and the node is assigned type Object to allow semantic analysis
		to continue. If there are no errors then the node is assigned the type of the least
		upper bound of its THEN and ELSE child.
	*/
	case NodeType::AST_IF: {
		//Node(AST_IF,3,$2,$4,$6)
		auto children = node->getChildren();
		Node * iftest = (Node *)children[0];
		Node * thenchild = (Node *)children[1];
		Node * elsechild = (Node *)children[2];

		//if stuff
		if (iftest->valType != "Bool") {
			cerr << node->lineNumber << ": IF CONDITION DOES NOT EVALUATE TO BOOLEAN" << endl;
			numErrors++;
		}
		vector<string> types;
		types.push_back(thenchild->valType);
		types.push_back(elsechild->valType);
		
		//Checking to make sure we have the types before calling Lub

		if ((globalTypeList.count(types[0]) == 0 && types[0] != "SELF_TYPE" ) || (globalTypeList.count(types[1]) == 0 && types[1] != "SELF_TYPE")) {
			cerr << node->lineNumber << ": Undeclared type in IF-Then-Else Block" << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = lub(types);
		}

		break;
	}

	/*	This case handles AST_ISVOID nodes. The case simply assigns type Bool to the node.
	*/
	case NodeType::AST_ISVOID:
		node->valType = "Bool";
		break;

	/*	This case handles AST_NEW nodes. The case first checks to make sure that the type
		being used in the NEW actually exists and is useable in the current scope. If the
		type is not valid then an error message is printed, the error counter is incremented,
		and the node is assigned type object to allow semantic analysis to continue. If the 
		type being used is SELF_TYPE then the node is assigned the type of the current class.
		Otherwise the node is assigned the type of its child.
	*/
	case NodeType::AST_NEW: {
		Node * child = (Node *)node->getChild();
		if (globalTypeList.count(child->valType) == 0 && child->valType != "SELF_TYPE") {
			cerr << node->lineNumber << ": type '" << child->valType << "' not defined" << endl;
			numErrors++;
			node->valType = "Object";
			break;
		}
		if (child->valType == "SELF_TYPE") {
			node->valType = globalSymTable->getCurrentClass();	
		}
		else {
			node->valType = child->valType;
		}
		break;
	}

	/*	This case handles AST_NOT nodes. The case checks to make sure that the expression
		to the right side of the NOT is of type Bool, sice NOT only works on Bools.
		If the expresion is not of type Bool then an error message is printed and the 
		error counter is incremented. Regardless of whether or not there were errors 
		the node is assigned type Bool. This is done to allow semantic analysis to continue
		and to avoid cascading errors.
	*/
	case NodeType::AST_NOT: {
		
		Node* child = (Node *)node->getChild();
		if (child->valType != "Bool") {
			cerr << node->lineNumber << ": RIGHT HAND SIDE OF AST_NOT IS NOT BOOLEAN TYPE" << endl;
			node->valType = "Bool";
			numErrors++;
		}
		else {
			node->valType = "Bool";
		}
		break;
	}

	/*	This case handles AST_WHILE nodes. The case first checks to ensure that the 
		comparison expression is type Bool and if it is not type Bool then an error is 
		printed and the error counter is incremented. Regardless of whether or not there
		were errors the node is assigned type Object because whiles always evaluate to type
		Object anyway.
	*/
	case NodeType::AST_WHILE: {
		//Node(AST_WHILE,2,$2,$4) Our grammar rule.

		auto children = node->getChildren();
		Node * expressiontest = (Node *)children[0];

		//check the type of the comparison
		if (expressiontest->valType != "Bool") {
			cerr << node->lineNumber << ": ERROR IN EXPRESION TEST, SHOULD BE BOOLEAN BUT IS TYPE " << expressiontest->valType << endl;
			numErrors++;
		}
		node->valType = "Object";
		break;
	}

	/*	This case handles AST_INTEGERLITERAL nodes. The case simply assigns type
		Int to the node.
	*/
	case NodeType::AST_INTEGERLITERAL:
		node->valType = "Int";
		break;

	/*	This case handles AST_STRING nodes. The case simply assigns type String to the node.
	*/
	case NodeType::AST_STRING:
		node->valType = "String";
		break;

	/*	This case handles AST_TRUE and AST_FALSE nodes since they are semantically 
		equivalent. The case just assigns the node type Bool.
	*/
	case NodeType::AST_TRUE:
	case NodeType::AST_FALSE:
		node->valType = "Bool";
		break;

	/*	This case handles AST_MINUS, AST_PLUS, AST_DIVIDE, and AST_TIMES nodes since they
		are all equivalent for semantic analysis. All four of these operators only work
		with type Int so the case first makes sure both sides of the operator are type Int.
		If there are any error then an error message is printed, the error counter is 
		incremented, and the node is assigned type Object to allow semantic analysis to
		continue. If there are no errors then the node is assigned type Int.
	*/
	case NodeType::AST_MINUS:
	case NodeType::AST_PLUS:
	case NodeType::AST_DIVIDE:
	case NodeType::AST_TIMES: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != "Int") {
			cerr << node->lineNumber << ": TYPE MISMATCH IN " << enum2string(node->type) << " ";
			cerr << "LEFT  TYPE IS " << left->valType << " ";
			cerr << "RIGHT TYPE IS " << right ->valType << endl;
			numErrors++;
			node->valType = "Object";
		}
		else if (right->valType != "Int"){
			cerr << node->lineNumber << ": TYPE MISMATCH IN " << enum2string(node->type) << " ";
			cerr << "LEFT  TYPE IS " << left->valType << " ";
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = "Int";
		}
		break;
	}
	
	/*	This case handles AST_EQUALS nodes. Since Ints, Strings, and Bools can only be compared
		with the same type we have two cases. If one of the two sides of the comparison is
		Int, String, or Bool then we check that the other side is also Int, String, or Bool.
		So Int must be compared with Int, Bool must be compared with Bool, and String must be
		compared with String. If the two sides are not the same type then an error message
		is printed and the error counter is incremented. If both sides of the comparison
		are any type other than Int,String,Bool then the node is assigned type Bool. However,
		in the event of an error the node is assigned type Bool anyway to both allow 
		semantic analysis to continue and to avoid cascading errors.
	*/
	case NodeType::AST_EQUALS: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType == "Bool" || left->valType == "Int" || left->valType == "String") {
			if (right->valType != left->valType) {
				cerr << node->lineNumber << ": TYPE MISMATCH IN EQUALS ";
				cerr << "LEFT  TYPE IS " << left->valType << " ";
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				numErrors++;
			}
		}
		else if (right->valType == "Bool" || right->valType == "Int" || right->valType == "String") {
			if (right->valType != left->valType) {
				cerr << node->lineNumber << ": TYPE MISMATCH IN EQUALS ";
				cerr << "LEFT  TYPE IS " << left->valType << " ";
				cerr << "RIGHT TYPE IS " << right->valType << endl;
				numErrors++;
			}
		}
		node->valType = "Bool";
		break;
	}

	/*	This case handles AST_LE and AST_LT since they are both equivalent in semantic
		analysis. Since < and <= only works on type Int the case checks to make sure
		both the left and right sides of the comparison are type Int. If they are not
		then an error message is printed and the error counter is incremented. If both 
		sides are type Int then the node is assigned type Bool.
	*/
	case NodeType::AST_LE:
	case NodeType::AST_LT: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if (left->valType != "Int") {
			cerr << node->lineNumber << ": LEFT SIDE OF COMPARE IS NOT OF TYPE INT BUT TYPE " << left->valType << endl;
			numErrors++;
		}
		else if (right->valType != "Int") {
			cerr << node->lineNumber << ": RIGHT SIDE OF COMPARE IS NOT OF TYPE INT BUT TYPE " << right->valType << endl;
			numErrors++;
		}
		node->valType = "Bool";
		break;
	}

	/*	This case handles AST_LARROW nodes. The case first checks to make sure
		that the type of the left side of the <- matches the type of the right
		side of the <-. If both sides match then the node is assigned the type of
		both sides. If the sides do not match then an error message is printed out,
		the error counter is incremented, and the node is assigned type object to
		allow semantic analysis to continue.
	*/
	case NodeType::AST_LARROW: {
		node->getChildren();
		Node *left = (Node *)node->getLeftChild();
		Node *right = (Node *)node->getRightChild();
		if(!globalSymTable->isSubClass(right->valType,left->valType)) {
			cerr << node->lineNumber << ": TYPE MISMATCH IN AST_LARROW ";
			cerr << "LEFT  TYPE IS " << left->valType << " ";
			cerr << "RIGHT TYPE IS " << right->valType << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = left->valType;
		}
		break;
	}

	/*	This case handles AS_LET nodes. The case simply assigns the type of the
		expression at the end of the let to the let node.
	*/
	case NodeType::AST_LET: {
		auto children = node->getChildren();
		Node * letexpression  = (Node *)children[1];
		node->valType = letexpression->valType;
		break;
	}
	
	/*	This case handles AST_TILDE nodes. The case first checks to make sure that
		the expresion to the right of the tilde evaluates to type Int. If the 
		expression is not of type Int then an error message is printed and the error
		counter is incremented. If there are no errors then the node is assigned type 
		Int.
	*/
	case NodeType::AST_TILDE: {
		
		Node* child = (Node *)node->getChildren()[0];
		if (child->valType != "Int") {
			cerr << node->lineNumber <<  ": RIGHT SIDE OF TILDE EXPECTS TYPE INT BUT IS TYPE "<< child->valType << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = "Int";
		}
		break;
	}

	/*	This case handles AST_DISPATCH nodes. In case 1 we simply set a temporary variable 
		for handling later. In case 2 we again just set a temporary variable for later.
		In case 3 we check to make sure we do not have @SELF_TYPE since that is not allowed
		in COOL, otherwise we set a temporary variable for later. We then check to make sure
		the method being called can actually be reached from the current scope. We then make
		a vector of input arguements being passed to the method being called and make sure 
		that the vector of arguements matches the method definition for the method being called.
		Finally we assign the type of the node based on that temporary variable we set earlier
		if all error checking is passed.
	*/
	case NodeType::AST_DISPATCH: {
		Node *caller = (Node *)node->getChildren()[0];
		Node *atType = (Node *)node->getChildren()[1];
		Node *id = (Node *)node->getChildren()[2];
		Node *params = (Node *)node->getChildren()[3];
		SymTableMethod *method;
		int type;

		/* Case 1: calling a method within the class */
		if (caller->type == NodeType::AST_NULL || (caller->valType == "SELF_TYPE" && atType->type == NodeType::AST_NULL)) {
			method = globalSymTable->getMethod(id->value);
			type = 0;
		}
		/* Case 2: calling a method from an object */
		else if (atType->type == NodeType::AST_NULL) { //specify an object
			method = globalSymTable->getMethodByClass(id->value, caller->valType);
			type = 1;
		}
		/* Calling a method with the @ symbol */
		else { //Specify which class to use
			if (atType->valType == "SELF_TYPE") {
				cerr << node->lineNumber << ": Cannot have @SELF_TYPE" << endl;
				node->valType = "Object";
				numErrors++;
				break;
			}
			method = globalSymTable->getMethodByClass(id->value, atType->valType);
			type = 2;
		}

		if (method == nullptr) {
			cerr << node->lineNumber << ": Cannot find method '" << id->value << "' in current scope" << endl;
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
				param_types.push_back(globalSymTable->getCurrentClass());
			}
			else {
				param_types.push_back(actual->valType);
			}
		}

		/* error check */
		if (param_types.size() != method->argTypes.size()) {
			cerr << node->lineNumber << ": Mismatch in number of expected parameters in function " << id->value << endl;
			numErrors++;
		}
		else {
			for (int i = 0; i < param_types.size(); i++) {
				if (!globalSymTable->isSubClass(param_types[i], method->argTypes[i])) {
					cerr << node->lineNumber << ": Type of parameter given: " << param_types[i] << ", expected: " << method->argTypes[i] << " in method " << id->value << endl;
					numErrors++;
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
				cerr << node->lineNumber <<  ": how? how did you get here?" << endl;
				node->valType = "Object";
			}
		}
		else {
			node->valType = method->returnType;
		}
		break;
	}

	/*	This case handles AST_EXPRESEMILIST nodes. This case just assigns the type of the 
		last child to the type of the node.
	*/
	case NodeType::AST_EXPRSEMILIST: {
		auto children = node->getChildren();
		Node * lastchild = (Node *)children[children.size()-1];

		node->valType = lastchild->valType;

		break;
	}
	
	/*	This case handles AST_IDTYPEEXPR nodes. The case starts by checking to make sure 
		that both children are valid/declared type. If both children are valid types then
		no type is assigned to the node because it is not needed. If they are not both valid
		types then the proper error messages are printed and the error count is incremented.
	*/
	case NodeType::AST_IDTYPEEXPR: {
		Node *expr = (Node *)node->getChildren()[2];
		Node *type = (Node *)node->getChildren()[1];
		if (globalTypeList.count(type->valType) == 0 && type->valType != "SELF_TYPE") {
			cerr << node->lineNumber << ": type '" << type->valType << "' not defined" << endl;
			numErrors++;
			break;
		}
		if (expr->type != NodeType::AST_NULL && !globalSymTable->isSubClass(expr->valType, type->valType)) {
			cerr << node->lineNumber << ": Type mismatch. DECLARED TYPE OF " << type->valType << " BUT DERIVED TYPE OF " << expr->valType << endl;
			numErrors++;
		}
		break;
	}

	/*	This case handles AST_CASESTATMENT nodes. It first makes a vector of the types
		of all the children of the node. It then checks that every element of the vector is
		a valid/declared type. Finally if all types are valid then the casestatement is 
		set to be the least upper bound of all the types in the vector. If not all types are 
		valid/declared then the proper error messages are printed and the error counter is 
		incremented.
	*/
	case NodeType::AST_CASESTATEMENT: {
		vector<string> types;
		unordered_map<string, bool> dupTypeCheck; //only for checking duplicate types
		Node *case_node;
		for (auto c : node->getChildren()[1]->getChildren()) {
			case_node = (Node *) c;
			types.push_back(case_node->valType);

			//check for duplicate types
			Node *caseTypeNode = (Node *)case_node->getChildren()[1];
			if (dupTypeCheck.count(caseTypeNode->value) > 0) {
				//duplicate type in case statements
				cerr << caseTypeNode->lineNumber << ": Duplicate type \"" + caseTypeNode->value + "\" in Case Statement" << endl;
				numErrors++;
			}
			else {
				dupTypeCheck[caseTypeNode->value]; //add to map
			}
		}
		dupTypeCheck.clear(); //no point in keeping the map

		bool badType = false;
		//for-loop to check each type
		for (string type : types) {
			if (globalTypeList.count(type) == 0 && type != "SELF_TYPE") {
				badType = true;
				break;
			}
		}

		if (badType) {
			cerr << node->lineNumber << ": Undeclared type in Case Statement" << endl;
			numErrors++;
			node->valType = "Object";
		}
		else {
			node->valType = lub(types);
		}
		break;
	}

	/*	This case handles AST_FEATURE_METHOD nodes. It first checks to make sure that
	the children of the feature are all valid/declared types. It then checks to make sure
	that the expression side of the statement evaluates to the same type as the type side
	of the statement. If no errors are found then no type is assigned to the node since
	feature attributes dont need to be type checked. If an error is found then the proper
	error is printed and the error count is incremented.
	*/
	case NodeType::AST_FEATURE_METHOD: {
		Node *type = (Node *)node->getChildren()[2];
		Node *expr = (Node *)node->getChildren()[3];
		if (globalTypeList.count(type->valType) == 0 && type->valType != "SELF_TYPE") {
			cerr << node->lineNumber << ": type '" << type->valType << "' not defined" << endl;
			numErrors++;
			break;
		}
		if(!globalSymTable->isSubClass(expr->valType, type->valType)) {
			cerr << node->lineNumber <<  ": Type mismatch. DECLARED TYPE OF " << type->valType << " BUT DERIVED TYPE OF " << expr->valType << endl;
			numErrors++;
		}
		break;
	}
	
	/*	This case handles AST_FEATURE_ATTRIBUTE nodes. It first checks to make sure that
		the children of the feature are all valid/declared types. It then checks to make sure
		that the expression side of the statement evaluates to the same type as the type side
		of the statement. If no errors are found then no type is assigned to the node since 
		feature attributes dont need to be type checked. If an error is found then the proper
		error is printed and the error count is incremented.
	*/
	case NodeType::AST_FEATURE_ATTRIBUTE: {
		Node *expr = (Node *)node->getChildren()[2];
		Node *type = (Node *)node->getChildren()[1];
		if (globalTypeList.count(type->valType) == 0 && type->valType != "SELF_TYPE") {
			cerr << node->lineNumber << ": type '" << type->valType << "' not defined" << endl;
			numErrors++;
			break;
		}
		if (expr->type != NodeType::AST_NULL && !globalSymTable->isSubClass(expr->valType, type->valType)) {
			cerr << node->lineNumber << ": Type mismatch. LEFT SIDE IS TYPE " << type->valType << " BUT RIGHT SIDE IS TYPE " << expr->valType << endl;
			numErrors++;
		}
		break;
	}
	default:
		break;

	}

	return TypeErr::TYPE_OK;
}

/*	This function is used to find the class which is the least upper bound of a given vector
	of types. 
 */
string lub(vector<string> classes) {
	
	int counter = 0;

	/* check if classes contain self type*/
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


	vector<int> distances(classes.size()); //all init to 0
	string tmp;

	/*	Finds the distance to object for each type in the vector */
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