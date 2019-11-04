/////////////////////////////////////////////////////////////////////////////
/// @file task_queue.h
/// Implementation of the class 'task_queue'
/// @date 04-Nov-2019
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2019, Frank Pagliughi
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

#ifndef __cooper_task_queue_h
#define __cooper_task_queue_h

#include <thread>
#include <mutex>
#include <condition_variable>
#include <limits>
#include <deque>
#include <queue>

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

/**
 * A thread-safe queue for passing job requests.
 *
 * This is essentially a @ref thread_queue, but each item submitted is
 * assumed to be a task that needs to be handled/run by the receiving
 * thread. After the job obtained from the queue has been completed by the
 * receiving thread, it needs to call task_done() to indicate that the job
 * has completed processing.
 *
 * The queue keeps a count of outstanding jobs. Each put() increments the
 * number of outstanding tasks, but the count is not decremented when the
 * item is removed from the queue, but rather later when the thread manually
 * insicates that processing has completed with a call to task_done().
 *
 * This behavior is adapted from the Python queue.Queue.
 *
 * This is a lockinq queue with blocking operations. The get() operations
 * can always block on an empty queue, but have variations for non-blocking
 * (try_get) and bounded-time blocking (try_get_for, try_get_until).
 *
 * @par
 * The default queue has a capacity that is unbounded in the practical
 * sense, limited by the system RAM. In this mode the object will not block
 * when placing values into the queue. A capacity can bet set with the
 * construtcor or, at any time later by calling the capacity(size_type)
 * method. Using this latter method, the capacity can be set to an amount
 * smaller than the current size of the queue. In that case all put's to the
 * queue will block until the number of items are removed from the queue to
 * bring the size below the new capacity.
 *
 * @par
 * Note that the queue uses move semantics to place items into the queue and
 * remove items from the queue. This means that the type, T, of the data
 * held by the queue only needs to follow move semantics; not copy
 * semantics. In addition, this means that copies of the value will @em not
 * be left in the queue. This is especially useful when creating queues of
 * shared pointers, as the "dead" part of the queue will not hold onto a
 * reference count after the item has been removed from the queue.
 *
 * @param T The type of the items to be held in the queue.
 * @param Container The type of the underlying container to use. It must
 * support back(), front(), push_back(), pop_front().
 */
template <typename T, class Container=std::deque<T>>
class thread_queue
{
public:
	/** The underlying container type to use for the queue. */
	using container_type = Container;
	/** The type of items to be held in the queue. */
	using value_type = T;
	/** The type used to specify number of items in the container. */
	using size_type = typename Container::size_type;

	/** The maximum capacity of the queue. */
	static constexpr size_type MAX_CAPACITY = std::numeric_limits<size_type>::max();

private:
	/** Object lock */
	mutable std::mutex lock_;
	/** Condition get signaled when item added to empty queue */
	std::condition_variable notEmptyCond_;
	/** Condition gets signaled when item removed from full queue */
	std::condition_variable notFullCond_;
	/** Condition gets signaled when all tasks completed */
	std::condition_variable tasksDoneCond_;
	/** The capacity of the queue */
	size_type cap_;
	/** The number of outstanding tasks */
	size_type nTasks_;
	/** The actual STL container to hold data */
	std::queue<T,Container> que_;

	/** Simple, scope-based lock guard */
	using guard = std::lock_guard<std::mutex>;
	/** General purpose guard */
	using unique_guard = std::unique_lock<std::mutex>;

