// CLI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Handler.h"
#include <iostream>
using namespace std;


//left to fix

int _tmain(int argc, _TCHAR* argv[])
{
	internalH * myInternalHandler = new internalH();
	includedH * myIncludedHandler = new includedH();
	externalH * myExternalHandler = new externalH();

	myInternalHandler->set_next_handler(myIncludedHandler);
	myIncludedHandler->set_next_handler(myExternalHandler);
	
	string commands[] = { "dir", "assign t 1000", "test eq t() plus 500 500", "test gt 2 1", "plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 plus 1 1;", 
							"minus 2 1;", "assign a 1;", "plus 1 a();", "minus a() a();", "assign b 2;", "plus a() b();", "assign c plus 1 2;", "plus c() c();" };

	int number_of_commands = sizeof(commands) / sizeof(commands[0]);

	for (int i = 0; i < number_of_commands; i++) {
		myInternalHandler->set_command(commands[i]);
		cout << "\nExecuting: " + commands[i];
		myInternalHandler->handle();

		if (myInternalHandler->success) {
			cout << "\n\tAnswer: " << myInternalHandler->answer << "\n\n";
			system("pause");
		}
		else if (myIncludedHandler->success) {
			cout << "\n\tIncluded command result: " << myInternalHandler->success << "\n\n";
			system("pause");
		}
	}

	return 0;
}

/*

left to do: 
	implement class Test (to compare)
	implement includedhandler
	implement externalhandler

	could easily extend variable names to longer than 1 char 

*/