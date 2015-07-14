
# **MAME** #

[![Build Status](https://travis-ci.org/mamedev/mame.svg)](https://travis-ci.org/mamedev/mame) [![Build status](https://ci.appveyor.com/api/projects/status/te0qy56b72tp5kmo?svg=true)](https://ci.appveyor.com/project/startaq/mame) [![Join the chat at https://gitter.im/mamedev/mame](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mamedev/mame?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

What is MAME?
=============

MAME stands for Multiple Arcade Machine Emulator.

MAME's purpose is to preserve decades of video-game history. As gaming technology continues to rush forward, MAME prevents these important "vintage" games from being lost and forgotten. This is achieved by documenting the hardware and how it functions. The source code to MAME serves as this documentation. The fact that the games are playable serves primarily to validate the accuracy of the documentation (how else can you prove that you have recreated the hardware faithfully?).


What is MESS?
=============

MESS (Multi Emulator Super System) is the sister project of MAME. MESS documents the hardware for a wide variety of (mostly vintage) computers, video game consoles, and calculators, as MAME does for arcade games.

The MESS and MAME projects live in the same source repository and share much of the same code, but are different build targets.


License
=======

MAME is in the process of becoming a Free and Open Source project. We are still in the process of contacting all developers who have contributed in the past. We have received approval for the vast majority of contributions.

Going forward, we will be using the 3-Clause BSD license for the core, and the LGPL version 2.1 or later, and the GPL version 2.0 or later, for certain drivers. As a whole, MAME will be delivered under the GPL version 2.0 or later.
As we are still contacting developers, MAME is still distributed under the [MAME license](docs/mamelicense.txt) as of this time. If you have not been contacted yet, and believe you have contributed code to MAME in the past, please [contact us](mailto:mamedev@mamedev.org).

How to compile?
=============

If you're on a *nix system, it could be as easy as typing

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

for a MESS build (provided you have all the [prerequisites](http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=35138)).

For Windows users, we provide a ready-made [build environment](http://mamedev.org/tools/) based on MinGW-w64. [Visual Studio builds](http://wiki.mamedev.org/index.php?title=Building_MAME_using_Microsoft_Visual_Studio_compilers) are also possible.




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

All contributors need to either add standard header for license info (on new files) or send us their wish under which of licenses they would like their code to be published under :[BSD-3-Clause](http://spdx.org/licenses/BSD-3-Clause), or for new files in mame/ or mess/, either the [BSD-3-Clause](http://spdx.org/licenses/BSD-3-Clause) license, the [LGPL-2.1+](http://spdx.org/licenses/LGPL-2.1+), or the [GPL-2.0+](http://spdx.org/licenses/GPL-2.0+).
