# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## expat library objects
##################################################

add_library(expat STATIC EXCLUDE_FROM_ALL)

# fake out the enough of expat_config.h to get by
# could possibly add more defines here for specific targets
target_compile_definitions(expat PRIVATE
	HAVE_MEMMOVE
	HAVE_STDINT_H
	HAVE_STDLIB_H
	HAVE_STRING_H
	PACKAGE="expat"
	PACKAGE_BUGREPORT="expat-bugs@libexpat.org"
	PACKAGE_NAME="expat"
	PACKAGE_STRING="expat 2.2.10"
	PACKAGE_TARNAME="expat"
	PACKAGE_URL=""
	PACKAGE_VERSION="2.2.10"
	STDC_HEADERS
	VERSION="2.2.10"
	XML_CONTEXT_BYTES=1024
	XML_DTD
	XML_NS)

if(BIGENDIAN)
	target_compile_definitions(expat PRIVATE BYTEORDER=4321 WORDS_BIGENDIAN)
else()
	target_compile_definitions(expat PRIVATE BYTEORDER=1234)
endif()

if((${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD"))
	target_compile_definitions(expat PRIVATE HAVE_ARC4RANDOM)
endif()

if(UNIX)
	target_compile_definitions(expat PRIVATE
		HAVE_DLFCN_H
		HAVE_FCNTL_H
		HAVE_MMAP
		HAVE_SYS_STAT_H
		HAVE_SYS_TYPES_H
		HAVE_UNISTD_H
		XML_DEV_URANDOM
	)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(expat PRIVATE /wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	target_compile_options(expat PRIVATE /wd4127) # warning C4127: conditional expression is constant
	target_compile_options(expat PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	target_compile_options(expat PRIVATE /wd4456) # warning C4456: declaration of 'xxx' hides previous local declaration
endif()

target_sources(expat PRIVATE
	${MAME_DIR}/3rdparty/expat/lib/xmlparse.c
	${MAME_DIR}/3rdparty/expat/lib/xmlrole.c
	${MAME_DIR}/3rdparty/expat/lib/xmltok.c
)
