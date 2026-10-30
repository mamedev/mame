# This is a test app to test consuming utf8proc with CMake.
cmake_minimum_required(VERSION 3.16)
project(utf8proc-test)
find_package(utf8proc REQUIRED)
add_executable(app app.c)
target_link_libraries(app utf8proc::utf8proc)
