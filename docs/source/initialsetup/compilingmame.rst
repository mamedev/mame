Compiling MAME
==============

.. contents:: :local:

.. _compiling-all:

All Platforms
-------------

* To compile MAME, you need a C++14 compiler and runtime library.  We
  support building with GCC version 7.2 or later and clang version 5 or
  later.  MAME should run with GNU libstdc++ version 5.1 or later.

* Whenever you are changing build parameters, (such as switching between
  a SDL-based build and a native Windows renderer one, or adding tools
  to the compile list) you need to run a **make REGENIE=1** to allow the
  settings to be regenerated.  Failure to do this will cause you very
  difficult to troubleshoot problems.

* If you want to add various additional tools to the compile, such as
  *CHDMAN*, add a **TOOLS=1** to your make statement, like
  **make REGENIE=1 TOOLS=1**

* You can do driver specific builds by using *SOURCES=<driver>* in your
  make statement.  For instance, building Pac-Man by itself would be
  **make SOURCES=src/mame/drivers/pacman.cpp REGENIE=1** including the
  necessary *REGENIE* for rebuilding the settings.

* Speeding up the compilation can be done by using more cores from your
  CPU.  This is done with the **-j** parameter.  *Note: a good number to
  start with is the total number of CPU cores in your system plus one.
  An excessive number of concurrent jobs may increase compilation time.
  The optimal number depends on many factors, including number of CPU
  cores, available RAM, disk and filesystem performance, and memory
  bandwidh.* For instance, **make -j5** is a good starting point on a
  system with a quad-core CPU.

* Debugging information can be added to a compile using *SYMBOLS=1*
  though most users will not want or need to use this.  This increases
  compile time and disk space used.

Putting all of these together, we get a couple of examples:

Rebuilding MAME for just the Pac-Man driver, with tools, on a quad-core
(e.g. i5 or i7) machine:

| **make SOURCES=src/mame/drivers/pacman.cpp TOOLS=1 REGENIE=1 -j5**
|

Rebuilding MAME on a dual-core (e.g. i3 or laptop i5) machine:

| **make -j3**
|


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
  ``sdl`` prefix prepended (e.g. ``sdlmame64.exe`` or ``sdlmame.exe``).  You
  will need to install the MSYS2 packages for SDL 2 version 2.0.3 or later.
* By default, MAME will include the native Windows debugger.  To also inculde
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
* Add the ``mame`` repository to ``/etc/pacman.conf`` using
  ``/etc/pacman.d/mirrorlist.mame`` for locations.
* Install packages necessary to build MAME.  At the very least, you'll need
  ``bash``, ``git``, ``make``.
* For 64-bit builds you'll need ``mingw-w64-x86_64-gcc`` and
  ``mingw-w64-x86_64-python2``.
* For 32-bit builds you'll need ``mingw-w64-i686-gcc`` and
  ``mingw-w64-i686-python2``.
* For debugging you may want to install ``gdb``.
* To build against the portable SDL interfaces, you'll need
  ``mingw-w64-x86_64-SDL2`` and ``mingw-w64-x86_64-SDL2_ttf`` for 64-bit builds,
  or ``mingw-w64-i686-SDL2`` and ``mingw-w64-i686-SDL2_ttf`` for 32-bit builds.
* To build the Qt debugger, you'll need ``mingw-w64-x86_64-qt5`` for 64-bit
  builds, or ``mingw-w64-i686-qt5`` for 32-bit builds.
* To generate API documentation from source, you'll need ``doxygen``.
* For 64-bit builds, open **MSYS2 MinGW 64-bit** from the start menu, and set
  up the environment variables ``MINGW64`` to ``/mingw64`` and ``MINGW32`` to an
  empty string (e.g. using the command **export MINGW64=/mingw64 MINGW32=** in
  the Bash shell).
* For 32-bit builds, open **MSYS2 MinGW 32-bit** from the start menu, and set
  up the environment variables ``MINGW32`` to ``/mingw32`` and ``MINGW64`` to an
  empty string (e.g. using the command **export MINGW32=/mingw32 MINGW64=** in
  the Bash shell).