	/**
	 * Places an item into the queue.
	 * This is an unconditional insert, which assumes that the caller has locked
	 * the guard, and that there is room for at least one more item in the
	 * queue. It does the signaling for "not empty", if necessary.
	 * @param g The guard (assumed to be locked)
	 * @param val The value to add to the queue.
	 */
	void queue_item(unique_guard& g, value_type val) {
		que_.emplace(std::move(val));
		++nTask_;
		if (n == 0) {
			g.unlock();
			notEmptyCond_.notify_one();
		}
	}
	/**
	 * Removes a value from the queue.
	 * This is an unconditional remove, which assumes that the caller has locked
	 * the guard and checked that there is at least one item in the queue. It
	 * does the signaling if the queue was previously full.
	 * @param g The guard (assumed to be locked)
	 * @param n The number of items in the queue, i.e. que_.size()
	 */
	value_type deque_item(unique_guard& g, size_type n) {
		value_type val = std::move(que_.front());
		que_.pop();
		if (n == cap_) {
			g.unlock();
			notFullCond_.notify_one();
		}
		return value_type;
	}

public:
	/**
	 * Creates a task queue with the largest capacity supported by the
	 * system.
	 */
	task_queue() : cap_{MAX_CAPACITY}, nTasks_{0} {}
	/**
	 * Creats a task queue with the specified maximum capacity.
	 * @param cap The maximum number of items the queue can hold.
	 */
	explicit task_queue(size_t cap) : cap_{cap}, nTasks_{0} {}
	/**
	 * Determine if the queue is empty.
	 * @return @em true if there are no elements in the queue, @em false if
	 *  	   there are any items in the queue.
	 */
	bool empty() const {
		guard g(lock_);
		return que_.empty();
	}
	/**
	 * Gets the capacity of the queue.
	 * @return The maximum number of elements before the queue is full.
	 */
	size_type capacity() const {
		guard g(lock_);
		return cap_;
	}
	/**
	 * Sets the capacity of the queue.
	 * Note that the capacity can be set to a value smaller than the current
	 * size of the queue. In that event, all calls to put() will block until
	 * a suffucuent number of items are removed to open a slot.
	 */
	void capacity(size_type cap) {
		guard g(lock_);
		cap_ = cap;
	}
	/**
	 * Gets the number of items in the queue.
	 * @return The number of items in the queue.
	 */
	size_type size() const {
		guard g(lock_);
		return que_.size();
	}
	/**
	 * Gets the number of outstanding tasks.
	 * @return The number of outstanding tasks.
	 */
	size_type num_tasks() const {
		guard g(lock_);
		return nTask_;
	}
	/**
	 * Put an item into the queue.
	 * If the queue is full, this will block the caller until items are
	 * removed bringing the size less than the capacity.
	 * @param val The value to add to the queue.
	 */
	void put(value_type val) {
		unique_guard g(lock_);
		size_type n = que_.size();
		if (n >= cap_)
			notFullCond_.wait(g, [=]{return que_.size() < cap_;});
		queue_item(g, std::move(val));
	}
	/**
	 * Non-blocking attempt to place an item into the queue.
	 * @param val The value to add to the queue.
	 * @return @em true if the item was added to the queue, @em false if the
	 *  	   item was not added because the queue is currently full.
	 */
	bool try_put(value_type val) {
		unique_guard g(lock_);
		size_type n = que_.size();
		if (n >= cap_)
			return false;
		queue_item(g, std::move(val));
		return true;
	}
	/**
	 * Attempt to place an item in the queue with a bounded wait.
	 * This will attempt to place the value in the queue, but if it is full,
	 * it will wait up to the specified time duration before timing out.
	 * @param val The value to add to the queue.
	 * @param relTime The amount of time to wait until timing out.
	 * @return @em true if the value was added to the queue, @em false if a
	 *  	   timeout occurred.
	 */
	template <typename Rep, class Period>
	bool try_put_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(lock_);
		size_type n = que_.size();
		if (n >= cap_ && !notFullCond_.wait_for(g, relTime, [=]{return que_.size() < cap_;}))
			return false;
		queue_item(g, std::move(val));
		return true;
	}
	/**
	 * Attempt to place an item in the queue with a bounded wait to an
	 * absolute time point.
	 * This will attempt to place the value in the queue, but if it is full,
	 * it will wait up until the specified time before timing out.
	 * @param val The value to add to the queue.
	 * @param absTime The absolute time to wait to before timing out.
	 * @return @em true if the value was added to the queue, @em false if a
	 *  	   timeout occurred.
	 */
	template <class Clock, class Duration>
	bool try_put_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(lock_);
		size_type n = que_.size();
		if (n >= cap_ && !notFullCond_.wait_until(g, absTime, [=]{return que_.size() < cap_;}))
			return false;
		queue_item(g, std::move(val));
		return true;
	}
	/**
	 * Retrieve a value from the queue.
	 * If the queue is empty, this will block indefinitely until a value is
	 * added to the queue by another thread,
	 * @param val Pointer to a variable to receive the value.
	 */
	void get(value_type* val) {
		unique_guard g(lock_);
		auto n = que_.size();
		if (n == 0)
			notEmptyCond_.wait(g, [=]{return !que_.empty();});
		*val = dequeue_item(g, n);
	}
	/**
	 * Retrieve a value from the queue.
	 * If the queue is empty, this will block indefinitely until a value is
	 * added to the queue by another thread,
	 * @return The value removed from the queue
	 */
	value_type get() {
		unique_guard g(lock_);
		auto n = que_.size();
		if (n == 0)
			notEmptyCond_.wait(g, [=]{return !que_.empty();});
		return dequeue_item(g, n);
	}
	/**
	 * Attempts to remove a value from the queue without blocking.
	 * If the queue is currently empty, this will return immediately with a
	 * failure, otherwise it will get the next value and return it.
	 * @param val Pointer to a variable to receive the value.
	 * @return @em true if a value was removed from the queue, @em false if
	 *  	   the queue is empty.
	 */
	bool try_get(value_type* val) {
		unique_guard g(lock_);
		auto n = que_.size();
		if (n == 0)
			return false;
		*val = dequeue_item(g, n);
		return true;
	}
	/**
	 * Attempt to remove an item from the queue for a bounded amout of time.
	 * This will retrieve the next item from the queue. If the queue is
	 * empty, it will wait the specified amout of time for an item to arive
	 * before timing out.
	 * @param val Pointer to a variable to receive the value.
	 * @param relTime The amount of time to wait until timing out.
	 * @return @em true if the value was removed the queue, @em false if a
	 *  	   timeout occurred.
	 */
	template <typename Rep, class Period>
	bool try_get_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
		unique_guard g(lock_);
		auto n = que_.size();
		if (n == 0 && !notEmptyCond_.wait_for(g, relTime, [=]{return !que_.empty();}))
			return false;
		*val = dequeue_item(g, n);
		return true;
	}
	/**
	 * Attempt to remove an item from the queue for a bounded amout of time.
	 * This will retrieve the next item from the queue. If the queue is
	 * empty, it will wait until the specified time for an item to arive
	 * before timing out.
	 * @param val Pointer to a variable to receive the value.
	 * @param absTime The absolute time to wait to before timing out.
	 * @return @em true if the value was removed from the queue, @em false
	 *  	   if a timeout occurred.
	 */
	template <class Clock, class Duration>
	bool try_get_until(value_type* val, const std::chrono::time_point<Clock,Duration>& absTime) {
		unique_guard g(lock_);
		auto n = que_.size();
		if (n == 0 && !notEmptyCond_.wait_until(g, absTime, [=]{return !que_.empty();}))
			return false;
		*val = dequeue_item(g, n);
		return true;
	}
	/**
	 * Mark a task complete.
	 * This decrements the number of outstanding tasks. It must be called,
	 * manually by the receiver thread once for each item removed from the queue
	 * when processing of the item is complete.
	 */
	void task_done() {
		unique_guard g(lock_);
		if (nTask_ == 0)
			return;	// TODO: Return false/error? Throw?
		if (--nTask_ == 0) {
			g.unlock();
			tasksDoneCond_.notify_all();
		}
	}
	/**
	 * Waits for all tasks to complete.
	 * Note that new tasks can be added and processed by other threads while
	 * this is blocked.
	 */
	void wait() {
		unique_guard g(lock_);
		if (nTask_ != 0)
			tasksDoneCond_.wait(g, [=]{return nTask_ == 0;});
	}

	// TODO: Add a shutdown() method to prevent any new items from being
	//		added to the queue.

};

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_task_queue_h

