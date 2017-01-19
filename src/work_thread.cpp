// work_thread.cpp

#include "cooper/work_thread.h"

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

// The constructor sets the quit flag to false before starting up the
// internal thread.

work_thread::work_thread() : quit_(false)
{
	thr_ = std::thread(&work_thread::thread_func, this);
}

// --------------------------------------------------------------------------
// The thread function. This runs in the context of the internal thread to
// process the queued tasks.

void work_thread::thread_func()
{
	while (!(quit_ && que_.empty())) {
		try {
			que_.get()();
		}
		catch (...) {}
	}
}

/////////////////////////////////////////////////////////////////////////////
// end namespace 'cooper'
}

