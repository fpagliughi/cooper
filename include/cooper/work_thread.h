/////////////////////////////////////////////////////////////////////////////
/// @file work_thread.h
/// Implementation of the class 'work_thread'
/// @date 16-Jan-2017
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
	 * Destroys the work thread, blocking until all tasks are complete.
	 */
	~work_thread();
	/**
	 * Request that the thread quit operation.
	 */
	void quit() {
		quit_ = true;
		cast([]{});
	}
    /**
     * Joins the underlying thread, blocking until it completes all tasks.
     */
    void join() {
        thr_.join();
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
     * Gets the curent size of the task message queue.
     *
     * This is the number of jobs that are queued to run. Note, however,
     * that even when zero, there still could be a job running in the
     * thread.
     *
	 * @return The size of the task message queue.
	 */
	size_type queue_size() const {
		return que_.size();
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

/**
 * A collection of work threads that can be used like a thread pool, but
 * individual threads are accessible.
 */
class work_threads
{
	/** The collection of worker threads */
    std::vector<work_thread> thrs_;
	/** The count for the next available thread */
	mutable std::atomic<size_t> nextThr_;

public:
	/**
	 * Creates a collection of work threads with a default number of threads
	 * (one per core).
	 */
    work_threads() : work_threads(std::thread::hardware_concurrency()) {}
	/**
	 * Creates a collection of work threads with the specified number of
	 * threads.
	 *
	 * @param n The number of threads to add to the collection.
	 */
    work_threads(size_t n) : thrs_(n) {}
    /**
     * Gets the index for the next thread that can be assigned.
     * @return size_t The index for the next thread that can be assigned.
     */
	size_t next_thread_idx() const {
		return nextThr_++ % thrs_.size();
	}
    /**
     * Gets a reference to the next thread in the collection that can be
     * assigned.
     *
     * @return A reference to the next thread in the collection that can be
     * assigned.
     */
	work_thread& next_thread() { return thrs_[next_thread_idx()]; }
    /**
     * Gets a reference to the specific work thread in the collection.
     *
     * @param i Index into the collection.
     * @return A reference to the specific work thread in the collection.
     */
	work_thread& operator[](size_t i) { return thrs_[i]; }

	void flush() {
		for (auto& thr : thrs_)
			thr.flush();
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A singleton for a system-wide collection of work threads.
 *
 * By default this creates one thread per processor core.
 */
class sys_work_threads : public work_threads
{
	sys_work_threads() : work_threads() {}

public:
    /**
     * Gets a reference to the singleton collection of work threads.
     *
     * This creates the collection the first time that it's called.
     * @return A reference to the singleton collection of work threads.
     */
	static sys_work_threads& instance() {
		static sys_work_threads thrs;
		return thrs;
	}
};

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_work_thread_h

