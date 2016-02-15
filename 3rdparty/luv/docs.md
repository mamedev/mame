# LibUV in Lua

The [luv][] project provides access to the multi-platform support library
[libuv][] to lua code.  It was primariliy developed for the [luvit][] project as
the `uv` builtin module, but can be used in other lua environments.

### TCP Echo Server Example

Here is a small example showing a TCP echo server:

```lua
local uv = require('uv')

local server = uv.new_tcp()
server:bind("127.0.0.1", 1337)
server:listen(128, function (err)
  assert(not err, err)
  local client = uv.new_tcp()
  server:accept(client)
  client:read_start(function (err, chunk)
    assert(not err, err)
    if chunk then
      client:write(chunk)
    else
      client:shutdown()
      client:close()
    end
  end)
end)
print("TCP server listening at 127.0.0.1 port 1337")
uv.run()
```

### Methods vs Functions

As a quick note, [libuv][] is a C library and as such, there are no such things
as methods.  The [luv][] bindings allow calling the libuv functions as either
functions or methods.  For example, calling `server:bind(host, port)` is
equivalent to calling `uv.tcp_bind(server, host, port)`.  All wrapped uv types
in lua have method shortcuts where is makes sense.  Some are even renamed
shorter like the `tcp_` prefix that removed in method form.  Under the hood it's
the exact same C function.

## Table Of Contents

The rest of the docs are organized by libuv type.  There is some hierarchy as
most types are considered handles and some are considered streams.

 - [`uv_loop_t`][] — Event loop
 - [`uv_handle_t`][] — Base handle
   - [`uv_timer_t`][] — Timer handle
   - [`uv_prepare_t`][] — Prepare handle
   - [`uv_check_t`][] — Check handle
   - [`uv_idle_t`][] — Idle handle
   - [`uv_async_t`][] — Async handle
   - [`uv_poll_t`][] — Poll handle
   - [`uv_signal_t`][] — Signal handle
   - [`uv_process_t`][] — Process handle
   - [`uv_stream_t`][] — Stream handle
     - [`uv_tcp_t`][] — TCP handle
     - [`uv_pipe_t`][] — Pipe handle
     - [`uv_tty_t`][] — TTY handle
   - [`uv_udp_t`][] — UDP handle
   - [`uv_fs_event_t`][] — FS Event handle
   - [`uv_fs_poll_t`][] — FS Poll handle
 - [Filesystem operations][]
 - [DNS utility functions][]
 - [Miscellaneous utilities][]

## `uv_loop_t` — Event loop

[`uv_loop_t`]: #uv_loop_t--event-loop

The event loop is the central part of libuv’s functionality. It takes care of
polling for i/o and scheduling callbacks to be run based on different sources of
events.

In [luv][], there is an implicit uv loop for every lua state that loads the
library.  You can use this library in an multithreaded environment as long as
each thread has it's own lua state with corresponsding own uv loop.

### `uv.loop_close()`

Closes all internal loop resources. This function must only be called once the
loop has finished its execution or it will raise a UV_EBUSY error.

### `uv.run([mode])`

> optional `mode` defaults to `"default"`

This function runs the event loop. It will act differently depending on the
specified mode:

 - `"default"`: Runs the event loop until there are no more active and
   referenced handles or requests. Always returns `false`.

 - `"once"`: Poll for i/o once. Note that this function blocks if there are no
   pending callbacks. Returns `false` when done (no active handles or requests
   left), or `true` if more callbacks are expected (meaning you should run
   the event loop again sometime in the future).

 - `"nowait"`: Poll for i/o once but don’t block if there are no
   pending callbacks. Returns `false` if done (no active handles or requests
   left), or `true` if more callbacks are expected (meaning you should run
   the event loop again sometime in the future).

Luvit will implicitly call `uv.run()` after loading user code, but if you use
the `luv` bindings directly, you need to call this after registering your
initial set of event callbacks to start the event loop.

### `uv.loop_alive()`

Returns true if there are active handles or request in the loop.

### `uv.stop()`

Stop the event loop, causing `uv_run()` to end as soon as possible. This
will happen not sooner than the next loop iteration. If this function was called
before blocking for i/o, the loop won’t block for i/o on this iteration.

### `uv.backend_fd()`

Get backend file descriptor. Only kqueue, epoll and event ports are supported.

This can be used in conjunction with `uv_run("nowait")` to poll in one thread
and run the event loop’s callbacks in another.

**Note**: Embedding a kqueue fd in another kqueue pollset doesn’t work on all
platforms. It’s not an error to add the fd but it never generates events.

### `uv.backend_timeout()`

Get the poll timeout. The return value is in milliseconds, or -1 for no timeout.

### `uv.now()`

Return the current timestamp in milliseconds. The timestamp is cached at the
start of the event loop tick, see `uv.update_time()` for details and rationale.

The timestamp increases monotonically from some arbitrary point in time. Don’t
make assumptions about the starting point, you will only get disappointed.