Building with Microsoft Visual Studio
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* You can generate Visual Studio 2017 projects using **make vs2017**.  The
  solution and project files will be created in
  ``build/projects/windows/mame/vs2017`` by default (the name of the ``build``
  folder can be changed using the ``BUILDDIR`` option).  This will always
  regenerate the settings, so **REGENIE=1** is *not* needed.
* Adding **MSBUILD=1** to the make options will build build the solution using
  the Microsoft Build Engine after generating the project files.  Note that this
  requires paths and environment variables to be configured so the correct
  Visual Studio tools can be located.
* MAME can only be compiled with the Visual Studio 15.7.6 tools.  Bugs in newer
  versions of the Microsoft Visual C/C++ compiler prevent it from compiling
  MAME.
* The MSYS2 environment is still required to generate the project files, convert
  built-in layouts, compile UI translations, etc.


.. _compiling-fedora:

Fedora Linux
------------

You'll need a few prerequisites from your distro. Make sure you get SDL2 2.0.3 or 2.0.4 as earlier versions are buggy.

**sudo dnf install gcc gcc-c++ SDL2-devel SDL2_ttf-devel libXi-devel libXinerama-devel qt5-qtbase-devel qt5-qttools expat-devel fontconfig-devel alsa-lib-devel**

Compilation is exactly as described above in All Platforms.


.. _compiling-ubuntu:

Debian and Ubuntu (including Raspberry Pi and ODROID devices)
-------------------------------------------------------------

You'll need a few prerequisites from your distro. Make sure you get SDL2 2.0.3 or 2.0.4 as earlier versions are buggy.

**sudo apt-get install git build-essential python libsdl2-dev libsdl2-ttf-dev libfontconfig-dev qt5-default**

Compilation is exactly as described above in All Platforms.


.. _compiling-arch:

Arch Linux
----------

You'll need a few prerequisites from your distro.

**sudo pacman -S base-devel git sdl2 gconf sdl2_ttf gcc qt5**

Compilation is exactly as described above in All Platforms.


.. _compiling-macos:

Apple Mac OS X
--------------

You'll need a few prerequisites to get started. Make sure you're on OS X 10.9 Mavericks or later. You will NEED SDL2 2.0.4 for OS X.

* Install **Xcode** from the Mac App Store
* Launch **Xcode**. It will download a few additional prerequisites. Let this run through before proceeding.
* Once that's done, quit **Xcode** and open a **Terminal** window
* Type **xcode-select --install** to install additional tools necessary for MAME

Next you'll need to get SDL2 installed.

