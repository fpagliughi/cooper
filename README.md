# cooper
A simple and novel approach to the Actor pattern in C++

The actor pattern is a means of concurrent programming in which active "actor" objects run concurrently although each runs in its own single thread of control. The actors keep their internal data hidden from the others and only communicate through messages to each other.

Languages that support the actor pattern natively, such as Elixir and Erlang, usually do so by pattern matching data messages that are passed between the actors. C++ does not support the powerful pattern matching paradims of these other languages, and therefore some efforts to bring the Actor pattern to C++ have turned rather unwieldy.

The classical C++ object-oriented means of passing messages within a single process uses functon calls to the public interface of a C++ class, with overloading to match different parameter sets.

The 'cooper' library attempts to bring that simplicity to the Actor pattern in C++, by also mixing in the Actor client/server model of the Elixir/Erlang OTP behaviour of the "GenServer". 

In this model, a C++ actor class creates a public "client" interface of standard class methods that can be called by any thread or other actor in the application. The public methods then schedule the operations to run, in sequential order, on the actor thread by queueing matched methods from the private "server" API.

Like the Elixir GenServer, the C++ clients send messages to the sever thread by using the "call()" and "cast()" operations of the actor base class. The call() performs a synchronous operation in which the caller thread/actor is blocked until the operation completes in the internal actor thread. A cast() sends an asynchronous request to the server thread.

## Example

```
class file_manager : public cooper::actor
{
    /** The output file */
    std::ofstream os_;

    // ----- The server API -----

    // The server methods run sequentially in the actor thread. When 
    // running, the server functions have exclusive access to the 
    // private data of the object without requiring any locking.

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
    // using the call() and cast() methods. They should *never* touch
    // the object's data directly.

    void async_write(const std::string& s) {
        cast(&file_manager::handle_write, this, s);
    }

    void write(const std::string& s) {
		call(&file_manager::handle_write, this, s);
    }
};
```
