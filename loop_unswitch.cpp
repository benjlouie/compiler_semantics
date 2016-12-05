
#include "loop_unswitch.h"

struct varData {
	bool used = false;
	bool assigned = false;
	bool dispatched = false;

	varData &operator+=(const varData &rhs) {
		this->used = this->used || rhs.used;
		this->assigned = this->assigned || rhs.assigned;
		this->dispatched = this->dispatched || rhs.dispatched;
		return *this;
	}
};

struct ifData {
	Node *ifNode;
	unordered_map<string, varData> *ifCondVarUse;
	//TODO: do I need a copy of vars at the time of the if?
};

class varNames {
	unordered_map<string, stack<string>> variables;
public:
	string get(string var) {
		if (variables.count(var) == 0) {
			//not in there, error
			throw "variable \"" + var + "\" not in stack";
		}
		return variables[var].top();
	}

	void push(string var, string newName) {
		variables[var].push(newName);
	}

	void pop(string var) {
		variables[var].pop();
		if (variables[var].size() == 0) {
			variables.erase(var);
		}
	}

	void clear(void) {
		variables.clear();
	}
};

class aliasVars {
	struct aliasData {
		bool unknownAlias = false;
		string target;
		unordered_map<string, bool> aliases;
	};
	unordered_map<string, aliasData> variables;

public:
	aliasVars &operator=(const aliasVars &rhs) {
		variables = rhs.variables;
		return *this;
	}

	bool contains(string varName, bool classVar = false) {
		if (variables.count(varName) > 0) {
			return true;
		}

		return false;
	}

	bool isUnknown(string varName) {
		if (!this->contains(varName)) {
			return false; //not in vars
		}
		string target = variables[varName].target;
		if (target == "") {
			return variables[varName].unknownAlias;
		}
		else {
			return variables[target].unknownAlias;
		}
	}

	vector<string> getAliases(string varName) {
		if (!this->contains(varName)) {
			return{}; //not in vars
		}

		vector<string> retVars;
		string target = variables[varName].target;
		if (target == "") {
			//points too self
			for (auto vars : variables[varName].aliases) {
				retVars.push_back(vars.first);
			}
			retVars.push_back(varName);
		}
		else {
			for (auto vars : variables[target].aliases) {
				retVars.push_back(vars.first);
			}
			retVars.push_back(target);
		}

		return retVars;
	}

	void setTargetUnknown(string varName) {
		if (!this->contains(varName)) {
			//doesn't exist, error
			cerr << "aliasVar.setTargetUnknown(): variable \"" + varName + "\"doesn't exist\n";
			return;
		}
		//TODO: if the target is already unknownVariable, do all the class vars need to be changed to unknown too?

		string target = variables[varName].target;
		if (target == "") {
			//has no target, is itself
			variables[varName].unknownAlias = true;
		}
		else {
			variables[target].unknownAlias = true;
		}
	}

	void set(string varName, string target = "") {
		//create it if not already made
		variables[varName];
		//keep track of prev unknownAlias
		bool prevUnknown = variables[varName].unknownAlias;

		if (target == "?") {
			variables[varName].unknownAlias = true;
			target = ""; //to work with the rest of the method
		}
		else {
			variables[varName].unknownAlias = false;
		}

		//check for cyclics
		//get end of target path
		if (target != "") {
			while (variables[target].target != "" && target != varName) {
				target = variables[target].target;
			}
			if (target == varName) {
				//cyclic, don't do anything
				return;
			}
		}

		//fix any vars that reference varName
		auto aliasList = variables[varName].aliases;
		if (aliasList.size() > 0) {
			auto it = aliasList.begin();
			string newMainVar = it->first;
			variables[newMainVar].target = ""; //first var is now owner of that mem
			variables[newMainVar].unknownAlias = prevUnknown;
			variables[newMainVar].aliases.clear();
			it++;
			//set the rest of them to point to the first var
			while (it != aliasList.end()) {
				variables[it->first].target = newMainVar;
				variables[newMainVar].aliases[it->first] = true; //update aliases on newMainVar
				it++;
			}
		}
		variables[varName].aliases.clear();

		//remove varName from target alias list
		string oldTarget = variables[varName].target;
		if (oldTarget != "") {
			variables[oldTarget].aliases.erase(varName);
		}
		if (target != "") {
			//add varName to final target (when not empty target)
			variables[target].aliases[varName] = true;
		}
		variables[varName].target = target;
	}

