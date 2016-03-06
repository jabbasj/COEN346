using namespace std;

class fairShareScheduler {

private:
	int quantum = -1;
	float currentTime = 0;

	vector<user> myUsers;
	vector<string> * trace = new vector<string>;

	vector<process*> ready_queue;
	vector<process*> waiting_queue;

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

	/*could add: trap handler*/
	static bool FINISHED_TRAP;
	static bool PAUSED_TRAP;
	static bool RESUMED_TRAP;
};