**Note**: Use `uv.hrtime()` if you need sub-millisecond granularity.

### `uv.update_time()`

Update the event loop’s concept of “now”. Libuv caches the current time at the
start of the event loop tick in order to reduce the number of time-related
system calls.

You won’t normally need to call this function unless you have callbacks that
block the event loop for longer periods of time, where “longer” is somewhat
subjective but probably on the order of a millisecond or more.

### `uv.walk(callback)`

Walk the list of handles: `callback` will be executed with the handle.

```lua
-- Example usage of uv.walk to close all handles that aren't already closing.
uv.walk(function (handle)
  if not handle:is_closing() then
    handle:close()
  end
end)
```

## `uv_handle_t` — Base handle

[`uv_handle_t`]: #uv_handle_t--base-handle

`uv_handle_t` is the base type for all libuv handle types.

Structures are aligned so that any libuv handle can be cast to `uv_handle_t`.
All API functions defined here work with any handle type.

### `uv.is_active(handle)`

> method form `handle:is_active()`

Returns `true` if the handle is active, `false` if it’s inactive. What “active”
means depends on the type of handle:

 - A [`uv_async_t`][] handle is always active and cannot be deactivated, except
   by closing it with uv_close().

 - A [`uv_pipe_t`][], [`uv_tcp_t`][], [`uv_udp_t`][], etc. handlebasically any
   handle that deals with i/ois active when it is doing something that
   involves i/o, like reading, writing, connecting, accepting new connections,
   etc.

 - A [`uv_check_t`][], [`uv_idle_t`][], [`uv_timer_t`][], etc. handle is active
   when it has been started with a call to `uv.check_start()`,
   `uv.idle_start()`, etc.

Rule of thumb: if a handle of type `uv_foo_t` has a `uv.foo_start()` function,
then it’s active from the moment that function is called. Likewise,
`uv.foo_stop()` deactivates the handle again.

### `uv.is_closing(handle)`

> method form `handle:is_closing()`

Returns `true` if the handle is closing or closed, `false` otherwise.

**Note**: This function should only be used between the initialization of the
handle and the arrival of the close callback.

### `uv.close(handle, callback)`

> method form `handle:close(callback)`

Request handle to be closed. `callback` will be called asynchronously after this
call. This MUST be called on each handle before memory is released.

Handles that wrap file descriptors are closed immediately but `callback` will
still be deferred to the next iteration of the event loop. It gives you a chance
to free up any resources associated with the handle.

In-progress requests, like `uv_connect_t` or `uv_write_t`, are cancelled and
have their callbacks called asynchronously with `status=UV_ECANCELED`.

### `uv.ref(handle)`

> method form `handle:ref()`

Reference the given handle. References are idempotent, that is, if a handle is
already referenced calling this function again will have no effect.

See [Reference counting][].

### `uv.unref(handle)`

> method form `handle:unref()`

Un-reference the given handle. References are idempotent, that is, if a handle
is not referenced calling this function again will have no effect.

See [Reference counting][].

### `uv.has_ref(handle)`

> method form `handle:has_ref()`

Returns `true` if the handle referenced, `false` otherwise.

See [Reference counting][].

### `uv.send_buffer_size(handle, [size]) -> size`

> method form `handle:send_buffer_size(size)`

Gets or sets the size of the send buffer that the operating system uses for the
socket.

If `size` is omitted, it will return the current send buffer size, otherwise it
will use `size` to set the new send buffer size.

This function works for TCP, pipe and UDP handles on Unix and for TCP and UDP
handles on Windows.

**Note**: Linux will set double the size and return double the size of the
original set value.

### `uv.recv_buffer_size(handle, [size])`

> method form `handle:recv_buffer_size(size)`

Gets or sets the size of the receive buffer that the operating system uses for
the socket.

If `size` is omitted, it will return the current receive buffer size, otherwise
it will use `size` to set the new receive buffer size.

This function works for TCP, pipe and UDP handles on Unix and for TCP and UDP
handles on Windows.

**Note**: Linux will set double the size and return double the size of the
original set value.

### `uv.fileno(handle)`

> method form `handle:fileno()`

Gets the platform dependent file descriptor equivalent.

The following handles are supported: TCP, pipes, TTY, UDP and poll. Passing any
other handle type will fail with UV_EINVAL.

If a handle doesn’t have an attached file descriptor yet or the handle itself
has been closed, this function will return UV_EBADF.

**Warning**: Be very careful when using this function. libuv assumes it’s in
control of the file descriptor so any change to it may lead to malfunction.

## Reference counting

[reference counting]: #reference-counting

The libuv event loop (if run in the default mode) will run until there are no
active and referenced handles left. The user can force the loop to exit early by
unreferencing handles which are active, for example by calling `uv.unref()`
after calling `uv.timer_start()`.

A handle can be referenced or unreferenced, the refcounting scheme doesn’t use a
counter, so both operations are idempotent.

