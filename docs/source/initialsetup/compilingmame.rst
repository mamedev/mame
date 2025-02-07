Compiling MAME
==============

.. contents:: :local:

.. _compiling-all:

All Platforms
-------------

* To compile MAME, you need a C++17 compiler and runtime library.  We
  support building with GCC version 10.3 or later and clang version 11
  or later.  MAME should run with GNU libstdc++ version 10.3 or later or
  libc++ version 11 or later.  The initial release of any major version
  of GCC should be avoided.  For example, if you want to compile MAME
  with GCC 12, you should use version 12.1 or later.

* Whenever you are changing build parameters, (for example changing
  optimisation settings, or adding tools to the compile list), or system
  drivers sources are added, removed, or renamed, the project files need
  to be regenerated.  You can do this by adding **REGENIE=1** to the
  make arguments, or updating the modification time of the makefile (for
  example using the **touch** command).  Failure to do this may cause
  difficult to troubleshoot problems.

* If you want to add various additional tools to the compile, such as
  *chdman*, add a **TOOLS=1** to your make command, like
  **make REGENIE=1 TOOLS=1**

* You can build an emulator for a subset of the systems supported by
  MAME by using *SOURCES=<driver>,...* in your make command.  For
  example
  **make SUBTARGET=pacem SOURCES=src/mame/pacman/pacman.cpp REGENIE=1**
  would build an emulator called *pacem* including the system drivers
  from the source file pacman.cpp (*REGENIE=1* is specified to ensure
  project files are regenerated).  You can specify folders to include
  their entire contents, and you can separate multiple files/folders
  with commas.  You can also omit the *src/mame/* prefix in many cases.

  If you encounter linking errors after changing the included sources,
  delete the static libraries for the subtarget from the build folder.
  For the previous example on Windows using GCC, these would be in
  *build/mingw-gcc/bin/x64/Release/mame_pacem* by default.

* On a system with multiple CPU cores, compilation can be sped up by
  compiling multiple source files in parallel.  This is done with the
  **-j** parameter.  For instance, **make -j5** is a good starting point
  on a system with a quad-core CPU.

  *Note: a good number to start with is the total number of CPU cores
  in your system plus one.  An excessive number of concurrent jobs will
  increase compilation time, particularly if the compiler jobs exhaust
  available memory.  The optimal number depends on many factors,
  including number of CPU cores, available RAM, disk and filesystem
  performance, and memory bandwidth.*

* Debugging information can be added to a compile using *SYMBOLS=1*
  though most users will not want or need to use this.  This increases
  compile time and disk space used.  Note that a full build of MAME
  including internal debugging symbols will exceed the maximum size for
  an executable on Windows, and will not be possible to run without
  first stripping the symbols.

Putting all of these together, we get a couple of examples:

Rebuilding MAME on a dual-core (e.g. i3 or laptop i5) machine::

    make -j3

Rebuilding MAME for just the Pac-Man and Galaxian families of systems,
with tools, on a quad-core (e.g. i5 or i7) machine::

    make SUBTARGET=pacem SOURCES=src/mame/pacman,src/mame/galaxian TOOLS=1 REGENIE=1 -j5

Rebuilding MAME for just the Apple II systems, compiling up to six
sources in parallel::

    make SUBTARGET=appulator SOURCES=apple/apple2.cpp,apple/apple2e.cpp,apple/apple2gs.cpp REGENIE=1 -j6


.. _compiling-windows:

Microsoft Windows
-----------------

MAME for Windows is built using the MSYS2 environment.  You will need Windows 7
or later and a reasonably up-to-date MSYS2 installation.  We strongly recommend
building MAME on a 64-bit system.  Instructions may need to be adjusted for
32-bit systems.

* A pre-packaged MSYS2 installation including the prerequisites for building
  MAME can be downloaded from the `MAME Build Tools
  <http://mamedev.org/tools/>`_ page.
* After initial installation, you can update the MSYS2 environment using the
  **pacman** (Arch package manage) command.
* By default, MAME will be built using native Windows OS interfaces for
  window management, audio/video output, font rendering, etc.  If you want to
  use the portable SDL (Simple DirectMedia Layer) interfaces instead, you can
  add **OSD=sdl** to the make options.  The main emulator binary will have an
  ``sdl`` prefix prepended (e.g. ``sdlmame.exe``).  You
  will need to install the MSYS2 packages for SDL 2 version 2.0.14 or later.
* By default, MAME will include the native Windows debugger.  To also include
  the portable Qt debugger, add **USE_QTDEBUG=1** to the make options.  You
  will need to install the MSYS2 packages for Qt 5.

Using a standard MSYS2 installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You may also build MAME using a standard MSYS2 installation and adding the tools
needed for building MAME.  These instructions assume you have some familiarity
with MSYS2 and the **pacman** package manager.

* Install the MSYS2 environment from  the `MSYS2 homepage
  <https://www.msys2.org/>`_.
* Download the latest version of the ``mame-essentials`` package from the
  `MAME package repository <https://repo.mamedev.org/x86_64/>`_ and install it
  using the **pacman** command.
* Add the ``mame`` package repository to ``/etc/pacman.conf`` using
  ``/etc/pacman.d/mirrorlist.mame`` for locations, and disable signature
  verification for this repository (``SigLevel = Never``).
* Install packages necessary to build MAME.  At the very least, you’ll need
  ``bash``, ``git``, ``make``.
* For 64-bit builds you’ll need ``mingw-w64-x86_64-gcc`` and
  ``mingw-w64-x86_64-python``.
* For 32-bit builds you’ll need ``mingw-w64-i686-gcc`` and
  ``mingw-w64-i686-python``.
* For debugging you may want to install ``gdb``.
* To link using the LLVM linker (generally much faster than the GNU linker),
  you’ll need ``mingw-w64-x86_64-lld`` and ``mingw-w64-x86_64-libc++`` for
  64-bit builds, or ``mingw-w64-i686-lld`` and ``mingw-w64-i686-libc++`` for
  32-bit builds.
* To build against the portable SDL interfaces, you’ll need
  ``mingw-w64-x86_64-SDL2`` and ``mingw-w64-x86_64-SDL2_ttf`` for 64-bit builds,
  or ``mingw-w64-i686-SDL2`` and ``mingw-w64-i686-SDL2_ttf`` for 32-bit builds.
* To build the Qt debugger, you’ll need ``mingw-w64-x86_64-qt5`` for 64-bit
  builds, or ``mingw-w64-i686-qt5`` for 32-bit builds.
* To build the HTML user/developer documentation, you’ll need
  ``mingw-w64-x86_64-librsvg``, ``mingw-w64-x86_64-python-sphinx``,
  ``mingw-w64-x86_64-python-sphinx_rtd_theme`` and
  ``mingw-w64-x86_64-python-sphinxcontrib-svg2pdfconverter`` for a 64-bit MinGW
  environment (or alternatively ``mingw-w64-i686-librsvg``,
  ``mingw-w64-i686-python-sphinx``, ``mingw-w64-i686-python-sphinx_rtd_theme``
  and ``mingw-w64-x86_64-python-sphinxcontrib-svg2pdfconverter`` a 32-bit MinGW
  environment).
* To build the PDF documentation, you’ll additionally need
  ``mingw-w64-x86_64-texlive-latex-extra`` and
  ``mingw-w64-x86_64-texlive-fonts-recommended`` (or
  ``mingw-w64-i686-texlive-latex-extra`` and
  ``mingw-w64-i686-texlive-fonts-recommended`` for a 32-but MinGW environment).
* To generate API documentation from source, you’ll need ``doxygen``.
* If you plan to rebuild bgfx shaders and you want to rebuild the GLSL parser,
  you’ll need ``bison``.
* For 64-bit builds, open **MSYS2 MinGW 64-bit** from the start menu, and set
  up the environment variables ``MINGW64`` to ``/mingw64`` and ``MINGW32`` to an
  empty string (e.g. using the command **export MINGW64=/mingw64 MINGW32=** in
  the Bash shell).
* For 32-bit builds, open **MSYS2 MinGW 32-bit** from the start menu, and set
  up the environment variables ``MINGW32`` to ``/mingw32`` and ``MINGW64`` to an
  empty string (e.g. using the command **export MINGW32=/mingw32 MINGW64=** in
  the Bash shell).

For example you could use these commands to ensure you have the packages you
need to compile MAME, omitting the ones for configurations you don’t plan to
build for or combining multiple **pacman** commands to install more packages at
once::

    pacman -Syu
    pacman -S curl git make
    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-libc++ mingw-w64-x86_64-lld mingw-w64-x86_64-python
    pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
    pacman -S mingw-w64-x86_64-qt5
    pacman -S mingw-w64-i686-gcc mingw-w64-i686-libc++ mingw-w64-i686-lld mingw-w64-i686-python
    pacman -S mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_ttf
    pacman -S mingw-w64-i686-qt5

You could use these commands to install the current version of the
mame-essentials package and add the MAME package repository to your pacman
configuration::

    curl -O "https://repo.mamedev.org/x86_64/mame-essentials-1.0.6-1-x86_64.pkg.tar.xz"
    pacman -U mame-essentials-1.0.6-1-x86_64.pkg.tar.xz
    echo -e '\n[mame]\nInclude = /etc/pacman.d/mirrorlist.mame\nSigLevel = Never' >> /etc/pacman.conf

Building with Microsoft Visual Studio
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* You can generate Visual Studio 2022 projects using **make vs2022**.  The
  solution and project files will be created in
  ``build/projects/windows/mame/vs2022`` by default (the name of the ``build``
  folder can be changed using the ``BUILDDIR`` option).  This will always
  regenerate the settings, so **REGENIE=1** is *not* needed.
* Adding **MSBUILD=1** to the make options will build the solution using
  the Microsoft Build Engine after generating the project files.  Note that this
  requires paths and environment variables to be configured so the correct
  Visual Studio tools can be located; please refer to the Microsoft-provided
  instructions on `using the Microsoft C++ toolset from the command line
  <https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line>`_.
  You may find it easier to not use **MSBUILD=1** and load the project file into
  Visual Studio’s GUI for compilation.
* The MSYS2 environment is still required to generate the project files, convert
  built-in layouts, compile UI translations, etc.

Some notes about the MSYS2 environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MSYS2 uses the pacman tool from Arch Linux for package management.  There is a
`page on the Arch Linux wiki <https://wiki.archlinux.org/index.php/Pacman>`_
with helpful information on using the pacman package management tool.

The MSYS2 environment includes two kinds of tools: MSYS2 tools designed to work
in a UNIX-like environment on top of Windows, and MinGW tools designed to work
in a more Windows-like environment.  The MSYS2 tools are installed in
``/usr/bin`` while the MinGW tools are installed in ``/ming64/bin`` and/or
``/mingw32/bin`` (relative to the MSYS2 installation directory).  MSYS2 tools
work best in an MSYS2 terminal, while MinGW tools work best in a Microsoft
command prompt.

The most obvious symptom of this is that arrow keys don’t work in interactive
programs if you run them in the wrong kind of terminal.  If you run MinGW gdb or
python from an MSYS2 terminal window, command history won’t work and it may not
be possible to interrupt an attached program with gdb.  Similarly it may be very
difficult to edit using MSYS2 vim in a Microsoft command prompt window.

MAME is built using the MinGW compilers, so the MinGW directories are included
earlier in the ``PATH`` for the build environments.  If you want to use an
interactive MSYS2 program from an MSYS2 shell, you may need to type the absolute
path to avoid using the MinGW equivalent instead.

MSYS2 gdb may have issues debugging MinGW programs like MAME.  You may get
better results by installing the MinGW version of gdb and running it from a
Microsoft command prompt window to debug MAME.

GNU make supports both POSIX-style shells (e.g. bash) and the Microsoft cmd.exe
shell.  One issue to be aware of when using the cmd.exe shell is that the
``copy`` command doesn’t provide a useful exit status, so file copy tasks can
fail silently.

It is not possible to cross-compile a 32-bit version of MAME using 64-bit MinGW
tools on Windows, the 32-bit MinGW tools must be used.  This causes issues due
to the size of MAME.  It is not possible to link a full 32-bit MAME build
including the SDL OS-dependent layer and the Qt debugger.  GNU ld and lld will
both run out of memory, leaving an output file that doesn’t work.  It’s also
impossible to make a 32-bit build with full local variable symbols.  GCC may run
out of memory, and certain source files may exceed the limit of 32,768 sections
imposed by the PE/COFF object file format.


.. _compiling-fedora:

Fedora Linux
------------

You’ll need a few prerequisites from your Linux distribution.  Make sure you get
SDL 2 version 2.0.14 or later as earlier versions lack required functionality::

    sudo dnf install gcc gcc-c++ SDL2-devel SDL2_ttf-devel libXi-devel libXinerama-devel qt5-qtbase-devel qt5-qttools expat-devel fontconfig-devel alsa-lib-devel pulseaudio-libs-devel

If you want to use the more efficient LLVM tools for archiving static libraries
and linking, you’ll need to install the corresponding packages::

    sudo dnf install lld llvm

Compilation is exactly as described above in All Platforms.

To build the HTML user/developer documentation, you’ll need Sphinx, as well as
the theme and the SVG converter::

    sudo dnf install python3-sphinx python3-sphinx_rtd_theme python3-sphinxcontrib-rsvgconverter

The HTML documentation can be built with this command::

    make -C docs SPHINXBUILD=sphinx-build-3 html


.. _compiling-ubuntu:

Debian and Ubuntu (including Raspberry Pi and ODROID devices)
-------------------------------------------------------------

You’ll need a few prerequisites from your Linux distribution.  Make sure you get
SDL 2 version 2.0.14 or later as earlier versions lack required functionality::

    sudo apt-get install git build-essential python3 libsdl2-dev libsdl2-ttf-dev libfontconfig-dev libpulse-dev qtbase5-dev qtbase5-dev-tools qtchooser qt5-qmake

Compilation is exactly as described above in All Platforms.  Note the Ubuntu
Linux modifies GCC to enable the GNU C Library “fortify source” feature by
default, which may cause issues compiling MAME (see :ref:`compiling-issues`).


.. _compiling-arch:

Arch Linux
----------

You’ll need a few prerequisites from your distro::

    sudo pacman -S base-devel git sdl2_ttf python libxinerama libpulse alsa-lib qt5-base

Compilation is exactly as described above in All Platforms.


.. _compiling-macos:

Apple macOS
-----------

You’ll need a few prerequisites to get started.  Make sure you’re on macOS 11.0
Big Sur or later.  You will need SDL 2 version 2.0.14 or later.  You’ll also
need to install Python 3 – it’s currently included with the Xcode command line
tools, but you can also install a stand-alone version or get it via the Homebrew
package manager.

* Install **Xcode** from the Mac App Store or
  `ADC <https://developer.apple.com/download/more/>`_ (AppleID required).
* To find the corresponding Xcode for your MacOS release please visit
  `xcodereleases.com <https://xcodereleases.com>`_ to find the latest version of
  Xcode available to you.
* Launch **Xcode**. It will download a few additional prerequisites.  Let this
  run through before proceeding.
* Once that’s done, quit **Xcode** and open a **Terminal** window.
* Type **xcode-select --install** to install additional tools necessary for MAME
  (also available as a package on ADC).

Next you’ll need to get SDL 2 installed.

* Go to `this site <http://libsdl.org/download-2.0.php>`_ and download the
  *macOS* .dmg file
* If the .dmg doesn’t open automatically, open it
* Click “Macintosh HD” (or whatever your Mac’s hard disk is named) in the left
  pane of a **Finder** window, then open the **Library** folder and drag the
  **SDL2.framework** folder from the SDL disk image into the **Frameworks**
  folder. You will have to authenticate with your user password.

If you don’t already have it, get Python 3 set up:

* Go to the official Python site, navigate to the
  `releases for macOS <https://www.python.org/downloads/macos/>`_, and click the
  link to download the installer for the latest stable release (this was
  `Python 3.10.4 <https://www.python.org/ftp/python/3.10.4/python-3.10.4-macos11.pkg>`_
  at the time of writing).
* Scroll down to the “Files” section, and download the macOS version (called
  “macOS 64-bit universal2 installer” or similar).
* Once the package downloads, open it and follow the standard installation
  process.

Finally to begin compiling, use Terminal to navigate to where you have the MAME
source tree (*cd* command) and follow the normal compilation instructions from
above in All Platforms.


.. _compiling-emscripten:

Emscripten Javascript and HTML
------------------------------

First, download and install Emscripten 3.1.35 or later by following the
instructions at the `official site <https://emscripten.org/docs/getting_started/downloads.html>`_.

Once Emscripten has been installed, it should be possible to compile MAME
out-of-the-box using Emscripten’s **emmake** tool. Because a full MAME
compile is too large to load into a web browser at once, you will want to use
the SOURCES parameter to compile only a subset of the project, e.g. (in the
MAME directory):

.. code-block:: bash

    emmake make SUBTARGET=pacmantest SOURCES=src/mame/pacman/pacman.cpp

The **SOURCES** parameter should have the path to at least one driver **.cpp**
file.  The make process will attempt to locate and include all dependencies
necessary to produce a complete build including the specified driver(s).
However, sometimes it is necessary to manually specify additional files (using
commas) if this process misses something. e.g.

.. code-block:: bash

    emmake make SUBTARGET=apple2e SOURCES=src/mame/apple/apple2e.cpp,src/devices/machine/applefdc.cpp

The value of the **SUBTARGET** parameter serves only to differentiate multiple
builds and need not be set to any specific value.

Emscripten supports compiling to WebAssembly with a JavaScript loader instead of
all-JavaScript, and in later versions this is actually the default. To force
WebAssembly on or off, add **WEBASSEMBLY=1** or **WEBASSEMBLY=0** to the make
command line, respectively.

Other make parameters can also be used, e.g. **-j** for multithreaded
compilation as described earlier.

When the compilation reaches the emcc phase, you may see a number of
*"unresolved symbol"* warnings.  At the moment, this is expected for
OpenGL-related functions such as glPointSize.  Any others may indicate that an
additional dependency file needs to be specified in the **SOURCES** list.
Unfortunately this process is not automated and you will need to search the
source tree to locate the files supplying the missing symbols.  You may also be
able to get away with ignoring the warnings if the code path referencing them is
not used at run-time.

If all goes well, a **.js** file will be output to the current directory.  This
file cannot be run by itself, but requires an HTML loader to provide it with a
canvas to draw to and to pass in command-line parameters.  The
`Emularity project <https://github.com/db48x/emularity>`_ provides such a
loader.

There are example **.html** files in that repository which can be edited to
point to your newly compiled MAME **.js** file and pass in whatever parameters
you desire. You will then need to place all of the following on a web server:

* The compiled MAME **.js** file
* The compiled MAME **.wasm** file if using WebAssembly
* The **.js** files from the Emularity package (**loader.js**, **browserfs.js**,
  etc.)
* A **.zip** file with the ROMs for the MAME driver you would like to run (if
  any)
* Any software files you would like to run with the MAME driver
* An Emularity loader **.html** modified to point to all of the above

You need to use a web server instead of opening the local files directly due to
security restrictions in modern web browsers.

If the result fails to run, you can open the Web Console in your browser to see
any error output which may have been produced (e.g. missing or incorrect ROM
files).  A “ReferenceError: foo is not defined” error most likely indicates that
a needed source file was omitted from the **SOURCES** list.


.. _compiling-docs:

Compiling the Documentation
---------------------------

Compiling the documentation will require you to install several packages
depending on your operating system.

.. _compiling-docs-windows:

Compiling the Documentation on Microsoft Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On Windows, you’ll need a couple of packages from the MSYS2 environment. You
can install these packages with

.. code-block:: bash

    pacman -S mingw-w64-x86_64-librsvg mingw-w64-x86_64-python-sphinx mingw-w64-x86_64-python-sphinxcontrib-svg2pdfconverter

If you intend to make a PDF via LaTeX, you’ll need to install a LaTeX
distribution such as TeX Live:

.. code-block:: bash

    pacman -S mingw-w64-x86_64-texlive-fonts-recommended mingw-w64-x86_64-texlive-latex-extra

.. _compiling-docs-debian:

Compiling the Documentation on Debian and Ubuntu
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On Debian/Ubuntu flavors of Linux, you’ll need **python3-sphinx/python-sphinx**
and the **python3-pip/python-pip** packages:

.. code-block:: bash

    sudo apt-get install python3-sphinx python3-pip
    pip3 install sphinxcontrib-svg2pdfconverter

On Debian, you’ll need to install the **librsvg2-bin** package:

.. code-block:: bash

    sudo apt-get install librsvg2-bin

If you intend to make a PDF via LaTeX, you’ll need to install a LaTeX
distribution such as TeX Live:

.. code-block:: bash

    sudo apt-get install librsvg2-bin latexmk texlive texlive-science texlive-formats-extra

From this point you can do ``make html`` or ``make latexpdf`` from the **docs**
folder to generate the output of your choice. Typing ``make`` by itself will
tell you all available formats. The output will be in the docs/build folder in
a subfolder based on the type chosen (e.g. ``make html`` will create
*docs/build/html* with the output.)


.. _compiling-options:

Useful Options
--------------

This section summarises some of the more useful options recognised by the main
makefile.  You use these options by appending them to the **make** command,
setting them as environment variables, or adding them to your prefix makefile.
Note that in order to apply many of these settings when rebuilding, you need to
set **REGENIE=1** the first time you build after changing the option(s).  Also
note that GENie *does not* automatically rebuild affected files when you change
an option that affects compiler settings.

Overall build options
~~~~~~~~~~~~~~~~~~~~~

PREFIX_MAKEFILE
    Name of a makefile to include for additional options if found (defaults to
    **useroptions.mak**).  May be useful if you want to quickly switch between
    different build configurations.
BUILDDIR
    Set to change the name of the subfolder used for project files, generated
    sources, object files, and intermediate libraries (defaults to **build**).
REGENIE
    Set to **1** to force project files to be regenerated.
VERBOSE
    Set to **1** to show full commands when using GNU make as the build tool.
    This option applies immediately without needing regenerate project files.
IGNORE_GIT
    Set to **1** to skip the working tree scan and not attempt to embed a git
    revision description in the version string.

Tool locations
~~~~~~~~~~~~~~

OVERRIDE_CC
    Set the C/Objective-C compiler command.  (This sets the target C compiler
    command when cross-compiling.)
OVERRIDE_CXX
    Set the C++/Objective-C++ compiler command.  (This sets the target C++
    compiler command when cross-compiling.)
OVERRIDE_LD
    Set the linker command.  This is often not necessary or useful because the C
    or C++ compiler command is used to invoke the linker.  (This sets the target
    linker command when cross-compiling.)
PYTHON_EXECUTABLE
    Set the Python interpreter command.  You need Python 3.2 or later to build
    MAME.
CROSS_BUILD
    Set to **1** to use separate host and target compilers and linkers, as
    required for cross-compilation.  In this case, **OVERRIDE_CC**,
    **OVERRIDE_CXX** and **OVERRIDE_LD** set the target C compiler, C++ compiler
    and linker commands, while **CC**, **CXX** and **LD** set the host C
    compiler, C++ compiler and linker commands.

Including subsets of supported systems
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SUBTARGET
    Set emulator subtarget to build.  Some pre-defined subtargets are provided,
    using Lua scripts in *scripts/target/mame* and system driver filter files in
    *src/mame*.  User-defined substargets can be created using the **SOURCES**
    or **SOURCEFILTER** option.
SOURCES
    Specify system driver source files and/or folders to include.  Usually used
    in conjunction with the **SUBTARGET** option.  Separate multiple
    files/folders with commas.
SOURCEFILTER
    Specify a system driver filter file.  Usually used in conjunction with the
    **SUBTARGET** option.  The filter file can specify source files to include
    system drivers from, and individual system drivers to include or exclude.
    There are some example system driver filter files in the *src/mame* folder.

Optional features
~~~~~~~~~~~~~~~~~

TOOLS
    Set to **1** to build additional tools along with the emulator, including
    **unidasm**, **chdman**, **romcmp**, and **srcclean**.
EMULATOR
    When set to **0**, the main emulator target will not be created.  This is
    intended to be used in conjunction with setting **TOOLS** to **1** to build
    the additional tools without building the emulator.
NO_OPENGL
    Set to **1** to disable building the OpenGL video output module.
NO_USE_PORTAUDIO
    Set to **1** to disable building the PortAudio sound output module and the
    PortAudio library.
NO_USE_PULSEAUDIO
    Set to **1** to disable building the PulseAudio sound output module on
    Linux.
USE_WAYLAND
    Set to **1** to include support for bgfx video output with the Wayland
    display server.
USE_TAPTUN
    Set to **1** to include the tap/tun network module, or set to **0** to
    disable building the tap/tun network module.  The tap/tun network module is
    included by default on Windows and Linux.
USE_PCAP
    Set to **1** to include the pcap network module, or set to **0** to disable
    building the pcap network module.  The pcap network module is included by
    default on macOS and NetBSD.
USE_QTDEBUG
    Set to **1** to include the Qt debugger on platforms where it’s not built by
    default (e.g. Windows or macOS), or to **0** to disable it.  You’ll need to
    install Qt development libraries and tools to build the Qt debugger.  The
    process depends on the platform.

Compilation options
~~~~~~~~~~~~~~~~~~~

NOWERROR
    Set to **1** to disable treating compiler warnings as errors.  This may be
    needed in marginally supported configurations.
DEPRECATED
    Set to **0** to disable deprecation warnings (note that deprecation warnings
    are not treated as errors).
DEBUG
    Set to **1** to enable runtime assertion checks and additional diagnostics.
    Note that this has a performance cost, and is most useful for developers.
OPTIMIZE
    Set optimisation level.  The default is **3** to favour performance at the
    expense of larger executable size.  Set to **0** to disable optimisation
    (can make debugging easier), **1** for basic optimisation that doesn’t have
    a space/speed trade-off and doesn’t have a large impact on compile time,
    **2** to enable most optimisation that improves performance and reduces
    size, or **s** to enable only optimisations that generally don’t increase
    executable size.  The exact set of supported values depends on your
    compiler.
SYMBOLS
    Set to **1** to include additional debugging symbols over the default for
    the target platform (many target platforms include function name symbols by
    default).
SYMLEVEL
    Numeric value that controls the level of detail in debugging symbols.
    Higher numbers make debugging easier at the cost of increased build time and
    executable size.  The supported values depend on your compiler.  For GCC and
    similar compilers, **1** includes line number tables and external variables,
    **2** also includes local variables, and **3** also includes macro
    definitions.
ARCHOPTS
    Additional command-line options to pass to the compiler and linker.  This is
    useful for supplying code generation or ABI options, for example to enable
    support for optional CPU features.
ARCHOPTS_C
    Additional command-line options to pass to the compiler when compiling C
    source files.
ARCHOPTS_CXX
    Additional command-line options to pass to the compiler when compiling C++
    source files.
ARCHOPTS_OBJC
    Additional command-line options to pass to the compiler when compiling
    Objective-C source files.
ARCHOPTS_OBJCXX
    Additional command-line options to pass to the compiler when compiling
    Objective-C++ source files.

Library/framework locations
~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDL_INSTALL_ROOT
    SDL installation root directory for shared library style SDL.
SDL_FRAMEWORK_PATH
    Search path for SDL framework.
USE_LIBSDL
    Set to **1** to use shared library style SDL on targets where framework is
    default.
USE_SYSTEM_LIB_ASIO
    Set to **1** to prefer the system installation of the Asio C++ asynchronous
    I/O library over the version provided with the MAME source.
USE_SYSTEM_LIB_EXPAT
    Set to **1** to prefer the system installation of the Expat XML parser
    library over the version provided with the MAME source.
USE_SYSTEM_LIB_ZLIB
    Set to **1** to prefer the system installation of the zlib data compression
    library over the version provided with the MAME source.
USE_SYSTEM_LIB_ZSTD
    Set to **1** to prefer the system installation of the Zstandard data
    compression library over the version provided with the MAME source.
USE_SYSTEM_LIB_JPEG
    Set to **1** to prefer the system installation of the libjpeg image
    compression library over the version provided with the MAME source.
USE_SYSTEM_LIB_FLAC
    Set to **1** to prefer the system installation of the libFLAC audio
    compression library over the version provided with the MAME source.
USE_SYSTEM_LIB_LUA
    Set to **1** to prefer the system installation of the embedded Lua
    interpreter over the version provided with the MAME source.
USE_SYSTEM_LIB_SQLITE3
    Set to **1** to prefer the system installation of the SQLITE embedded
    database engine over the version provided with the MAME source.
USE_SYSTEM_LIB_PORTMIDI
    Set to **1** to prefer the system installation of the PortMidi library over
    the version provided with the MAME source.
USE_SYSTEM_LIB_PORTAUDIO
    Set to **1** to prefer the system installation of the PortAudio library over
    the version provided with the MAME source.
USE_SYSTEM_LIB_UTF8PROC
    Set to **1** to prefer the system installation of the Julia utf8proc library
    over the version provided with the MAME source.
USE_SYSTEM_LIB_GLM
    Set to **1** to prefer the system installation of the GLM OpenGL Mathematics
    library over the version provided with the MAME source.
USE_SYSTEM_LIB_RAPIDJSON
    Set to **1** to prefer the system installation of the Tencent RapidJSON
    library over the version provided with the MAME source.
USE_SYSTEM_LIB_PUGIXML
    Set to **1** to prefer the system installation of the pugixml library over
    the version provided with the MAME source.


.. _compiling-issues:

Known Issues
------------

Issues with specific compiler versions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* GCC 7 for 32-bit x86 targets produces spurious out-of-bounds access warnings.
  Adding **NOWERROR=1** to your build options works around this by not treating
  warnings as errors.

GNU C Library fortify source feature
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The GNU C Library has options to perform additional compile- and run-time
checks on string operations, enabled by defining the ``_FORTIFY_SOURCE``
preprocessor macro.  This is intended to improve security at the cost of a
small amount of overhead.  MAME is not secure software, and we do not
support building with ``_FORTIFY_SOURCE`` defined.

Some Linux distributions (including Gentoo and Ubuntu) have patched GCC to
define ``_FORTIFY_SOURCE`` to ``1`` as a built-in macro.  This is problematic
for more projects than just MAME, as it makes it hard to disable the additional
checks (e.g. if you don’t want the performance impact of the run-time checks),
and it also makes it hard to define ``_FORTIFY_SOURCE`` to ``2`` if you want to
enable stricter checks.  You should really take it up with the distribution
maintainers, and make it clear you don’t want non-standard GCC behaviour. It
would be better if these distributions defined this macro by default in their
packaging environments if they think it’s important, rather than trying to force
it on everything compiled on their distributions. (This is what Red Hat does:
the ``_FORTIFY_SOURCE`` macro is set in the RPM build environment, and not by
distributing a modified version of GCC.)

If you get compilation errors in ``bits/string_fortified.h`` you should first
ensure that the ``_FORTIY_SOURCE`` macro is defined via the environment (e.g.
a **CFLAGS** or **CXXFLAGS** environment variable).  You can check to see
whether the ``_FORTIFY_SOURCE`` macro is a built-in macro with your version of
GCC with a command like this:

**gcc -dM -E - < /dev/null | grep _FORTIFY_SOURCE**

If ``_FORTIFY_SOURCE`` is defined to a non-zero value by default, you can work
around it by adding **-U_FORTIFY_SOURCE** to the compiler flags (e.g. by using
the **ARCHOPTS** setting, or setting the **CFLAGS** and **CXXFLAGS** environment
variables.

Issues affecting Microsoft Visual Studio
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Microsoft introduced a new version of XAudio2 with Windows 8 that’s incompatible
with the version included with DirectX for prior Windows versions at the API
level.  Newer versions of the Microsoft Windows SDK include headers and libraries
for the new version of XAudio2.  By default, the target Windows version is set to
Windows Vista (6.0) when compiling MAME, which prevents the use of this version
of the XAudio2 headers and libraries.  To build MAME with XAudio2 support using
the Microsoft Windows SDK, you must do one of the following:

* Add ``MODERN_WIN_API=1`` to the options passed to make when generating the
  Visual Studio project files.  This will set the target Windows version to
  Windows 8 (6.2).  The resulting binaries may not run on earlier versions of
  Windows.
* Install the `DirectX SDK <https://www.microsoft.com/en-US/download/details.aspx?id=6812>`_ (already included since Windows 8.0 SDK and
  automatically installed with Visual Studio 2013 and later).  Configure the
  **osd_windows** project to search the DirectX header/library paths before
  searching the Microsoft Windows SDK paths.

The MSVC compiler produces spurious warnings about potentially uninitialised
local variables.  You currently need to add ``NOWERROR=1`` to the options passed
to make when generating the Visual Studio project files.  This stops warnings
from being treated as errors.  (MSVC seems to lack options to control which
specific warnings are treated as errors, which other compilers support.)


.. _compiling-unusual:

Unusual Build Configurations
----------------------------

Linking using the LLVM linker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The LLVM linker is generally faster than the GNU linker that GCC uses by
default.  This is more pronounced on systems with a high overhead for file
system operations (e.g. Microsoft Windows, or when compiling on a disk mounted
over a network).  To use the LLVM linker with GCC, ensure the LLVM linker is
installed and add ``-fuse-ld=lld`` to the linker options (e.g. in the
**LDFLAGS** environment variable or in the **ARCHOPTS** setting).

Cross-compiling MAME
~~~~~~~~~~~~~~~~~~~~

MAME’s build system has basic support for cross-compilation.  Set
**CROSS_BUILD=1** to enable separate host and target compilers, set
**OVERRIDE_CC** and **OVERRIDE_CXX** to the target C/C++ compiler commands, and
if necessary set **CC** and **CXX** to the host C/C++ compiler commands.  If the
target OS is different to the host OS, set it with **TARGETOS**.  For example it
may be possible to build a MinGW32 x64 build on a Linux host using a command
like this::

    make TARGETOS=windows PTR64=1 OVERRIDE_CC=x86_64-w64-mingw32-gcc OVERRIDE_CXX=x86_64-w64-mingw32-g++ OVERRIDE_LD=x86_64-w64-mingw32-ld MINGW64=/usr**

(The additional packages required for producing a standard MinGW32 x64 build on
a Fedora Linux host are ``mingw64-gcc-c++``, ``mingw64-winpthreads-static`` and
their dependencies.  Non-standard builds may require additional packages.)

Using libc++ on Linux
~~~~~~~~~~~~~~~~~~~~~

MAME may be built using the LLVM project’s “libc++” C++ Standard Library.  The
prerequisites are a working clang/LLVM installation, and the libc++ development
libraries.  On Fedora Linux, the necessary packages are **libcxx**,
**libcxx-devel**, **libcxxabi** and **libcxxabi-devel**.  Set the C and C++
compiler commands to use clang, and add **-stdlib=libc++** to the C++ compiler
and linker options.  You could use a command like this::

    env LDFLAGS=-stdlib=libc++ make OVERRIDE_CC=clang OVERRIDE_CXX=clang++ ARCHOPTS_CXX=-stdlib=libc++ ARCHOPTS_OBJCXX=-stdlib=libc++

The options following the **make** command may be placed in a prefix makefile if
you want to use this configuration regularly, but **LDFLAGS** needs to be be set
in the environment.

Using a GCC/GNU libstdc++ installation in a non-standard location on Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GCC may be built and installed to a custom location, typically by supplying the
**--prefix=** option to the **configure** command.  This may be useful if you
want to build MAME on a Linux distribution that still uses a version of GNU
libstdC++ that predates C++17 support.  To use an alternate GCC installation to,
build MAME, set the C and C++ compilers to the full paths to the **gcc** and
**g++** commands, and add the library path to the run-time search path.  If you
installed GCC in /opt/local/gcc72, you might use a command like this::

    make OVERRIDE_CC=/opt/local/gcc72/bin/gcc OVERRIDE_CXX=/opt/local/gcc72/bin/g++ ARCHOPTS=-Wl,-R,/opt/local/gcc72/lib64

You can add these options to a prefix makefile if you plan to use this
configuration regularly.
