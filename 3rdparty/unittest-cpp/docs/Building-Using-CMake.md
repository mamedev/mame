While there are some bundled makefiles and projects, UnitTest++ is primarily built and supported using [CMake](http://cmake.org). This guide assumes you have already downloaded and installed CMake, and that you have already downloaded the UnitTest++ source.


In Two Easy Steps
-------------------

Once you've obtained the UnitTest++ source, you can use the empty 'builds' folder as a place to put your cmake-generated project files. The valid "generators" differ per platform; you can run `cmake --help` to see which ones are supported on your platform.

    cd path/to/unittest-cpp/builds
    cmake -G "<Choose a valid generator>" ../
    cmake --build ./

This will build the library and the self-tests, and also run the self-tests.

Then, if you already understand what CMake does (or are just reckless), and you'd like to run the install step:

    cmake --build ./ --target install

This will install the headers and built libs to the `CMAKE_INSTALL_PREFIX`. Note this might require a `sudo` in *nix or an Administrative Command Prompt on Windows depending on your system configuration.
