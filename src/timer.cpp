// timer.cpp
//
// This file is part of the Cooper project.
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

