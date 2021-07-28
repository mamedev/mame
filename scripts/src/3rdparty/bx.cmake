# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## BX library objects
##################################################

add_library(bx STATIC EXCLUDE_FROM_ALL)

target_compile_definitions(bx PUBLIC
	__STDC_LIMIT_MACROS
	__STDC_FORMAT_MACROS
	__STDC_CONSTANT_MACROS
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	if (MSVC)
		target_include_directories(bx PUBLIC ${MAME_DIR}/3rdparty/bx/include/compat/msvc)
	else()
		target_include_directories(bx PUBLIC ${MAME_DIR}/3rdparty/bx/include/compat/mingw)
	endif()
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_include_directories(bx PUBLIC ${MAME_DIR}/3rdparty/bx/include/compat/osx)
endif()

if ((${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD"))
	target_include_directories(bx PUBLIC ${MAME_DIR}/3rdparty/bx/include/compat/freebsd)
endif()

target_include_directories(bx
	PUBLIC
		${MAME_DIR}/3rdparty/bx/include
	PRIVATE
		${MAME_DIR}/3rdparty/bx/3rdparty
)

target_sources(bx PRIVATE
	${MAME_DIR}/3rdparty/bx/src/allocator.cpp
	${MAME_DIR}/3rdparty/bx/src/bx.cpp
	${MAME_DIR}/3rdparty/bx/src/commandline.cpp
	${MAME_DIR}/3rdparty/bx/src/crtnone.cpp
	${MAME_DIR}/3rdparty/bx/src/debug.cpp
	${MAME_DIR}/3rdparty/bx/src/dtoa.cpp
	${MAME_DIR}/3rdparty/bx/src/easing.cpp
	${MAME_DIR}/3rdparty/bx/src/file.cpp
	${MAME_DIR}/3rdparty/bx/src/filepath.cpp
	${MAME_DIR}/3rdparty/bx/src/hash.cpp
	${MAME_DIR}/3rdparty/bx/src/math.cpp
	${MAME_DIR}/3rdparty/bx/src/mutex.cpp
	${MAME_DIR}/3rdparty/bx/src/os.cpp
	${MAME_DIR}/3rdparty/bx/src/process.cpp
	${MAME_DIR}/3rdparty/bx/src/semaphore.cpp
	${MAME_DIR}/3rdparty/bx/src/settings.cpp
	${MAME_DIR}/3rdparty/bx/src/sort.cpp
	${MAME_DIR}/3rdparty/bx/src/string.cpp
	${MAME_DIR}/3rdparty/bx/src/thread.cpp
	${MAME_DIR}/3rdparty/bx/src/timer.cpp
	${MAME_DIR}/3rdparty/bx/src/url.cpp
)
