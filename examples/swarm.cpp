// cooper/examples/swarm.cpp
//
// This is an example of creating a large number of actors.
//
// Copyright (c) 2023, Frank Pagliughi. All Rights Reserved.
//

#include "cooper/actor.h"
#include "cooper/work_thread.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::literals::chrono_literals;

/////////////////////////////////////////////////////////////////////////////

class swarmer : public cooper::actor
{
public:
	using ptr_t = std::unique_ptr<swarmer>;

private:
    /** An identifier for this object */
	const size_t n_;
    /** The next object which should receive alerts. */
	ptr_t next_;

public:
	swarmer(size_t n) : n_(n) {}
	swarmer(size_t n, ptr_t next) : n_(n), next_(std::move(next)) {}
	swarmer(swarmer&& other) : n_(other.n_), next_(std::move(other.next_)) {}
    ~swarmer() {
        cout << "[Shutting down " << n_ << "]" << endl;
        flush();
    }

	static ptr_t create(size_t n) {
		return std::make_unique<swarmer>(n);
	}
	static ptr_t create(size_t n, ptr_t next) {
		return std::make_unique<swarmer>(n, std::move(next));
	}

	size_t num() const { return n_; }

	void alert(const std::string& msg) {
		cast([this, msg] {
			if (next_) {
				cout << "[Alerting " << next_->num() << "]" << endl;
				next_->alert(msg);
			}
			else
				cout << msg << endl;
		});
	}

    void flush() {
        call([]{});
    }
};

// --------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	size_t n = (argc > 1) ? atoi(argv[1]) : 1024;

	auto next = swarmer::create(0);
	for (size_t i=1; i<n; ++i) {
		auto sw = swarmer::create(i, std::move(next));
		next = std::move(sw);
	}

	next->alert("Hi there!");

	cooper::sys_work_threads::instance().flush();

    next.reset();
	return 0;
}


