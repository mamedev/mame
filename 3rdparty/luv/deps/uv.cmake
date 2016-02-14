## Modifications
## Copyright 2014 The Luvit Authors. All Rights Reserved.

## Original Copyright
# Copyright (c) 2014 David Capello
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

include(CheckTypeSize)

cmake_minimum_required(VERSION 2.8.9)

set(LIBUVDIR ${CMAKE_CURRENT_LIST_DIR}/libuv)

include_directories(
  ${LIBUVDIR}/src
  ${LIBUVDIR}/include
)

set(SOURCES
  ${LIBUVDIR}/include/uv.h
  ${LIBUVDIR}/include/tree.h
  ${LIBUVDIR}/include/uv-errno.h
  ${LIBUVDIR}/include/uv-threadpool.h
  ${LIBUVDIR}/include/uv-version.h
  ${LIBUVDIR}/src/fs-poll.c
  ${LIBUVDIR}/src/heap-inl.h
  ${LIBUVDIR}/src/inet.c
  ${LIBUVDIR}/src/queue.h
  ${LIBUVDIR}/src/threadpool.c
  ${LIBUVDIR}/src/uv-common.c
  ${LIBUVDIR}/src/uv-common.h
  ${LIBUVDIR}/src/version.c
)

if(WIN32)
  add_definitions(
    -D_WIN32_WINNT=0x0600
    -D_CRT_SECURE_NO_WARNINGS
    -D_GNU_SOURCE
  )
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/include/uv-win.h
    ${LIBUVDIR}/src/win/async.c
    ${LIBUVDIR}/src/win/atomicops-inl.h
    ${LIBUVDIR}/src/win/core.c
    ${LIBUVDIR}/src/win/dl.c
    ${LIBUVDIR}/src/win/error.c
    ${LIBUVDIR}/src/win/fs.c
    ${LIBUVDIR}/src/win/fs-event.c
    ${LIBUVDIR}/src/win/getaddrinfo.c
    ${LIBUVDIR}/src/win/getnameinfo.c
    ${LIBUVDIR}/src/win/handle.c
    ${LIBUVDIR}/src/win/handle-inl.h
    ${LIBUVDIR}/src/win/internal.h
    ${LIBUVDIR}/src/win/loop-watcher.c
    ${LIBUVDIR}/src/win/pipe.c
    ${LIBUVDIR}/src/win/thread.c
    ${LIBUVDIR}/src/win/poll.c
    ${LIBUVDIR}/src/win/process.c
    ${LIBUVDIR}/src/win/process-stdio.c
    ${LIBUVDIR}/src/win/req.c
    ${LIBUVDIR}/src/win/req-inl.h
    ${LIBUVDIR}/src/win/signal.c
    ${LIBUVDIR}/src/win/snprintf.c
    ${LIBUVDIR}/src/win/stream.c
    ${LIBUVDIR}/src/win/stream-inl.h
    ${LIBUVDIR}/src/win/tcp.c
    ${LIBUVDIR}/src/win/tty.c
    ${LIBUVDIR}/src/win/timer.c
    ${LIBUVDIR}/src/win/udp.c
    ${LIBUVDIR}/src/win/util.c
    ${LIBUVDIR}/src/win/winapi.c
    ${LIBUVDIR}/src/win/winapi.h
    ${LIBUVDIR}/src/win/winsock.c
    ${LIBUVDIR}/src/win/winsock.h
  )
else()
  include_directories(${LIBUVDIR}/src/unix)
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/include/uv-unix.h
    ${LIBUVDIR}/include/uv-linux.h
    ${LIBUVDIR}/include/uv-sunos.h
    ${LIBUVDIR}/include/uv-darwin.h
    ${LIBUVDIR}/include/uv-bsd.h
    ${LIBUVDIR}/include/uv-aix.h
    ${LIBUVDIR}/src/unix/async.c
    ${LIBUVDIR}/src/unix/atomic-ops.h
    ${LIBUVDIR}/src/unix/core.c
    ${LIBUVDIR}/src/unix/dl.c
    ${LIBUVDIR}/src/unix/fs.c
    ${LIBUVDIR}/src/unix/getaddrinfo.c
    ${LIBUVDIR}/src/unix/getnameinfo.c
    ${LIBUVDIR}/src/unix/internal.h
    ${LIBUVDIR}/src/unix/loop.c
    ${LIBUVDIR}/src/unix/loop-watcher.c
    ${LIBUVDIR}/src/unix/pipe.c
    ${LIBUVDIR}/src/unix/poll.c
    ${LIBUVDIR}/src/unix/process.c
    ${LIBUVDIR}/src/unix/signal.c
    ${LIBUVDIR}/src/unix/spinlock.h
    ${LIBUVDIR}/src/unix/stream.c
    ${LIBUVDIR}/src/unix/tcp.c
    ${LIBUVDIR}/src/unix/thread.c
    ${LIBUVDIR}/src/unix/timer.c
    ${LIBUVDIR}/src/unix/tty.c
    ${LIBUVDIR}/src/unix/udp.c
  )
endif()

check_type_size("void*" SIZEOF_VOID_P)
if(SIZEOF_VOID_P EQUAL 8)
  add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif()

## Freebsd
if("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/kqueue.c
    ${LIBUVDIR}/src/unix/freebsd.c
  )
endif()

## Linux
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  add_definitions(
    -D_GNU_SOURCE
  )
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/proctitle.c
    ${LIBUVDIR}/src/unix/linux-core.c
    ${LIBUVDIR}/src/unix/linux-inotify.c
    ${LIBUVDIR}/src/unix/linux-syscalls.c
    ${LIBUVDIR}/src/unix/linux-syscalls.h
  )
endif()

## SunOS
if("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
  add_definitions(
    -D__EXTENSIONS__
    -D_XOPEN_SOURCE=500
  )
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/sunos.c
  )
endif()

## Darwin
if(APPLE)
  add_definitions(
    -D=_DARWIN_USE_64_BIT_INODE
  )
  set(SOURCES ${SOURCES}
    ${LIBUVDIR}/src/unix/proctitle.c
    ${LIBUVDIR}/src/unix/darwin.c
    ${LIBUVDIR}/src/unix/fsevents.c
    ${LIBUVDIR}/src/unix/darwin-proctitle.c
    ${LIBUVDIR}/src/unix/kqueue.c
  )
endif()

add_library(uv STATIC ${SOURCES})
set_property(TARGET uv PROPERTY POSITION_INDEPENDENT_CODE ON)

if("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
  target_link_libraries(uv
    pthread
    kvm
  )
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  target_link_libraries(uv
    pthread
  )
endif()

if(WIN32)
  target_link_libraries(uv
    ws2_32.lib
    shell32.lib
    psapi.lib
    iphlpapi.lib
    advapi32.lib
    Userenv.lib
  )
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
  target_link_libraries(uv
    kstat
    socket
    sendfile
  )
endif()

if(APPLE)
  find_library(FOUNDATION_LIBRARY Foundation)
  find_library(CORESERVICES_LIBRARY CoreServices)
  find_library(APPLICATION_SERVICES_LIBRARY ApplicationServices)
  target_link_libraries(uv
    ${FOUNDATION_LIBRARY}
    ${CORESERVICES_LIBRARY}
    ${APPLICATION_SERVICES_LIBRARY}
  )
endif()
