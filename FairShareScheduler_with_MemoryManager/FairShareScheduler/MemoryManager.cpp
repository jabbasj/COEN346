#include "all_my_headers.h"
using namespace std;


stack<string> MemoryManager::myCommands;
vector<Page> MemoryManager::mMainMemory;
int MemoryManager::SIZE_OF_MAIN_MEMORY = 0;

//resolves any variables in the command before saving it.
void MemoryManager::add_command(string command) {
	myCommands.push(command);
	return;
}

void MemoryManager::main_thread() {
	do {
		//check for commands, set result in scheduler queue
		if (myCommands.size() > 0) {
			results_for_scheduler->push(handle_top_command());
			myCommands.pop();
			*MEMORY_ACCESS_FINISHED = true;
		}

		while (fairShareScheduler::PAUSED_TRAP) {};

	} while (true);
	return;
}


MemoryManager::MemoryManager() {

	results_for_scheduler = &(fairShareScheduler::results);
	MEMORY_ACCESS_FINISHED = &(fairShareScheduler::MEMORY_ACCESS_FINISHED);

	initialize_main_memory();

	thread my_thread([this] { main_thread(); });
	my_handle = my_thread.native_handle();
	my_thread.detach(); //detached thread
}


int MemoryManager::handle_top_command() {

	string command = myCommands.top();

	if (command.find("Store") != string::npos) {
		int pos = command.find(' ');
		command = command.substr(pos + 1, -1);
		int var_id = stoi(command.substr(0, pos));
		pos = command.find(' ');
		command = command.substr(pos + 1, -1);
		int var_value = stof(command.substr(0, -1));

		(new Store())->execute(var_id, var_value);
		return 0;
	}
	else if (command.find("Lookup") != string::npos){
		int pos = command.find(' ');
		command = command.substr(pos + 1, -1);
		int var_id = stoi(command.substr(0, pos));

		int result = (new Lookup())->execute(var_id);
		return result;

	}
	else if (command.find("Release") != string::npos) {
		int pos = command.find(' ');
		command = command.substr(pos + 1, -1);
		int var_id = stoi(command.substr(0, pos));
		pos = command.find(' ');
		command = command.substr(pos + 1, -1);
		int var_value = stof(command.substr(0, -1));

		(new Release())->execute(var_id, var_value);
		return 0;

	}

	return -1;
}


void MemoryManager::initialize_main_memory() {

	ifstream input_file("memconfig.txt");

	string line;
	if (input_file.is_open()) {
		getline(input_file, line);
		SIZE_OF_MAIN_MEMORY = stoi(line);
	}
}

//returns result or -1 if not found
//fetches from virtual memory if not in main memory
int MemoryManager::fetch_page_from_memory(int val_id) {

	int result = -1;
	bool found_in_main_memory = false;

	for (int i = 0; i < SIZE_OF_MAIN_MEMORY && i < mMainMemory.size(); i++) {
		if (mMainMemory[i].val_id == val_id) {
			result = mMainMemory[i].val_value;
			found_in_main_memory = true;
			mMainMemory[i] = Page(val_id, result);
			break;
		}
	}


	if (!found_in_main_memory) {
		result = fetch_page_from_virtual_memory(val_id);
	}

	return result;
}

//returns value
int MemoryManager::fetch_page_from_virtual_memory(int val_id) {

	ifstream input_file("virtualmemory.txt");

	ofstream output_file("temp.txt");

	string line;
	bool found = false;
	int result = -1;
	if (input_file.is_open()) {
		while (getline(input_file, line) && !found) {
			int pos = line.find(' ');
			string value_id_line = line.substr(0, pos);
			int saved_val_id = stoi(value_id_line);

			if (saved_val_id == val_id) {
				found = true;
				line = line.substr(pos, -1);
				result = stoi(line);
			}
			else {
				output_file << line + "\n";
			}
		}

		input_file.close();
		output_file.close();
		// delete the original file
		remove("virtualmemory.txt");
		// rename old to new
		rename("temp.txt", "virtualmemory.txt");

		if (result != -1) {
			load_into_main_memory(val_id, result);
		}
	}


	return result;
}

