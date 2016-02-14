luv
===

[![Linux Build Status](https://travis-ci.org/luvit/luv.svg?branch=master)](https://travis-ci.org/luvit/luv)

[![Windows Build status](https://ci.appveyor.com/api/projects/status/uo1qhdcc0vcqsiok/branch/master?svg=true)](https://ci.appveyor.com/project/racker-buildbot/luv/branch/master)

[libuv](https://github.com/joyent/libuv) bindings for
[luajit](http://luajit.org/) and [lua](http://www.lua.org/)
[5.1](http://www.lua.org/manual/5.1/manual.html)/
[5.2](http://www.lua.org/manual/5.2/manual.html)/
[5.3](http://www.lua.org/manual/5.3/manual.html).

This library makes libuv available to lua scripts.  It was made for the [luvit](http://luvit.io/) project but should usable from nearly any lua project.

The library can be used by multiple threads at once.  Each thread is assumed to load the library from a different `lua_State`.  Luv will create a unique `uv_loop_t` for each state.  You can't share uv handles between states/loops.

The best docs currently are the [libuv docs](http://docs.libuv.org/) themselves.  Hopfully soon we'll have a copy locally tailored for lua.

```lua
local uv = require('luv')

-- Create a handle to a uv_timer_t
local timer = uv.new_timer()

-- This will wait 1000ms and then continue inside the callback
timer:start(1000, 0, function ()
  -- timer here is the value we passed in before from new_timer.

  print ("Awake!")

  -- You must always close your uv handles or you'll leak memory
  -- We can't depend on the GC since it doesn't know enough about libuv.
  timer:close()
end)

print("Sleeping");

-- uv.run will block and wait for all events to run.
-- When there are no longer any active handles, it will return
uv.run()
```


Here is an example of an TCP echo server
```lua
local uv = require('luv')

local function create_server(host, port, on_connection)

  local server = uv.new_tcp()
  server:bind(host, port)

  server:listen(128, function(err)
    -- Make sure there was no problem setting up listen
    assert(not err, err)

    -- Accept the client
    local client = uv.new_tcp()
    server:accept(client)

    on_connection(client)
  end)

  return server
end

local server = create_server("0.0.0.0", 0, function (client)

  client:read_start(function (err, chunk)

    -- Crash on errors
    assert(not err, err)

    if chunk then
      -- Echo anything heard
      client:write(chunk)
    else
      -- When the stream ends, close the socket
      client:close()
    end
  end)
end)

print("TCP Echo serverr listening on port " .. server:getsockname().port)

uv.run()
```

More examples can be found in the [examples](examples) and [tests](tests) folders.

## Building From Source

To build, first install your compiler tools.

### Get a Compiler

On linux this probably means `gcc` and `make`.  On Ubuntu, the `build-essential`
package is good for this.

On OSX, you probably want XCode which comes with `clang` and `make` and friends.

For windows the free Visual Studio Express works.  If you get the 2013 edition,
make sure to get the `Windows Deskop` edition.  The `Windows` version doesn't
include a working C compiler.  Make sure to run all of setup including getting a
free license.

### Install CMake

Now install Cmake.  The version in `brew` on OSX or most Linux package managers
is good.  The version on Travis CI is too old and so I use a PPA there.  On
windows use the installer and make sure to add cmake to your command prompt
path.

### Install Git

If you haven't already, install git and make sure it's in your path.  This comes
with XCode on OSX.  On Linux it's in your package manager.  For windows, use the
installer at <http://git-scm.com>.  Make sure it's available to your windows
command prompt.

### Clone the Code

Now open a terminal and clone the code.  For windows I recommend the special
developer command prompt that came with Visual Studio.

```
git clone https://github.com/luvit/luv.git --recursive
cd luv
```

### Build the Code and Test

On windows I wrote a small batch file that runs the correct cmake commands and
copies the output files for easy access.

```
C:\Code\luv> msvcbuild.bat
C:\Code\luv> luajit tests\run.lua
```

On unix systems, use the Makefile.

```
~/Code/luv> make test
```

This will build luv as a module library. Module libraries are plugins that are
not linked into other targets.

#### Build with PUC Lua 5.3
By default luv is linked with LuaJIT 2.0.4. If you rather like to link luv
with PUC Lua 5.3 you can run make with:

```
~/Code/luv> WITH_LUA_ENGINE=Lua make
```

#### Build as static library

If you want to build luv as a static library run make with:

```
~/Code/luv> BUILD_MODULE=OFF make
```

This will create a static library `libluv.a`.

#### Build as shared library

If you want to build luv as a shared library run make with:

```
~/Code/luv> BUILD_MODULE=OFF BUILD_SHARED_LIBS=ON make
```

This will create a shared library `libluv.so`.

#### Build with shared libraries

By default the build system will build luv with the supplied dependencies.
These are:
  * libuv
  * LuaJIT or Lua

However, if your target system has already one or more of these dependencies
installed you can link `luv` against them.

##### Linking with shared libuv

The default shared library name for libuv is `libuv`. To link against it use:

```
~/Code/luv> WITH_SHARED_LIBUV=ON make
```

##### Linking with shared LuaJIT

The default shared library name for LuaJIT is `libluajit-5.1`. To link against
it use:

```
~/Code/luv> LUA_BUILD_TYPE=System make
```

##### Linking with shared Lua 5.x

The default shared library name for Lua 5.x is `liblua5.x`. To link against
it use:

```
~/Code/luv> LUA_BUILD_TYPE=System WITH_LUA_ENGINE=Lua make
```
