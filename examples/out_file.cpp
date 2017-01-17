// out_file.cpp
//
// This is a typical example of an actor being used to manage and serialize
// access to a shared resource - in this case an output disk file.
//

#include <fstream>
#include <cassert>
#include "cooper/actor.h"

/////////////////////////////////////////////////////////////////////////////

/**
 * This is a simple example of an actor to control access to a shared
 * resource.
 * This actor will manage an output file that
 */
class file_manager : public cooper::actor
{
	/** The output file */
	std::ofstream os_;

	// ----- The server API -----

	/**
	 * The internal "server" call to write to the file.
	 * This function should run solely in the context of the actor thread.
	 * It therefore has exclusive access to the file object, and can access
	 * it without any locks, and without worry of any contention for the
	 * object or race conditions associated with it. From the view of
	 * external clients, all writes done here execute atomically.
	 * @param s The string to write to the file. It appends a newline after
	 *  		all writes.
	 */
	void handle_write(const std::string& s) {
		assert(on_actor_thread());
		os_ << s << std::endl;
	}

public:
	/**
	 * Create a file manager to operate on the specified file.
	 * @param name The name of the file to manage.
	 */
	explicit file_manager(const std::string& name)
		: os_(name) {}

	// ----- The client API -----

	// The client methods should submit sever tasks to the actor thread
	// using the call() and cast() methods. They should *never* touch the
	// object's data directly.

	/**
	 * Asynchronous write to the file.
	 * By using the actor::cast(), this will queue up the write operation
	 * without waiting for it to execute. So, from the view of the caller,
	 * this is an asynchronous operation. The write will execute in the
	 * order received (FIFO), and occur atomically.
	 * @param s The string to write. A newline will be appended.
	 */
	void async_write(const std::string& s) {
		cast(&file_manager::handle_write, this, s);
	}
	/**
	 * Synchronous write to the file.
	 * By using the actor::call(), this queues up a write operation, and
	 * then blocks the caller until the write is complete. Since all
	 * operations are queued in FIFO order, this will block until all other
	 *
	 * @param s
	 */
	void write(const std::string& s) {
		call(&file_manager::handle_write, this, s);
	}
	/**
	 * Wait for all pending write operations to complete.
	 * As a simple "trick" you can always wait for all pending operations to
	 * complete by calling an empty lambda function on the actor thread.
	 * This will block the caller until the empty function completes. Since
	 * all operations run to completion in the order submitted, by the time
	 * this runs, it means that all previous operations have completed!
	 */
	void flush() { call([]{}); }
};

/////////////////////////////////////////////////////////////////////////////

int main()
{
	file_manager fm("fm.txt");

	// Queue a write, but don't wait for it to complete.
	fm.async_write("Hello, world!");

	// Queue a write and block until it is finished. This will always occur
	// after the previous, async_write, despite the fact that the
	// caller/client did not wait for the previous operation to complete.
	fm.write("Nice to see you.");

	return 0;
}

