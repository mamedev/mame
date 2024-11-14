# Copyright  2008 Matthias Kretz <kretz@kde.org>
# Copyright 2009 Marcus Hufgard <Marcus.Hufgard@hufgard.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# SPDX-FileCopyrightText: 2008 Matthias Kretz <kretz@kde.org>
# SPDX-FileCopyrightText: 2009 Marcus Hufgard <Marcus.Hufgard@hufgard.de>
#
# SPDX-License-Identifier: BSD-3-Clause

#.rst:
# FindPulseAudio
# --------------
#
# This is base on
# https://invent.kde.org/frameworks/extra-cmake-modules/-/blob/master/find-modules/FindPulseAudio.cmake
#
# Try to locate the PulseAudio library.
# If found, this will define the following variables:
#
# ``PulseAudio_FOUND``
#      True if the system has the PulseAudio library of at least
#      the minimum version specified by either the version parameter
#      to find_package() or the variable PulseAudio_MINIMUM_VERSION
# ``PulseAudio_INCLUDE_DIRS``
#      The PulseAudio include directory
# ``PulseAudio_LIBRARIES``
#      The PulseAudio libraries for linking
# ``PulseAudio_MAINLOOP_LIBRARY``
#      The libraries needed to use PulseAudio Mainloop
# ``PulseAudio_VERSION``
#      The version of PulseAudio that was found
# ``PulseAudio_INCLUDE_DIR``
#     Deprecated, use ``PulseAudio_INCLUDE_DIRS``
# ``PulseAudio_LIBRARY``
#     Deprecated, use ``PulseAudio_LIBRARIES``
#
# If ``PulseAudio_FOUND`` is TRUE, it will also define the following
# imported target:
#
# ``PulseAudio::PulseAudio``
#     The PulseAudio library
#
# Since 5.41.0.

# Support PulseAudio_MINIMUM_VERSION for compatibility:
if(NOT PulseAudio_FIND_VERSION)
  set(PulseAudio_FIND_VERSION "${PulseAudio_MINIMUM_VERSION}")
endif()

# the minimum version of PulseAudio we require
if(NOT PulseAudio_FIND_VERSION)
  set(PulseAudio_FIND_VERSION "1.0.0")
endif()

find_package(PkgConfig)
pkg_check_modules(PC_PulseAudio QUIET libpulse>=${PulseAudio_FIND_VERSION})
pkg_check_modules(PC_PulseAudio_MAINLOOP QUIET libpulse-mainloop-glib)

find_path(PulseAudio_INCLUDE_DIRS pulse/pulseaudio.h
  HINTS
  ${PC_PulseAudio_INCLUDEDIR}
  ${PC_PulseAudio_INCLUDE_DIRS}
  )

find_library(PulseAudio_LIBRARIES NAMES pulse libpulse
  HINTS
  ${PC_PulseAudio_LIBDIR}
  ${PC_PulseAudio_LIBRARY_DIRS}
  )

find_library(PulseAudio_MAINLOOP_LIBRARY
  NAMES pulse-mainloop pulse-mainloop-glib libpulse-mainloop-glib
  HINTS
  ${PC_PulseAudio_LIBDIR}
  ${PC_PulseAudio_LIBRARY_DIRS}
  )

# Store the version number in the cache,
# so we don't have to search every time again:
if(PulseAudio_INCLUDE_DIRS AND NOT PulseAudio_VERSION)

  # get PulseAudio's version from its version.h
  file(STRINGS "${PulseAudio_INCLUDE_DIRS}/pulse/version.h" pulse_version_h
    REGEX ".*pa_get_headers_version\\(\\).*")
  string(REGEX REPLACE ".*pa_get_headers_version\\(\\)\ \\(\"([0-9]+\\.[0-9]+\\.[0-9]+)[^\"]*\"\\).*" "\\1"
    _PulseAudio_VERSION "${pulse_version_h}")

  set(PulseAudio_VERSION "${_PulseAudio_VERSION}"
    CACHE STRING "Version number of PulseAudio"
    FORCE)
endif()

# Use the new extended syntax of
# find_package_handle_standard_args(),
# which also handles version checking:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PulseAudio
  REQUIRED_VARS PulseAudio_LIBRARIES
  PulseAudio_INCLUDE_DIRS
  VERSION_VAR PulseAudio_VERSION)

# Deprecated synonyms
set(PulseAudio_INCLUDE_DIR "${PulseAudio_INCLUDE_DIRS}")
set(PulseAudio_LIBRARY "${PulseAudio_LIBRARIES}")
set(PulseAudio_MAINLOOP_LIBRARY "${PulseAudio_MAINLOOP_LIBRARY}")
set(PulseAudio_FOUND "${PulseAudio_FOUND}")

if(PulseAudio_FOUND AND NOT TARGET PulseAudio::PulseAudio)
  add_library(PulseAudio::PulseAudio UNKNOWN IMPORTED)
  set_target_properties(PulseAudio::PulseAudio PROPERTIES
    IMPORTED_LOCATION "${PulseAudio_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${PulseAudio_INCLUDE_DIRS}")
endif()

mark_as_advanced(PulseAudio_INCLUDE_DIRS PulseAudio_INCLUDE_DIR
  PulseAudio_LIBRARIES PulseAudio_LIBRARY
  PulseAudio_MAINLOOP_LIBRARY PulseAudio_MAINLOOP_LIBRARY)

include(FeatureSummary)
set_package_properties(PulseAudio PROPERTIES
  URL "https://www.freedesktop.org/wiki/Software/PulseAudio"
  DESCRIPTION "Sound server, for sound stream routing and mixing")
