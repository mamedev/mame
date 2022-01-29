An Introduction to MAME
-----------------------

MAME, formerly an acronym which stood for Multi Arcade Machine Emulator,
documents and reproduces through emulation the inner components of arcade
machines, computers, consoles, chess computers, calculators, and many other
types of electronic amusement machines. As a nice side-effect, MAME allows to
use on a modern PC those programs and games which were originally developed for
the emulated machines.

At one point there were actually two separate projects, MAME and MESS. MAME
covered arcade video games, while MESS covered home and business systems. They
are now merged into the one MAME.

MAME is written in C++ and can currently emulate over 32,000 individual systems
from the last five decades.


Purpose of MAME
---------------

The primary purpose of MAME is to preserve decades of arcade, computer, and
console history. As technology continues to rush forward, MAME prevents these
important “vintage” systems from being lost and forgotten.


Systems Emulated by MAME
------------------------

The `Arcade Database <http://adb.arcadeitalia.net/>`_  contains a complete list
of the systems currently emulated. As you will notice, being supported does not
always mean that the status of the emulation is perfect. You may want

1. to check the status of the emulation in the wiki pages of each system,
   accessible from the drivers page (e.g. for Apple Macintosh, from the page for
   the mac128.cpp driver you can reach the pages for both **macplus** and
   **macse**),
2. to read the corresponding **sysinfo.dat** entry in order to better understand
   which issues you may encounter while running a system in MAME (again, for
   Apple Macintosh Plus you have to check this entry).

Alternatively, you can simply see the status by yourself, launching the system
emulation and taking a look at the red or yellow warning screen which appears
before the emulation starts, if any. Notice that if you have information which
can help to improve the emulation of a supported system, or if you can directly
contribute fixes and/or addition to the current source, you can do any of the
following:

* Send in a pull request (for code) or open an issue (information) on our
  `GitHub page <https://github.com/mamedev/mame/>`_
* Post the information or code on the
  `MAME Forums <https://forum.mamedev.org/>`_
* Follow the instructions on our
  `contact page <https://www.mamedev.org/contact.html>`_


Supported OS
------------

The current source code can be directly compiled under all the main operating
systems: Microsoft Windows (both with DirectX/BGFX native support or with SDL
support), Linux, FreeBSD, and macOS.

System Requirements
-------------------

MAME is written in C++, and has been ported to numerous platforms. Over time, as
computer hardware has evolved, the MAME code has evolved as well to take
advantage of the greater processing power and hardware capabilities offered.

The official MAME binaries are compiled and designed to run on a standard
Windows-based system. The minimum requirements are:

* Intel Core 2-series CPU or equivalent, at least 2.0 GHz
* 64-bit OS (Windows 7 or later on Windows, macOS 10.9 or later on Mac)
* 4 GB RAM
* DirectX 9.0c for Windows
* A Direct3D, or OpenGL capable graphics card

Of course, the minimum requirements are just that: minimal. You may not get
optimal performance from such a system, but MAME should run. Modern versions of
MAME require more power than older versions, so if you have a less-capable PC,
you may find that using an older version of MAME may get you better
performance, at the cost of greatly lowered accuracy and fewer supported
systems.

MAME will take advantage of 3D hardware for compositing artwork and scaling
displayed software to full screen. To make use of this, you should have at least
a semi-modern computer with semi-modern 3D hardware made within the last five
to ten years.

HLSL or GLSL special effects such as CRT simulation will put a very heavy load
on your video card, especially at higher resolutions. You will need a fairly
powerful modern video card, and the load on your video card goes up
exponentially as your resolution increases. If HLSL or GLSL are too intensive,
try reducing your output resolution.

Keep in mind that even on the fastest computers available, MAME is still
incapable of playing some systems at full speed. The goal of the project isn’t
to make all system run speedy on your system; the goal is to document the
hardware and reproduce the behavior of the hardware as faithfully as possible.


BIOS Dumps and Software
-----------------------

Most of the systems emulated by MAME requires a dump of the internal chips of
the original system. These can be obtained by extracting the data from an
original unit, or finding them (at your own risk) on various place on the
Internet. Being copyrighted material, MAME does not come with any of these.

Also, you may want to find some software to be run on the emulated machine
where it does not have internal software (e.g. some computers will need a
disk to boot to an operating system).

Again, Google and other search engines are your best friends. MAME does not
provide any software in the MAME package to be run on the emulated machines
because it is very often (almost always, in the case of console software)
protected by copyright.

The MAME team has been permitted to redistribute some old software, which can be
found in the `ROMS section of the MAME site <https://www.mamedev.org/roms/>`_.
