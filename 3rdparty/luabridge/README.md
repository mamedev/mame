<a href="http://lua.org">
<img src="http://vinniefalco.github.com/LuaBridgeDemo/powered-by-lua.png">
</a><br>

# LuaBridge 2.0

[LuaBridge][1] is a lightweight and dependency-free library for mapping data,
functions, and classes back and forth between C++ and [Lua][2] (a powerful,
fast, lightweight, embeddable scripting language) . LuaBridge has been tested
and works with Lua revisions starting from 5.1.5., although it should work in
any version of Lua from 5.1.0 as well as [LuaJit][3].

LuaBridge offers the following features:

- [MIT Licensed][4]
- A printable [Reference Manual][5].
- Headers-only: No Makefile, no .cpp files, just one #include!
- Simple, light, and nothing else needed (like Boost).
- No macros, settings, or configuration scripts needed.
- Supports different object lifetime management models.
- Convenient, type-safe access to the Lua stack.
- Automatic function parameter type binding.
- Easy access to Lua objects like tables and functions.
- Written in a clear and easy to debug style.
- Does not require C++11.

Please read the [LuaBridge Reference Manual][5] for more details on the API.

## LuaBridge Demo and Unit Tests

LuaBridge provides both a command line program and a stand-alone graphical
program for compiling and running the test suite. The graphical program brings
up an interactive window where you can enter execute Lua statements in a
persistent environment. This application is cross platform and works on
Windows, Mac OS, iOS, Android, and GNU/Linux systems with X11. The stand-alone
program should work anywhere. Both of these applications include LuaBridge,
Lua version 5.2, and the code necessary to produce a cross platform graphic
application. They are all together in a separate repository, with no
additional dependencies, available on Github at [LuaBridge Demo and Tests][6].
This is what the GUI application looks like, along with the C++ code snippet
for registering the two classes:

<a href="https://github.com/vinniefalco/LuaBridgeDemo">
<img src="http://vinniefalco.github.com/LuaBridgeDemo/LuaBridgeDemoScreenshot1.0.2.png">
</a><br>

## Official Repository

LuaBridge is published under the terms of the [MIT License][4].

The original version of LuaBridge was written by Nathan Reed. The project has
been taken over by [Vinnie Falco][7], who added new functionality, wrote the new
documentation, and incorporated contributions from Nigel Atkinson.

For questions, comments, or bug reports feel free to open a Github issue
or contact Vinnie Falco directly at the email address indicated below.

Copyright 2012, [Vinnie Falco][7] (<[vinnie.falco@gmail.com][8]>)<br>
Copyright 2008, Nigel Atkinson<br>
Copyright 2007, Nathan Reed<br>

Portions from The Loki Library:<br>
Copyright (C) 2001 by Andrei Alexandrescu

Older versions of LuaBridge up to and including 0.2 are distributed under the
BSD 3-Clause License. See the corresponding license file in those versions
(distributed separately) for more details.

[1]:  https://github.com/vinniefalco/LuaBridge "LuaBridge"
[2]:  http://lua.org "The Lua Programming Language"
[3]:  http://luajit.org/ "The LuaJIT Probject"
[4]:  http://www.opensource.org/licenses/mit-license.html "The MIT License"
[5]:  http://vinniefalco.github.com/LuaBridge "LuaBridge Reference Manual"
[6]:  https://github.com/vinniefalco/LuaBridgeDemo "LuaBridge Demo"
[7]:  https://github.com/vinniefalco "Vinnie Falco's Github"
[8]:  mailto:vinnie.falco@gmail.com "Vinnie Falco (Email)"