	void setAll(string newTarget) {
		vector<string> vars;
		for (auto var : variables) {
			vars.push_back(var.first);
		}
		//empty it and replace
		variables.clear();
		for (auto var : vars) {
			this->set(var, newTarget);
		}
	}

	void remove(string varName) {
		if (variables.count(varName) == 0) {
			cerr << "aliasVar.remove(): variable \"" + varName + "\"doesn't exist\n";
			return;
		}
		//keep track of prev unknownAlias
		bool prevUnknown = variables[varName].unknownAlias;
		//remove varName from target alias list
		string oldTarget = variables[varName].target;
		if (oldTarget != "") {
			variables[oldTarget].aliases.erase(varName);
		}

		//cleanup all vars targeting varName
		auto aliasList = variables[varName].aliases;
		if (aliasList.size() > 0) {
			auto it = aliasList.begin();
			string newMainVar = it->first;
			variables[newMainVar].target = ""; //first var is now owner of that mem
			variables[newMainVar].unknownAlias = prevUnknown;
			variables[newMainVar].aliases.clear();
			it++;
			//set the rest of them to point to the first var
			while (it != aliasList.end()) {
				variables[it->first].target = newMainVar;
				variables[newMainVar].aliases[it->first] = true; //update aleases on newMainVar
				it++;
			}
		}

		variables.erase(varName);
	}
};

string unswitchLoops_recursive(aliasVars &vars, unordered_map<string, varData> &varUse, vector<ifData> &ifStatements, Node *root);
void setVarsFromTemporaries(aliasVars &vars, unordered_map<string, varData> &varUse, unordered_map<string, varData> &tmpVarUse);
bool attemptToUnswitch(aliasVars &vars, unordered_map<string, varData> &loopVarUse, vector<ifData> &ifStatements, Node *loopNode);
bool unswitchVarsValid(aliasVars &vars, unordered_map<string, varData> &combinedVarUse, unordered_map<string, varData> &ifVarUse);
void AstUnswitch(Node *whileNode, Node *ifNode);

size_t globalLocalCount = 0;
varNames globalVarNames;

void unswitchLoops(void) {
	//get to each functions and start the recursive call
	auto classNodes = root->getChildren();
	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;
		Node* features = ((Node *)classDescNodes[2]);

		//go to correct class
		globalSymTable->goToClass(className);

		for (Tree *tFeature : features->getChildren()) {
			Node *feature = (Node *)tFeature;

			//Only care about methods
			if (feature->type == AST_FEATURE_METHOD) {
				auto kids = feature->getChildren();
				string methName = ((Node *)kids[0])->value;
				Node *methodExpr = (Node *)kids[3];

				aliasVars vars;
				//all class vars and formals are unknown aliases, add them
				vector<string> localVars = globalSymTable->getAllVariables();
				for (string classVar : localVars) {
					vars.set(classVar, "?");
					globalVarNames.push(classVar, classVar);
				}
				localVars.clear();
				//get formals, add them to vars
				globalSymTable->enterScope(methName);
				localVars = globalSymTable->getCurrentVariables();
				for (string formalVar : localVars) {
					string formalFinal = formalVar + ".formal";
					vars.set(formalFinal, "?");
					globalVarNames.push(formalVar, formalFinal);
				}
				localVars.clear();

				//recurse, build vars down, build the if list and stuff from the bottom
				unordered_map<string, varData> varUse;
				vector<ifData> ifStatements;
				unswitchLoops_recursive(vars, varUse, ifStatements, methodExpr);

				globalSymTable->leaveScope();
				//reset all the stuff for the next method
				globalLocalCount = 0;
				globalVarNames.clear();
			}
		}
	}
}

