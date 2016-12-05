#include "constprop.h"

bool ConstProp::init() 
{
	this->setSettings(ConstPropSettings());
	auto rootChildren = root->getChildren();
	for (Tree *tchild : rootChildren) {
		Node *classNode = (Node *)tchild;

		if (classNode->type != NodeType::AST_CLASS) {
			//shouldn't EVER get here. If you do, what the hell man.
			numErrors++;
			cerr << "Constant propogation ran into a case when starting a new class that wasn't a class. What." << endl;
			continue;
		}

		Node *features = (Node *)(classNode->getChildren().at(3));
		if (features->type != NodeType::AST_FEATURESET) {
			//shouldn't EVER get here. If you do, what the hell man.
			numErrors++;
			cerr << "Constant propogation ran into a case when starting a new class that didn't have a feature set. What." << endl;
			continue;
		}

		vector<Node *> attrVector;
		vector<Node *> methodVector;

		auto featChildren = features->getChildren();
		for (auto tFeatChild : featChildren) {
			Node *featChild = (Node *)tFeatChild;
			switch (featChild->type) {
			case AST_FEATURE_ATTRIBUTE: {
				attrVector.push_back(featChild);
				break;
			}
			case AST_FEATURE_METHOD: {
				methodVector.push_back(featChild);
				break;
			}
			default:
				//Should NEVER be reached
				numErrors++;
				cerr << "Constant propogation ran into a case when starting a new class that a node of ast_featureset is not a feature method or attribute. What." << endl;
			}
		}

		//go through all of the attributes

		//go through each method, but before calling massiveSwitch, 
		//add the formal variable and its type


		
		

	}

	if (numErrors > 0) {
		return false;
	}

	return true;
}


bool ConstProp::doConstProp(vector<Node *> nodes)
{

	for (auto node : nodes) {

	}
	cerr << "doConstProp with nodes isn't yet implemented." << endl;
	return false;
}

bool ConstProp::doConstProp(Node * node)
{
	vector<Node *> nodeVect;
	nodeVect.push_back(node);
	return doConstProp(nodeVect);
}

