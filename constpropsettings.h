#pragma once
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
	ConstPropSettings::ConstPropSettings(ConstPropSettings &toTakeIn);
	string ConstPropSettings::getVal(string name);
	void addLocal(string name, string value);
	void addFormal(string name);
	void addOther(string name, string value);
	void addValToVar(string name, string value);
	void removeOthers();
	void removeChanged(vector<set<string>> changed);
	void removeVar(string name);
	vector<set<string>> getChanged();
	map<string, string> ConstPropSettings::getOtherMap();
	map<string, string> ConstPropSettings::getFormalMap();
	map<string, stack<string>> ConstPropSettings::getLocalMap();

private:
	map<string, string> otherVarMap;
	map<string, string> formalVarMap;
	map<string, stack<string>> localVarMap;
	vector<set<string>> changed;
};