All handles are referenced when active by default, see `uv.is_active()` for a
more detailed explanation on what being active involves.

## `uv_timer_t` — Timer handle

[`uv_timer_t`]: #uv_timer_t--timer-handle

Timer handles are used to schedule callbacks to be called in the future.

### `uv.new_timer() -> timer`

Creates and initializes a new `uv_timer_t`. Returns the lua userdata wrapping
it.

```lua
-- Creating a simple setTimeout wrapper
local function setTimeout(timeout, callback)
  local timer = uv.new_timer()
  timer:start(timeout, 0, function ()
    timer:stop()
    timer:close()
    callback()
  end)
  return timer
end

-- Creating a simple setInterval wrapper
local function setInterval(interval, callback)
  local timer = uv.new_timer()
  timer:start(interval, interval, function ()
    timer:stop()
    timer:close()
    callback()
  end)
  return timer
end

-- And clearInterval
local function clearInterval(timer)
  timer:stop()
  timer:close()
end
```

### `uv.timer_start(timer, timeout, repeat, callback)`

> method form `timer:start(timeout, repeat, callback)`

Start the timer. `timeout` and `repeat` are in milliseconds.

If `timeout` is zero, the callback fires on the next event loop iteration. If
`repeat` is non-zero, the callback fires first after timeout milliseconds and
then repeatedly after repeat milliseconds.

### `uv.timer_stop(timer)`

> method form `timer:stop()`

Stop the timer, the callback will not be called anymore.

### `uv.timer_again(timer)`

> method form `timer:again()`

Stop the timer, and if it is repeating restart it using the repeat value as the
timeout. If the timer has never been started before it raises `EINVAL`.

### `uv.timer_set_repeat(timer, repeat)`

> method form `timer:set_repeat(repeat)`

Set the repeat value in milliseconds.

**Note**: If the repeat value is set from a timer callback it does not
immediately take effect. If the timer was non-repeating before, it will   have
been stopped. If it was repeating, then the old repeat value will   have been
used to schedule the next timeout.

### `uv.timer_get_repeat(timer) -> repeat`

> method form `timer:get_repeat() -> repeat`

Get the timer repeat value.

## `uv_prepare_t` — Prepare handle

[`uv_prepare_t`]: #uv_prepare_t--prepare-handle

Prepare handles will run the given callback once per loop iteration, right before polling for i/o.

```lua
local prepare = uv.new_prepare()
prepare:start(function()
  print("Before I/O polling")
end)
```

### `uv.new_prepare() -> prepare`

Creates and initializes a new `uv_prepare_t`. Returns the lua userdata wrapping
it.

### `uv.prepare_start(prepare, callback)`

> method form `prepare:start(callback)`

Start the handle with the given callback.

### `uv.prepare_stop(prepare)`

> method form `prepare:stop()`

Stop the handle, the callback will no longer be called.

## `uv_check_t` — Check handle

[`uv_check_t`]: #uv_check_t--check-handle

Check handles will run the given callback once per loop iteration, right after
polling for i/o.

```lua
local check = uv.new_check()
check:start(function()
  print("After I/O polling")
end)
```

### `uv.new_check() -> check`

Creates and initializes a new `uv_check_t`. Returns the lua userdata wrapping
it.

### `uv.check_start(check, callback)`

> method form `check:start(callback)`

Start the handle with the given callback.

### `uv.check_stop(check)`

> method form `check:stop()`

Stop the handle, the callback will no longer be called.

## `uv_idle_t` — Idle handle

[`uv_idle_t`]: #uv_idle_t--idle-handle

Idle handles will run the given callback once per loop iteration, right before
the [`uv_prepare_t`][] handles.

**Note**: The notable difference with prepare handles is that when there are
active idle handles, the loop will perform a zero timeout poll instead of
blocking for i/o.

**Warning**: Despite the name, idle handles will get their callbacks called on
every loop iteration, not when the loop is actually “idle”.

```lua
local idle = uv.new_idle()
idle:start(function()
  print("Before I/O polling, no blocking")
end)
```
### `uv.new_idle() -> idle`

Creates and initializes a new `uv_idle_t`. Returns the lua userdata wrapping
it.

### `uv.idle_start(idle, callback)`

> method form `idle:start(callback)`

Start the handle with the given callback.

### `uv.idle_stop(check)`

> method form `idle:stop()`

Stop the handle, the callback will no longer be called.

## `uv_async_t` — Async handle

[`uv_async_t`]: #uv_async_t--async-handle

Async handles allow the user to “wakeup” the event loop and get a callback
called from another thread.

```lua
local async
async = uv.new_async(function()
  print("async operation ran")
  async:close()
end)

async:send()
```

### `uv.new_async(callback) -> async`

Creates and initializes a new `uv_async_t`. Returns the lua userdata wrapping
it. A NULL callback is allowed.

**Note**: Unlike other handle initialization functions, it immediately starts
the handle.

