// test_task_queue.cpp
//
// Test of the task_queue class in the cooper library.
//
// This file is part of the "cooper" C++ actor library.
//

/****************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2019-2024, Frank Pagliughi
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

#include "cooper/task_queue.h"
#include "catch2_version.h"

using namespace std::chrono;
using namespace cooper;

TEST_CASE("task_queue constructors", "[task_queue]") {

	SECTION("default constructor") {
		task_queue<int> que;
		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.capacity() == task_queue<int>::MAX_CAPACITY);
	}

	SECTION("sized constructor") {
		constexpr size_t N = 16;
		task_queue<int> que(N);
		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.capacity() == N);
	}
}

TEST_CASE("task_queue put", "[task_queue]") {
	constexpr auto TIMEOUT = 10ms;
	constexpr size_t N = 3;

	task_queue<int> que(N);

	SECTION("put") {
		for (size_t i=1; i<=N; ++i)
			que.put(int(i));

		REQUIRE(!que.empty());
		REQUIRE(que.size() == N);
		REQUIRE(que.num_tasks() == N);
	}

	SECTION("try_put") {
		REQUIRE(que.empty());

		for (size_t i=1; i<=N; ++i)
			REQUIRE(que.try_put(int(i)));

		REQUIRE(!que.empty());
		REQUIRE(que.size() == N);
		REQUIRE(que.num_tasks() == N);

		REQUIRE(!que.try_put(int(N+1)));
	}

	SECTION("try_put_for") {
		REQUIRE(que.empty());

		for (size_t i=1; i<=N; ++i)
			REQUIRE(que.try_put_for(int(i), TIMEOUT));

		REQUIRE(!que.empty());
		REQUIRE(que.size() == N);
		REQUIRE(que.num_tasks() == N);

		REQUIRE(!que.try_put_for(int(N+1), TIMEOUT));
	}

	SECTION("try_put_until") {
		REQUIRE(que.empty());

		for (size_t i=1; i<=N; ++i)
			REQUIRE(que.try_put_until(int(i), system_clock::now() + TIMEOUT));

		REQUIRE(!que.empty());
		REQUIRE(que.size() == N);
		REQUIRE(que.num_tasks() == N);

		REQUIRE(!que.try_put_until(int(N+1), system_clock::now() + TIMEOUT));
	}
}

TEST_CASE("task_queue get", "[task_queue]") {
	constexpr auto TIMEOUT = 10ms;
	constexpr size_t N = 3;

	task_queue<int> que(N);
	for (size_t i=1; i<=N; ++i)
		que.put(int(i));

	SECTION("get") {
		REQUIRE(que.size() == N);

		for (size_t i=1; i<=N; ++i) {
			REQUIRE(que.get() == int(i));
			REQUIRE(que.size() == N-i);
		}

		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.num_tasks() == N);
	}

	SECTION("try_get") {
		int val;
		REQUIRE(que.size() == N);

		for (size_t i=1; i<=N; ++i) {
			REQUIRE(que.try_get(&val));
			REQUIRE(val == int(i));
			REQUIRE(que.size() == N-i);
		}

		REQUIRE(!que.try_get(&val));
		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.num_tasks() == N);
	}

	SECTION("try_get_for") {
		int val;
		REQUIRE(que.size() == N);

		for (size_t i=1; i<=N; ++i) {
			REQUIRE(que.try_get_for(&val, TIMEOUT));
			REQUIRE(val == int(i));
			REQUIRE(que.size() == N-i);
		}

		REQUIRE(!que.try_get_for(&val, TIMEOUT));
		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.num_tasks() == N);
	}

	SECTION("try_get_until") {
		int val;
		REQUIRE(que.size() == N);

		for (size_t i=1; i<=N; ++i) {
			REQUIRE(que.try_get_until(&val, system_clock::now() + TIMEOUT));
			REQUIRE(val == int(i));
			REQUIRE(que.size() == N-i);
		}

		REQUIRE(!que.try_get_until(&val, system_clock::now() + TIMEOUT));
		REQUIRE(que.empty());
		REQUIRE(que.size() == 0);
		REQUIRE(que.num_tasks() == N);
	}
}