//3 errors in here
bool ConstProp::massiveSwitch(Node *expr)
{
	switch (expr->type) {
	case AST_INTEGERLITERAL:
	case AST_STRING:
	case AST_TRUE:
	case AST_FALSE:
	case AST_NULL: 
	case AST_NEW:
		return true;
	case AST_NOT: {
		Node *notChild = (Node *)(expr->getChild());
		if (notChild->valType == "Bool") {
			massiveSwitch(notChild);
		}
		else {
			numErrors++;
			cerr << "Constant propogation ran into a scenario where a not worked on something other than a bool. Wat." << endl;
		}

		notChild = (Node *)(expr->getChild());

		if (notChild->type == AST_TRUE) {
			expr->replaceSelf(new Node(AST_FALSE));
		}
		else if (notChild->type == AST_FALSE) {
			expr->replaceSelf(new Node(AST_TRUE));
		}

		break;
	}
	case AST_TILDE: {
		Node *tildeChild = (Node *)(expr->getChild());
		if (tildeChild->valType == "Int") {
			massiveSwitch(tildeChild);
		}
		else {
			numErrors++;
			cerr << "Constant propogation ran into a scenario where a tilde worked on something other than an int on line " << expr->lineNumber << ". Wat." << endl;
		}

		tildeChild = (Node *)(expr->getChild());

		if (tildeChild->type == AST_INTEGERLITERAL) {
			int val = this->getVal(tildeChild);
			
			//0 - <positive> = <negative>, 0 - <negative> = <positive>
			expr->replaceSelf(new Node(AST_INTEGERLITERAL, to_string(0 - val)));
		}
		break;
	}
	case AST_ISVOID: {
		Node *isVoidChild = (Node *)(expr->getChild());
		massiveSwitch(isVoidChild);
		isVoidChild = (Node *)(expr->getChild());

		switch (isVoidChild->type)
		{
		case AST_INTEGERLITERAL:
		case AST_TRUE:
		case AST_FALSE:
		case AST_STRING:
			expr->replaceSelf(new Node(AST_FALSE));
		default:
			//Can't do anything else because you don't know if a value is null until run time
			//Or the child is a while. However, if it's a while, we need to exec the stuff 
			//inside the while.
			break;
		}

		break;
	}
	case AST_MINUS:
	case AST_PLUS: {
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 2) {
			numErrors++;
			cerr << "A plus op on line " << expr->lineNumber << " doesn't have 2 children. Wat." << endl;
			break;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *rightChild = (Node *)(tChildren.at(1));
		massiveSwitch(leftChild);
		massiveSwitch(rightChild);
		leftChild = (Node *)(tChildren.at(0));
		rightChild = (Node *)(tChildren.at(1));

		int leftVal = -1;
		int rightVal = -1;
		if (leftChild->type == AST_INTEGERLITERAL && rightChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			rightVal = getVal(rightChild);
			int total;
			if (expr->type == AST_PLUS) {
				total = leftVal + rightVal;
			}
			else {
				total = leftVal - rightVal;
			}
			expr->replaceSelf(new Node(AST_INTEGERLITERAL, to_string(total)));
		}
		else if (leftChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			if (leftVal == 0) {
				if (expr->type == AST_PLUS) {
					expr->replaceSelf(rightChild);
				}
				else {
					expr->replaceSelf(new Node(AST_TILDE, 1, rightChild));
				}
			}
		}
		else if (rightChild->type == AST_INTEGERLITERAL) {
			rightVal = getVal(rightChild);
			if (rightVal== 0) {
				expr->replaceSelf(leftChild);
			}
		}
		
	}
	case AST_TIMES: { //multiply, cause times is confusing.
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 2) {
			numErrors++;
			cerr << "A plus op on line " << expr->lineNumber << " doesn't have 2 children. Wat." << endl;
			break;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *rightChild = (Node *)(tChildren.at(1));
		massiveSwitch(leftChild);
		massiveSwitch(rightChild);
		leftChild = (Node *)(tChildren.at(0));
		rightChild = (Node *)(tChildren.at(1));

		int leftVal = -1;
		int rightVal = -1;
		if (leftChild->type == AST_INTEGERLITERAL && rightChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			rightVal = getVal(rightChild);
			int total = leftVal * rightVal;
			expr->replaceSelf(new Node(AST_INTEGERLITERAL, to_string(total)));
		}
		else if (leftChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			if (leftVal == 0) {
				expr->replaceSelf(new Node(AST_INTEGERLITERAL, "0"));
			}
			else if (leftVal == 1) {
				expr->replaceSelf(rightChild);
			}
		}
		else if (rightChild->type == AST_INTEGERLITERAL) {
			rightVal = getVal(rightChild);
			if (rightVal == 0) {
				expr->replaceSelf(new Node(AST_INTEGERLITERAL, "0"));
			}
			else if (rightVal == 1) {
				//Good idea? SHould I do new node with leftChilds val?
				expr->replaceSelf(leftChild);
			}
		}
		
	}
	case AST_DIVIDE:	{
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 2) {
			numErrors++;
			cerr << "A plus op on line " << expr->lineNumber << " doesn't have 2 children. Wat." << endl;
			break;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *rightChild = (Node *)(tChildren.at(1));
		massiveSwitch(leftChild);
		massiveSwitch(rightChild);
		leftChild = (Node *)(tChildren.at(0));
		rightChild = (Node *)(tChildren.at(1));


		int leftVal = -1;
		int rightVal = -1;
		if (leftChild->type == AST_INTEGERLITERAL && rightChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			rightVal = getVal(rightChild);
			if (rightVal == 0) {
				numErrors++;
				cerr << "Division by 0 on line " << expr->lineNumber << "." << endl;
				return false;
			}
			int total = leftVal / rightVal;
			expr->replaceSelf(new Node(AST_INTEGERLITERAL, to_string(total)));
		}
		else if (leftChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			if (leftVal == 0) {
				expr->replaceSelf(new Node(AST_INTEGERLITERAL, "0"));
			}
		}
		else if (rightChild->type == AST_INTEGERLITERAL) {
			rightVal = getVal(rightChild);
			if (rightVal == 0) {
				numErrors++;
				cerr << "ERROR: Division by 0 on line " << expr->lineNumber << "." << endl;
				return false;
			}
			else if (rightVal == 1) {
				expr->replaceSelf(leftChild);
			}
		}
	}
	case AST_LT: //fall through
	case AST_LE: {
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 2) {
			numErrors++;
			cerr << "a less than or less than equal on line " << expr->lineNumber << " doesn't have 2 children. Wat." << endl;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *rightChild = (Node *)(tChildren.at(1));
		massiveSwitch(leftChild);
		massiveSwitch(rightChild);
		leftChild = (Node *)(tChildren.at(0));
		rightChild = (Node *)(tChildren.at(1));

		int leftVal = -1;
		int rightVal = -1;

		if (leftChild->type == AST_INTEGERLITERAL && rightChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			rightVal = getVal(rightChild);
			bool res;
			if (expr->type == AST_LT) {
				res = leftVal < rightVal;
			}
			else {
				res = leftVal <= rightVal;
			}

			if (res) {
				expr->replaceSelf(new Node(AST_TRUE));
			}
			else {
				expr->replaceSelf(new Node(AST_FALSE));
			}
		}
		else if (leftChild->type == AST_INTEGERLITERAL) {
			leftVal = getVal(leftChild);
			if (expr->type == AST_LE) {
				//INT_MIN <= x is always true (x can't be below the min)
				if (leftVal == INT32_MIN) {
					expr->replaceSelf(new Node(AST_FALSE));
				}
			}
			else {
				//INT_MAX < x is false always (nothing above max)
				if (leftVal == INT32_MAX) {
					expr->replaceSelf(new Node(AST_FALSE));
				}
			}

		}
		else if (rightChild->type == AST_INTEGERLITERAL) {
			rightVal = getVal(rightChild);
			if (expr->type == AST_LT) {
				//x < INT_MIN is always false (nothing below min)
				if (rightVal == INT32_MIN) {
					expr->replaceSelf(new Node(AST_FALSE));
				}
			}
			else {
				//x <= INT_MAX is always true (nothing above max)
				if (rightVal == INT32_MAX) {
					expr->replaceSelf(new Node(AST_TRUE));
				}
			}
		}

	}
	case AST_EQUALS: {
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 2) {
			numErrors++;
			cerr << "An equal on line " << expr->lineNumber << " doesn't have 2 children. Wat." << endl;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *rightChild = (Node *)(tChildren.at(1));
		massiveSwitch(leftChild);
		massiveSwitch(rightChild);
		leftChild = (Node *)(tChildren.at(0));
		rightChild = (Node *)(tChildren.at(1));

		//Only gonna do primitives (bool, int, string) cause I don't wanna deal with 
		//anything else.
		int leftType = leftChild->type;
		int rightType = rightChild->type;
		string leftValType = leftChild->valType;
		string rightValType = rightChild->valType;

		Node *newFalseNode = new Node(AST_FALSE);
		Node *newTrueNode = new Node(AST_TRUE);

		//Remember: Making the assumption that massiveSwitch has already replaced anything that could have been replaced, so no nead to check identifiers here. Also, not doing anything but non-inheritable primitives cause I'm not insane.
		if ((leftType == AST_TRUE && rightType == AST_FALSE) || (leftType == AST_FALSE && rightType == AST_TRUE)) {
			expr->replaceSelf(newFalseNode);
		}
		else if (AST_TRUE == leftType == rightType || AST_FALSE == leftType == rightType) {
			expr->replaceSelf(newTrueNode);
		}
		else if (AST_STRING == leftType == rightType || AST_INTEGERLITERAL == leftType == rightType) {
			if (leftChild->value.compare(rightChild->value) == 0){
				expr->replaceSelf(newTrueNode);
			}
			else {
				expr->replaceSelf(newFalseNode);
			}
		}
	}
	case AST_EXPRLIST://FALL through
	case AST_EXPRSEMILIST: { //Block statements - nothing special about em.
		auto tChildren = expr->getChildren();
		for (Tree *tChild : tChildren) {
			massiveSwitch((Node *)tChild);
		}
	}
	case AST_DISPATCH: {
		auto tChildren = expr->getChildren();
		Node *leftChild = (Node *)(tChildren.at(0));
		Node *exprList = (Node *)tChildren.at(3);
		massiveSwitch(leftChild);
		massiveSwitch(exprList);
		leftChild = (Node *)(tChildren.at(0));
		exprList = (Node *)tChildren.at(3);

		this->settings.removeOthers();
	}
	case AST_LARROW: { //assignment
		auto tChildren = expr->getChildren();
		Node *id = (Node *)tChildren.at(0);
		Node *expression = (Node *)tChildren.at(1);
		massiveSwitch(expression);
		expression = (Node *)tChildren.at(1);
		if (expression->type == AST_INTEGERLITERAL || expression->type == AST_STRING || expression->type == AST_TRUE || expression->type == AST_FALSE) {
			this->settings.addValToVar(id->value, expression->value);
		}
	}
	case AST_IF: {
		auto tChildren = expr->getChildren();
		Node *test = (Node*)tChildren.at(0);
		Node *trueSection = (Node *)tChildren.at(1);
		Node *falseSection = (Node *)tChildren.at(2);
		massiveSwitch(test);
		test = (Node *)tChildren.at(0);
		
		if (test->type == AST_TRUE) {
			massiveSwitch(trueSection);
		}
		else if (test->type == AST_FALSE) {
			massiveSwitch(falseSection);
		}
		else {
			ConstProp tCProp = ConstProp(this->settings);
			ConstProp fCProp = ConstProp(this->settings);
			tCProp.massiveSwitch(trueSection);
			fCProp.massiveSwitch(falseSection);
			vector<set<string>> trueChanged =  tCProp.getSettings().getChanged();
			vector<set<string>> falseChanged = fCProp.getSettings().getChanged();

			this->settings.removeChanged(trueChanged);
			this->settings.removeChanged(falseChanged);
		}
	}
	case AST_LET: {
		auto tChildren = expr->getChildren();
		//Let(0) -> ID_TYPE_EXPR(2) -> expression
		Node *idExpr = (Node *)(tChildren.at(0)->getChildren().at(2));
		massiveSwitch(idExpr);
		idExpr = (Node *)(tChildren.at(0)->getChildren().at(2));
		Node *idType = (Node *)(tChildren.at(0)->getChildren().at(1));
		Node *id = (Node *)(tChildren.at(0)->getChildren().at(0));

		if (idExpr->type == idType->type) {
			this->settings.addLocal(id->value, idType->value,idExpr->value);
		}
		else {
			this->settings.addLocal(id->value, idType->value, "");
		}

		Node *other = (Node *)tChildren.at(1);
		massiveSwitch(other);
		this->settings.removeVar(id->value);
	}
	case AST_CASE: {
		auto tChildren = expr->getChildren();
		Node *caseExpr = (Node *)tChildren.at(0);
		massiveSwitch(caseExpr);
		Node *caseList = (Node *)tChildren.at(1);

		auto tCaseListChildren = caseList->getChildren();
		vector<vector<set<string>>> toRemove = vector<vector<set<string>>>();
		for (Tree *tCaseListChild : tCaseListChildren) {
			Node *caseListChild = (Node *)tCaseListChild;
			ConstProp cp = ConstProp(this->settings);

			cp.massiveSwitch((Node *)caseListChild->getChildren().at(3));
toRemove.emplace(toRemove.end(), cp.getSettings().getChanged());
		}

	}
	case AST_WHILE: {
		auto tChildren = expr->getChildren();
		Node *whileTest = (Node *)tChildren.at(0);
		massiveSwitch(whileTest);
		whileTest = (Node *)tChildren.at(0);
		if (whileTest->type == AST_FALSE) {
			return true;
		}

		//get all assigned before going into the while loop body
		set<string> toRemove = getAssigned((Node *)tChildren.at(1));
		//remove the values of all the changed vals.
		//Class vars and formals won't exist anymore, and any locals
		//will have "", which is ignored from being replaced in AST_IDENTIFIER
		for (string s : toRemove) {
			this->settings.removeVar(s);
		}

		massiveSwitch((Node *)tChildren.at(1));

		break;
	}
	case AST_IDENTIFIER: {
		string name = expr->value;
		string val = settings.getVal(name);
		string type = settings.getType(name);
		if (val != "") {
			if (type == "Int") {
				expr->replaceSelf(new Node(AST_INTEGERLITERAL, val));
			}
			else if (type == "Bool") {
				if (val == "true") {
					expr->replaceSelf(new Node(AST_TRUE));
				}
				else {
					expr->replaceSelf(new Node(AST_FALSE));
				}
			}
			else if (type == "String") {
				expr->replaceSelf(new Node(AST_STRING, val));
			}
			else {
				cerr << "There was an AST_IDENTIFIER with the name " << name << " with type " << type << " that shouldn't be there!" << endl;
			}
		}

		break;
	}
	default:
		cerr << "In massiveSwitch, got " << expr->type << " for some reason." << endl;
		numErrors++;


	}

	cerr << "Massive switch ain't done yet. Fuck off." << endl;
	return false;
}