* Go to `this site <http://libsdl.org/download-2.0.php>`_ and download the *Mac OS X* .dmg file
* If the .dmg doesn't auto-open, open it
* Click 'Macintosh HD' (or whatever your Mac's hard disk is named) in the left pane of a **Finder** window, then open the **Library** folder and drag the **SDL2.framework** folder from the SDL disk image into the **Frameworks** folder

Lastly to begin compiling, use Terminal to navigate to where you have the MAME source tree (*cd* command) and follow the normal compilation instructions from above in All Platforms.

It's possible to get MAME working from 10.6, but a bit more complicated:

* You'll need to install clang-3.7, ld64, libcxx and python27 from MacPorts
* Then add these options to your make command or useroptions.mak:

|
| OVERRIDE_CC=/opt/local/bin/clang-mp-3.7
| OVERRIDE_CXX=/opt/local/bin/clang++-mp-3.7
| PYTHON_EXECUTABLE=/opt/local/bin/python2.7
| ARCHOPTS=-stdlib=libc++
|


.. _compiling-emscripten:

Emscripten Javascript and HTML
------------------------------

First, download and install Emscripten 1.37.29 or later by following the instructions at the `official site <https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html>`_

Once Emscripten has been installed, it should be possible to compile MAME out-of-the-box using Emscripten's '**emmake**' tool. Because a full MAME compile is too large to load into a web browser at once, you will want to use the SOURCES parameter to compile only a subset of the project, e.g. (in the mame directory):

**emmake make SUBTARGET=pacmantest SOURCES=src/mame/drivers/pacman.cpp**

The SOURCES parameter should have the path to at least one driver .cpp file. The make process will attempt to locate and include all dependencies necessary to produce a complete build including the specified driver(s). However, sometimes it is necessary to manually specify additional files (using commas) if this process misses something. E.g.:

**emmake make SUBTARGET=apple2e SOURCES=src/mame/drivers/apple2e.cpp,src/mame/machine/applefdc.cpp**

The value of the SUBTARGET parameter serves only to differentiate multiple builds and need not be set to any specific value.

Emscripten supports compiling to WebAssembly with a JavaScript loader instead of all-JavaScript, and in later versions this is actually the default. To force WebAssembly on or off, add WEBASSEMBLY=1 or WEBASSEMBLY=0 to the make command line.

Other make parameters can also be used, e.g. *-j* for multithreaded compilation as described earlier.

When the compilation reaches the emcc phase, you may see a number of *"unresolved symbol"* warnings. At the moment, this is expected for OpenGL-related functions such as glPointSize. Any others may indicate that an additional dependency file needs to be specified in the SOURCES list. Unfortunately this process is not automated and you will need to search the source tree to locate the files supplying the missing symbols. You may also be able to get away with ignoring the warnings if the code path referencing them is not used at run-time.

If all goes well, a .js file will be output to the current directory. This file cannot be run by itself, but requires an HTML loader to provide it with a canvas to output to and pass in command-line parameters. The `Emularity project <https://github.com/db48x/emularity>`_ provides such a loader.

There are example .html files in that repository which can be edited to point to your newly compiled MAME js filename and pass in whatever parameters you desire. You will then need to place all of the following on a web server:

* The compiled MAME .js file
* The compiled MAME .wasm file if using WebAssembly
* The .js files from the Emularity package (loader.js, browserfs.js, etc.)
* A .zip file with the ROMs for the MAME driver you would like to run (if any)
* Any software files you would like to run with the MAME driver
* An Emularity loader .html modified to point to all of the above

You need to use a web server instead of opening the local files directly due to security restrictions in modern web browsers.

If the result fails to run, you can open the Web Console in your browser to see any error output which may have been produced (e.g. missing or incorrect ROM files). A "ReferenceError: foo is not defined" error most likely indicates that a needed source file was omitted from the SOURCES list.

.. _compiling-docs:

Compiling the Documentation
---------------------------

Compiling the documentation will require you to install several packages depending on your operating system.

On Debian/Ubuntu flavors of Linux, you'll need python3-sphinx/python-sphinx and the python3-pip/python-pip packages.

**sudo apt-get install python3-sphinx python3-pip** or **sudo apt-get install python-sphinx python-pip** depending on whether you're using Python 3 or Python 2.

You'll then need to install the SVG handler.

**pip3 install sphinxcontrib-svg2pdfconverter** or **pip install sphinxcontrib-svg2pdfconverter** depending on whether you're using Python 3 or Python 2.

If you intend on making a PDF via LaTeX, you'll need to install a LaTeX distribution such as TeX Live.

**sudo apt-get install latexmk texlive texlive-science texlive-formats-extra**

From this point you can do **make html** or **make latexpdf** from the docs folder to generate the output of your choice. Typing **make** by itself will tell you all available formats. The output will be in the docs/build folder in a subfolder based on the type chosen (e.g. **make html** will create *docs/build/html* with the output.)


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
   Set the Python interpreter command.  You need Python 2.7 or Python 3 to build
   MAME.
CROSS_BUILD
   Set to **1** to use separate host and target compilers and linkers, as
   required for cross-compilation.  In this case, **OVERRIDE_CC**,
   **OVERRIDE_CXX** and **OVERRIDE_LD** set the target C compiler, C++ compiler
   and linker commands, while **CC**, **CXX** and **LD** set the host C
   compiler, C++ compiler and linker commands.

Optional features
~~~~~~~~~~~~~~~~~

TOOLS
   Set to **1** to build additional tools along with the emulator, including
   **unidasm**, **chdman**, **romcmp**, and **srcclean**.
NO_USE_PORTAUDIO
   Set to **1** to disable building the PortAudio sound output module.
USE_QTDEBUG
   Set to **1** to include the Qt debugger on platforms where it's not built by
   default (e.g. Windows or MacOS), or to **0** to disable it.  You'll need to
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
   expense of larger executable size.  Set to **0** to disable optimisation (can
   make debugging easier), **1** for basic optimisation that doesn't have a
   space/speed trade-off and doesn't have a large impact on compile time, **2**
   to enable most optimisation that improves performance and reduces size, or
   **s** to enable only optimisations that generally don't increase executable
   size.  The exact set of supported values depends on your compiler.
SYMBOLS
   Set to **1** to include additional debugging symbols over the default for the
   target platform (many target platforms include function name symbols by
   default).
SYMLEVEL
   Numeric value that controls the level of detail in debugging symbols.  Higher
   numbers make debugging easier at the cost of increased build time and
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
USE_BUNDLED_LIB_SDL2
   Set to **1** to prefer the version of SDL provided with the MAME source over
   the system installation.  (This is enabled by default for Visual Studio and
   Android builds.  For other configurations, the system installation of SDL is
   preferred.)
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
* Initial versions of GNU libstdc++ 6 have a broken ``std::unique_ptr``
  implementation.  If you encounter errors with ``std::unique_ptr`` you need to
  upgrade to a newer version of libstdc++ that fixes the issue.

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
checks (e.g. if you don't want the performance impact of the run-time checks),
and it also makes it hard to define ``_FORTIFY_SOURCE`` to ``2`` if you want to
enable stricter checks.  You should really take it up with the distribution
maintainers, and make it clear you don't want non-standard GCC behaviour. It
would be better if these distributions defined this macro by default in their
packaging environments if they think it's important, rather than trying to force
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


.. _compiling-unusual:

Unusual Build Configurations
----------------------------

Cross-compiling MAME
~~~~~~~~~~~~~~~~~~~~

MAME's build system has basic support for cross-compilation.  Set
**CROSS_BUILD=1** to enable separate host and target compilers, set
**OVERRIDE_CC** and **OVERRIDE_CXX** to the target C/C++ compiler commands, and
if necessary set **CC** and **CXX** to the host C/C++ compiler commands.  If the
target OS is different to the host OS, set it with **TARGETOS**.  For example it
may be possible to build a MinGW32 x64 build on a Linux host using a command
like this:

**make TARGETOS=windows PTR64=1 OVERRIDE_CC=x86_64-w64-mingw32-gcc OVERRIDE_CXX=x86_64-w64-mingw32-g++ OVERRIDE_LD=x86_64-w64-mingw32-ld MINGW64=/usr**

(The additional packages required for producing a standard MinGW32 x64 build on
a Fedora Linux host are ``mingw64-gcc-c++``, ``mingw64-winpthreads-static`` and
their dependencies.  Non-standard builds may require additional packages.)

Using libc++ on Linux
~~~~~~~~~~~~~~~~~~~~~

MAME may be built using the LLVM project's "libc++" C++ Standard Library.  The
prerequisites are a working clang/LLVM installation, and the libc++ development
libraries.  On Fedora Linux, the necessary packages are **libcxx**,
**libcxx-devel**, **libcxxabi** and **libcxxabi-devel**.  Set the C and C++
compiler commands to use clang, and add **-stdlib=libc++** to the C++ compiler
and linker options.  You could use a command like this:

**env LDFLAGS=-stdlib=libc++ make OVERRIDE_CC=clang OVERRIDE_CXX=clang++ ARCHOPTS_CXX=-stdlib=libc++ ARCHOPTS_OBJCXX=-stdlib=libc++**

The options following the **make** command may be placed in a prefix makefile if
you want to use this configuration regularly, but **LDFLAGS** needs to be be set
in the environment.

Using a GCC/GNU libstdc++ installation in a non-standard location on Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GCC may be built and installed to a custom location, typically by supplying the
**--prefix=** option to the **configure** command.  This may be useful if you
want to build MAME on a Linux distribution that still uses a version of GNU
libstdC++ that predates C++14 support.  To use an alternate GCC installation to,
build MAME, set the C and C++ compilers to the full paths to the **gcc** and
**g++** commands, and add the library path to the run-time search path.  If you
installed GCC in /opt/local/gcc72, you might use a command like this:

**make OVERRIDE_CC=/opt/local/gcc72/bin/gcc OVERRIDE_CXX=/opt/local/gcc72/bin/g++ ARCHOPTS=-Wl,-R,/opt/local/gcc72/lib64**

You can add these options to a prefix makefile if you plan to use this
configuration regularly.
