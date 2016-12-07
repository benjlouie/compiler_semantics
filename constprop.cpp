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
		auto classChildren = classNode->getChildren();
		if (classChildren.size() != 3) {
			cerr << "The children of a class isn't 3. What." << endl;
			continue;
		}

		Node *features = (Node *)(classChildren.at(2));

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
		doConstPropAttr(attrVector);

		//go through each method
		doConstPropMethod(methodVector);
	}

	if (numErrors > 0) {
		return false;
	}

	return true;
}


bool ConstProp::doConstPropAttr(vector<Node *> nodes)
{

	for (auto node : nodes) {
		massiveSwitch(node);
	}
	
	this->settings = new ConstPropSettings();

	return true;
}

bool ConstProp::doConstPropMethod(vector<Node *> nodes)
{

	for (auto node : nodes) {
		massiveSwitch(node);
		//this->se
	}
	//cerr << "doConstProp with nodes isn't yet implemented." << endl;
	return true;
}

/*
bool ConstProp::doConstProp(Node * node)
{
	vector<Node *> nodeVect = vector<Node *>();
	nodeVect.push_back(node);
	return doConstPropMethod(nodeVect);
}*/

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
		Node *notChild = (Node *)(expr->getChildren().at(0));
		if (notChild->valType == "Bool") {
			massiveSwitch(notChild);
		}
		else {
			numErrors++;
			cerr << "Constant propogation ran into a scenario where a not worked on something other than a bool. Wat." << endl;
		}

		notChild = (Node *)(expr->getChildren().at(0));

		if (notChild->type == AST_TRUE) {
			expr->replaceSelf(new Node(AST_FALSE));
		}
		else if (notChild->type == AST_FALSE) {
			expr->replaceSelf(new Node(AST_TRUE));
		}

		break;
	}
	case AST_TILDE: {
		Node *tildeChild = (Node *)(expr->getChildren().at(0));
		if (tildeChild->valType == "Int") {
			massiveSwitch(tildeChild);
		}
		else {
			numErrors++;
			cerr << "Constant propogation ran into a scenario where a tilde worked on something other than an int on line " << expr->lineNumber << ". Wat." << endl;
		}

		tildeChild = (Node *)(expr->getChildren().at(0));

		if (tildeChild->type == AST_INTEGERLITERAL) {
			int val = this->getVal(tildeChild);
			
			//0 - <positive> = <negative>, 0 - <negative> = <positive>
			expr->replaceSelf(new Node(AST_INTEGERLITERAL, to_string(0 - val)));
		}
		break;
	}
	case AST_ISVOID: {
		Node *isVoidChild = (Node *)(expr->getChildren().at(0));
		massiveSwitch(isVoidChild);
		isVoidChild = (Node *)(expr->getChildren().at(0));

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

		tChildren = expr->getChildren();
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
		break;
		
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

		tChildren = expr->getChildren();

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
		break;
		
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

		tChildren = expr->getChildren();
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
		break;
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

		tChildren = expr->getChildren();
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
		break;

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
		//reget vector of children
		tChildren = expr->getChildren();
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
		break;
	}
	case AST_EXPRLIST://FALL through
	case AST_EXPRSEMILIST: { //Block statements - nothing special about em.
		auto tChildren = expr->getChildren();
		for (Tree *tChild : tChildren) {
			massiveSwitch((Node *)tChild);
		}
		break;
	}
	case AST_DISPATCH: {
		auto tChildren = expr->getChildren();
		if (tChildren.size() != 4) {
			cerr << "ERROR! Dispatch with " << tChildren.size() << " children!" << endl;
		}

		Node *leftChild = (Node *)(tChildren.at(0));
		Node *exprList = (Node *)tChildren.at(3);
		massiveSwitch(leftChild);
		massiveSwitch(exprList);

		this->settings->removeOthers();
		break;
	}
	case AST_LARROW: { //assignment
		auto tChildren = expr->getChildren();
		Node *id = (Node *)tChildren.at(0);
		Node *expression = (Node *)tChildren.at(1);
		massiveSwitch(expression);
		expression = (Node *)expr->getChildren().at(1);
		//If not int, string, or bool, don't do anything.
		if (expression->type == AST_INTEGERLITERAL) {
			this->settings->addValToVar(id->value, "Int", expression->value);
		}
		else if (expression->type == AST_STRING) {
			this->settings->addValToVar(id->value, "String", expression->value);
		}
		else if (expression->type == AST_TRUE) {
			this->settings->addValToVar(id->value, "Bool", "true");
		}
		else if (expression->type == AST_FALSE) {
			this->settings->addValToVar(id->value, "Bool", "false");
		}
		else {
			this->settings->removeVar(id->value);
		}

		break;
	}
	case AST_IF: {
		
		Node *test = (Node*)expr->getChildren().at(0);
		massiveSwitch(test);

		auto tChildren = expr->getChildren();
		test = (Node *)expr->getChildren().at(0);
		Node *trueSection = (Node *)tChildren.at(1);
		Node *falseSection = (Node *)tChildren.at(2);

		if (test->type == AST_TRUE) {
			massiveSwitch(trueSection);
		}
		else if (test->type == AST_FALSE) {
			massiveSwitch(falseSection);
		}
		else {
			ConstProp *tCProp = new ConstProp(*this->settings);
			ConstProp *fCProp = new ConstProp(*this->settings);
			tCProp->massiveSwitch(trueSection);
			fCProp->massiveSwitch(falseSection);
			vector<set<string>> trueChanged =  tCProp->getSettings()->getChanged();
			vector<set<string>> falseChanged = fCProp->getSettings()->getChanged();

			this->settings->removeChanged(trueChanged);
			this->settings->removeChanged(falseChanged);
		}
		break;
	}
	case AST_LET: {
		//Let(0) -> ID_TYPE_EXPR(2) -> expression
		Node *idTypeExpr = (Node *)expr->getChildren().at(0);
		massiveSwitch((Node *)(idTypeExpr->getChildren().at(2)));
		//need to reget a copy of the vector of children
		auto tChildren = expr->getChildren();
		idTypeExpr = (Node *)expr->getChildren().at(0);
		auto idTypeExprChildren = idTypeExpr->getChildren();

		Node *idExpr = (Node *)(idTypeExprChildren.at(2));
		Node *idType = (Node *)(idTypeExprChildren.at(1));
		Node *id = (Node *)(idTypeExprChildren.at(0));


		cout << "adding local with name " << id->value << endl;
		if (idType->value == "Int" && idExpr->type == AST_INTEGERLITERAL) {
			this->settings->addLocal(id->value, idType->value, idExpr->value);
		}
		else if (idType->value == "String" && idExpr->type == AST_STRING) {
			this->settings->addLocal(id->value, idType->value, idExpr->value);
		}
		else if (idType->value == "Bool") {
			if (idExpr->type == AST_TRUE) {
				this->settings->addLocal(id->value, idType->value, "true");
			} else if (idExpr->type == AST_FALSE) {
				this->settings->addLocal(id->value, idType->value, "false");
			}
		}

		Node *other = (Node *)tChildren.at(1);
		massiveSwitch(other);
		this->settings->removeVar(id->value);
		break;
	}
	case AST_CASESTATEMENT: {
		Node *caseExpr = (Node *)expr->getChildren().at(0);
		massiveSwitch(caseExpr);

		Node *caseList = (Node *)expr->getChildren().at(1);

		auto tCases = caseList->getChildren();
		vector<vector<set<string>>> toRemove = vector<vector<set<string>>>();
		for (Tree *tCaseChild : tCases) {
			Node *caseChild = (Node *)tCaseChild;
			ConstProp *cp = new ConstProp(*this->settings);

			cp->massiveSwitch((Node *)caseChild->getChildren().at(2));
			toRemove.emplace(toRemove.end(), cp->getSettings()->getChanged());
		}

		for (vector<set<string>> x : toRemove) {
			this->settings->removeChanged(x);
		}

		break;

	}
	case AST_WHILE: {
		/*
		massiveSwitch(whileTest);*/
		bool canModifyTest = true;
		auto tChildren = expr->getChildren();

		//get assigned and see if one of the ID's in the test is changed
		set<string> toRemove = getAssigned((Node *)tChildren.at(1));
		cout << "Removing: ";
		for (string s : toRemove) {
			cout << s << ", ";
		}
		cout << endl;

		Node *whileTest = (Node *)tChildren.at(0);
		set<string> ids = getIDs(whileTest);

		cout << "Ids found: ";
		for (string s : ids) {
			cout << s << ", ";
		}
		cout << endl;

		for (string id : ids) {
			//if it is changed, we can't modify the test
			if (toRemove.count(id)) {
				canModifyTest = false;
				cout << "Can't modify." << endl;
			}
		}

		//if we CAN modify the test, run massive switch on it.
		if (canModifyTest) {
			cout << "Can modify" << endl;
			massiveSwitch(whileTest);
			tChildren = expr->getChildren();
		}

		whileTest = (Node *)tChildren.at(0);
		//If we know we aren't gonna run the loop ever, don't even bother with it.
		if (whileTest->type == AST_FALSE) {
			return true;
		}
		
		//remove the values of all the changed vals.
		//After removeVar, any ID that was changed that was a class var or formals 
		//won't exist anymore, and any locals will have "", which is ignored
		//when AST_IDENTIFIER finds a local in there. 
		for (string s : toRemove) {
			this->settings->removeVar(s);
		}

		//NOW run massiveSwitch on the body
		massiveSwitch((Node *)tChildren.at(1));

		break;
	}
	case AST_IDENTIFIER: {
		string name = expr->value;
		string val = this->settings->getVal(name);
		string type = this->settings->getType(name);
		cout << name << " with val " << val << " with type " << type << endl;
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
	case AST_FEATURE_ATTRIBUTE: {
		auto fAttrChildren = expr->getChildren();
		if (fAttrChildren.size() != 3) {
			numErrors++;
			cerr << "Feature attribute doesn't have 3 children!" << endl;
			break;
		}
		//get name
		string name = ((Node *)fAttrChildren.at(0))->value;
		//get type
		string type = ((Node *)fAttrChildren.at(1))->value;
		//runn massiveSwitch on expr
		massiveSwitch((Node *)fAttrChildren.at(2));
		//if the new expr is same as type gotten above, add the value to the settings
		//else add a blank to the settings.
		fAttrChildren = expr->getChildren();
		Node *exprChild = (Node *)fAttrChildren.at(2);
		if (exprChild->valType == type) {
			this->settings->addLocal(name, type, exprChild->value);
		}
		break;
	}
	case AST_FEATURE_METHOD: {
		auto fMethChildren = expr->getChildren();
		if (fMethChildren.size() != 4) {
			//should never happen
			numErrors++;
			cerr << "Feature Method doesn't have 4 children!" << endl;
			break;
		}
		//get formals
		Node *formalList = (Node *)fMethChildren.at(1);
		auto fListChildren = formalList->getChildren();
		
		for (Tree *tFListChild : fListChildren) {
			Node *fListChild = (Node *)tFListChild;
			string id = ((Node *)(fListChild->getChildren().at(0)))->value;
			string type = ((Node *)(fListChild->getChildren().at(1)))->value;
			this->settings->addFormal(id, type);
		}

		massiveSwitch((Node *)fMethChildren.at(3));

		break;
	}
	default:
		cerr << "In massiveSwitch, got " << enum2string(expr->type) << " for some reason." << endl;
		numErrors++;


	}
	return true;
}

