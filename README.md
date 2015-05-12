Shock HTTP Server
=================

Shock is a very simple HTTP server library designed for low memory usage, and
no dynamic allocations. The goal is to be suitable for use on embedded systems
where memory comes at a premium.

##Goals
1. **Be easy to use.** There's no point in making a library if no one can
   understand it!
1. **Be tiny.** The less memory we need, the better. Shock needs at most ~20
   bytes of HTTP data at a time (for parsing the Transfer-Encoding header.)
1. **Be as close to HTTP/1.1 as possible.** There is definitely some low
   hanging fruit (100-continue, trailers, etc.) here.

##Limitations
Being new software, shock has quite a few limitations. It is therefore not
recommended for use in a production environment without extensive testing.
Here are *some* of the known limitations:

* Does not support receiving `Transfer-Encoding: chunked` entities, although it
  does send them.
* Does not support multipart encoding.
* Many, many more...

##Compiling

Shock uses cmake for building. CMake can generate many types of build files for
all sorts of systems. I've only tested Makefiles on Ubuntu 14.04.

Here's the basic steps to build after you've cloned the repository:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make all_cppcheck
$ make test
```

Assuming everything worked correctly, you'll end up with an example `shocked`
binary under `./build/ex/`, a static library `./build/src/libshock.a`, and a
unit test executable `./build/src/test/http_test`.

If you aren't making changes to the code, you might want to disable cppcheck.
Instead of the above, use the following:

```bash
$ mkdir build
$ cd build
$ cmake .. -DCPPCHECK=OFF
$ make
```
##Usage
*There are some examples in the ex folder, expect more over time.*

##API Documentation
###Callbacks
```c
typedef void (shock_output_fn)(shock_http* self,
                               void* closure,
                               const char* data,
                               size_t length);
```
A function provided by the application that writes bytes to the connected
client, be it over `stdin`/`stdout`, sockets, or messenger pigeons.
* **self** Pointer to a `shock_http` structure.
* **closure** A pointer given during `shock_http_init`.
* **data** The data to be written, not null terminated.
* **length** The length of data.

```c
typedef void (shock_dispatch_fn)(shock_http* self,
                                 void* closure,
                                 shock_request_part state,
                                 const char* data,
                                 size_t length);
```

A function provided by the application that is called when there is data ready
to be read. This function will always be called with the complete value of a
part, meaning you'll always get `Content-Length` and never `Conten` followed by
`t-Length`. The only exception is the body, which is dispatched as it arrives.
* **self** Pointer to a `shock_http` structure.
* **closure** A pointer given during `shock_http_init`.
* **state** An enum representing what the data is, for example
  `SHOCK_REQUEST_METHOD` means the HTTP method the client is using.
* **data** Array of bytes ready to be read by the application.
* **length** Number of bytes in data.

###Init
```c
void shock_http_init(shock_http* self,
                     shock_dispatch_fn* dispatch,
                     shock_output_fn* output,
                     void* closure);
```
Initializes a `shock_http` instance. Must be called before any other function.
* **self** Pointer to a `shock_http` structure.
* **dispatch** Called when there is data ready to be read by the application.
* **output** Called when there is data ready to be written to the client.
* **closure** A pointer passed to dispatch and output.

###Fini
```c
void shock_http_fini(shock_http* self);
```
Finishes a `shock_http` instance. Should be called before disconnecting from
the client. Doesn't do much right now, but in the future it might flush
buffers, or write trailer headers.
* **self** Pointer to a `shock_http` instance.

###Recv
```c
size_t shock_http_recv(shock_http* self, const char* data, size_t length);
```
Call when there is data available from the client that needs to be processed.
Returns the number of bytes consumed. If this returns `0`, that usually means
there isn't enough data available to finish parsing the state (ex. no newline
after a header.) If the application's receive buffer is full, call
`shock_http_drop` to skip to the next state.
* **self** Pointer to a `shock_http` structure.
* **data** Array of bytes from the client, not null terminated.
* **length** Number of bytes in data.

###Send
```c
void shock_http_send(shock_http* self,
                     shock_response_part part,
                     const char* data,
                     size_t length);
```
Send something to the client, like the version of HTTP or a header value.
Do not send `Transfer-Encoding` this way, it is set automatically. When sending
the body, finish by writing with zero length, in essence
`shock_http_send(*x, SHOCK_RESPONSE_BODY, NULL, 0);`
* **self** A pointer to a `shock_http` instance.
* **part** Which part of the HTTP response is being written. For example
  `SHOCK_RESPONSE_VERSION` or `SHOCK_RESPONSE_HEADER_NAME`.
* **data** Array of bytes to send to the client, not null terminated.
* **length** Number of bytes in data.

###Drop
```c
void shock_http_drop(shock_http* self);
```
Skips dispatching the current part of the HTTP request. Useful if you don't
care about a part, or if a value is too big to fit in your buffer (you can tell
because `shock_http_recv` will return zero.)
* **self** Pointer to a `shock_http` instance.
