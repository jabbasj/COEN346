#include <string>
using namespace std;

class Plus {
public:
	Plus(string command);
	string fullCommand;
	void interpret();
	int add(int op1, int op2);
	int result;
};

class Minus {
public:
	Minus(string command);
	string fullCommand;
	void interpret();
	int sub(int op1, int op2);
	int result;
};

class Test {
public:
	string fullCommand;
	Test(string command);
	void interpret();
	bool compare(string type, int op1, int op2);
	bool result;
};



class Assign {

public:
	string fullCommand;
	Assign(string command);
	void interpret();
	char var_name;
	int var_value;
};