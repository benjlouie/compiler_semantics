#pragma once
#include <iostream>
#include <string>
#include <map>
#include <stack>
#include <set>
#include <vector>

using namespace std;

class ConstPropSettings
{
public:
	ConstPropSettings();
	~ConstPropSettings();
	ConstPropSettings(ConstPropSettings &toTakeIn);
	string getVal(string name);
	string getType(string name);
	void addLocal(string name, string type, string value);
	void addFormal(string name, string type);
	void addOther(string name, string type, string value);
	void addValToVar(string name, string type,string value);
	void removeOthers();
	void removeChanged(vector<set<string>> changed);
	void removeVar(string name);
	vector<set<string>> getChanged();
	map<string, pair<string, string>> getOtherMap();
	map<string, pair<string, string>> getFormalMap();
	map<string, pair<string, stack<string>>> getLocalMap();

private:
	map<string, pair<string,string>> otherVarMap;
	map<string, pair<string, string>> formalVarMap;
	map<string, pair<string,stack<string>>> localVarMap;
	vector<set<string>> changed;
};
