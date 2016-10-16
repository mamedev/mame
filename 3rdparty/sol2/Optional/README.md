Optional
========

A single-header header-only library for representing optional (nullable) objects for C++14 (and C++11 to some extent) and passing them by value. This is the reference implementation of proposal N3793 (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3793.html). Optional is now accepted into Library Fundamentals Technical Specification (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3848.html). The interface is based on Fernando Cacciola's Boost.Optional library (see http://www.boost.org/doc/libs/1_52_0/libs/optional/doc/html/index.html)


Usage
-----

```cpp
optional<int> readInt(); // this function may return int or a not-an-int

if (optional<int> oi = readInt()) // did I get a real int
  cout << "my int is: " << *oi;   // use my int
else
  cout << "I have no int";
```

For more usage examples and the overview see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3527.html


Supported compilers
-------------------

Clang 3.2, Clang 3.4, G++ 4.7.2, G++ 4.8.1. Tested only with libstdc++, versions 20130531, 20120920, 20110428. Others have reported it also works with libc++.



Known Issues
------------

 - Currently, the reference (and the only known) implementation of certain pieces of functionality explore what C++11 identifies as undefined behavior (see national body comment FI 15: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3770.html#FI15). This is mostly why Optional was removed from C++14 and put into Library Fundamentals TS. Luckily what the Standard identifies as UB is well defined on all known platforms. We expect that the C++14 wil fix this problem, so that our trick becomes well-defined.
 - In libstdc++ versions below 20130531 the constructor taking `initializer_list` argument is not `constexpr`. This is because `initializer_list` operations are not `constexpr` in C++11. This works however in version 20130531. It is also not enabled for libc++ because I do not have access to it and do nto know if it provides `constexpr` `initializer_list`.
 - In G++ 4.7.2 and 4.8.0 member function `value_or` does not have rvalue reference overload. These compilers do not support rvalue overloding on `*this`.
 - In order for the library to work with slightly older compilers, we emulate some missing type traits. On some platforms we cannot correctly detect the available features, and attempts at workarounds for missing type trait definitions can cause compile-time errors. Define macro `TR2_OPTIONAL_DISABLE_EMULATION_OF_TYPE_TRAITS` if you know that all the necessary type traits are defined, and no emulation is required.

License
-------
Distributed under the [Boost Software License, Version 1.0](http://www.boost.org/LICENSE_1_0.txt).
