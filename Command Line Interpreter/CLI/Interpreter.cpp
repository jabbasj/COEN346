#include "stdafx.h"
#include "Interpreter.h"
using namespace std;


Plus::Plus(string command) {
	fullCommand = command;
	interpret();
};

int Plus::add(int op1, int op2) {
	return op1 + op2;
}

int Minus::sub(int op1, int op2) {
	return op1 - op2;
}

void Plus::interpret() {
	string argument1 = "";
	string argument2 = "";
	int position = -1;
	position = fullCommand.rfind("plus");

	string lastCommand = "";

	if (position >= 0) {
		lastCommand = fullCommand.substr(position, 4); //4 = length("plus")
		int pos1 = position + lastCommand.length() + 1;
		int pos2 = fullCommand.find(' ', pos1);
		argument1 = fullCommand.substr(pos1, pos2 - pos1);
		pos1 = fullCommand.find(' ', pos2) + 1;
		pos2 = fullCommand.find(';');
		argument2 = fullCommand.substr(pos1, pos2 - pos1);

		if (lastCommand == "plus") {
			result = add(stoi(argument1), stoi(argument2));
			fullCommand = fullCommand.substr(0, position) + to_string(result) + ';';
		}
	}
	else {
		return;
	}

	return;
}

Minus::Minus(string command) {
	fullCommand = command;
	interpret();
};

void Minus::interpret() {
	string argument1 = "";
	string argument2 = "";
	int position = -1;
	position = fullCommand.rfind("minus");

	string lastCommand = ""; //just used as sanity check

	if (position >= 0) {
		lastCommand = fullCommand.substr(position, 5); //5 = length("minus") -- can use find(' ') to ensure it's not minusJUNK
		int pos1 = position + lastCommand.length() + 1;
		int pos2 = fullCommand.find(' ', pos1);
		argument1 = fullCommand.substr(pos1, pos2 - pos1);
		pos1 = fullCommand.find(' ', pos2) + 1;
		pos2 = fullCommand.find(';');          //even if command doesn't end with ;, this will work by going to the end of the line (feature aka bug  :P)
		argument2 = fullCommand.substr(pos1, pos2 - pos1);

		if (lastCommand == "minus") {
			result = sub(stoi(argument1), stoi(argument2));
			fullCommand = fullCommand.substr(0, position) + to_string(result) + ';';
		}
	}
	else {
		return;
	}
	return;
}


Assign::Assign(string command){
	fullCommand = command;
	interpret();
}

void Assign::interpret(){
	string argument1 = "";
	string argument2 = "";
	int position = -1;
	position = fullCommand.rfind("assign");

	string lastCommand = "";

	if (position >= 0) {
		lastCommand = fullCommand.substr(position, 6); //6 = length("assign")
		int pos1 = position + lastCommand.length() + 1;
		int pos2 = fullCommand.find(' ', pos1);
		argument1 = fullCommand.substr(pos1, pos2 - pos1);
		pos1 = pos2 + argument1.length();
		pos2 = fullCommand.find(';');
		argument2 = fullCommand.substr(pos1, pos2 - pos1);

		if (lastCommand == "assign") {
			var_name = argument1[0];
			var_value = stoi(argument2);
			fullCommand = fullCommand.substr(0, position) + ';';
		}
	}

	return;
}

Test::Test(string command) {
	fullCommand = command;
	interpret();
}

void Test::interpret() {
	string type = "";
	string argument1 = "";
	string argument2 = "";
	int position = -1;
	position = fullCommand.rfind("test");

	string lastCommand = "";

	if (position == 0) {
		lastCommand = fullCommand.substr(position, 4);		// 4 = length("test")
		int pos1 = position + lastCommand.length() + 1;		// should be first ' '
		int pos2 = fullCommand.find(' ', pos1);				// find first ' ' after pos1
		type = fullCommand.substr(pos1, pos2 - pos1);		// substr from pos1 length pos2-pos1 (eq, gt, lt)
		pos1 = fullCommand.find(' ', pos2) + 1;				// find first ' ' after pos2
		pos2 = fullCommand.find(' ', pos1);					// find first ' ' after pos1
		argument1 = fullCommand.substr(pos1, pos2 - pos1);  // pos2 - pos1 is the length of op1
		pos1 = fullCommand.find(' ', pos2) + 1;
		pos2 = fullCommand.find(';');
		argument2 = fullCommand.substr(pos1, pos2 - pos1);

		if (lastCommand == "test") {
			result = compare(type, stoi(argument1), stoi(argument2));
			fullCommand = to_string(result) + ";";
		}
	}
	else {
		return;
	}

	return;
}

bool Test::compare(string type, int op1, int op2) {

	if (type == "gt") {
		return (op1 > op2);
	}
	if (type == "lt") {
		return (op1 < op2);
	}
	if (type == "eq") {
		return (op1 == op2);
	}

	return false;
}