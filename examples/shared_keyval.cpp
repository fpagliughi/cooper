// shared_map.cpp
//
// This is an example of a map data structure that can be shared across
// threads, implemented with an Actor.
//
// A classic way to implement this might be to pair a std::map with a mutex.
// Each thread would lock the mutex in order to access the map, and by doing
// so would insure that only one thread could access it at a time, avoiding
// race conditions. A slight improvement would be to use reader/writer
// locks, such as the std::shared_mutex in C++17. This would allow multiple
// readers to access the map simultaneously, so long as each promises not to
// change the data structure.
//
// This Actor implementation offers a number of improvements:
//  - Control. The 'shared_map' class retains full control of the data
//    structure. An errant thread can't hog a lock and starve out other
//    threads.
//  - Fairness. Threads can queue up requests which get processed in the
//    order received.
//  - 'Set' operations can run asynchronously and don't need to block the
//    calling thread.
//  - Sequential guarantee means that a 'get' operation after a 'set' will
//    always retrieve the last 'set' value, even though the set ran
//    asynchronously from the context of the setter thread.
//
// The implementation in this example uses std::optional<>, and thus
// requires C++17. It works with GCC 7 and Clang 5.0 (or later).

#include <map>
#include <optional>
#include <iostream>
#include <cassert>
#include "cooper/actor.h"

/////////////////////////////////////////////////////////////////////////////

/**
 * This is a simple actor that manages a shared key/value map.
 */
class shared_keyval : public cooper::actor
{
public:
    using string = std::string;

    template <typename T>
    using optional = std::optional<T>;

private:
    /** The data store */
    std::map<string, string> kv_;

    // ----- The server API -----

    // Set the value in the key/value store.
    void handle_set(const string& key, const string& val) {
        assert(on_actor_thread());
        kv_[key] = val;
    }

    // Get the value from the key/val store.
    // Returns the value if the key is found, otherwise it return nullopt.
    optional<string> handle_get(const string& key) {
        assert(on_actor_thread());
        auto p = kv_.find(key);
        if (p != kv_.end())
            return { p->second };

        return {};
    }

public:
    /**
     * Create an empty key/value store.
     */
    shared_keyval() {}

    // ----- The client API -----

    /**
     * Sets a value in the key/value store.
     * This is an asynchronous operation. The operation is queued, but the
     * caller is not blocked waiting for the value to be set.
     * @param key The key
     * @param val The value
     */
    void set(const string& key, const string& val) {
        cast(&shared_keyval::handle_set, this, key, val);
    }

    /**
     * Retrieves a value from the key/value store.
     *
     * @param key The key to query.
     * @return An optional value. If the key is found, the value is
     *         returned, otherwise it returns nullopt.
     */
    optional<string> get(const string& key) {
        return call(&shared_keyval::handle_get, this, key);
    }

	/**
     * Wait for all pending operations to complete.
     * As a simple "trick" you can always wait for all pending operations to
     * complete by calling an empty lambda function on the actor thread.
     * This will block the caller until the empty function completes. Since
     * all operations run to completion in the order submitted, by the time
     * this runs, it means that all previous operations have completed!
	 */
	void flush() { call([]{}); }
};


/////////////////////////////////////////////////////////////////////////////

using namespace std;

int main()
{
    shared_keyval kv;

    string k { "bubba" };
    kv.set(k, "wally");
    auto optv = kv.get(k);

    if (optv) {
        cout << "Got: " << *optv << endl;
    }
    else {
        cout << "No value for key: " << k << endl;
    }
    return 0;
}


