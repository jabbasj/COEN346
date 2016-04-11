#include "all_my_headers.h"
using namespace std;

class user {
	friend class fairShareScheduler;

private:
	string userName;
	int number_of_processes = 0;
	int pid = 0;

	vector<process> myProcesses;

	void addProcess(float, float);
	int checkReady(float time);

public:
	user();
	user(string name);
};