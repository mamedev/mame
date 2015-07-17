# **MEWUI** #

[![Build Status](https://travis-ci.org/dankan1890/mewui.svg)](https://travis-ci.org/dankan1890/mewui) [![Build status](https://ci.appveyor.com/api/projects/status/rswl8b39pw8gl5b1?svg=true)](https://ci.appveyor.com/project/dankan1890/mewui)

What is MEWUI?
=============

MEWUI stands for MAME Extended Windows User Interface.

MAME's purpose is to preserve decades of video-game history. As gaming technology continues to rush forward, MAME prevents these important "vintage" games from being lost and forgotten. This is achieved by documenting the hardware and how it functions. The source code to MAME serves as this documentation. The fact that the games are playable serves primarily to validate the accuracy of the documentation (how else can you prove that you have recreated the hardware faithfully?).

MESS (Multi Emulator Super System) is the sister project of MAME. MESS documents the hardware for a wide variety of (mostly vintage) computers, video game consoles, and calculators, as MAME does for arcade games.

The MESS and MAME projects live in the same source repository and share much of the same code, but are different build targets.


How to compile?
=============

If you're on a *nix system, it could be as easy as typing

```
make
```

For Windows users, we provide a ready-made [build environment](http://mamedev.org/tools/) based on MinGW-w64. [Visual Studio builds](http://wiki.mamedev.org/index.php?title=Building_MAME_using_Microsoft_Visual_Studio_compilers) are also possible.
