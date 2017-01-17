/////////////////////////////////////////////////////////////////////////////
/// @file func_wrapper.h
/// Implementation of the class 'func_wrapper'
/// @date 14-Jan-2017
/////////////////////////////////////////////////////////////////////////////

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
	/** Concretae template class to wrap function objects */
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

