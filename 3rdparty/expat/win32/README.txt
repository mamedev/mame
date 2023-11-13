
Expat can be built on Windows in two ways:
  using MS Visual Studio .NET or Cygwin.

* Cygwin:
  This follows the Unix build procedures.

* MS Visual Studio 2013, 2015 and 2017:
  Use CMake to generate a solution file for Visual Studio, then use msbuild
  to compile.  For example:

  md build
  cd build
  cmake -G"Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
  msbuild /m expat.sln

* All MS C/C++ compilers:
  The output for all projects will be generated in the <CMAKE_BUILD_TYPE>\
  and xmlwf\<CMAKE_BUILD_TYPE>\ directories.
  
* Creating MinGW dynamic libraries from MS VC++ DLLs:
  
  On the command line, execute these steps:
  pexports libexpat.dll > expat.def
  pexports libexpatw.dll > expatw.def
  dlltool -d expat.def -l libexpat.a
  dlltool -d expatw.def -l libexpatw.a
  
  The *.a files are mingw libraries.

* Special note about MS VC++ and runtime libraries:

  There are three possible configurations: using the
  single threaded or multithreaded run-time library,
  or using the multi-threaded run-time Dll. That is, 
  one can build three different Expat libraries depending
  on the needs of the application.

  Dynamic Linking:

  By default the Expat Dlls are built to link dynamically
  with the multi-threaded run-time library. 
  The libraries are named
  - libexpat(w).dll 
  - libexpat(w).lib (import library)
  The "w" indicates the UTF-16 version of the library.

  Versions that are statically linking with the multi-threaded run-time library
  can be built with -DEXPAT_MSVC_STATIC_CRT=ON.

  Static Linking:  (through -DEXPAT_SHARED_LIBS=OFF)

  The libraries should be named like this:
  Multi-threaded:     libexpat(w)MT.lib
  Multi-threaded Dll: libexpat(w)MD.lib
  The suffixes conform to the compiler switch settings
  /MT and /MD for MS VC++.

  An application linking to the static libraries must
  have the global macro XML_STATIC defined.
