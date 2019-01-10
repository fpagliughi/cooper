# cooper
A simple and novel approach to the Actor pattern in C++

The actor pattern is a means of concurrent programming in which active "actor" objects run concurrently although each runs in its own single thread of control. The actors keep their internal data hidden from the others and only communicate through messages to each other.

Languages that support the actor pattern natively, such as Elixir and Erlang, usually do so by pattern matching data messages that are passed between the actors. C++ does not support the powerful pattern matching paradims of these other languages, and therefore some efforts to bring the Actor pattern to C++ have turned rather unwieldy.

The classical C++ object-oriented means of passing messages within a single process uses functon calls to the public interface of a C++ class, with overloading to match different parameter sets.

The 'cooper' library attempts to bring that simplicity to the Actor pattern in C++, by also mixing in the Actor client/server model of the Elixir/Erlang OTP behaviour of the "GenServer". 

In this model, a C++ actor class creates a public "client" interface of standard class methods that can be called by any thread or other actor in the application. The public methods then schedule the operations to run, in sequential order, on the actor thread by queueing matched methods from the private "server" API.

Like the Elixir GenServer, the C++ clients send messages to the sever thread by using the "call()" and "cast()" operations of the actor base class. The call() performs a synchronous operation in which the caller thread/actor is blocked until the operation completes in the internal actor thread. A cast() sends an asynchronous request to the server thread.

## Example

This is an example of an in-memory key/value store that can be shared between the threads of an application.

A classic approach might pair a `std::map` with a `mutex` to protect it from simultaneous access from multiple threads. Even better, a reader/writer lock would allow multiple reader threads to access it simultaneously.

This example hides the map inside an actor. It has a few advantages:

- The class maintains full control of access to the map.
- An errant thread can't lock the map indefinitely and starve out other threads.
- The internal actor doesn't need to use locks since the actor thread is the only one to ever directly access the map.
- There's a degree of fairness since requests from all the different client threads are queued sequentially, in the order received.
- Writer threads don't need to block waiting for access to the map. The _set(key,val)_ method can run asynchronously, but...
- Even though writes are asynchronous from the point of view of the client thread, there's still a deterministic outcome for the reads, since all operations are queued sequentially. A _get(key)_ will always return the last value set - there's no race condition due to the asynchronous behavior of the writes.
 

```
class shared_keyval : public cooper::actor
{
    /** The data store */
    std::map<string, string> kv_;

    // ----- The server API -----

    // Set the value in the key/value store.
    void handle_set(const string& key, const string& val) {
        assert(on_actor_thread());
        kv_[key] = val;
    }

    // Get the value from the key/val store.
    // Returns the value if found, otherwise it return nullopt.
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
     * This is an asynchronous operation.
     */
    void set(const string& key, const string& val) {
        cast(&shared_keyval::handle_set, this, key, val);
    }

    /**
     * Retrieves a value from the key/value store.
     * This blocks the caller until the value is retrieved.
     */
    optional<string> get(const string& key) {
        return call(&shared_keyval::handle_get, this, key);
    }

    /**
     * Wait for all pending operations to complete.
     */
    void flush() { call([]{}); }
};
```
The public members of the class create the API that can be called by, and also run in the context of the client thread. In this case, it is the _get(key)_ and _set(key, value)_ calls. These public functions, by convention, are not allowed to touch the member data in the class. Instead they each send a message to the internal actor thread to run a closure with a matching call signature to the public API, _handle_get(key)_ and _handle_set(key, val)_, respectively. These private methods are expected to run sequentially in the context of the actor thread for the object.

The private *handle_...()* methods are the only ones that can touch the internal data for the object. Since they are guaranteed to be run in the context of a single actor thread, they do not need to perform any locking on the data. They are free to manipulate the data however necessary without worry of thread contention or race conditions.

The Actor has two primary ways to queue closures for the internal thread, _cast()_ and _call()_. The first, _cast()_ is an asynchronous operation. It puts the closure into the queue for the internal thread, and then return immediately. It doesn't return a value.

The second way is the _call()_ operation. This queues the closure, and then blocks the client thread until the operation runs to completion inside the actor's thread. It then returns the value from the operation (i.e., the return value from the private call in the closure). So, for example, the _get(key)_ will queue up a call to _handle_get(key)_, wait for it to run, and then return the value from _handle_get(key)_ back to the client that called it originally.

And exceptions thrown by the internal actor thread will also get passed back to the client thread.

## Conventions

There are several conventions that are helpful (and possibly essential) to follow:

- The public API is callable by external code and runs in the context of the calling thread. This should be considered the "Client API".
- The public methods should **never** touch the internal data in the object (neither write nor read).
- The private methods in the class (or a subset of them) should run in the context of the actor thread. This should be considered the "Server API).
- It is helpful to match calls between the Client and Server API's, and simply have the client call the matching server method.
- Even if you don't touch the data directly, you should never do a read/modify/write operation from the client API. It wouldn't be guaranteed to run atomically from the perspective of the other clients. Rather, that should be moved into a server call and then all clients would percieve it as being atomic.
- The server methods should try to run as quickly as possible and return. Each object has a single execution context, and a blocked call will prevent any other operations from running.
- A server call should **never** block waiting for another client operation, since the blocked actor thread will not be able to run the other calls and deadlock will occur.
- Server calls that are assumed to be running in the actor thread context should probably test that that is the case - at least during the develop and debug cycles. A good idea is to have them assert that they are actually running on the correct actor thread:
<p align="center">
`assert(on_actor_thread());`
</p>

- Actors are also a good way to share resources, such as sockets, serial ports, database connections, etc.
- The _cast()_ operation is very helpful to keep a client from blocking on an opperation, but an errant client can overload an actor with a lot of asynchronous (cast) operations. Sometimes, even if a client method does not require a return value, it might be helpful to code it as a _call()_ to apply back-pressure to the client.
