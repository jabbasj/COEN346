/*COEN 346 - Assignment 1

Multi-threaded QuickSort implementation
By: Jafar Abbas (26346650)
	Shuixi Li	(26327338)
*/
#include "stdafx.h"
using namespace std;

void quickSort(vector<int>::iterator start, vector<int>::iterator end);
vector<int>::iterator  myPartition(vector<int>::iterator start, vector<int>::iterator end, int pivot);
void readFileIntoVector();
void writeVectorIntoFile();

vector<int> * input = new vector<int>;
int threadCounter = -1;

/*  Reads input.txt into vector<int> input - ignores first element
	Sorts with quickSort recursively using async for each call
	Writes sorted vector into output.txt
	Prints out time it took for each step
*/
int _tmain(int argc, _TCHAR* argv[])
{
	try {
			cout << "Multi-threaded Quick Sort - Note: First element 'length' is discarded\n\n";
			cout << "Reading file: ";
			auto start = chrono::steady_clock::now();
		readFileIntoVector();
			auto end = chrono::steady_clock::now();
			cout << chrono::duration <double, milli>(end-start).count() << " ms\n";

			cout << "Sorting file: ";
			start = chrono::steady_clock::now();
		quickSort(input->begin(), input->end());
			end = chrono::steady_clock::now();
			cout << chrono::duration <double, milli>(end - start).count() << " ms - " << input->size() << " integers sorted\n";

			cout << "Writing file: ";
			start = chrono::steady_clock::now();
		writeVectorIntoFile();
			end = chrono::steady_clock::now();
			cout << chrono::duration <double, milli>(end - start).count() << " ms\n";		
	}
	catch (exception e) {
		cout << "\nError occured: " << e.what() <<endl;
	}
	cout << "Recursions called: " << threadCounter << "\n\n";

	system("pause");
	return 0;
}

//recursive quickSort function
void quickSort(vector<int>::iterator start, vector<int>::iterator end) {
	try {
		threadCounter++;
		int size = (end - start);
		if (size <= 10) {											// not enough to justify async overhead
			sort(start, end);
			return;													// done case
		}

		vector<int>::iterator pivot = start + ((end - start) / 2);	// choose midpoint as pivot
		int value_of_pivot = *pivot;
		iter_swap(pivot, start);									// move pivot to start
		pivot = myPartition(next(start), end, value_of_pivot);		// partition into [pivot| < pivot | > pivot]
		iter_swap(start, pivot);									// move pivot to appropriate spot

		
		
		async(quickSort, start, pivot).get();						// quick-sort the values less than pivot (& wait for return)
		async(quickSort, next(pivot), end).get();					// quick-sort the values larger than pivot (& wait for return)
		return;
	}
	catch (exception e) {
		throw e;
	}
}

//partition array into [pivot| <= pivot | > pivot]
vector<int>::iterator  myPartition(vector<int>::iterator start, vector<int>::iterator end, int pivot) {
	try {
		for (vector<int>::iterator i = start; i < end; i++) {
			if (*i < pivot) {
				iter_swap(i, start);
				start++;
			}
		}
		return prev(start);
	}
	catch (exception e) {
		throw e;
	}
}

void readFileIntoVector() {
	try {
		ifstream input_file("input.txt");
		string line;
		if (input_file.is_open()) {
			//discard first line because my program doesn't need to know the length in advance
			getline(input_file, line);

			while (getline(input_file, line)) {
				input->push_back(stoi(line));
			}
		}
	}
	catch (exception e) {
		throw e;
	}

}

void writeVectorIntoFile() {
	try {
		ofstream output_file("output.txt");
		if (output_file.is_open()) {
			for (int it : *input) {
				output_file << it << endl;
			}
		}
	}
	catch (exception e) {
		throw e;
	}

}