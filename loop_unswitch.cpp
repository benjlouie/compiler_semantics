
#include "loop_unswitch.h"

struct varData {
	bool used;
	bool assigned;
	bool dispatched;
	bool alias;
	string aliasVar;

	varData() {
		used = false;
		assigned = false;
		dispatched = false;
		alias = false;
		aliasVar = "";
	}
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
		bool classVar = false;
		string target;
		unordered_map<string, bool> aliases;
	};
	unordered_map<string, aliasData> variables;

public:
	bool contains(string varName, bool classVar = false) {
		if (variables.count(varName) > 0) {
			if (variables[varName].classVar == classVar) {
				return true;
			}
		}

		return false;
	}

	void set(string varName, string target = "", bool classVar = false) {
		//check if it's actually the var we want
		if (variables.count(varName) > 0) {
			if (variables[varName].classVar && classVar) {
				//var isn't there, don't change anything
				return;
			}
		}

		//create it if not already made
		variables[varName];
		//keep track of prev unknownAlias
		bool prevUnknown = variables[varName].unknownAlias;
		bool prevClassVar = variables[varName].classVar;

		if (target == "?") {
			variables[varName].unknownAlias = true;
			target = ""; //to work with the rest of the method
		}
		else {
			variables[varName].unknownAlias = false;
		}
		variables[varName].classVar = classVar;

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
			variables[newMainVar].classVar = prevClassVar;
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
		bool prevClassVar = variables[varName].classVar;
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
			variables[newMainVar].classVar = prevClassVar;
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

string unswitchLoops_recursive(aliasVars &vars, Node *root);
string getAttrTarget(aliasVars &vars, Node *attrExpr);

size_t globalLetCount = 0;
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
					vars.set(classVar, "?", true);
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
				unswitchLoops_recursive(vars, methodExpr);

				globalSymTable->leaveScope();
				//reset all the stuff for the next method
				globalLetCount = 0;
				globalVarNames.clear();
			}
		}
	}
}

//returns the var the expression came out to
string unswitchLoops_recursive(aliasVars &vars, Node *root)
{
	switch (root->type)
	{
	case AST_IDENTIFIER:
		//TODO: add that it was used for when it goes up
		return globalVarNames.get(root->value);
		break;
	case AST_LARROW:
	{
		//TODO: add that it was assigned for when it goes up
		auto assignChildren = root->getChildren();
		string varName = ((Node *)assignChildren[0])->value;
		Node *rightExpr = (Node *)assignChildren[1];

		varName = globalVarNames.get(varName);
		vars.set(varName, unswitchLoops_recursive(vars, rightExpr));

		return varName;
		break;
	}
	case AST_WHILE:
		//TODO: special stuff, put into own function

		break;
	case AST_EXPRLIST:
	{
		auto children = root->getChildren();
		size_t i = 0;
		for (; i < children.size() - 1; i++) {
			unswitchLoops_recursive(vars, (Node *)children[i]);
		}
		return "";
		break;
	}
	case AST_EXPRSEMILIST:
	{
		auto children = root->getChildren();
		size_t i = 0;
		for (; i < children.size() - 1; i++) {
			unswitchLoops_recursive(vars, (Node *)children[i]);
		}
		return unswitchLoops_recursive(vars, (Node *)children[i]);
		break;
	}
	case AST_LET:
	{
		auto letChildren = root->getChildren();
		Node *idTypeExpr = (Node *)letChildren[0];
		string letId = ((Node *)idTypeExpr->getChildren()[0])->value;
		Node *letIdExpr = (Node *)idTypeExpr->getChildren()[2];
		Node *letExpr = (Node *)letChildren[1];

		string letIdRet = ""; //only use if there is an expr
		if (letIdExpr->type != AST_NULL) {
			letIdRet = unswitchLoops_recursive(vars, letIdExpr);
		}
		string letIdFinal = letId + ".let" + to_string(globalLetCount);
		globalLetCount++;
		vars.set(letIdFinal, letIdRet);
		globalVarNames.push(letId, letIdFinal);

		unswitchLoops_recursive(vars, letExpr);

		//remove val used in let
		vars.remove(globalVarNames.get(letId));
		globalVarNames.pop(letId);

		break;
	}
	case AST_DISPATCH:
	{
		auto dispatchChildren = root->getChildren();
		Node *callerExpr = (Node *)dispatchChildren[0];
		Node *funcClass = (Node *)dispatchChildren[1];
		Node *argExprs = (Node *)dispatchChildren[3];

		//go through argument expressions
		unswitchLoops_recursive(vars, argExprs);

		//go through caller expression
		string caller = "self";
		if (callerExpr->type != AST_NULL) {
			caller = unswitchLoops_recursive(vars, callerExpr);
		}

		//action based on caller
		if (caller == "") {
			//this shouldn't happen, type checking should catch it
			cerr << "unswitchLoops_recursive(): caller = \"\"\n";
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
						vars.set(var, "?", true);
					}
				}
			}
		}
		else {
			//it's a var, invalidate it
			vars.set(caller, "?");
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
		//TODO: implement
		break;
	}
	case AST_CASESTATEMENT:
		break;
	default:
		//recurse to all children
		for (auto tchild : root->getChildren()) {
			unswitchLoops_recursive(vars, (Node *)tchild);
		}
		return "";
		break;
	}
	return "?";
}