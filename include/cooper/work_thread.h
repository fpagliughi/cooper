/////////////////////////////////////////////////////////////////////////////
/// @file work_thread.h
/// Implementation of the class 'work_thread'
/// @date 16-Jan-2017
/////////////////////////////////////////////////////////////////////////////

#ifndef __cooper_work_thread_h
#define __cooper_work_thread_h

#include <thread>
#include <future>
#include <utility>
#include <atomic>
#include <functional>
#include "cooper/thread_queue.h"
#include "cooper/func_wrapper.h"

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

/**
 * A single thread that can execute arbitrary functions sequentially.
 * The work queue acts as a task executor that can run arbitrary functions
 * in an internal thread. Each object is guaranteed to have a single thread
 * for its own, exclusive use. Each task runs to completion in the order
 * received, and thus, unlike a thread pool which might share the same
 * public API, tasks executed by the same work thread can not
 */
class work_thread
{
	/** The thread to perform the work */
	std::thread thr_;
	/** The queue of tasks to perform */
	thread_queue<func_wrapper> que_;
	/** Signal to quit the thread */
	std::atomic<bool> quit_;
	/** The function to run in the thread's context  */
	void thread_func();

public:
	/**
	 * Type for specifying size and capacity of internal thread queue.
	 */
	using size_type = thread_queue<func_wrapper>::size_type;
	/**
	 * Create a new work thread and start it running.
	 */
	work_thread();
	/**
	 * Detsroys the work thread, blocking until all tasks are complete.
	 */
	~work_thread() {
		quit();
		thr_.join();
	}
	/**
	 * Request that the thread quit operation.
	 */
	void quit() {
		quit_ = true;
		cast([]{});
	}
	/**
	 * Get the ID of the work thread.
	 * @return The ID of the work thread.
	 */
	std::thread::id get_id() const { return thr_.get_id(); }
	/**
	 * Gets the capacity of the task message queue.
	 * @return The capacity of the task message queue.
	 */
	size_type queue_capacity() const {
		return que_.capacity();
	}
	/**
	 * Sets the capacity of the task message queue.
	 * This can be used to set the maximum number of tasks that can be
	 * queued up to the worker thread at a given time. When the queue
	 * reaches capacity, the caller will block when attempting to schedule
	 * tasks for the thread. This can be useful to apply back-pressure to
	 * the callers so that the task load does not grow out of bounds.
	 * @param cap The new capacity of the task message queue.
	 */
	void queue_capacity(size_type cap) {
		que_.capacity(cap);
	}
	/**
	 * Submit a task to the thread for execution.
	 * This is an asynchronous call which queues the task to the internal
	 * thread for execution. It does not wait for the task to execute, but
	 * rather returns a future which the caller can use to wait and/or get
	 * the return value from the task, if any.
	 * @param f The function object for the thread to execute.
	 * @return A future tied to the submitted task. It can be used to wait
	 *  	   for the task to complete and retrieve its return value.
	 */
	template<typename Func>
	std::future<typename std::result_of<Func()>::type> submit(Func f) {
		using result_type = typename std::result_of<Func()>::type;
		std::packaged_task<result_type()> task(std::move(f));
		std::future<result_type> fut(task.get_future());
		que_.put(std::move(task));
		return fut;
	}
	/**
	 * Submit a task to the thread for execution.
	 * This is an asynchronous call which queues the task to the internal
	 * thread for execution. It does not wait for the task to execute, but
	 * rather returns a future which the caller can use to wait and/or get
	 * the return value from the task, if any.
	 * @param f The function object for the thread to execute.
	 * @param args The arguments to the function. These will be bound to the
	 *  		   function and queued for execution.
	 * @return A future tied to the submitted task. It can be used to wait
	 *  	   for the task to complete and retrieve its return value.
	 */
	template <class Func, class... Args>
	std::future<typename std::result_of<Func(Args...)>::type>
			submit(Func&& f, Args&&... args) {
		return submit(std::bind(std::forward<Func>(f),
								std::forward<Args>(args)...));
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
		return submit(std::forward<Func>(f)).get();
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
		return submit(std::forward<Func>(f), std::forward<Args>(args)...).get();
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
	void cast(Func&& f) { submit(std::forward<Func>(f)); }
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
		submit(std::forward<Func>(f), std::forward<Args>(args)...);
	}
	/**
	 * Wait until all the tasks queued up until now have executed.
	 * This simply queues an empty (no-op) function and blocks the caller
	 * until it has executed.
	 * @par
	 * Note that when this returns, it doesn't necessarily mean that the
	 * queue is empty, but rather that all tasks queued up before it have
	 * finished. Other client threads might have queued tasks to run after
	 * this, while this call was blocked, waiting to execute.
	 */
	void flush() { call([]{}); }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_work_thread_h
