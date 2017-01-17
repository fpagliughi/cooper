/////////////////////////////////////////////////////////////////////////////
/// @file actor.h
/// Implementation of the class 'actor'
/// @date 09-Jan-2017
/////////////////////////////////////////////////////////////////////////////

#ifndef __cooper_actor_h
#define __cooper_actor_h

#include "cooper/work_thread.h"

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

/**
 * Base class for actor objects.
 */
class actor
{
	/** The actor's thread */
	work_thread thr_;

protected:
	bool on_actor_thread() const {
		return std::this_thread::get_id() == thr_.get_id();
	}
	/**
	 * Blocking call to wait for a task to execute in the internal thread.
	 * This queues a task to the internal thread, waits for it execute,
	 * retrieves its result, and returns it.
	 * Note that if the task function throws an exception it will be
	 * propagated back to the caller.
	 * @param f The function object for the thread to execute
	 * @return The task's return value.
	 * @throws Any exception thrown by the task.
	 */
	template <class Func>
	typename std::result_of<Func()>::type call(Func&& f) {
		return thr_.submit(std::forward<Func>(f)).get();
	}
	/**
	 * Blocking call to wait for a task to execute in the internal thread.
	 * This queues a task to the internal thread, waits for it execute,
	 * retrieves its result, and returns it.
	 * Note that if the task function throws an exception it will be
	 * propagated back to the caller.
	 * @param f The function object for the thread to execute
	 * @param args The arguments to the function. These will be bound to the
	 *  		   function and queued for execution.
	 * @return The task's return value.
	 * @throws Any exception thrown by the task.
	 */
	template <class Func, class... Args>
	typename std::result_of<Func(Args...)>::type call(Func&& f, Args&&... args) {
		return thr_.submit(std::forward<Func>(f), std::forward<Args>(args)...).get();
	}
	/**
	 * Sends a task to run in the thread asynchronously.
	 * This is the same as @ref submit, but completely discards the return
	 * value, indicating the intention of the caller that the task is purely
	 * asynchronous.
	 * @param f The function object for the thread to execute
	 * @param args The arguments to the function. These will be bound to the
	 *  		   function and queued for execution.
	 * @throws Any exception thrown by the task.
	 */
	template <class Func>
	void cast(Func&& f) { thr_.submit(std::forward<Func>(f)); }
	/**
	 * Sends a task to run in the thread asynchronously.
	 * This is the same as @ref submit, but completely discards the return
	 * value, indicating the intention of the caller that the task is purely
	 * asynchronous.
	 * @param f The function object for the thread to execute
	 * @param args The arguments to the function. These will be bound to the
	 *  		   function and queued for execution.
	 * @throws Any exception thrown by the task.
	 */
	template <class Func, class... Args>
	void cast(Func&& f, Args&&... args) {
		thr_.submit(std::forward<Func>(f), std::forward<Args>(args)...);
	}

public:

};

/////////////////////////////////////////////////////////////////////////////

template <typename T>
struct this_actor : actor
{
	using This = T;
};

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_actor_h

