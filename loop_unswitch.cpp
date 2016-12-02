
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

class aliasVars {
	struct aliasData {
		bool unknownAlias = false;
		string target;
		unordered_map<string, bool> aliases;
	};
	unordered_map<string, aliasData> variables;

public:


	void set(string varName, string target = "") {
		//create it if not already made
		variables[varName];
		if (target == "?") {
			variables[varName].unknownAlias = true;
			target = ""; //to work with the rest of the method
		}
		else {
			variables[varName].unknownAlias = false;
		}

		//check for cyclics
		//get end of target path
		while (variables[target].target != "" && target != varName) {
			target = variables[target].target;
		}
		if (target == varName) {
			//cyclic, don't do anything
			return;
		}

		//TODO: make sure unknownAlias is properly transfered
		//fix any vars that reference varName
		auto aliasList = variables[varName].aliases;
		if (aliasList.size() > 0) {
			auto it = aliasList.begin();
			string newMainVar = it->first;
			variables[newMainVar].target = ""; //first var is now owner of that mem
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

	void remove(string varName) {
		if (variables.count(varName) == 0) {
			cerr << "aliasVar.remove(): variable \"" + varName + "\"doesn't exist\n";
			return;
		}
		
		//remove varName from target alias list
		string oldTarget = variables[varName].target;
		if (oldTarget != "") {
			variables[oldTarget].aliases.erase(varName);
		}

		//TODO: make sure unknownAlias is properly transfered
		//cleanup all vars targeting varName
		auto aliasList = variables[varName].aliases;
		if (aliasList.size() > 0) {
			auto it = aliasList.begin();
			string newMainVar = it->first;
			variables[newMainVar].target = ""; //first var is now owner of that mem
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


string getAttrTarget(aliasVars &vars, Node *attrExpr);


void unswitch_loops(void) {
	//get to each functions and start the recursive call
	auto classNodes = root->getChildren();
	for (Tree *tchild : classNodes) {
		Node *child = (Node *)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;
		Node* features = ((Node *)classDescNodes[2]);

		//go to correct class
		globalSymTable->goToClass(className);

		vector<Node *>attributes;
		vector<Node *>methods;
		for (Tree *tFeature : features->getChildren()) {
			Node *feature = (Node *)tFeature;

			//Only care about methods
			if (feature->type == AST_FEATURE_METHOD) {
				methods.push_back(feature);
			}
			else 
			{ //AST_FEATURE_ATTRIBUTE
				attributes.push_back(feature);
			}
		}

		//keep track of the variables as you go
		aliasVars vars;

		//go through each attribute and mark the variable
		for (Node *attr : attributes) {
			auto children = attr->getChildren();
			string attrName = ((Node *)children[0])->value;
			Node *attrExpr = (Node *)children[2];

			string attrResult = getAttrTarget(vars, attrExpr);
			vars.set(attrName, attrResult);
		}

		for (Node *method : methods) {
			//copy vars so we can do what we want
			//get formals, add them to vars
			//delete previous entries of the same name (scope overrides)

			//recurse, build vars down, build the if list and stuff from the bottom
		}

	}
}

string getAttrTarget(aliasVars &vars, Node *attrExpr)
{
	switch (attrExpr->type)
	{
	case AST_IDENTIFIER:
		//return the variable
		return attrExpr->value;
		break;
	case AST_LARROW:
	{
		//return the var that is being assigned
		string leftVar = ((Node *)attrExpr->getChildren()[0])->value;
		string exprRet = getAttrTarget(vars, (Node *)attrExpr->getChildren()[1]);
		vars.set(leftVar, exprRet);
		return leftVar;
		break;
	}
	case AST_DISPATCH:
		//return "?" to signify that who knows (because dispatch)
		//TODO: run through the dispatch expression incase a variable is set
		return "?";
		break;
	case AST_WHILE:
		//go through the while expr for completeness
		//TODO: go through the while conditional
		getAttrTarget(vars, (Node *)attrExpr->getChildren()[1]);
		//TODO: go through the while conditional again (since it's the last thing that runs
		//TODO: or do I just record all vars in it as unknown (probably better)(need that func first)
		return ""; //while always returns null
		break;
	case AST_EXPRSEMILIST:
	{
		//return the block's final expression
		auto blockChildren = attrExpr->getChildren();
		//TODO: go through each expression so so other vars are set correctly
		Node *lastExpr = (Node *)blockChildren[blockChildren.size() - 1];
		return getAttrTarget(vars, lastExpr);
		break;
	}
	case AST_LET:
		//return the let's expression
		return getAttrTarget(vars, (Node *)attrExpr->getChildren()[1]);
		break;
	case AST_IF:
	{
		//will need to check both possibilities
		//TODO: go through the conditional (always)
		//TODO: invalidate anything that is assigned in the then or else
		auto ifChildren = attrExpr->getChildren();
		string thenRet = getAttrTarget(vars, (Node *)ifChildren[1]);
		string elseRet = getAttrTarget(vars, (Node *)ifChildren[2]);
		if (thenRet == elseRet) {
			return thenRet;
		}
		else {
			//could be anything
			return "?";
		}
		break;
	}
	case AST_CASESTATEMENT:
	{
		//will need to check all possibilities
		//TODO: invalidate anything asigned in any of the cases
		auto cases = attrExpr->getChildren()[1]->getChildren();
		string first = getAttrTarget(vars, (Node *)cases[0]);
		bool unknownAlias = false;
		for (size_t i = 1; i < cases.size(); i++) {
			string cur = getAttrTarget(vars, (Node *)cases[i]);
			if (cur != first) {
				//return unknown if any of them end up different
				unknownAlias = true;
			}
			first;
		}
		if (unknownAlias) {
			return "?";
		}
		//all the same, lucky
		return first;
		break;
	}
	default:
		//return "" to signify a new value or a null value
		return "";
		break;
	}
}