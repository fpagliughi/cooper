// work_thread.cpp
//
// This file is part of the cooper project.
//

/****************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2017-2023, Frank Pagliughi
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

work_thread::~work_thread()
{
	quit();
	if (thr_.joinable())
		thr_.join();
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

