// Coen346LabShortestBurstScheduler.cpp : Defines the entry point for the console application.

#include "stdafx.h"
using namespace std;

void schedulerThreadEntry(fairShareScheduler* sched){
	sched->runScheduler();
}

int _tmain(int argc, _TCHAR* argv[])
{
	try {
		fairShareScheduler myScheduler;
		thread schedulerThread(schedulerThreadEntry, &myScheduler);
		schedulerThread.join();

		system("pause");
	}
	catch (exception e) {
		cout << "\n\tError: " << e.what();
	}
	return 0;
}