ConstProp::ConstProp(ConstPropSettings &settings) {
	this->settings = new ConstPropSettings(settings);
}

ConstProp::ConstProp() {
	this->settings = new ConstPropSettings();
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

void ConstProp::setSettings(ConstPropSettings &settings)
{
	this->settings = new ConstPropSettings(settings);
}

ConstPropSettings *ConstProp::getSettings() {
	return this->settings;
}

//One of the UGLY issues in here
set<string> ConstProp::getAssigned(Node *expr)
{
	set<string> ret = set<string>();
	auto tChildren = expr->getChildren();
	if (tChildren.size() < 0) {
		return set<string>();
	}

	switch (expr->type) {
	case AST_LARROW: {
		//add changed to list and increment by 1
		string name = ((Node *)tChildren.at(0))->value;
		if (!ret.count(name)) {
			ret.emplace(name);
		}
		//recurse through the assignments' expression
		set<string> toAdd = getAssigned((Node *)tChildren.at(1));
		

		//add all changed and the number of times.
		ret.insert(toAdd.begin(), toAdd.end());
		break;
	}
	case AST_DISPATCH: {
		Node *firstExpr = (Node *)tChildren.at(0);
		Node *exprList = (Node *)tChildren.at(3);
		set<string> firstAssigned = getAssigned(firstExpr);
		set<string> exprListAssigned = getAssigned(exprList);

		ret.insert(firstAssigned.begin(), firstAssigned.end());
		ret.insert(exprListAssigned.begin(), exprListAssigned.end());

		//Always assume all class vars changed on dispatch.
		map<string, pair<string, string>> otherMap = this->settings->getOtherMap();
		map<string, pair<string, string>>::iterator it = otherMap.begin();
		map<string, pair<string, string>>::iterator endIt = otherMap.end();
		for (; it != endIt; it++) {
			string what = it->first;
			ret.emplace(what);
		}
		break;
	}
	case AST_LET: {
		Node *idTypeExpr = (Node *)tChildren.at(0);
		Node *secondExpr = (Node *)tChildren.at(1);

		Node *idTypeChild = (Node *)idTypeExpr->getChildren().at(2);
		set<string> idTypeChildAssigned = getAssigned(idTypeChild);

		ret.insert(idTypeChildAssigned.begin(), idTypeChildAssigned.end());

		set<string> changed = getAssigned(secondExpr);
		if (changed.count(((Node *)idTypeChild->getChildren().at(0))->value)) {
			changed.erase(((Node *)idTypeChild->getChildren().at(0))->value);
		}

		ret.insert(changed.begin(), changed.end());
		break;
	}
	default: {
		cout << "A node of type " << enum2string(expr->type) << " was found in getAssignCount" << endl;
		for (Tree *tChild : tChildren) {
			set<string> assigned = getAssigned((Node *)tChild);
			ret.insert(assigned.begin(), assigned.end());
		}
	}
	}

	
	return ret;
}

set<string> ConstProp::getIDs(Node *expr) {
	set<string> ret = set<string>();
	if (expr->type == AST_IDENTIFIER) {
		ret.insert(expr->value);
	}
	else {
		auto tChildren = expr->getChildren();
		for (Tree *tChild : tChildren) {
			set<string> tmp = getIDs((Node *)tChild);
			ret.insert(tmp.begin(),tmp.end());
		}
	}

	return ret;
}