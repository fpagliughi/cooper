/////////////////////////////////////////////////////////////////////////////
/// @file actor.h
/// Implementation of the class 'actor'
/// @date 09-Jan-2017
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
	/**
	 * Determines if the currently executing thread is the actor.
	 * @return @em true if the current thread is the internal actor thread,
	 *  	   @em false if not.
	 */
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