### `uv.async_send(async)`

> method form `async:send()`

Wakeup the event loop and call the async handle’s callback.

**Note**: It’s safe to call this function from any thread. The callback will be
called on the loop thread.

**Warning**: libuv will coalesce calls to `uv.async_send(async)`, that is, not
every call to it will yield an execution of the callback, the only guarantee is
that it will be called at least once. Thus, calling this function may not
wakeup the event loop if it was already called previously within a short period
of time.

## `uv_poll_t` — Poll handle

[`uv_poll_t`]: #uv_poll_t--poll-handle

Poll handles are used to watch file descriptors for readability and writability,
similar to the purpose of [poll(2)](http://linux.die.net/man/2/poll).

The purpose of poll handles is to enable integrating external libraries that
rely on the event loop to signal it about the socket status changes, like c-ares
or libssh2. Using `uv_poll_t` for any other purpose is not recommended;
`uv_tcp_t`, `uv_udp_t`, etc. provide an implementation that is faster and more
scalable than what can be achieved with `uv_poll_t`, especially on Windows.

It is possible that poll handles occasionally signal that a file descriptor is
readable or writable even when it isn’t. The user should therefore always be
prepared to handle EAGAIN or equivalent when it attempts to read from or write
to the fd.

It is not okay to have multiple active poll handles for the same socket, this
can cause libuv to busyloop or otherwise malfunction.

The user should not close a file descriptor while it is being polled by an
active poll handle. This can cause the handle to report an error, but it might
also start polling another socket. However the fd can be safely closed
immediately after a call to `uv.poll_stop()` or `uv.close()`.

**Note** On windows only sockets can be polled with poll handles. On Unix any
file descriptor that would be accepted by poll(2) can be used.


### `uv.new_poll(fd) -> poll`

Initialize the handle using a file descriptor.

The file descriptor is set to non-blocking mode.

### `uv.new_socket_poll(fd) -> poll`

Initialize the handle using a socket descriptor. On Unix this is identical to
`uv.poll_init()`. On windows it takes a SOCKET handle.

The socket is set to non-blocking mode.

### `uv.poll_start(poll, events, callback)`

> method form `poll:start()`

Starts polling the file descriptor. `events` is `"r"`, `"w"`, or `"rw"` and
translates to a bitmask made up of UV_READABLE and UV_WRITABLE. As soon as an
event is detected the callback will be called with status set to 0, and the
detected events set on the events field.

The user should not close the socket while the handle is active. If the user
does that anyway, the callback may be called reporting an error status, but this
is not guaranteed.

**Note** Calling `uv.poll_start()`` on a handle that is already active is fine.
Doing so will update the events mask that is being watched for.

## `uv.poll_stop(poll)`

> method form `poll:stop()`

Stop polling the file descriptor, the callback will no longer be called.

## `uv_signal_t` — Signal handle

[`uv_signal_t`]: #uv_signal_t--signal-handle

Signal handles implement Unix style signal handling on a per-event loop bases.

Reception of some signals is emulated on Windows:
* SIGINT is normally delivered when the user presses CTRL+C. However, like on
Unix, it is not generated when terminal raw mode is enabled.
* SIGBREAK is delivered when the user pressed CTRL + BREAK.
* SIGHUP is generated when the user closes the console window. On SIGHUP the
program is given approximately 10 seconds to perform cleanup. After that
Windows will unconditionally terminate it.
* SIGWINCH is raised whenever libuv detects that the console has been resized.
SIGWINCH is emulated by libuv when the program uses a uv_tty_t handle to write
to the console. SIGWINCH may not always be delivered in a timely manner; libuv
will only detect size changes when the cursor is being moved. When a readable
[`uv_tty_t`][] handle is used in raw mode, resizing the console buffer will
also trigger a SIGWINCH signal.

Watchers for other signals can be successfully created, but these signals are
never received. These signals are: SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGTERM
and SIGKILL.

Calls to raise() or abort() to programmatically raise a signal are not detected
by libuv; these will not trigger a signal watcher.

**Note**: On Linux SIGRT0 and SIGRT1 (signals 32 and 33) are used by the NPTL
pthreads library to manage threads. Installing watchers for those signals will
lead to unpredictable behavior and is strongly discouraged. Future versions of
libuv may simply reject them.

```lua
-- Create a new signal handler
local sigint = uv.new_signal()
-- Define a handler function
uv.signal_start(sigint, "sigint", function(signal)
print("got " .. signal .. ", shutting down")
os.exit(1)
end)
```

### `uv.new_signal() -> signal`

Creates and initializes a new `uv_signal_t`. Returns the lua userdata wrapping
it.

### `uv.signal_start(signal, signum, callback)`

> method form `signal:start(signum, callback)`

Start the handle with the given callback, watching for the given signal.

### `uv.signal_stop(signal)`

> method form `signal:stop()`

Stop the handle, the callback will no longer be called.

## `uv_process_t` — Process handle

[`uv_process_t`]: #uv_process_t--process-handle

Process handles will spawn a new process and allow the user to control it and
establish communication channels with it using streams.

### `uv.disable_stdio_inheritance()`

Disables inheritance for file descriptors / handles that this process inherited
from its parent. The effect is that child processes spawned by this process
don’t accidentally inherit these handles.

It is recommended to call this function as early in your program as possible,
before the inherited file descriptors can be closed or duplicated.

**Note** This function works on a best-effort basis: there is no guarantee that
libuv can discover all file descriptors that were inherited. In general it does
a better job on Windows than it does on Unix.

### `uv.spawn(file, options, onexit) -> process, pid`

Initializes the process handle and starts the process. If the process is
successfully spawned, this function will return the handle and pid of the child
process.

Possible reasons for failing to spawn would include (but not be limited to) the
file to execute not existing, not having permissions to use the setuid or setgid
specified, or not having enough memory to allocate for the new process.


```lua
local stdout = uv.new_pipe(false)
local stderr = uv.new_pipe(false)
local stdin = uv.new_pipe(false)

local handle, pid

local function onexit(code, signal)
  p("exit", {code=code,signal=signal})
end

local function onclose()
  p("close")
end

local function onread(err, chunk)
  assert(not err, err)
  if (chunk) then
    p("data", {data=chunk})
  else
    p("end")
  end
end

local function onshutdown()
  uv.close(handle, onclose)
end

handle, pid = uv.spawn("cat", {
  stdio = {stdin, stdout, stderr}
}, onexit)

p{
  handle=handle,
  pid=pid
}

uv.read_start(stdout, onread)
uv.read_start(stderr, onread)
uv.write(stdin, "Hello World")
uv.shutdown(stdin, onshutdown)
```

 - `options.args` - Command line arguments as a list of string. The first string
   should be the path to the program. On Windows this uses CreateProcess which
   concatenates the arguments into a string this can cause some strange errors.
   (See `options.verbatim` below for Windows.)
 - `options.stdio` - Set the file descriptors that will be made available to the
   child process. The convention is that the first entries are stdin, stdout,
   and stderr. (**Note** On Windows file descriptors after the third are
   available to the child process only if the child processes uses the MSVCRT
   runtime.)
 - `options.env` - Set environment variables for the new process.
 - `options.cwd` - Set current working directory for the subprocess.
 - `options.uid` - Set the child process' user id.
 - `options.gid` - Set the child process' group id.
 - `options.verbatim` - If true, do not wrap any arguments in quotes, or perform
   any other escaping, when converting the argument list into a command line
   string. This option is only meaningful on Windows systems. On Unix it is
   silently ignored.
 - `options.detached` - If true, spawn the child process in a detached state -
   this will make it a process group leader, and will effectively enable the
   child to keep running after the parent exits. Note that the child process
   will still keep the parent's event loop alive unless the parent process calls
   `uv.unref()` on the child's process handle.
 - `options.hide` - If true, hide the subprocess console window that would
   normally be created. This option is only meaningful on Windows systems. On
   Unix it is silently ignored.

The `options.stdio` entries can take many shapes.

- If they are numbers, then the child process inherits that same zero-indexed fd
  from the parent process.
- If `uv_stream_h` handles are passed in, those are used as a read-write pipe or
  inherited stream depending if the stream has a valid fd.
- Including `nil` placeholders means to ignore that fd in the child.

When the child process exits, the `onexit` callback will be called with exit
code and signal.

### `uv.process_kill(process, sigmun)`

> method form `process:kill(sigmun)`

Sends the specified signal to the given process handle.

### `uv.kill(pid, sigmun)`

Sends the specified signal to the given PID.

## `uv_stream_t` — Stream handle

[`uv_stream_t`]: #uv_stream_t--stream-handle

Stream handles provide an abstraction of a duplex communication channel.
[`uv_stream_t`][] is an abstract type, libuv provides 3 stream implementations in
the form of [`uv_tcp_t`][], [`uv_pipe_t`][] and [`uv_tty_t`][].

### `uv.shutdown(stream, [callback]) -> req`

> (method form `stream:shutdown([callback]) -> req`)

Shutdown the outgoing (write) side of a duplex stream. It waits for pending
write requests to complete. The callback is called after
shutdown is complete.

### `uv.listen(stream, backlog, callback)`

> (method form `stream:listen(backlog, callback)`)

Start listening for incoming connections. `backlog` indicates the number of
connections the kernel might queue, same as `listen(2)`. When a new incoming
connection is received the callback is called.

### `uv.accept(stream, client_stream)`

> (method form `stream:accept(client_stream)`)

This call is used in conjunction with `uv.listen()` to accept incoming
connections. Call this function after receiving a callback to accept the
connection.

When the connection callback is called it is guaranteed that this function
will complete successfully the first time. If you attempt to use it more than
once, it may fail. It is suggested to only call this function once per
connection call.

```lua
server:listen(128, function (err)
  local client = uv.new_tcp()
  server:accept(client)
end)
```

### `uv.read_start(stream, callback)`

> (method form `stream:read_start(callback)`)

Callback is of the form `(err, data)`.

Read data from an incoming stream. The callback will be made several times until
there is no more data to read or `uv.read_stop()` is called. When we’ve reached
EOF, `data` will be `nil`.

```lua
stream:read_start(function (err, chunk)
  if err then
    -- handle read error
  elseif chunk then
    -- handle data
  else
    -- handle disconnect
  end
end)
```

### `uv.read_stop(stream)`

> (method form `stream:read_stop()`)

Stop reading data from the stream. The read callback will no longer be called.

### `uv.write(stream, data, [callback])`

> (method form `stream:write(data, [callback])`)

Write data to stream.

`data` can either be a lua string or a table of strings.  If a table is passed
in, the C backend will use writev to send all strings in a single system call.

The optional `callback` is for knowing when the write is
complete.

### `uv.write2(stream, data, send_handle, callback)`

> (method form `stream:write2(data, send_handle, callback)`)

Extended write function for sending handles over a pipe. The pipe must be
initialized with ip option to `true`.

**Note: `send_handle` must be a TCP socket or pipe, which is a server or a
connection (listening or connected state). Bound sockets or pipes will be
assumed to be servers.

### `uv.try_write(stream, data)`

> (method form `stream:try_write(data)`)

Same as `uv.write()`, but won’t queue a write request if it can’t be completed
immediately.

Will return number of bytes written (can be less than the supplied buffer size).

### `uv.is_readable(stream)`

> (method form `stream:is_readable()`)

Returns `true` if the stream is readable, `false` otherwise.

### `uv.is_writable(stream)`

> (method form `stream:is_writable()`)

Returns `true` if the stream is writable, `false` otherwise.

### `uv.stream_set_blocking(stream, blocking)`

> (method form `stream:set_blocking(blocking)`)

Enable or disable blocking mode for a stream.

When blocking mode is enabled all writes complete synchronously. The interface
remains unchanged otherwise, e.g. completion or failure of the operation will
still be reported through a callback which is made asynchronously.

**Warning**: Relying too much on this API is not recommended. It is likely to
change significantly in the future. Currently this only works on Windows and
only for uv_pipe_t handles. Also libuv currently makes no ordering guarantee
when the blocking mode is changed after write requests have already been
submitted. Therefore it is recommended to set the blocking mode immediately
after opening or creating the stream.

## `uv_tcp_t` — TCP handle

[`uv_tcp_t`]: #uv_tcp_t--tcp-handle

TCP handles are used to represent both TCP streams and servers.

`uv_tcp_t` is a ‘subclass’ of [`uv_stream_t`][](#uv_stream_t--stream-handle).

### `uv.new_tcp() -> tcp`

Creates and initializes a new `uv_tcp_t`. Returns the lua userdata wrapping it.

### `uv.tcp_open(tcp, sock)`

> (method form `tcp:open(sock)`)

Open an existing file descriptor or SOCKET as a TCP handle.

**Note: The user is responsible for setting the file descriptor in non-blocking
mode.

### `uv.tcp_nodelay(tcp, enable)`

> (method form `tcp:nodelay(enable)`)

Enable / disable Nagle’s algorithm.

### `uv.tcp_keepalive(tcp, enable, [delay])`

> (method form `tcp:keepalive(enable, [delay])`)

Enable / disable TCP keep-alive. `delay` is the initial delay in seconds, ignored
when enable is `false`.

### `uv.tcp_simultaneous_accepts(tcp, enable)`

> (method form `tcp:simultaneous_accepts(enable)`)

Enable / disable simultaneous asynchronous accept requests that are queued by
the operating system when listening for new TCP connections.

This setting is used to tune a TCP server for the desired performance. Having
simultaneous accepts can significantly improve the rate of accepting connections
(which is why it is enabled by default) but may lead to uneven load distribution
in multi-process setups.

### `uv.tcp_bind(tcp, address, port)`

> (method form `tcp:bind(address, port)`)

Bind the handle to an address and port. `address` should be an IP address and
not a domain name.

When the port is already taken, you can expect to see an UV_EADDRINUSE error
from either `uv.tcp_bind()`, `uv.listen()` or `uv.tcp_connect()`. That is, a
successful call to this function does not guarantee that the call to `uv.listen()`
or `uv.tcp_connect()` will succeed as well.

Use a port of `0` to let the OS assign an ephemeral port.  You can look it up
later using `uv.tcp_getsockname()`.

### `uv.tcp_getsockname(tcp)`

> (method form `tcp:getsockname()`)

Get the current address to which the handle is bound.

### `uv.tcp_getpeername(tcp)`

> (method form `tcp:getpeername()`)

Get the address of the peer connected to the handle.

### `uv.tcp_connect(tcp, address, port, callback) -> req`

> (method form `tcp:connect(host, port, callback) -> req`)

### `uv.tcp_write_queue_size(tcp) -> size`

> (method form `tcp:write_queue_size() -> size`)

Establish an IPv4 or IPv6 TCP connection.

The callback is made when the connection has been established or when a
connection error happened.

```lua
local client = uv.new_tcp()
client:connect("127.0.0.1", 8080, function (err)
  -- check error and carry on.
end)
```

## `uv_pipe_t` — Pipe handle

[`uv_pipe_t`]: #uv_pipe_t--pipe-handle

Pipe handles provide an abstraction over local domain sockets on Unix and named
pipes on Windows.

```lua
local pipe = uv.new_pipe(false)

pipe:bind('/tmp/sock.test')

pipe:listen(128, function()
  local client = uv.new_pipe(false)
  pipe:accept(client)
  client:write("hello!\n")
  client:close()
end)
```

### `uv.new_pipe(ipc) -> pipe`

Creates and initializes a new `uv_pipe_t`. Returns the lua userdata wrapping
it. The `ipc` argument is a boolean to indicate if this pipe will be used for
handle passing between processes.

### `uv.pipe_open(file) -> pipe`

Open an existing file descriptor or [`uv_handle_t`][] as a pipe.

**Note**: The file descriptor is set to non-blocking mode.

### `uv.pipe_bind(pipe, name)`

> (method form `pipe:bind(name)`)

Bind the pipe to a file path (Unix) or a name (Windows).

**Note**: Paths on Unix get truncated to sizeof(sockaddr_un.sun_path) bytes,
typically between 92 and 108 bytes.

### `uv.pipe_connect(pipe, name, callback)`

> (method form `pipe:connect(name, callback)`)

Connect to the Unix domain socket or the named pipe.

**Note**: Paths on Unix get truncated to sizeof(sockaddr_un.sun_path) bytes,
typically between 92 and 108 bytes.

### `uv.pipe_getsockname(pipe)`

> (method form `pipe:getsockname()`)

Returns the name of the Unix domain socket or the named pipe.

### `uv.pipe_pending_instances(pipe, count)`

> (method form `pipe:pending_instances(count)`)

Set the number of pending pipe instance handles when the pipe server is waiting for connections.

**Note**: This setting applies to Windows only.

### `uv.pipe_pending_count(pipe)`

> (method form `pipe:pending_count()`)

Returns the pending pipe count for the named pipe.

### `uv.pipe_pending_type(pipe)`

> (method form `pipe:pending_type()`)

Used to receive handles over IPC pipes.

First - call [`uv.pipe_pending_count`][], if it’s > 0 then initialize a handle
of the given type, returned by [`uv.pipe_pending_type`][] and call
[`uv.accept(pipe, handle)`][].

## `uv_tty_t` — TTY handle

[`uv_tty_t`]: #uv_tty_t--tty-handle

TTY handles represent a stream for the console.

```lua
-- Simple echo program
local stdin = uv.new_tty(0, true)
local stdout = uv.new_tty(1, false)

stdin:read_start(function (err, data)
  assert(not err, err)
  if data then
    stdout:write(data)
  else
    stdin:close()
    stdout:close()
  end
end)
```

### uv.new_tty(fd, readable) -> tty

Initialize a new TTY stream with the given file descriptor. Usually the file
descriptor will be:

 - 0 - stdin
 - 1 - stdout
 - 2 - stderr

`readable, specifies if you plan on calling uv_read_start() with this stream.
`stdin is readable, stdout is not.

On Unix this function will try to open /dev/tty and use it if the passed file
descriptor refers to a TTY. This lets libuv put the tty in non-blocking mode
without affecting other processes that share the tty.

Note: If opening `/dev/tty` fails, libuv falls back to blocking writes for
non-readable TTY streams.

### uv.tty_set_mode(mode)

> (method form `tty:set_mode(mode)`)

Set the TTY using the specified terminal mode.

Parameter `mode` is a C enum with the following values:

- 0 - UV_TTY_MODE_NORMAL: Initial/normal terminal mode

- 1 - UV_TTY_MODE_RAW: Raw input mode (On Windows, ENABLE_WINDOW_INPUT is
  also enabled)

- 2 - UV_TTY_MODE_IO: Binary-safe I/O mode for IPC (Unix-only)

## uv.tty_reset_mode()

To be called when the program exits. Resets TTY settings to default values for
the next process to take over.

This function is async signal-safe on Unix platforms but can fail with error
code UV_EBUSY if you call it when execution is inside uv_tty_set_mode().

## uv.tty_get_winsize() -> w, h

> (method form `tty:get_winsize() -> w, h`)

Gets the current Window size.

## `uv_udp_t` — UDP handle

[`uv_udp_t`]: #uv_udp_t--udp-handle

UDP handles encapsulate UDP communication for both clients and servers.

### uv.new_udp() -> udp

Initialize a new UDP handle. The actual socket is created lazily.

### uv.udp_open(udp, fd)

> (method form `udp:open(fd)`)

Opens an existing file descriptor or Windows SOCKET as a UDP handle.

Unix only: The only requirement of the sock argument is that it follows the
datagram contract (works in unconnected mode, supports sendmsg()/recvmsg(),
etc). In other words, other datagram-type sockets like raw sockets or netlink
sockets can also be passed to this function.

The file descriptor is set to non-blocking mode.

Note: The passed file descriptor or SOCKET is not checked for its type, but
it’s required that it represents a valid datagram socket.

### uv.udp_bind(udp, host, port)

> (method form `udp:bind(host, port)`)

Bind the UDP handle to an IP address and port.

### uv.udp_getsockname(udp)

> (method form `udp:getsockname()`)

Get the local IP and port of the UDP handle.

### uv.udp_set_membership(udp, multicast_addr, interface_addr, membership)

> (method form `udp:set_membership(multicast_addr, interface_addr, membership)`)

Set membership for a multicast address.

`multicast_addr` is multicast address to set membership for.

`interface_addr` is interface address.

`membership` can be the string `"leave"` or `"join"`.

### uv.udp_set_multicast_loop(udp, on)

> (method form `udp:set_multicast_loop(on)`)

Set IP multicast loop flag. Makes multicast packets loop back to local
sockets.

`on` is a boolean.

### uv.udp_set_multicast_ttl(udp, tty)

> (method form `udp:set_multicast_ttl(tty)`)

Set the multicast ttl.

`ttl` is an integer 1 through 255.

### uv.udp_set_multicast_interface(udp, interface_addr)

> (method form `udp:set_multicast_interface(interface_addr)`)

Set the multicast interface to send or receive data on.

### uv.udp_set_broadcast(udp, on)

Set broadcast on or off.

> (method form `udp:set_broadcast(, on)`)

### uv.udp_set_ttl(udp, ttl)

> (method form `udp:set_ttl(ttl)`)

Set the time to live.

`ttl` is an integer 1 through 255.

### uv.udp_send(udp, data, host, port, callback)

> (method form `udp:send(data, host, port, callback)`)

Send data over the UDP socket. If the socket has not previously been bound
with `uv_udp_bind()` it will be bound to `0.0.0.0` (the “all interfaces” IPv4
address) and a random port number.

### uv.udp_try_send(udp, data, host, port)

> (method form `udp:try_send(data, host, port)`)

Same as `uv_udp_send()`, but won’t queue a send request if it can’t be
completed immediately.

### uv.udp_recv_start(udp, callback)

> (method form `udp:recv_start(callback)`)

Prepare for receiving data. If the socket has not previously been bound with
`uv_udp_bind()` it is bound to `0.0.0.0` (the “all interfaces” IPv4 address)
and a random port number.

### uv.udp_recv_stop(udp)

> (method form `udp:recv_stop()`)

## `uv_fs_event_t` — FS Event handle

[`uv_fs_event_t`]: #uv_fs_event_t--fs-event-handle

**TODO**: port docs from [docs.libuv.org](http://docs.libuv.org/en/v1.x/fs_event.html)
using [functions](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L174-L177)
and [methods](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L265-L270)
from [fs_event.c](https://github.com/luvit/luv/blob/master/src/fs_event.c)

## `uv_fs_poll_t` — FS Poll handle

[`uv_fs_poll_t`]: #uv_fs_poll_t--fs-poll-handle

**TODO**: port docs from [docs.libuv.org](http://docs.libuv.org/en/v1.x/fs_poll.html)
using [functions](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L180-L183)
and [methods](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L272-L277)
from [fs_poll.c](https://github.com/luvit/luv/blob/master/src/fs_poll.c)

## Filesystem operations

[Filesystem operations]:#filesystem-operations

**TODO**: port docs from [docs.libuv.org](http://docs.libuv.org/en/v1.x/fs.html)
using [functions](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L186-L213)
from [fs.c](https://github.com/luvit/luv/blob/master/src/fs.c)

## DNS utility functions

[DNS utility functions]: #dns-utility-functions

**TODO**: port docs from [docs.libuv.org](http://docs.libuv.org/en/v1.x/dns.html)
using [functions](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L216-L217)
from [dns.c](https://github.com/luvit/luv/blob/master/src/dns.c)

## Miscellaneous utilities

[Miscellaneous utilities]: #miscellaneous-utilities

**TODO**: port docs from [docs.libuv.org](http://docs.libuv.org/en/v1.x/misc.html)
using [functions](https://github.com/luvit/luv/blob/25278a3871962cab29763692fdc3b270a7e96fe9/src/luv.c#L220-L235)
from [misc.c](https://github.com/luvit/luv/blob/master/src/misc.c)

[luv]: https://github.com/luvit/luv
[luvit]: https://github.com/luvit/luvit
[libuv]: https://github.com/libuv/libuv
