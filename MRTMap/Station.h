#pragma once
#include "List.h"

class Station
{
private:
	string name;
	List code;
	bool interchange = false;

public:
	// constructor
	Station();
	Station(string n, string c);

	// add station code
	bool addCode(string c);

	// check if is interchange
	bool isInterchange();
};