ConstProp::ConstProp()
{
}

ConstProp::~ConstProp()
{
	//lol
}

int ConstProp::getVal(Node *expr) {
	int retVal = -1;
	try {
		retVal = stoi(expr->value);
	}
	catch (const std::out_of_range& oor) {
		numErrors++;
		cerr << "An integer had a value that couldn't be read into a C++ int on line " << expr->lineNumber << ". Wat." << endl;
	}
	catch (const invalid_argument& ia) {
		numErrors++;
		cerr << "An integer had a value that couldn't be converted into a C++ int on line " << expr->lineNumber << " with value " << expr->value << ". Wat." << endl;
	}

	return retVal;
}

void ConstProp::setSettings(ConstPropSettings settings)
{
	this->settings = settings;
}

//One of the UGLY issues in here
set<string> ConstProp::getAssigned(Node *expr)
{
	set<string> ret = set<string>();
	auto tChildren = expr->getChildren();
	if (tChildren.size() < 0) {
		return set<string>();
	}

	for (Tree *tChild : tChildren) {
		Node *child = (Node *)tChild;
		auto childChildren = child->getChildren();
		switch (child->type) {
		case AST_LARROW: {
			//add changed to list and increment by 1
			string name = ((Node *)childChildren.at(0))->value;
			if (!ret.count(name)) {
				ret.emplace(name);
			}
			//recurse through the assignments' expression
			set<string> toAdd = getAssigned((Node *)childChildren.at(1));

			//add all changed and the number of times.
			ret.emplace(toAdd);
		}
		case AST_DISPATCH: {
			Node *firstExpr = (Node *)childChildren.at(0);
			Node *exprList = (Node *)childChildren.at(3);
			ret.emplace(getAssigned(firstExpr));
			ret.emplace(getAssigned(exprList));
			//Always assume all class vars changed on dispatch.
			for (map<string, pair<string,string>>::iterator it = this->settings.getOtherMap().begin(); it != this->settings.getOtherMap().end(); it++) {
				ret.emplace(it->first);
			}
		}
		case AST_LET: {
			Node *idTypeExpr = (Node *)childChildren.at(0);
			Node *secondExpr = (Node *)childChildren.at(1);

			Node *idTypeChild = (Node *)idTypeExpr->getChildren().at(2);

			ret.emplace(getAssigned(idTypeChild));
			set<string> changed = getAssigned(secondExpr);
			if (changed.count(((Node *)idTypeChild->getChildren().at(0))->value)) {
				changed.erase(((Node *)idTypeChild->getChildren().at(0))->value);
			}

			ret.emplace(changed);
		}
		default: {
			cout << "A node of type " << expr->type << " was found in getAssignCount" << endl;
			ret.emplace(getAssigned(child));
		}

		}
	}
	return ret;
}
