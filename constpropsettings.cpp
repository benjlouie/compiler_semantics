#include "constpropsettings.h"



ConstPropSettings::ConstPropSettings()
{
	otherVarMap = map<string, pair<string, string>>();
	formalVarMap = map<string, pair<string, string>>();
	localVarMap = map<string, pair<string, stack<string>>>();
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
	cout << "Getting val of " << name;
	if (localVarMap.count(name)) {
		cout << " Local ";
		ret = localVarMap.find(name)->second.second.top();
		cout << ret << endl;
		return ret;
	}
	else if (formalVarMap.count(name)) {
		cout << " Formal ";
		ret = formalVarMap.find(name)->second.second;
		cout << ret << endl;
		return ret;
	}
	else if(otherVarMap.count(name)) {
		cout << " Other ";
		ret = otherVarMap.find(name)->second.second;
		cout << ret << endl;
	}
	return ret;
}

string ConstPropSettings::getType(string name)
{
	string ret = "";
	cout << "Getting type of " << name;
	if (localVarMap.count(name)) {
		cout << " Local ";
		ret = localVarMap.find(name)->second.first;
		cout << ret << endl;
	}
	else if (formalVarMap.count(name)) {
		cout << " Formal ";
		ret = formalVarMap.find(name)->second.first;
		cout << ret << endl;
	}
	else if (otherVarMap.count(name)) {
		cout << " Other " ;
		ret = otherVarMap.find(name)->second.first;
		cout << ret << endl;
	}
	return ret;
}

void ConstPropSettings::addLocal(string name, string type, string value = "")
{
	cout << "Adding local " << name << " with type " << type << " and value " << value << endl;
	if (localVarMap.count(name)) {
		localVarMap.find(name)->second.second.push("");
		changed.at(0).emplace(name);
	}
	else { //not in locals yet.	
		pair<string,stack<string>> tmp = pair<string,stack<string>>();
		tmp.second = stack<string>();

		tmp.first = type;
		tmp.second.push(value);

		localVarMap.emplace(name, tmp);
	}
}

void ConstPropSettings::addFormal(string name, string type) 
{
	if (formalVarMap.count(name)) {
		formalVarMap.find(name)->second.second = "";
		changed.at(1).emplace(name);
	}
	else { //not in formals yet.		
		pair<string, string> tmp = pair<string, string>();
		tmp.first = type;
		tmp.second = "";
		formalVarMap.emplace(name, tmp);
	}
}

void ConstPropSettings::addOther(string name, string type, string value)
{
	if (otherVarMap.count(name) == 1) {
		otherVarMap.find(name)->second.second = value;
		changed.at(2).emplace(name);
	}
	else { //not in others yet.		
		pair<string, string> sec = pair<string, string>();
		sec.first = type;
		sec.second = value;
		otherVarMap.emplace(name, sec);
	}
}

void ConstPropSettings::addValToVar(string name, string type, string value)
{
	if (localVarMap.count(name)) {
		this->changed.at(0).emplace(name);
		this->localVarMap.find(name)->second.second.pop();
		this->localVarMap.find(name)->second.second.push(value);
	}
	else if (formalVarMap.count(name)) {
		this->changed.at(1).emplace(name);
		formalVarMap.find(name)->second.second = value;
	}
	else if (otherVarMap.count(name)) {
		this->changed.at(2).emplace(name);
		otherVarMap.find(name)->second.second = value;
	}
	else {
		this->changed.at(2).emplace(name);
		pair<string, string> tmp = pair<string, string>();
		tmp.first = type;
		tmp.second = value;
		otherVarMap.emplace(name, tmp);
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


map<string, pair<string, stack<string>>> ConstPropSettings::getLocalMap()
{
	return this->localVarMap;
}

map<string, pair<string, string>> ConstPropSettings::getFormalMap()
{
	return this->formalVarMap;
}

map<string, pair<string, string>> ConstPropSettings::getOtherMap()
{
	return this->otherVarMap;
}

void ConstPropSettings::removeVar(string name)
{
	if (localVarMap.count(name)) {
		this->changed.at(0).emplace(name);
		localVarMap.find(name)->second.second.pop();
		if (localVarMap.find(name)->second.second.empty()) {
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
			//make sure we count it as changed
			this->changed.at(0).emplace(s);
			//put the value to empty string, i.e. no value
			this->localVarMap.at(s).second.pop();
			this->localVarMap.at(s).second.push("");
		}
	} 

	for (string s : formalChanged) {
		if (this->formalVarMap.count(s)) {
			this->changed.at(1).emplace(s);
			this->formalVarMap.at(s).second.assign("");
		}
	}

	for (string s : otherChanged) {
		if (this->otherVarMap.count(s)) {
			this->changed.at(2).emplace(s);
			this->otherVarMap.at(s).second.assign("");
		}
	}

}