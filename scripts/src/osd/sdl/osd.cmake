# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   sdl.cmake
##
##   Rules for the building with SDL
##
##########################################################################

########################
#       OPTIONS
########################

set(SDL_INI_PATH "" CACHE STRING "Default search path for .ini files.")

if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Haiku") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"))
	set(NO_X11_DEFAULT ON)
else()
	set(NO_X11_DEFAULT OFF)
endif()
option(NO_X11 "Disable use of X11" ${NO_X11_DEFAULT})

if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Haiku") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"))
	set(NO_USE_XINPUT_DEFAULT ON)
else()
	set(NO_USE_XINPUT_DEFAULT OFF)
endif()
option(NO_USE_XINPUT "Disable use of Xinput" ${NO_USE_XINPUT_DEFAULT})

option(NO_USE_XINPUT_WII_LIGHTGUN_HACK "Disable use of Xinput Wii Lightgun Hack" ON)

########################
# Setup
########################

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	if (WITH_SYSTEM_SDL2)
		find_package(SDL2 REQUIRED)
		if(NOT SDL2_FOUND)
			message(FATAL_ERROR "SDL2 not found")
		endif()
		set(EXTLIB_SDL2_LIBRARY SDL2::Core)
	else()
		set(EXTLIB_SDL2_LIBRARY SDL2)
	endif()
else()
	set(EXTLIB_SDL2_LIBRARY SDL2)
endif()

set(BASE_TARGETOS "unix")
set(SDLOS_TARGETOS "unix")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(BASE_TARGETOS "win32")
	set(SDLOS_TARGETOS "win32")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(SDLOS_TARGETOS "macosx")
endif()

if (${BASE_TARGETOS} STREQUAL "unix")
	if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		if (NO_X11)
			set_option(USE_QTDEBUG OFF)
			set_option(NO_USE_XINPUT ON)
		endif()
	endif()
endif()

if(NOT NO_X11)
	find_package(X11 REQUIRED)
	if(NOT X11_FOUND)
		message(FATAL_ERROR "X11 not found")
	endif()
	if(NOT X11_Xinerama_FOUND)
		message(FATAL_ERROR "X11 Xinerama not found")
	endif()

	if(NOT NO_USE_XINPUT)
		if(NOT X11_Xext_FOUND)
			message(FATAL_ERROR "X11 Xext not found")
		endif()
		if(NOT X11_Xi_FOUND)
			message(FATAL_ERROR "X11 Xi not found")
		endif()
	endif()
endif()

########################
# qtdbg_sdl library
########################

qtdebuggerbuild(qtdbg_${OSD})
osd_cfg(qtdbg_${OSD})

########################
# osd_sdl library
########################

add_library(osd_${OSD} ${LIBTYPE})

osdmodulesbuild(osd_${OSD})
osd_cfg(osd_${OSD})

target_include_directories(osd_${OSD} PRIVATE
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/devices # accessing imagedev from debugger
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/src/osd/modules/file
	${MAME_DIR}/src/osd/modules/render
	${MAME_DIR}/3rdparty
	${MAME_DIR}/src/osd/sdl
)

target_link_libraries(osd_${OSD} PRIVATE ${EXTLIB_SDL2_LIBRARY})

