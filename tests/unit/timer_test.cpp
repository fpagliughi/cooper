// timer_test.cpp
// Test of the timer class in the cooper library.
//

#include "cooper/timer.h"
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;

condition_variable cond;
mutex lck;

bool done = false;
int count = 0;

// --------------------------------------------------------------------------

void on_timer()
{
	cout << "Timer callback" << endl;

	unique_lock<mutex> g(lck);
	done = true;
	++count;
	cond.notify_one();
}

/////////////////////////////////////////////////////////////////////////////

class TimerProc
{
	std::mutex lck_;
	std::condition_variable cond_;
	int cnt_;

public:
	TimerProc() : cnt_(0) {}
	void operator()() {
		std::unique_lock<std::mutex> g(lck_);
		++cnt_;
	}

	void wait(int cnt) {
		std::unique_lock<std::mutex> g(lck_);
		cond_.wait(g, [=]{return cnt_ == cnt;});
	}
};

// --------------------------------------------------------------------------

int main()
{
	cout << "Creating a one_shot timer" << endl;
	{
		cooper::one_shot shot(on_timer);
		cout << "Starting the timer" << endl;
		shot.start(2s);
		cout << "Timer running" << endl;
		unique_lock<mutex> g(lck);
		cond.wait(g, []{return done;});
		cout << "Timer finished" << endl;

		cout << "\nStarting the timer again" << endl;
		done = false;
		shot.start(2s);
		cout << "Timer running" << endl;
		cond.wait(g, []{return done;});
		cout << "Timer finished" << endl;
	}
	cout << "one_shot timer destroyed" << endl;

	cout << "\nCreating a periodic timer." << endl;
	{
		cooper::timer tmr(on_timer);
		cout << "Waiting for 5 ticks" << endl;
		unique_lock<mutex> g(lck);
		count = 0;
		tmr.start(1s);
		cond.wait(g, []{return count == 5;});
	}
	cout << "periodic timer destroyed" << endl;

	return 0;
}

