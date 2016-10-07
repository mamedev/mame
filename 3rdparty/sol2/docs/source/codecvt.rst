std::(w/u16/u32)string support
==============================
because this is surprisingly hard using standard C++
----------------------------------------------------

Individuals using Visual Studio 2015, or on Windows with the VC++ and MinGW compilers (possibly Clang++ on Windows as well) have ``<codecvt>`` headers, and thusly Sol will attempt to include it. Individuals on GC 4.9.x, Clang 3.5.x, Clang 3.6.x do not seem to have ``<codecvt>`` shipped with the standard library that comes with installation of these compilers. If you want ``std::wstring``, ``std::u16string``, ``std::u32string`` automatic handling then you need to make sure you have those headers and then define ``SOL_CODECVT_SUPPORT``.

ThePhD did not want this to have to be a thing, but slow implementations and such force their hand. When GCC 7.x comes out, ThePhD will consider removing the effect of defining this macro and leaving <codecvt> support in at all times.
