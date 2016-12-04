#include "constpropsettings.h"



ConstPropSettings::ConstPropSettings()
{
	otherVarMap = map<string,string>();
	formalVarMap = map<string, string>();
	localVarMap = map<string, stack<string>>();
	this->changed = vector<set<string>>();
	changed.emplace(changed.end(), set<string>());
	changed.emplace(changed.end(), set<string>());
	changed.emplace(changed.end(), set<string>()); 
	
}

ConstPropSettings::~ConstPropSettings()
{
	//lol
}

ConstPropSettings::ConstPropSettings(ConstPropSettings &toTakeIn)
{
	otherVarMap = toTakeIn.getOtherMap();
	formalVarMap = toTakeIn.getFormalMap();
	localVarMap = toTakeIn.getLocalMap();
	this->changed = vector<set<string>>();
	changed.emplace(changed.end(), set<string>());
	changed.emplace(changed.end(), set<string>());
	changed.emplace(changed.end(), set<string>());
}

string ConstPropSettings::getVal(string name) {
	string ret = "";
	if (localVarMap.count(name)) {
		ret = localVarMap.find(name)->second.top();
	}
	else if (formalVarMap.count(name)) {
		ret = formalVarMap.find(name)->second;
	}
	else if(otherVarMap.count(name)) {
		ret = otherVarMap.find(name)->second;
	}
	return ret;
}

void ConstPropSettings::addLocal(string name, string value = "")
{
	if (localVarMap.count(name)) {
		localVarMap.find(name)->second.push("");
	}
	else { //not in locals yet.	
		stack<string> tmp = stack<string>();
		tmp.push(value);

		localVarMap.emplace(name, tmp);
	}
}

void ConstPropSettings::addFormal(string name) 
{
	if (formalVarMap.count(name)) {
		formalVarMap.find(name)->second = "";
	}
	else { //not in locals yet.		
		formalVarMap.emplace(name, "");
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
		this->changed.at(0).emplace(name);
		this->localVarMap.find(name)->second.pop();
		this->localVarMap.find(name)->second.push(value);
	}
	else if (formalVarMap.count(name) == 1) {
		this->changed.at(1).emplace(name);
		formalVarMap.find(name)->second = value;
	}
	else if (otherVarMap.count(name) == 1) {
		this->changed.at(2).emplace(name);
		otherVarMap.find(name)->second = value;
	}
	else {
		this->changed.at(2).emplace(name);
		otherVarMap.emplace(name, value);
	}
}

void ConstPropSettings::removeOthers()
{
	otherVarMap.clear();
}

vector<set<string>> ConstPropSettings::getChanged()
{
	return this->changed;
}

map<string, stack<string>> ConstPropSettings::getLocalMap()
{
	return this->localVarMap;
}

map<string, string> ConstPropSettings::getFormalMap()
{
	return this->formalVarMap;
}

map<string, string> ConstPropSettings::getOtherMap()
{
	return this->otherVarMap;
}

void ConstPropSettings::removeVar(string name)
{
	if (localVarMap.count(name)) {
		this->changed.at(0).emplace(name);
		localVarMap.find(name)->second.pop();
		if (localVarMap.find(name)->second.empty()) {
			localVarMap.erase(name);
		}
	}
	else if (formalVarMap.count(name)) {
		this->changed.at(1).emplace(name);
		formalVarMap.erase(name);
	}
	else if (otherVarMap.count(name)) {
		this->changed.at(2).emplace(name);
		otherVarMap.erase(name);
	}
}


void ConstPropSettings::removeChanged(vector<set<string>> changed)
{
	set<string> localChanged = changed.at(0);
	set<string> formalChanged = changed.at(1);
	set<string> otherChanged = changed.at(2);

	for (string s : localChanged) {
		if (this->localVarMap.count(s)) {
			this->changed.at(0).emplace(s);
			this->localVarMap.at(s).pop();
			this->localVarMap.at(s).push("");
		}
	} 

	for (string s : formalChanged) {
		if (this->formalVarMap.count(s)) {
			this->changed.at(1).emplace(s);
			this->formalVarMap.at(s).assign("");
		}
	}

	for (string s : otherChanged) {
		if (this->otherVarMap.count(s)) {
			this->changed.at(2).emplace(s);
			this->otherVarMap.at(s).assign("");
		}
	}

}