//erases page from main memory
void MemoryManager::write_to_virtual_memory(int val_id, int val_value) {

	//get from virtual memory if it exists (to avoid duplicates)
	fetch_page_from_virtual_memory(val_id);

	// remove from main memory
	for (int i = 0; i < SIZE_OF_MAIN_MEMORY && i < mMainMemory.size(); i++) {
		if (mMainMemory[i].val_id == val_id) {
			mMainMemory.erase(mMainMemory.begin() + i);
			break;
		}
	}

	ifstream input_file("virtualmemory.txt");
	ofstream output_file("temp.txt");

	string line;
	if (input_file.is_open()) {
		while (getline(input_file, line)) {			
			output_file << line + "\n";
		}

		output_file << to_string(val_id) + " " + to_string(val_value) + "\n";

		input_file.close();
		output_file.close();
		// delete the original file
		remove("virtualmemory.txt");
		// rename old to new
		rename("temp.txt", "virtualmemory.txt");
	}

}


//stores into empty page or swaps oldest
int MemoryManager::load_into_main_memory(int val_id, int val_value) {


	//see if it's in main memory, just update it
	if (fetch_page_from_memory(val_id) != -1) {
		for (int i = 0; i < mMainMemory.size() && i < SIZE_OF_MAIN_MEMORY; i++) {
			if (mMainMemory[i].val_id == val_id) {
				mMainMemory[i] = Page(val_id, val_value);
				break;
			}
		}
	} 
	else if (mMainMemory.size() < SIZE_OF_MAIN_MEMORY) {
		mMainMemory.push_back(*(new Page(val_id, val_value)));
	}
	else {
		// not found and memory full, swap oldes
		Page oldest_page = mMainMemory[0];
		int old_index = 0;

		for (int i = 1; i < mMainMemory.size() && i < SIZE_OF_MAIN_MEMORY; i++) {
			if (mMainMemory[i].last_access < oldest_page.last_access) {
				oldest_page = mMainMemory[i];
				old_index = i;
			}
		}
		//save old page in virtual memoery
		write_to_virtual_memory(oldest_page.val_id, oldest_page.val_value);

		cout << "\n\nMemory Manager, SWAP - Variable: " + to_string(oldest_page.val_id) + "  with:  " + to_string(val_id) + "\n\n";
		fairShareScheduler::trace->push_back("\n\nMemory Manager, SWAP - Variable: " + to_string(oldest_page.val_id) + "  with:  " + to_string(val_id) + "\n\n");

		//create new page
		mMainMemory.push_back(*(new Page(val_id, val_value)));

	}


	return 0;
}


long unsigned int Page::currentTimeStamp() {
	return time(NULL);
}


// if variable is in virtual memory, it gets it and updates it
void Store::execute(int val_id, int val_value) {

	cout << "\tStore:  Variable: " + to_string(val_id) + " Value: " + to_string(val_value) + "\n";
	fairShareScheduler::trace->push_back("\tStore:  Variable: " + to_string(val_id) + " Value: " + to_string(val_value) + "\n");

	MemoryManager::fetch_page_from_memory(val_id);
	MemoryManager::load_into_main_memory(val_id, val_value);

}



void Release::execute(int val_id, int val_value) {

	cout << "\tRelease:  Variable: " + to_string(val_id) + "\n";
	fairShareScheduler::trace->push_back("\tRelease:  Variable: " + to_string(val_id) + "\n");

	MemoryManager::write_to_virtual_memory(val_id, val_value);

}


int Lookup::execute(int val_id) {
	int result = MemoryManager::fetch_page_from_memory(val_id);

	cout << "\tLookup:  Variable: " + to_string(val_id) + " Value: " + to_string(result) + "\n";
	fairShareScheduler::trace->push_back("\tLookup:  Variable: " + to_string(val_id) + " Value: " + to_string(result) + "\n");

	return result;
}