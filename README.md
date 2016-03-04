
# **MAME** #

[![Join the chat at https://gitter.im/mamedev/mame](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mamedev/mame?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

What is MAME?
=============

MAME stands for Multiple Arcade Machine Emulator.

MAME's purpose is to preserve decades of video-game history. As gaming technology continues to rush forward, MAME prevents these important "vintage" games from being lost and forgotten. This is achieved by documenting the hardware and how it functions. The source code to MAME serves as this documentation. The fact that the games are playable serves primarily to validate the accuracy of the documentation (how else can you prove that you have recreated the hardware faithfully?). During time MAME absorbed his sister project MESS (Multi Emulator Super System), so MAME now documents a wide variety of (mostly vintage) computers, video game consoles, and calculators.

How to compile?
===============

If you're on a *nix or OSX system, it could be as easy as typing

```
make
```

for a MAME build,

```
make SUBTARGET=arcade
```

for an arcade-only build, or

```
make SUBTARGET=mess
```

for MESS build.

For a Linux users we have provided you all the [prerequisites](http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=35138)).

For Windows users, we provide a ready-made [build environment](http://mamedev.org/tools/) based on MinGW-w64. 

Visual Studio builds are also possible, but you still need [build environment](http://mamedev.org/tools/) based on MinGW-w64.
In order to generatesolution and project just run:

```
make vs2015
```
or  use next to directly build it using msbuild

```
make vs2015 MSBUILD=1
```


Where can I find out more?
=============

* [Official MAME Development Team Site](http://mamedev.org/) (includes binary downloads for MAME and MESS, wiki, forums, and more)
* [Official MESS Wiki](http://www.mess.org/)
* [MAME Testers](http://mametesters.org/) (official bug tracker for MAME and MESS)


Contributing
=============

## Coding standard

MAME source code should be viewed and edited with your editor set to use four spaces per tab. Tabs are used for initial indentation of lines, with one tab used per indentation level. Spaces are used for other alignment within a line.

Some parts of the code follow [GNU style](http://www.gnu.org/prep/standards/html_node/Formatting.html); some parts of the code follow [K&R style](https://en.wikipedia.org/wiki/Indent_style#K.26R_style) -- mostly depending on who wrote the original version. **Above all else, be consistent with what you modify, and keep whitespace changes to a minimum when modifying existing source.** For new code, the majority tends to prefer GNU style, so if you don't care much, use that.

All contributors need to either add standard header for license info (on new files) or send us their wish under which of licenses they would like their code to be published under :[BSD-3-Clause](http://opensource.org/licenses/BSD-3-Clause) license, the [LGPL-2.1](http://opensource.org/licenses/LGPL-2.1), or the [GPL-2.0](http://opensource.org/licenses/GPL-2.0).

License
=======
MAME project as a whole is licnesed under [GPL-2.0](http://opensource.org/licenses/GPL-2.0) license, since it contains code that is under multiple licenses. Great majority of files (over 90%) (including core files) are under [BSD-3-Clause](http://opensource.org/licenses/BSD-3-Clause) and we would encourage new developers to distribute files under this license. 

[License (GPL-2.0)](LICENSE)
============================

<a href="http://opensource.org/licenses/GPL-2.0" target="_blank">
<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">
</a>

<pre>
MAME - Multiple Arcade Machine Emulator
Copyright (C) 1997-2016 Nicola Salmoria and the MAME team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
</pre>