target_sources(osd_${OSD} PRIVATE
	${MAME_DIR}/src/osd/sdl/osdsdl.h
	${MAME_DIR}/src/osd/sdl/sdlprefix.h
	${MAME_DIR}/src/osd/sdl/sdlmain.cpp
	${MAME_DIR}/src/osd/osdepend.h
	${MAME_DIR}/src/osd/sdl/video.cpp
	${MAME_DIR}/src/osd/sdl/window.cpp
	${MAME_DIR}/src/osd/sdl/window.h
	${MAME_DIR}/src/osd/modules/osdwindow.cpp
	${MAME_DIR}/src/osd/modules/osdwindow.h
	${MAME_DIR}/src/osd/modules/render/drawsdl.cpp
	${MAME_DIR}/src/osd/modules/render/draw13.cpp
	${MAME_DIR}/src/osd/modules/render/blit13.h
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(osd_${OSD} PRIVATE ${MAME_DIR}/src/osd/windows/main.cpp)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_sources(osd_${OSD} PRIVATE
		${MAME_DIR}/src/osd/modules/debugger/debugosx.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/breakpointsview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/breakpointsview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/consoleview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/consoleview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/debugcommandhistory.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/debugcommandhistory.h
		${MAME_DIR}/src/osd/modules/debugger/osx/debugconsole.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/debugconsole.h
		${MAME_DIR}/src/osd/modules/debugger/osx/debugview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/debugview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/debugwindowhandler.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/debugwindowhandler.h
		${MAME_DIR}/src/osd/modules/debugger/osx/deviceinfoviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/deviceinfoviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/devicesviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/devicesviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/disassemblyview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/disassemblyviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/disassemblyviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/errorlogview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/errorlogview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/disassemblyview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/errorlogviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/errorlogviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/memoryview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/memoryview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/memoryviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/memoryviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/pointsviewer.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/pointsviewer.h
		${MAME_DIR}/src/osd/modules/debugger/osx/registersview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/registersview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/watchpointsview.mm
		${MAME_DIR}/src/osd/modules/debugger/osx/watchpointsview.h
		${MAME_DIR}/src/osd/modules/debugger/osx/debugosx.h
	)
endif()

########################
# ocore_sdl library
########################

add_library(ocore_${OSD} ${LIBTYPE})

target_include_directories(ocore_${OSD} PRIVATE
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/src/osd/sdl
)

if(${BASE_TARGETOS} STREQUAL "win32")
	target_include_directories(ocore_${OSD} PRIVATE
		${MAME_DIR}/src/osd/windows
	)
endif()

if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	target_link_libraries(ocore_${OSD} PUBLIC dl)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	target_link_libraries(ocore_${OSD} PUBLIC android log)
else()
	if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_link_libraries(ocore_${OSD} PUBLIC pthread)
	endif()
endif()

if (NOT MINGW)
	target_link_libraries(ocore_${OSD} PUBLIC ${EXTLIB_SDL2_LIBRARY})
else()
	target_link_libraries(ocore_${OSD} PUBLIC mingw32)
	if (WITH_SYSTEM_SDL2)
		target_link_libraries(ocore_${OSD} PUBLIC SDL2::Main SDL2::Core)
	else()
		target_link_libraries(ocore_${OSD} PUBLIC SDL2)
	endif()
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(ocore_${OSD} PUBLIC
		comctl32
		comdlg32
		psapi
		ole32
		shlwapi
	)
	target_link_libraries(ocore_${OSD} PUBLIC
		utils
		wsock32
		ws2_32
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_link_libraries(ocore_${OSD} PUBLIC "-framework Carbon")
endif()

target_sources(ocore_${OSD} PRIVATE
	${MAME_DIR}/src/osd/osdcore.cpp
	${MAME_DIR}/src/osd/osdcore.h
	${MAME_DIR}/src/osd/osdfile.h
	${MAME_DIR}/src/osd/strconv.cpp
	${MAME_DIR}/src/osd/strconv.h
	${MAME_DIR}/src/osd/osdsync.cpp
	${MAME_DIR}/src/osd/osdsync.h
	${MAME_DIR}/src/osd/modules/osdmodule.cpp
	${MAME_DIR}/src/osd/modules/osdmodule.h
	${MAME_DIR}/src/osd/modules/lib/osdlib_${SDLOS_TARGETOS}.cpp
	${MAME_DIR}/src/osd/modules/lib/osdlib.h
)

if(${BASE_TARGETOS} STREQUAL "unix")
	target_sources(ocore_${OSD} PRIVATE
		${MAME_DIR}/src/osd/modules/file/posixdir.cpp
		${MAME_DIR}/src/osd/modules/file/posixfile.cpp
		${MAME_DIR}/src/osd/modules/file/posixfile.h
		${MAME_DIR}/src/osd/modules/file/posixptty.cpp
		${MAME_DIR}/src/osd/modules/file/posixsocket.cpp
	)
elseif(${BASE_TARGETOS} STREQUAL "win32")
	target_sources(ocore_${OSD} PRIVATE
		${MAME_DIR}/src/osd/modules/file/windir.cpp
		${MAME_DIR}/src/osd/modules/file/winfile.cpp
		${MAME_DIR}/src/osd/modules/file/winfile.h
		${MAME_DIR}/src/osd/modules/file/winptty.cpp
		${MAME_DIR}/src/osd/modules/file/winsocket.cpp
		${MAME_DIR}/src/osd/windows/winutil.cpp # FIXME put the necessary functions somewhere more appropriate
	)
else()
	target_sources(ocore_${OSD} PRIVATE
		${MAME_DIR}/src/osd/modules/file/stdfile.cpp
	)
endif()

osd_cfg(ocore_${OSD})

########################
# maintargetosdoptions
########################

macro(maintargetosdoptions _projectname)
	osdmodulestargetconf(${_projectname})

	if(NOT NO_X11)
		target_link_libraries(${_projectname} PRIVATE
			${X11_X11_LIB}
			${X11_Xinerama_LIB}
		)

		if(NOT NO_USE_XINPUT)
			target_link_libraries(${_projectname} PRIVATE
				${X11_Xext_LIB}
				${X11_Xi_LIB}
			)
		endif()
	else()
		if((${CMAKE_SYSTEM_NAME} STREQUAL "Linux") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD"))
			target_link_libraries(${_projectname} PRIVATE OpenGL::EGL)
		endif()
	endif()


	if((${BASE_TARGETOS} STREQUAL "unix") AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android") AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"))
		find_package(SDL2_ttf REQUIRED)
		find_package(Fontconfig REQUIRED)
		if(NOT SDL2_ttf_FOUND)
			message(FATAL_ERROR "SDL2_ttf not found")
		endif()
		if(NOT Fontconfig_FOUND)
			message(FATAL_ERROR "Fontconfig not found")
		endif()
		target_link_libraries(${_projectname} PRIVATE SDL2::TTF)
		target_link_libraries(${_projectname} PRIVATE Fontconfig::Fontconfig)
	endif()
endmacro()

