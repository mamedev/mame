Compiling MAME
==============

.. _compiling-MAME:

All Platforms
-------------

* Whenever you are changing build parameters, (such as switching between a SDL-based build and a native Windows renderer one, or adding tools to the compile list) you need to run a **make REGENIE=1** to allow the settings to be regenerated. Failure to do this will cause you very difficult to troubleshoot problems.

* If you want to add various additional tools to the compile, such as *CHDMAN*, add a **TOOLS=1** to your make statement, like **make REGENIE=1 TOOLS=1**

* You can do driver specific builds by using *SOURCES=<driver>* in your make statement. For instance, building Pac-Man by itself would be **make SOURCES=src/mame/drivers/pacman.cpp REGENIE=1** including the necessary *REGENIE* for rebuilding the settings.

* Speeding up the compilation can be done by using more cores from your CPU. This is done with the **-j** parameter. *Note: the maximum number you should use is the number of cores your CPU has, plus one. No higher than that will speed up the compilation, and may in fact slow it down.* For instance, **make -j5** on a quad-core CPU will provide optimal speed.

* Debugging information can be added to a compile using *SYMBOLS=1* though most users will not want or need to use this.

Putting all of these together, we get a couple of examples:

Rebuilding MAME for just the Pac-Man driver, with tools, on a quad-core (e.g. i5 or i7) machine:

| **make SOURCES=src/mame/drivers/pacman.cpp TOOLS=1 REGENIE=1 -j5**
|

Rebuilding MAME on a dual-core (e.g. i3 or laptop i5) machine:

| **make -j3**
|


Microsoft Windows
-----------------

Here are specific notes about compiling MAME for Microsoft Windows.

* Refer to `the MAME tools site <http://mamedev.org/tools/>`_ for the latest toolkit for getting MAME compiled on Windows.

* You will need to download the toolset from that link to begin. Periodically, these tools are updated and newer versions of MAME from that point on will **require** updated tools to compile.

* You can do compilation on Visual Studio 2017 (if installed on your PC) by using **make vs2017**. This will always regenerate the settings, so **REGENIE=1** is *not* needed.

* Make sure you get SDL 2 2.0.3 or 2.0.4 as earlier versions are buggy.


Fedora Linux
------------

You'll need a few prerequisites from your distro. Make sure you get SDL2 2.0.3 or 2.0.4 as earlier versions are buggy.

**sudo dnf install gcc gcc-c++ SDL2-devel SDL2_ttf-devel libXinerama-devel qt5-qtbase-devel qt5-qttools expat-devel fontconfig-devel alsa-lib-devel**

Compilation is exactly as described above in All Platforms.


Debian and Ubuntu (including Raspberry Pi and ODROID devices)
-------------------------------------------------------------

You'll need a few prerequisites from your distro. Make sure you get SDL2 2.0.3 or 2.0.4 as earlier versions are buggy.

**sudo apt-get install git build-essential libsdl2-dev libsdl2-ttf-dev libfontconfig-dev qt5-default**

Compilation is exactly as described above in All Platforms.


Arch Linux
----------

You'll need a few prerequisites from your distro.

**sudo pacman -S base-devel git sdl2 gconf sdl2_ttf gcc qt5**

Compilation is exactly as described above in All Platforms.


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
