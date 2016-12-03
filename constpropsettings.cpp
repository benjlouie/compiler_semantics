#include "constpropsettings.h"



ConstPropSettings::ConstPropSettings()
{
	otherVarMap = map<string,string>();
	formalVarMap = map<string, string>();
	localVarMap = map<string, stack<string>>();
}

ConstPropSettings::~ConstPropSettings()
{
	//lol
}

void ConstPropSettings::addLocal(string name, string value = nullptr)
{
	if (localVarMap.count(name) == 1) {
		localVarMap.find(name)->second.push(value);
	}
	else { //not in locals yet.		
		localVarMap.emplace(name, value);
	}
}

void ConstPropSettings::addFormal(string name) 
{
	if (formalVarMap.count(name) == 1) {
		formalVarMap.find(name)->second = nullptr;
	}
	else { //not in locals yet.		
		formalVarMap.emplace(name, nullptr);
	}
}

void ConstPropSettings::addOther(string name, string value)
{
	if (otherVarMap.count(name) == 1) {
		otherVarMap.find(name)->second = value;
	}
	else { //not in locals yet.		
		formalVarMap.emplace(name, value);
	}
}

void ConstPropSettings::addValToVar(string name, string value)
{
	if (localVarMap.count(name) == 1) {
		localVarMap.find(name)->second.pop();
		localVarMap.find(name)->second.push(value);
	}
	else if (formalVarMap.count(name) == 1) {
		formalVarMap.find(name)->second = value;
	}
	else if (otherVarMap.count(name) == 1) {
		otherVarMap.find(name)->second = value;
	}
	else {
		otherVarMap.emplace(name, value);
	}
}

void ConstPropSettings::removeOthers()
{
	otherVarMap.clear();
}