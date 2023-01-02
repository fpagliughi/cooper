/////////////////////////////////////////////////////////////////////////////
/// @file func_wrapper.h
/// Implementation of the class 'func_wrapper'
/// @date 14-Jan-2017
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

#ifndef __cooper_func_wrapper_h
#define __cooper_func_wrapper_h

#include <memory>
#include <utility>

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

/**
 * Type-erasure class for a moveable, invokable function object.
 */
class func_wrapper
{
	/** Abstract base class for objects. */
	struct base {
		virtual ~base() {}
		virtual void invoke() =0;
	};
	/** Concrete template class to wrap function objects */
	template<typename F>
	class impl_t: public base {
		F f;
	public:
		impl_t(F&& f_): f(std::move(f_)) {}
		void invoke() override { f(); }
	};

	/** Pointer to the function object */
	std::unique_ptr<base> impl_;

	// Non-copyable
	func_wrapper(const func_wrapper&) =delete;
	func_wrapper& operator=(const func_wrapper&) =delete;

public:
	func_wrapper() =default;
	/**
	 * Standard move constructor.
	 * Creates a new object by moving the other one into this object.
	 * @param other The other function wrapper object.
	 */
	func_wrapper(func_wrapper&& other) : impl_(std::move(other.impl_)) {}
	/**
	 * Generic move constructor.
	 * Creates a new object by moving another one into this one, but in this
	 * case, the other object can be any type of callable object.
	 * @param f The other object.
	 */
	template<typename F>
	func_wrapper(F&& f) : impl_(new impl_t<F>(std::move(f))) {}
	/**
	 * Copy assignment.
	 * Move the other object into this one.
	 * @param rhs The right-hand-side of the statement (other object)
	 * @return A reference to this object.
	 */
	func_wrapper& operator=(func_wrapper&& rhs) {
		impl_ = std::move(rhs.impl_);
		return *this;
	}
	/**
	 * Invoke the function object.
	 */
	void operator()() { impl_->invoke(); }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

#endif		// __cooper_func_wrapper_h

