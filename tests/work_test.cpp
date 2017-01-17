// work_test.cpp

#include <iostream>
#include <chrono>
#include "cooper/work_thread.h"

using namespace std;
using namespace std::chrono;

int do_something(double sec)
{
	cout << "starting something" << endl;
	this_thread::sleep_for(duration<double>(sec));
	cout << "finished something" << endl;
	return 42;
}

int main()
{
	cooper::work_thread thr;

	cout << "queuing a task" << endl;
	thr.call(do_something, 2.5);
	cout << "finished the task" << endl;
	return 0;
}

