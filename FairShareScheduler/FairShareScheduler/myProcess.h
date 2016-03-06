using namespace std;

class process {
	friend class user;
	friend class fairShareScheduler;

private:
	int pId;
	float burstTime;
	float arrivalTime;
	long addition = 0;
	float quantum = 0;
	unsigned int program_counter = 0;
	vector<string> instructions;
	string parent;

	bool ready = false;
	bool pause = false;
	bool created = false;
	bool finished = false;

	void run_process();
	void execute_next_instruction();
	void checkReady(float time);
	void create_thread(float time);
	void resume_thread(float time);
	void pause_thread();
	void update_burst(float time);
	void CALL_TRAP_IF_FINISHED();

	bool * FINISHED_TRAP;
	bool * PAUSED_TRAP;
	bool * RESUMED_TRAP;

	thread::native_handle_type my_handle;

public:
	process(int pid, float burst, float arrival, string name);
	process();
};