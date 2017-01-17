// work_thread.cpp

#include "cooper/work_thread.h"

namespace cooper {

/////////////////////////////////////////////////////////////////////////////

work_thread::work_thread() : quit_(false)
{
	thr_ = std::thread(&work_thread::thread_func, this);
}


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

