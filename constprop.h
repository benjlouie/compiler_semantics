#pragma once
#include <iostream>
#include <array>
#include "constpropsettings.h"
#include"semant.h"


using namespace std;

extern Node *root;
extern int numErrors;

class ConstProp
{
public:
	ConstProp();
	ConstProp(ConstPropSettings settings);
	~ConstProp();
	bool init();
	bool doConstProp(vector<Node *> nodes);
	bool doConstProp(Node *node);
	ConstPropSettings getSettings();
	void setSettings(ConstPropSettings settings);
	bool massiveSwitch(Node *expr);

private:
	ConstPropSettings settings;
	int getVal(Node *);
	set<string> getAssigned(Node *expr);
};