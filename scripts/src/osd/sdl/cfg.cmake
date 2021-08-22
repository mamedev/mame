# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

macro(osd_cfg _project)
	add_project_to_group(libs ${_project})

	if(MSVC)
		target_compile_options(${_project} PRIVATE "/FI\"${MAME_DIR}/src/osd/sdl/sdlprefix.h\"")
	else()
		# GCC or Clang
		target_compile_options(${_project} PRIVATE "SHELL:-include ${MAME_DIR}/src/osd/sdl/sdlprefix.h")
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
		target_compile_options(${_project} PRIVATE "SHELL:-s USE_SDL=2")
		target_compile_options(${_project} PRIVATE "SHELL:-s USE_SDL_TTF=2")
	endif()

	if(USE_TAPTUN OR USE_PCAP)
		target_compile_definitions(${_project} PRIVATE USE_NETWORK)
		if (USE_TAPTUN)
			target_compile_definitions(${_project} PRIVATE OSD_NET_USE_TAPTUN)
		endif()
		if (USE_PCAP)
			target_compile_definitions(${_project} PRIVATE OSD_NET_USE_PCAP)
		endif()
	endif()

	if(NOT SDL_INI_PATH STREQUAL "")
		target_compile_definitions(${_project} PRIVATE INI_PATH="${SDL_INI_PATH}")
	endif()

	if(NO_X11)
		target_compile_definitions(${_project} PRIVATE SDLMAME_NO_X11)
	else()
		target_compile_definitions(${_project} PRIVATE SDLMAME_X11)
		target_include_directories(${_project} PRIVATE ${X11_INCLUDE_DIR})
	endif()

	if(NO_USE_XINPUT)
		target_compile_definitions(${_project} PRIVATE USE_XINPUT=0)
	else()
		target_compile_definitions(${_project} PRIVATE
			USE_XINPUT=1
			USE_XINPUT_DEBUG=0
		)
	endif()

	if(NO_USE_XINPUT_WII_LIGHTGUN_HACK)
		target_compile_definitions(${_project} PRIVATE USE_XINPUT_WII_LIGHTGUN_HACK=0)
	else()
		target_compile_definitions(${_project} PRIVATE USE_XINPUT_WII_LIGHTGUN_HACK=1)
	endif()

	target_compile_definitions(${_project} PRIVATE SDLMAME_SDL2=1)

	target_compile_definitions(${_project} PRIVATE OSD_SDL)

	if(${BASE_TARGETOS} STREQUAL "unix")
		target_compile_definitions(${_project} PRIVATE SDLMAME_UNIX)
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		target_compile_definitions(${_project} PRIVATE
			WIN32
			_WIN32
			UNICODE
			_UNICODE
			_WIN32_WINNT=0x0501
			WIN32_LEAN_AND_MEAN
			NOMINMAX
		)
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		target_include_directories(${_project} PRIVATE ${MAME_DIR}/3rdparty/bx/include/compat/osx)
		target_compile_definitions(${_project} PRIVATE SDLMAME_MACOSX SDLMAME_DARWIN)
	endif()
	if((${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD"))
		# /usr/local/include is not considered a system include directory on FreeBSD. GL.h resides there.
		target_compile_options(${_project} PRIVATE -isystem /usr/local/include)
		target_include_directories(${_project} PRIVATE ${MAME_DIR}/3rdparty/bx/include/compat/freebsd)
	endif()
endmacro()