//returns the var the expression came out to
string unswitchLoops_recursive(aliasVars &vars, unordered_map<string, varData> &varUse, vector<ifData> &ifStatements, Node *root)
{
	switch (root->type)
	{
	case AST_IDENTIFIER:
		//mark var as used when going up
		varUse[globalVarNames.get(root->value)].used = true;
		return globalVarNames.get(root->value);
		break;
	case AST_LARROW:
	{
		auto assignChildren = root->getChildren();
		string varName = ((Node *)assignChildren[0])->value;
		Node *rightExpr = (Node *)assignChildren[1];

		varName = globalVarNames.get(varName);
		vars.set(varName, unswitchLoops_recursive(vars, varUse, ifStatements, rightExpr));

		//mark that the var was assigned when it goes up
		varUse[varName].assigned = true;

		return varName;
		break;
	}
	case AST_WHILE:
	{
		auto whileChildren = root->getChildren();
		Node *condExpr = (Node *)whileChildren[0];
		Node *loopExpr = (Node *)whileChildren[1];

		//go through cond (will always execute)
		unordered_map<string, varData> tmpCondVarUse;
		unswitchLoops_recursive(vars, tmpCondVarUse, ifStatements, condExpr);

		//make temporaries for loop body (may not be executed)
		unordered_map<string, varData> tmpVarUse;
		aliasVars tmpVars = vars;
		unswitchLoops_recursive(tmpVars, tmpVarUse, ifStatements, loopExpr);

		//combine both tmporary varUses while attempting to unswitch
		for (auto use : tmpCondVarUse) {
			tmpVarUse[use.first] += use.second;
		}
		//special stuff for determining if it should be unswitched, put into own function
		bool success = attemptToUnswitch(vars, tmpVarUse, ifStatements, root);
		if (success) {
			//remove all ifStatements from the list, so it doesn't affect upward loops
			//TODO: should be able to add the unswitched if with updated usages and ptr
			ifStatements.clear();
		}

		//add tmpCondVarUse and tmpVarUse to varUse (order is important)
		setVarsFromTemporaries(vars, varUse, tmpCondVarUse);
		setVarsFromTemporaries(vars, varUse, tmpVarUse);

		break;
	}
	case AST_EXPRLIST:
	{
		auto children = root->getChildren();
		size_t i = 0;
		for (; i < children.size() - 1; i++) {
			unswitchLoops_recursive(vars, varUse, ifStatements, (Node *)children[i]);
		}
		return "";
		break;
	}
	case AST_EXPRSEMILIST:
	{
		auto children = root->getChildren();
		size_t i = 0;
		for (; i < children.size() - 1; i++) {
			unswitchLoops_recursive(vars, varUse, ifStatements, (Node *)children[i]);
		}
		return unswitchLoops_recursive(vars, varUse, ifStatements, (Node *)children[i]);
		break;
	}
	case AST_LET:
	{
		auto letChildren = root->getChildren();
		Node *idTypeExpr = (Node *)letChildren[0];
		string letId = ((Node *)idTypeExpr->getChildren()[0])->value;
		Node *letIdExpr = (Node *)idTypeExpr->getChildren()[2];
		Node *letExpr = (Node *)letChildren[1];

		string letIdRet = "";
		if (letIdExpr->type != AST_NULL) {
			letIdRet = unswitchLoops_recursive(vars, varUse, ifStatements, letIdExpr);
		}
		string letIdFinal = letId + ".let" + to_string(globalLocalCount);
		globalLocalCount++;
		vars.set(letIdFinal, letIdRet);
		globalVarNames.push(letId, letIdFinal);

		unswitchLoops_recursive(vars, varUse, ifStatements, letExpr);

		//remove val used in let
		vars.remove(globalVarNames.get(letId));
		globalVarNames.pop(letId);
		//TODO: destroy the let var from the returning list of used vars
		//do I need to? shouldn't affect anything...

		break;
	}
	case AST_DISPATCH:
	{
		auto dispatchChildren = root->getChildren();
		Node *callerExpr = (Node *)dispatchChildren[0];
		Node *funcClass = (Node *)dispatchChildren[1];
		Node *argExprs = (Node *)dispatchChildren[3];

		//go through argument expressions
		unswitchLoops_recursive(vars, varUse, ifStatements, argExprs);

		//go through caller expression
		string caller = "self";
		if (callerExpr->type != AST_NULL) {
			caller = unswitchLoops_recursive(vars, varUse, ifStatements, callerExpr);
		}

		//action based on caller
		if (caller == "") {
			//some random object, not a variable
			//TODO: do nothing?
		}
		else if (caller == "?") {
			//unknown caller, invalidate all
			vars.setAll("?");
		}
		else if (caller == "self") {
			if (funcClass->type == AST_NULL) {
				//invalidate all
				vars.setAll("?");
			}
			else {
				string funcType = funcClass->value;
				//only invalidate the ones from that class up
				if (globalSymTable->isSubClass(globalSymTable->getCurrentClass(), funcType)) {
					//update the vars if they are 
					vector<string> &changedVars = globalSymTable->getAllClassVariables(funcType);
					for (string var : changedVars) {
						//change only the class vars in the aliasVars
						vars.set(var, "?");
					}
				}
			}
			varUse["self"].dispatched = true;
		}
		else {
			//it's a var, invalidate it
			//TODO: need a thing to see what the var was aliased to (like self)
			//TODO: if target was previously unknown, do all the class vars need to be changed?

			//target of var must be set as unknown
			vars.setTargetUnknown(caller);

			vars.set(caller, "?");
			varUse[caller].dispatched = true;
		}
		return "?";
		break;
	}
	case AST_IF:
	{
		auto ifChildren = root->getChildren();
		Node *condExpr = (Node *)ifChildren[0];
		Node *thenExpr = (Node *)ifChildren[1];
		Node *elseExpr = (Node *)ifChildren[2];

		//do conditional, record everything that happened in it separately
		unordered_map<string, varData> *ifCondVarUse = new unordered_map<string, varData>();
		unswitchLoops_recursive(vars, *ifCondVarUse, ifStatements, condExpr);
		//TODO: need to record the currect vars in return of if statements?

		//copy vars for then and else, do them
		unordered_map<string, varData> tmpVarUse;
		aliasVars tmpVars = vars;
		unswitchLoops_recursive(tmpVars, tmpVarUse, ifStatements, thenExpr);
		tmpVars = vars;
		unswitchLoops_recursive(tmpVars, tmpVarUse, ifStatements, elseExpr);

		//get what variables were assigned/dispatched and change vars
		setVarsFromTemporaries(vars, varUse, tmpVarUse);

		//add the if to the list of ifs
		ifData ifInfo{ root, ifCondVarUse };
		ifStatements.push_back(ifInfo);

		return "?";
		break;
	}
	case AST_CASESTATEMENT:
	{
		auto children = root->getChildren();
		Node *condExpr = (Node *)children[0];
		Node *caseList = (Node *)children[1];
		//evaluate the case conditional expr
		string caseIdRet = unswitchLoops_recursive(vars, varUse, ifStatements, condExpr);

		//to keep track of all the things changed during the cases
		unordered_map<string, varData> tmpVarUse;

		auto cases = caseList->getChildren();
		for (auto cs: cases) {
			auto caseKids = cs->getChildren();
			string caseId = ((Node *)caseKids[0])->value;
			Node *caseExpr = (Node *)caseKids[2];

			//need to copy vars
			aliasVars tmpVars = vars;
			//get case var alt name
			string caseIdFinal = caseId + ".case" + to_string(globalLocalCount);
			globalLocalCount++;
			tmpVars.set(caseIdFinal, caseIdRet);
			globalVarNames.push(caseId, caseIdFinal);

			//need tmp varUse so I can see what needs to be invalidated
			unswitchLoops_recursive(tmpVars, tmpVarUse, ifStatements, caseExpr);

			//remove case var from varNames
			globalVarNames.pop(caseId);
			//TODO: destroy the case var from the returning list of used vars
			//TODO: do we need to?
		}

		//get what variables were assigned/dispatched in all cases and change vars
		setVarsFromTemporaries(vars, varUse, tmpVarUse);

		return "?";
		break;
	}
	default:
		//recurse to all children
		for (auto tchild : root->getChildren()) {
			unswitchLoops_recursive(vars, varUse, ifStatements, (Node *)tchild);
		}
		return "";
		break;
	}
	return "?";
}

