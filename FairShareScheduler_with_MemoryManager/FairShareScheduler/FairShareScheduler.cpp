#include "all_my_headers.h"
using namespace std;

bool fairShareScheduler::FINISHED_TRAP = false;
bool fairShareScheduler::PAUSED_TRAP = false;
bool fairShareScheduler::RESUMED_TRAP = false;
bool fairShareScheduler::MEMORY_ACCESS_FINISHED = false;
int  fairShareScheduler::quantum = 2;
vector<string> * fairShareScheduler::trace = new vector<string>;
stack<int> fairShareScheduler::results;

void fairShareScheduler::runScheduler() {

	readInputFile();
	scheduleUsers();
	writeTraceIntoFile();
}

//initialize queues, wait for ready process, keep running top process until done
void fairShareScheduler::scheduleUsers(){
	myMemoryManager = &(MemoryManager());
	initializeQueues();
	waitForReadyProcess();

	while (ready_queue.size() > 0) {
		runFirstReadyProcess();

		if (ready_queue.size() == 0 && waiting_queue.size() != 0) {
			waitForReadyProcess();
		}
	}
}

//main function that creates/resumes/suspends process threads
//synchronized using 3 traps for resumed, paused and finished
void fairShareScheduler::runFirstReadyProcess() {
	//reset traps
	FINISHED_TRAP = false;
	PAUSED_TRAP = false;
	RESUMED_TRAP = false;
	MEMORY_ACCESS_FINISHED = false;

	//compute quantum to assign
	float currentQuantum = computeProcessQuantum(ready_queue.front());
	process * current = ready_queue.front();

	cout << "\n--------------------------------------------------------------------------------";
	printTrace(currentTime, current->parent, current->pId, "Start", currentQuantum, current->burstTime);

	//create or resume process thread
	if (!current->finished) {
		if (current->created) {
			current->resume_thread(currentQuantum);
		}
		else {
			current->create_thread(currentQuantum);
		}
	}
	else {
		printTrace(currentTime, current->parent, current->pId, "!! ERROR !!", currentQuantum, current->burstTime);
		return;
	}

	//start timer
	bool quantum_finished = false;
	auto start = std::chrono::system_clock::now();

	//wait for process to resume
	while (!RESUMED_TRAP){ this_thread::yield(); };

	while (!MEMORY_ACCESS_FINISHED) { this_thread::yield(); };

	//wait for trap or quantum finished
	while (!(FINISHED_TRAP || quantum_finished)){
		quantum_finished = sleep_quantum(static_cast<int>(currentQuantum * 100));
	}

	//pause thread
	current->pause_thread();

	//wait for process to pause (could add: kick if takes too long)
	while (!PAUSED_TRAP){ this_thread::yield(); };

	auto end = std::chrono::system_clock::now();
	auto time_waited = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//increment currentTime by time_waited
	currentTime += static_cast<float>(time_waited) / 1000;

	//check if any waiting processes are ready now
	reevaluateQueues();

	if (current->burstTime <= 0) {
		//destroy process
		ready_queue.erase(ready_queue.begin());
		printTrace(currentTime, current->parent, current->pId, "Finish", currentQuantum, current->burstTime);
		cout << "\n--------------------------------------------------------------------------------";
	}
	else {
		//shift process to back of queue
		ready_queue.push_back(ready_queue.front());
		ready_queue.erase(ready_queue.begin());
		printTrace(currentTime, current->parent, current->pId, "Pause", currentQuantum, current->burstTime);
		cout << "\n--------------------------------------------------------------------------------";
	}
}




//return true after given time (quantum) has passed or when FINISHED_TRAP activated
//check FINISHED_TRAP every 5 ms interval
bool fairShareScheduler::sleep_quantum(int time){
	if (time > 0 && !FINISHED_TRAP) {
		this_thread::sleep_for(chrono::milliseconds(5));
		sleep_quantum(time - 5);
	}
	return true;
}

//compute the quantum share of this process
float fairShareScheduler::computeProcessQuantum(process* thisProcess){
	float processQuantum = 0;
	float numOfReadyUsers = 0;
	float numOfprocessesThisUser = 0;

	for (vector<user>::iterator it = myUsers.begin(); it != myUsers.end(); it++) {
		int rdyProcesses = it->checkReady(currentTime);
		if (rdyProcesses > 0) {
			numOfReadyUsers++;
		}

		if (it->userName == thisProcess->parent) {
			numOfprocessesThisUser += rdyProcesses;
		}
	}
	processQuantum = quantum / (numOfReadyUsers * numOfprocessesThisUser);

	return processQuantum;
}

//repeat: increment currentTime and check for ready processes
void fairShareScheduler::waitForReadyProcess() {

	while (ready_queue.size() == 0) {
		currentTime++;
		reevaluateQueues();
	}

}

int fairShareScheduler::memory_request(string command) {
	MemoryManager::add_command(command);

	while (!MEMORY_ACCESS_FINISHED) { this_thread::yield(); }

	int result = results.top();
	results.pop();
	return result;
}

//check if any processes in waiting_queue should be now moved to ready_queue
void fairShareScheduler::reevaluateQueues() {
	for (vector<process*>::iterator it = waiting_queue.begin(); it != waiting_queue.end();) {
		(*it)->checkReady(currentTime);

		if ((*it)->ready) {
			ready_queue.push_back(*it);
			it = waiting_queue.erase(it);
			if (waiting_queue.size() == 0) {
				break;
			}
		}
		else {
			it++;
		}
	}
}

//fill the waiting_queue and ready_queue initialy 
void fairShareScheduler::initializeQueues() {
	vector<user>::iterator it;
	vector<process>::iterator jt;
	for (it = myUsers.begin(); it != myUsers.end(); it++) {
		it->checkReady(currentTime);
		for (jt = it->myProcesses.begin(); jt != it->myProcesses.end(); jt++) {
			if (jt->ready) {
				ready_queue.push_back(&(*jt));
			}
			else {
				waiting_queue.push_back(&(*jt));
			}
		}
		
	}
}


//create the users and their processes
void fairShareScheduler::readInputFile() {
	try {
		ifstream input_file("processes.txt");
		string line;
		if (input_file.is_open()) {
			getline(input_file, line);
			int number_of_processes = stoi(line);
				
			for (int i = 0; i < number_of_processes; ++i) {
				string user_name = to_string(i + 1);
				user * thisUser = new user(user_name);
				getline(input_file, line);
				int pos = line.find(' ');
				float arrival = stof(line.substr(0, pos));
				line = line.substr(pos + 1, -1);
				float burst = stof(line.substr(0, -1));

				thisUser->addProcess(burst, arrival);
				myUsers.push_back(*thisUser);
			}

		}
	}
	catch (exception e) {
		throw e;
	}
}

//save trace into output file
void fairShareScheduler::writeTraceIntoFile() {
	try {
		ofstream output_file("output.txt");
		if (output_file.is_open()) {
			for (string it : *trace) {
				output_file << it << endl;
			}
		}
	}
	catch (exception e) {
		throw e;
	}
}

//output scheduler activity and save trace
void fairShareScheduler::printTrace(float time, string parent, int pid, string state, float quant, float burst) {
	string line = "\nTime: " + to_string(time) + " - Process: " + parent + " - " + state +"\n\n";
	trace->push_back(line);
	cout << line;
}