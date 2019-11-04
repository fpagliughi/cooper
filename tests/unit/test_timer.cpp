// test_timer.cpp
//
// Test of the timer class in the cooper library.
// This file is part of the cooper project.
//

/****************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2017, Frank Pagliughi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************/

#include "cooper/task_queue.h"
#include <catch2/catch.hpp>

#if 0
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
#endif

TEST_CASE("timer constructors", "[timer]") {
	REQUIRE(true);
}

