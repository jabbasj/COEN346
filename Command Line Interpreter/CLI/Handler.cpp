#include "stdafx.h"
#include <string>
#include "Handler.h"
#include "Interpreter.h""
using namespace std;


int Handler::resolveVar(char name) {
	return my_variables.find(name)->second;
}

internalH::internalH(){}

includedH::includedH(){}

externalH::externalH(){}

void internalH::set_next_handler(includedH * next) {
	next_handler = next;
}

void includedH::set_next_handler(externalH * next) {
	next_handler = next;
}

//resolves any variables in the command before saving it.
void Handler::set_command(string command) {
	fullCommand = command;
	int position = -1;
	while (fullCommand.find("()") != -1) {
		position = fullCommand.find("()");
		while (fullCommand[position] != ' ') {
			position = --position;
		}
		string varName = fullCommand.substr(position + 1, 1);
		int value = resolveVar(varName[0]);
		if (value) {
			fullCommand.replace(position + 1, 3, to_string(value));
		}
	}
}

void internalH::handle() {
	success = false;
	int result;
	int plus_position = -1;
	int minus_position = -1;
	int assign_position = -1;
	int test_position = 1;
	assign_position = fullCommand.rfind("assign");
	plus_position = fullCommand.rfind("plus");
	minus_position = fullCommand.rfind("minus");
	test_position = fullCommand.rfind("test");
	int finished = -4;

	// check if plus at right-most position in command
	if (plus_position > minus_position && plus_position > assign_position) {

		Plus * addition = new Plus(fullCommand);
		result = addition->result;
		fullCommand = addition->fullCommand;
		success = true;
		finished = fullCommand.rfind("minus") + fullCommand.rfind("plus") + fullCommand.rfind("assign");
		if (finished >= -2) {
			handle();
			return;
		}
		else {
			answer = to_string(result);
		}
	}

	// check if minus at right-most position in command
	if (minus_position > plus_position && minus_position > assign_position) {

		Minus * substraction = new Minus(fullCommand);
		result = substraction->result;
		fullCommand = substraction->fullCommand;
		success = true;
		finished = fullCommand.rfind("minus") + fullCommand.rfind("plus") + fullCommand.rfind("assign"); //returns -1 if not found or position (0+)
		if (finished >= -2) {
			handle(); //more internal commands next
			return;
		}
		else {
			answer = to_string(result);
		}
	} 

	// check if assign at right-most position in command
	if (assign_position > plus_position && assign_position > minus_position) {
		
		Assign * assigning = new Assign(fullCommand);
		fullCommand = assigning->fullCommand;
		my_variables.insert(pair<char, int>(assigning->var_name, assigning->var_value));
		success = true;
		answer = assigning->var_name;
		answer = answer + " assigned to " + to_string(assigning->var_value);
	}

	// testing returns a boolean value -> does not interact with adding / substracting / assigning
	// this should be the only command in the line
	if (test_position == 0) {
		Test * test = new Test(fullCommand);
		fullCommand = test->fullCommand;
		result = (int)test->result;
		success = true;
		if (result) {
			answer = "true";
		}
		else {
			answer = "false";
		}
		return;
	}

	if ((test_position + plus_position + minus_position + assign_position) <= -4) {
		next_handler->set_command(fullCommand);
		next_handler->handle();
	}
	return;
}

void externalH::handle() {
	// check if absolue path exists
	//set runExcluded variable ?
	return;
}

void includedH::handle() {
	//check if path exists in environment variable path
	//set runIncluded variable?
	if (fullCommand != "") {
		try {
			answer = to_string(system(fullCommand.c_str()));
			success = true;
		}
		catch (exception e) {
			answer = e.what();
		}
	}
	next_handler->handle();
	return;
}