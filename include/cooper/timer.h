/////////////////////////////////////////////////////////////////////////////
/// @file timer.h
/// Implementation of the class 'timer'
/// @date 24-Jan-2017
/////////////////////////////////////////////////////////////////////////////

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

#ifndef __cooper_timer_h
#define __cooper_timer_h

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

/**
 * TODO: Add class description.
 */
class timer
{
public:
	using func_type = std::function<void()>;

private:
	func_type func_;
	std::thread thr_;
	std::mutex lock_;
	std::condition_variable cond_;
	std::chrono::nanoseconds initTime_;
	std::chrono::nanoseconds interval_;
	std::atomic<bool> quit_;

	/** Simple, scope-based lock guard */
	using guard = std::lock_guard<std::mutex>;
	/** General purpose guard */
	using unique_guard = std::unique_lock<std::mutex>;

	void thread_func();

public:
	timer() : quit_{false} {}
	timer(func_type f) : func_(std::move(f)), quit_{false} {}

	virtual ~timer() { stop(); }

	void stop();

	void start(const std::chrono::nanoseconds& initTime,
			   const std::chrono::nanoseconds& interval);

	template <typename Rep, class Period>
	void start(const std::chrono::duration<Rep, Period>& interval) {
		start(std::chrono::nanoseconds(0),
			  std::chrono::nanoseconds(interval));
	}

	template <typename Rep1, class Period1, typename Rep2, class Period2>
	void start(const std::chrono::duration<Rep1, Period1>& initTime,
			   const std::chrono::duration<Rep2, Period2>& interval) {
		start(std::chrono::nanoseconds(initTime),
			  std::chrono::nanoseconds(interval));
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A one shot timer expires once.
 * The object can be reused by re-starting it when it expires.
 */
class one_shot : public timer
{
	using base = timer;

public:
	one_shot() =default;
	one_shot(func_type f) : base(f) {}

	template <typename Rep, class Period>
	void start(const std::chrono::duration<Rep, Period>& interval) {
		base::start(std::chrono::nanoseconds(interval),
					std::chrono::nanoseconds(0));
	}
};


/////////////////////////////////////////////////////////////////////////////

class periodic_timer : public timer
{
	using base = timer;

public:
	periodic_timer() =default;
	periodic_timer(func_type f) : base(f) {}

	template <typename Rep, class Period>
	void start(const std::chrono::duration<Rep, Period>& interval) {
		base::start(std::chrono::nanoseconds(interval),
					std::chrono::nanoseconds(interval));
	}
};


/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_timer_h

