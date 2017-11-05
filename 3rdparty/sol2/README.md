## Sol 2.15

[![Join the chat at https://gitter.im/chat-sol2/Lobby](https://badges.gitter.im/chat-sol2/Lobby.svg)](https://gitter.im/chat-sol2/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Build Status](https://travis-ci.org/ThePhD/sol2.svg?branch=develop)](https://travis-ci.org/ThePhD/sol2)
[![Documentation Status](https://readthedocs.org/projects/sol2/badge/?version=latest)](http://sol2.readthedocs.io/en/latest/?badge=latest)

Sol is a C++ library binding to Lua. It currently supports all Lua versions 5.1+ (LuaJIT 2.x included). Sol aims to be easy to use and easy to add to a project.
The library is header-only for easy integration with projects.

## Documentation

Find it [here](http://sol2.rtfd.io/). A run-through kind of tutorial is [here](http://sol2.readthedocs.io/en/latest/tutorial/all-the-things.html)! The API documentation goes over most cases (particularly, the "api/usertype" and "api/proxy" and "api/function" sections) that should still get you off your feet and going, and there's an examples directory [here](https://github.com/ThePhD/sol2/tree/develop/examples) as well.

## Sneak Peek

```cpp
#include <sol.hpp>
#include <cassert>

int main() {
    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x]{ ++x; });
    lua.script("beep()");
    assert(x == 1);
}
```

```cpp
#include <sol.hpp>
#include <cassert>

struct vars {
    int boop = 0;
};

int main() {
    sol::state lua;
    lua.new_usertype<vars>("vars", "boop", &vars::boop);
    lua.script("beep = vars.new()\n"
               "beep.boop = 1");
    assert(lua.get<vars>("beep").boop == 1);
}
```

More examples are given in the examples directory.

## Presentations

"A Sun For the Moon - A Zero-Overhead Lua Abstraction using C++"  
ThePhD  
Lua Workshop 2016 - Mashape, San Francisco, CA  
[Deck](https://github.com/ThePhD/sol2/blob/develop/docs/presentations/ThePhD%20-%20No%20Overhead%20C%20Abstraction%20-%202016.10.14.pdf)

## Creating a single header

You can grab a single header out of the library [here](https://github.com/ThePhD/sol2/tree/develop/single/sol). For stable version, check the releases tab on github for a provided single header file for maximum ease of use. A script called `single.py` is provided in the repository if there's some bleeding edge change that hasn't been published on the releases page. You can run this script to create a single file version of the library so you can only include that part of it. Check `single.py --help` for more info.

## Features

- [Fastest in the land](http://sol2.readthedocs.io/en/latest/benchmarks.html) (see: sol bar in graph).
- Supports retrieval and setting of multiple types including `std::string` and `std::map/unordered_map`.
- Lambda, function, and member function bindings are supported.
- Intermediate type for checking if a variable exists.
- Simple API that completely abstracts away the C stack API, including `protected_function` with the ability to use an error-handling function.
- `operator[]`-style manipulation of tables
- C++ type representations in lua userdata as `usertype`s with guaranteed cleanup.
- Customization points to allow your C++ objects to be pushed and retrieved from Lua as multiple consecutive objects, or anything else you desire!
- Overloaded function calls: `my_function(1); my_function("Hello")` in the same lua script route to different function calls based on parameters
- Support for tables, nested tables, table iteration with `table.for_each` / `begin()` and `end()` iterators.

## Supported Compilers

Sol makes use of C++11/14 features. GCC 4.9 and Clang 3.4 (with std=c++1z and appropriate standard library) or higher should be able to compile without problems. However, the
officially supported and CI-tested compilers are:

- GCC 4.9.0+
- Clang 3.5+
- Visual Studio 2015 Community (Visual C++ 14.0)+

## License

Sol is distributed with an MIT License. You can see LICENSE.txt for more info.
