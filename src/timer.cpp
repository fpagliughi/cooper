// timer.cpp

#include "cooper/timer.h"

using namespace std;
using namespace std::chrono;

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

void timer::thread_func()
{
	unique_guard g(lock_);
	if (initTime_.count() != 0 && initTime_.count() != interval_.count()) {
		if (cond_.wait_for(g, initTime_) == cv_status::no_timeout || quit_)
			return;
		g.unlock();
		func_();
		g.lock();
	}

	// If this was a one-shot, get out now.
	if (interval_.count() == 0)
		return;

	auto to = steady_clock::now() + interval_;

	while (!quit_) {
		if (cond_.wait_until(g, to) == cv_status::no_timeout || quit_)
			return;
		g.unlock();
		func_();
		g.lock();
		to = std::max(to+interval_, steady_clock::now());
	}
}

// --------------------------------------------------------------------------

void timer::stop()
{
	if (!thr_.joinable())
		return;

	quit_ = true;
	cond_.notify_one();
	thr_.join();
}

// --------------------------------------------------------------------------

void timer::start(const nanoseconds& initTime,
				  const nanoseconds& interval)
{
	// Cancel a running timer
	stop();

	initTime_ = initTime;
	interval_ = interval;

	quit_ = false;
	thr_ = thread(&timer::thread_func, this);
}


/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

