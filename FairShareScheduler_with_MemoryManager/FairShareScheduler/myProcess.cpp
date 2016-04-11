#include "all_my_headers.h"
using namespace std;

#define REAL_SCENARIO

//#define RANDOM_DELAY /*causes 500 ms wait when program_counter = 2*/

#define BURST_VALUE 1 /* For emulation: 
							MAX: 1

							burstTime / BURST_VALUE  = # of instructions per process
				 			sleep for BURST_VALUE * 100 (or quantum if smaller) per instruction
							burstTime and quantum are decremented by BURST_VALUE increments for each instruction

								As you decrease BURST_VALUE, more instructions are executed but there is some overhead in switching so not all quantum is turned into 'burst'
								It is assumed that 'burstTime' is an estimate of actual instruction processing and DOES NOT GET UPDATED based on actual time or instructions left (maybe TODO)
								
								In the end, there could be a mismatch between the last 'burstTime' and # of instructions left to execute
								because we assume any burst <= burst_value executes 1 instruction
						 */

vector<string> process::instructions;

process::process() {
	pId = -1;
	burstTime = -1;
	arrivalTime = -1;
	finished = true;
}

//create process, load fake instructions and save references to scheduler traps
process::process(int pid, float burst, float arrival, string name){
	pId = pid;
	burstTime = burst;
	arrivalTime = arrival;
	parent = name;

	//load fake program instructions
	read_instructions();

	FINISHED_TRAP = &(fairShareScheduler::FINISHED_TRAP);	
	PAUSED_TRAP = &(fairShareScheduler::PAUSED_TRAP);
	RESUMED_TRAP = &(fairShareScheduler::RESUMED_TRAP);
	MEMORY_ACCESS_FINISHED = &(fairShareScheduler::MEMORY_ACCESS_FINISHED);
}

void process::read_instructions() {

	try {
		if (instructions.size() == 0) {
			ifstream input_file("commands.txt");
			string line;
			if (input_file.is_open()) {
				while (getline(input_file, line)) {
					instructions.push_back(line);
				}
			}
		}
	}
	catch (exception e) {
		throw e;
	}
}


//method that runs on each process thread
void process::run_process() {

	while (!finished && ready) {
		//set resumed trap
		*RESUMED_TRAP = true;
		*MEMORY_ACCESS_FINISHED = false;

#ifdef REAL_SCENARIO
				//For real scenario
					auto start = std::chrono::system_clock::now();
					execute_next_instruction();
					int rand_sleep_time = rand() % 1000;
					this_thread::sleep_for(chrono::milliseconds(rand_sleep_time));

					auto end = std::chrono::system_clock::now();
					auto time_waited = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();     
					update_burst(static_cast<float>(time_waited) / 1000);
					CALL_TRAP_IF_FINISHED();
#else
//For Emulation:

		execute_next_instruction();

		//update my burstTime
		update_burst(BURST_VALUE);
		//set finished trap and pause if process finished
		CALL_TRAP_IF_FINISHED();

#endif
		if (pause || finished) {
			//waiting to be handled by scheduler
			*PAUSED_TRAP = true;
			while (pause) {
				this_thread::yield(); //busy wait while letting other threads with same priority ahead
			}
		}

	}
}

//fake instruction handler (could add: attach to my CLI)
void process::execute_next_instruction() {

	srand(time(NULL));

	int rand_index = rand() % instructions.size();
	string command = instructions[rand_index];

	while (!MEMORY_ACCESS_FINISHED) { this_thread::yield(); }
	int result = fairShareScheduler::memory_request(command);
	while (!MEMORY_ACCESS_FINISHED) { this_thread::yield(); }

	//cout << "\tProcess: " << parent << " - ";
	
	//instructions.erase(instructions.begin());

#ifndef REAL_SCENARIO

	int sleep_time = 0;

	//sleep for increments of BURST_VALUE
	//in case where quantum < BURST_VALUE, decrement by quantum and still execute 1 instruction
	if (quantum > BURST_VALUE) {
		sleep_time = BURST_VALUE * 100;
	}
	else {
		sleep_time = quantum * 100;
	}

#ifdef RANDOM_DELAY
	if (program_counter == 2) {
		sleep_time = 500;
		cout << "\n\t\t\t unexpected delay"; //currentTime will get updated
											 //less instructions will get executed, burstTime is not updated (feature, atm)
	}
#endif

	//cout << " - Sleeping for " << sleep_time << " ms\n";
	sleep_time = rand() % 1000;
	this_thread::sleep_for(chrono::milliseconds(sleep_time));

#else
	cout << "\n";
#endif
}



void process::create_thread(float time) {
	quantum = time;
	created = true;
	thread my_thread([this] { run_process(); });
	my_handle = my_thread.native_handle();
	my_thread.detach(); //detached thread
}


void process::resume_thread(float time) {
	quantum = time;
	pause = false;
}

void process::pause_thread(){
	pause = true;
}


void process::CALL_TRAP_IF_FINISHED() {

	if (burstTime <= 0 ) {

		burstTime = 0; //set this because scheduler kills finished processes based on burstTime not on # of instructions left
		*FINISHED_TRAP = true;
		finished = true;
	}

	if (quantum <= 0) {

		*FINISHED_TRAP = true;
		pause = true;
	}
}

//decrement burstTime (could add: statistical analysis, or modify based on # of instructions left)
void process::update_burst(float time = BURST_VALUE){

	//go by increments of BURST_VALUE
	//in case where quantum < BURST_VALUE, decrement by quantum and still execute 1 instruction
	if (quantum < time) {
		burstTime -= quantum;
	}
	else {
		burstTime -= time;
	}
	quantum -= time;

}


//compute if process should be in ready queue
void process::checkReady(float time) {
	if (time >= arrivalTime && burstTime > 0) {
		ready = true;
	}
	else {
		ready = false;
	}
}