void setVarsFromTemporaries(aliasVars &vars, unordered_map<string, varData> &varUse, unordered_map<string, varData> &tmpVarUse)
{
	//get what variables were assigned/dispatched and change vars
	for (auto varUsage : tmpVarUse) {
		string varName = varUsage.first;
		varData &data = varUsage.second;
		if (vars.contains(varName)) { //test incase it was a local var
			if (data.dispatched) {
				//set target to unknownAlias (like in dispatch) so all aliases are unknown
				vars.setTargetUnknown(varName);
			}
			//order is important here
			if (data.assigned) {
				vars.set(varName, "?"); //possibly assigned, no idea what it is now
			}
		}
		//add variable usages into the regular varUse
		varUse[varName] += data;
	}
}

bool attemptToUnswitch(aliasVars &vars, unordered_map<string, varData> &loopVarUse, vector<ifData> &ifStatements, Node *loopNode)
{
	//loop through all if statements and use the first one
	//start at top (likely most used if)(end of vector)
	size_t len = ifStatements.size();
	for (size_t i = 0, cur = len - 1; i < len; i++, cur--) {
		ifData ifStatement = ifStatements[cur];

		//combine all var use except cur if into one varUse
		unordered_map<string, varData> combinedVarUse;
		for (size_t ifInd = 0; ifInd < len; ifInd++) {
			if (ifInd != cur) { //all except cur
				unordered_map<string, varData> &tmpVarUse = *ifStatements[ifInd].ifCondVarUse;
				for (auto use : tmpVarUse) {
					combinedVarUse[use.first] += use.second;
				}
			}
		}
		//add in loop varUse
		for (auto use : loopVarUse) {
			combinedVarUse[use.first] += use.second;
		}

		//check varUse in the if conditional against all other values used (including the ones in the other if conditionals)
		if (unswitchVarsValid(vars, combinedVarUse, *ifStatement.ifCondVarUse)) {
			//can be unswitched! WOOHOO!
			//do the unswitching
			AstUnswitch(loopNode, ifStatement.ifNode);
			return true;
		}
	}
	return false;
}

