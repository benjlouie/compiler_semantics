#pragma once
#include <iostream>
#include <array>
#include "../src/ast.h";
#include "constpropsettings.h"


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
};