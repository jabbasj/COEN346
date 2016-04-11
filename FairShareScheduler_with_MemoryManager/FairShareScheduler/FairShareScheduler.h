#include "all_my_headers.h"
using namespace std;

class fairShareScheduler {
	friend class MemoryManager;

private:
	static int quantum;
	float currentTime = 0;

	vector<user> myUsers;

	vector<process*> ready_queue;
	vector<process*> waiting_queue;

	MemoryManager * myMemoryManager;
	static stack<int> results;

	void writeTraceIntoFile();
	void readInputFile();
	void printTrace(float time, string parent, int pid, string state, float quant, float burst);
	int totalAdditionsMade();
	int totalInstructions();

	void scheduleUsers();
	void initializeQueues();
	void waitForReadyProcess();
	void reevaluateQueues();
	float computeProcessQuantum(process*);
	void runFirstReadyProcess();
	bool sleep_quantum(int);

public:
	void runScheduler();
	static vector<string> * trace;
	static int memory_request(string command);

	/*could add: trap handler*/
	static bool FINISHED_TRAP;
	static bool PAUSED_TRAP;
	static bool RESUMED_TRAP;
	static bool MEMORY_ACCESS_FINISHED;
};