#.rst:
# FindUtf8Proc
# --------
#
# Find utf8proc
#
# Find the UTF-8 processing library
#
# ::
#
#   This module defines the following variables:
#      UTF8PROC_FOUND       - True if UTF8PROC_INCLUDE_DIR & UTF8PROC_LIBRARY are found
#      UTF8PROC_LIBRARIES   - Set when UTF8PROC_LIBRARY is found
#      UTF8PROC_INCLUDE_DIRS - Set when UTF8PROC_INCLUDE_DIR is found
#
#
#
# ::
#
#      UTF8PROC_INCLUDE_DIR - where to find utf8proc.h
#      UTF8PROC_LIBRARY     - the utf8proc library

#=============================================================================
# This module is adapted from FindALSA.cmake. Below are the original license
# header.
#=============================================================================
# Copyright 2009-2011 Kitware, Inc.
# Copyright 2009-2011 Philip Lowman <philip@yhbt.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_path(
	UTF8PROC_INCLUDE_DIR NAMES utf8proc.h DOC "The utf8proc include directory"
)

find_library(
	UTF8PROC_LIBRARY NAMES utf8proc DOC "The utf8proc library"
)

# handle the QUIETLY and REQUIRED arguments and set UTF8PROC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	Utf8Proc
	FOUND_VAR Utf8Proc_FOUND
	REQUIRED_VARS UTF8PROC_LIBRARY UTF8PROC_INCLUDE_DIR
)

if(Utf8Proc_FOUND)
  set( UTF8PROC_LIBRARIES ${UTF8PROC_LIBRARY} )
  set( UTF8PROC_INCLUDE_DIRS ${UTF8PROC_INCLUDE_DIR} )
endif()

mark_as_advanced(UTF8PROC_INCLUDE_DIR UTF8PROC_LIBRARY)