bool unswitchVarsValid(aliasVars &vars, unordered_map<string, varData> &combinedVarUse, unordered_map<string, varData> &ifVarUse)
{
	for (auto var : ifVarUse) {
		string varName = var.first;
		varData data = var.second;

		bool used = false;
		bool assigned = false;
		bool dispatched = false;

		//get what was done to the aliases
		vector<string> aliases = vars.getAliases(varName);
		for (string alias : aliases) {
			if (combinedVarUse.count(alias) > 0) {
				varData aliasData = combinedVarUse[alias];
				used = used || aliasData.used;
				assigned = assigned || aliasData.assigned;
				dispatched = dispatched || aliasData.dispatched;
			}
		}

		if (data.used) {
			//check that none of its aliases were assigned/dispatched
			if (assigned || dispatched) {
				return false;
			}
		}
		if (data.assigned) {
			//TODO: check this one
			//check that none of its aliases were used
			if (used) {
				return false;
			}
		}
		if (data.dispatched) {
			//check that none of its aliases were used/assigned/dispatched
			if (used || assigned || dispatched) {
				return false;
			}
		}

		//check that no dispatched vars are unknown aliases
		for (auto var : combinedVarUse) {
			if (var.second.dispatched == false) {
				continue;
			}
			if (vars.contains(var.first)) {
				if (vars.isUnknown(var.first)) {
					return false;
				}
			}
		}
	}

	return true;
}

void AstUnswitch(Node *whileNode, Node *ifNode)
{
	//get thenExpr and elseExpr from the if
	auto ifKids = ifNode->getChildren();
	Node *thenExpr = (Node *)ifKids[1];
	Node *elseExpr = (Node *)ifKids[2];

	// keep track of if expr

	//replace ifExpr with thenExpr
	ifNode->replaceSelf(thenExpr);

	//get path from while to ifNode (now thenExpr)
	vector<size_t> path = whileNode->getPath(thenExpr);

	//keep track of while expr
	//copy it and its subtree
	Node *whileCopy = whileNode->deepCopy();

	//replace copy's thenExpr with the elseExpre
	whileCopy->replaceDescendant(path, elseExpr);

	//replace while expr with the if
	//put while exprs as then and else espressions
	whileNode->replaceSelf(ifNode);

	//set the ifNodes's then to orig loop and else to copy loop
	ifNode->setChild(1, whileNode);
	ifNode->setChild(2, whileCopy);
}