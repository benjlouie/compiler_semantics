#pragma once
#include <map>
#include <stack>

using namespace std;

class ConstPropSettings
{
public:
	ConstPropSettings();
	~ConstPropSettings();
	void addLocal(string name, string value = nullptr);
	void addFormal(string name);
	void addOther(string name, string value);
	void addValToVar(string name, string value);

	void removeOthers();


private:
	map<string, string> otherVarMap;
	map<string, string> formalVarMap;
	map<string, stack<string>> localVarMap;

};