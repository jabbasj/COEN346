#include "stdafx.h"
using namespace std;

bool fairShareScheduler::FINISHED_TRAP = false;
bool fairShareScheduler::PAUSED_TRAP = false;
bool fairShareScheduler::RESUMED_TRAP = false;

void fairShareScheduler::runScheduler() {

	readInputFile();
	scheduleUsers();

	cout << "\nTotal instructions read: " << totalInstructions();
	cout << "\nTotal additions executed: " << totalAdditionsMade() << "\n\n";
	writeTraceIntoFile();
}

//initialize queues, wait for ready process, keep running top process until done
void fairShareScheduler::scheduleUsers(){
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
	while (!RESUMED_TRAP){ this_thread::yield; };

	//wait for trap or quantum finished
	while (!(FINISHED_TRAP || quantum_finished)){
		quantum_finished = sleep_quantum(static_cast<int>(currentQuantum* 100));
	}

	//pause thread
	current->pause_thread();

	//wait for process to pause (could add: kick if takes too long)
	while (!PAUSED_TRAP){ this_thread::yield; };

	auto end = std::chrono::system_clock::now();
	auto time_waited = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	//increment currentTime by time_waited
	currentTime += static_cast<float>(time_waited) / 100;

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
		ifstream input_file("input.txt");
		string line;
		if (input_file.is_open()) {
			int i = 0;
			getline(input_file, line); //first line = quantum
			quantum = stoi(line);

			while (getline(input_file, line)) {
				int pos = line.find(' ');
				string user_name = line.substr(0, pos);
				line = line.substr(pos + 1, -1);
				int number_of_processes = stoi(line);

				user * thisUser = new user(user_name);
				
				for (int i = 0; i < number_of_processes; ++i) {
					getline(input_file, line);
					pos = line.find(' ');
					float arrival = stof(line.substr(0, pos));
					line = line.substr(pos + 1, -1);
					float burst = stof(line.substr(0, -1));

					thisUser->addProcess(burst, arrival);
				}
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
	string line = "\nTime: " + to_string(time) + " - Usr: " + parent + " - Prcs: " + to_string(pid) + " - " + state + " - Qntm: " + to_string(quant) + " - Left: " + to_string(burst) +"\n\n";
	trace->push_back(line);
	cout << line;
}

//simply add up the additions made by each process, for emulation
int fairShareScheduler::totalAdditionsMade(){
	int totalAdditionsMade = 0;
	for (vector<user>::iterator it = myUsers.begin(); it != myUsers.end(); it++) {
		for (vector<process>::iterator jt = it->myProcesses.begin(); jt != it->myProcesses.end(); jt++) {
			totalAdditionsMade += (jt->addition);
		}
	}

	return totalAdditionsMade;
}

//counts the number of instructions loaded into process, for emulation
int fairShareScheduler::totalInstructions(){
	int total_instructions = 0;
	for (vector<user>::iterator it = myUsers.begin(); it != myUsers.end(); it++) {
		for (vector<process>::iterator jt = it->myProcesses.begin(); jt != it->myProcesses.end(); jt++) {
			total_instructions += (jt->instructions.size());
		}
	}

	return total_instructions;
}