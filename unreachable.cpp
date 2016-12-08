#include "unreachable.h"

using namespace std;

unordered_map<string, Node *> name2node;
queue<Node *> q;
string curClass;

void buildMap();
void search(Node *);
void doClass(Node *cls);
void doDispatch(Node *dis);
void doNew(Node *node);
void doMethod(Node *method);
void doIf(Node *ifNode);
void doLoop(Node *loopNode);
void doCase(Node *caseNode);
void doOther(Node *node);
void remove();
vector<string> getDescendants(string cls);

void eliminateUnreachable() {
	buildMap();
	q.push(name2node["Main"]); //know this exists because we've passed semantic analysis
	q.push(name2node["Main.main"]);
	while (q.size() != 0) {
		Node *next = q.front();
		q.pop();
		if (next == nullptr)
			continue;
		if (next->type == AST_CLASS) {
			curClass = ((Node *)next->getChildren()[0])->value;
		}
		search(next);
	}
	remove();
	
}

void buildMap() {
	/*map all classes to Nodes */
	for (auto tChld : root->getChildren()) {
		Node *cls = (Node *)tChld;
		string name = ((Node *)(cls->getChildren()[0]))->value;
		name2node[name] = cls;

		/*map all functions in class to nodes*/
		Node *featureSet = (Node *)cls->getChildren()[2];
		for (auto tChld2 : featureSet->getChildren()) {
			Node *feature = (Node *)tChld2;
			if (feature->type == AST_FEATURE_METHOD) {
				string method = ((Node *)feature->getChildren()[0])->value;
				name2node[name + "." + method] = feature;
			}
		}
	}
}

/* Calls appropriate function for node type
*  Result of calling this function should
*  Search through all children of node,
*  executing the appropriate action
*/
void search(Node *node) {
	if (node == nullptr)
		return;
	switch (node->type) {
	case AST_CLASS:
		doClass(node);
		break;
	case AST_DISPATCH:
		doDispatch(node);
		break;
	case AST_NEW:
		doNew(node);
		break;
	case AST_FEATURE_METHOD:
		doMethod(node);
		break;
	case AST_IF:
		doIf(node);
		break;
	case AST_CASE:
		doCase(node);
		break;
	case AST_WHILE:
		doLoop(node);
		break;
	default:
		doOther(node);
		break;
	}
}

void doClass(Node * cls)
{
	if (!cls->reachable) {
		cls->reachable = true;
		for (auto chld : cls->getChildren()[2]->getChildren()) {
			if (((Node *)chld)->type == AST_FEATURE_ATTRIBUTE)
				search((Node *)chld);
		}
		string name = ((Node *)(cls->getChildren()[0]))->value;
		q.push(name2node[globalTypeList[name]]); //queue the parent
	}
}

void doDispatch(Node * dis)
{
	string name = ((Node *)dis->getChildren()[2])->value;
	Node *caller = ((Node *)dis->getChildren()[0]);
	Node *stc = ((Node *)dis->getChildren()[1]);
	Node *definition = name2node[curClass + "." + name];
	if (stc->type == AST_NULL && (caller->type == AST_NULL || caller->valType == "SELF_TYPE")) { 
		vector<string> descendants = getDescendants(curClass);
		for (string desc : descendants) {
			if (name2node.count(curClass + "." + name) != 0) 
				if (!name2node[curClass + "." + name]->reachable)
					q.push(name2node[curClass + "." + name]);
		}
	}
	else {
		if (stc->type != AST_NULL) {
			if (!name2node[stc->valType + "." + name]->reachable)
				q.push(name2node[stc->valType + "." + name]);
		}
		else {
			if (!name2node[caller->valType + "." + name]->reachable)
				q.push(name2node[caller->valType + "." + name]);
		}
	}
	doOther(dis);
}

void doNew(Node * node)
{
	string clsName = ((Node *)node->getChildren()[0])->value;
	q.push(name2node[clsName]);
}

void doMethod(Node * method)
{
	if (!method->reachable) {
		method->reachable = true;
		for (auto chld : method->getChildren()) {
			search((Node *)chld);
		}
		string name = ((Node *)(method->getChildren()[0]))->value;
	}
}

void doIf(Node * ifNode)
{
	Node *cond = (Node *)ifNode->getChildren()[0];
	if (cond->type == AST_TRUE) {
		ifNode->replaceSelf((Node *)ifNode->getChildren()[1]);
	}
	else if (cond->type == AST_FALSE) {
		ifNode->replaceSelf((Node *)ifNode->getChildren()[2]);
	}
	else {
		search((Node *)ifNode->getChildren()[0]);
		search((Node *)ifNode->getChildren()[1]);
		search((Node *)ifNode->getChildren()[2]);
	}
}

void doLoop(Node * loopNode)
{
	Node *cond = (Node *)loopNode->getChildren()[0];
	Node *body = (Node *)loopNode->getChildren()[1];
	if (cond->type == AST_FALSE) {
		body->deleteSelf();
	}
	else {
		search((Node *)loopNode->getChildren()[0]);
		search((Node *)loopNode->getChildren()[1]);
	}
}

void doCase(Node * caseNode)
{
	//TODO
	doOther(caseNode);
}

void doOther(Node * node)
{
	for (auto chld : node->getChildren()) {
		search((Node *)chld);
	}
}

/* Removes any class or function that is unreachable
*  Need to check if I have to remove in symbol table
*  or other global data structures
*/
void remove() {
	string name;
	Node *node;
	for (auto table_entry : name2node) {
		name = table_entry.first;
		node = table_entry.second;
		if (node != nullptr && !node->reachable)
			node->deleteSelf();
	}
}

vector<string> getDescendants(string cls) {
	queue<string> clsQ;
	vector<string> descendants;
	string subcls = cls;
	clsQ.push(cls);
	while (clsQ.size() != 0) { //no cycles exist, this will terminate
		subcls = clsQ.front();
		clsQ.pop();
		descendants.push_back(subcls);
		for (auto pair : globalTypeList) {
			if (pair.second == subcls) {
				clsQ.push(pair.first);
			}
		}
	}
	return descendants;
}
