#include "all_my_headers.h"
using namespace std;


class Page {
	friend class MemoryManager;
private:
	int val_id = -1;
	int val_value;
	long unsigned int last_access;
	long unsigned int currentTimeStamp();

public:
	Page(){
		last_access = -1;
	}

	Page(int id, int value) {
		val_id = id;
		val_value = value;
		last_access = currentTimeStamp();
	}
};

class MemoryManager {
	friend class fairShareScheduler;
	friend class Release;
	friend class Lookup;
	friend class Store;

private:
	static stack<string> myCommands;

	void initialize_main_memory();
	static int SIZE_OF_MAIN_MEMORY;
	static vector<Page> mMainMemory;

	static int fetch_page_from_memory(int val_id);
	static int fetch_page_from_virtual_memory(int val_id);
	static int load_into_main_memory(int val_id, int val_value);
	static void write_to_virtual_memory(int val_id, int val_value);

	void main_thread();
	thread * my_thread;
	thread::native_handle_type my_handle;

	bool * MEMORY_ACCESS_FINISHED;
	stack<int> * results_for_scheduler;

public:
	static void add_command(string command);
	int handle_top_command();

	MemoryManager();
	
};


class Store {
	friend class MemoryManager;

private:
	void execute(int val_id, int val_value);
};


class Release {
	friend class MemoryManager;

private:
	void execute(int val_id, int val_value);
};


class Lookup {
	friend class MemoryManager;

private:
	int execute(int val_id);
};




