#include <string>
#include <map>
using namespace std;

class Handler {

public:
	string fullCommand;
	bool success = false;

	void set_command(string command);
	map<char, int> my_variables;
	int resolveVar(char name);
};

class externalH : public Handler {
public:
	externalH();
	void handle();
};

class includedH : public Handler {

public:
	includedH();
	externalH * next_handler;
	void set_next_handler(externalH * next);
	void handle();

	string answer = "";
};

class internalH : public Handler{
	friend class Plus;
	friend class Minus;
	friend class Assign;
	friend class Test;

public:
	internalH();

	includedH * next_handler;
	void set_next_handler(includedH * next);

	void handle();
	string answer = "";

};




