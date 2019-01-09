/////////////////////////////////////////////////////////////////////////////
/// @file timer.h
/// Implementation of the class 'timer'
/// @date 24-Jan-2017
/////////////////////////////////////////////////////////////////////////////

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

