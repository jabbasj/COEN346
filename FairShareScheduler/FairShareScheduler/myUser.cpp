#include "stdafx.h"

user::user(string name) {
	userName = name;
}

void user::addProcess(float burst, float arrival) {
	myProcesses.push_back(process(pid, burst, arrival, userName));
	++number_of_processes;
	++pid;
}

int user::checkReady(float time) {
	vector<process>::iterator it;
	int ready = 0;
	for (it = myProcesses.begin(); it != myProcesses.end(); it++) {
		it->checkReady(time);
		if (it->ready) {
			ready++;
		}
	}
	return ready;
}