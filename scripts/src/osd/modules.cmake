# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   modules.cmake
##
##   Rules for the building of modules
##
##########################################################################

########################
#       OPTIONS
########################

if((${CMAKE_SYSTEM_NAME} STREQUAL "Linux") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	set(USE_TAPTUN_DEFAULT ON)
else()
	set(USE_TAPTUN_DEFAULT OFF)
endif()
option(USE_TAPTUN "Include tap/tun network module." ${USE_TAPTUN_DEFAULT})

if((${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD"))
	set(USE_PCAP_DEFAULT ON)
else()
	set(USE_PCAP_DEFAULT OFF)
endif()
option(USE_PCAP "Include pcap network module." ${USE_PCAP_DEFAULT})

option(NO_OPENGL "Disable use of OpenGL." OFF)

option(USE_DISPATCH_GL "Use GL-dispatching." OFF)

if((${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")) # Also OpenBSD, Solaris and Haiku
	set(NO_USE_MIDI_DEFAULT ON)
else()
	set(NO_USE_MIDI_DEFAULT OFF)
endif()
option(NO_USE_MIDI "Disable MIDI I/O." ${NO_USE_MIDI_DEFAULT})

if((${CMAKE_SYSTEM_NAME} STREQUAL "Linux") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Windows") OR (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin"))
	set(NO_USE_PORTAUDIO_DEFAULT OFF)
else()
	set(NO_USE_PORTAUDIO_DEFAULT ON)
endif()
option(NO_USE_PORTAUDIO "Disable PortAudio interface." ${NO_USE_PORTAUDIO_DEFAULT})

set(NO_USE_PULSEAUDIO_DEFAULT OFF)
if (NOT(${CMAKE_SYSTEM_NAME} STREQUAL "Linux"))
	set(NO_USE_PULSEAUDIO_DEFAULT ON)
endif()
option(NO_USE_PULSEAUDIO "Disable PulseAudio interface." ${NO_USE_PULSEAUDIO_DEFAULT})

# OFF - Use classic Windows APIs - allows support for XP and later
# ON  - Use Modern Windows APIs - support for Windows 8.1 and later

option(MODERN_WIN_API "Use Modern Windows APIs." OFF)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(USE_QTDEBUG_DEFAULT ON)
else()
	set(USE_QTDEBUG_DEFAULT OFF)
endif()
option(USE_QTDEBUG "Use QT debugger." ${USE_QTDEBUG_DEFAULT})

########################
# Setup
########################

if ((NOT NO_OPENGL) AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"))
	find_package(OpenGL REQUIRED)
	if(NOT OpenGL_FOUND)
		message(FATAL_ERROR "OpenGL not found")
	endif()
endif()

########################
# osdmodulesbuild
########################

macro(osdmodulesbuild _project)

	target_sources(${_project} PRIVATE
		${MAME_DIR}/src/osd/osdnet.cpp
		${MAME_DIR}/src/osd/osdnet.h
		${MAME_DIR}/src/osd/watchdog.cpp
		${MAME_DIR}/src/osd/watchdog.h
		${MAME_DIR}/src/osd/modules/debugger/debug_module.h
		${MAME_DIR}/src/osd/modules/font/font_module.h
		${MAME_DIR}/src/osd/modules/midi/midi_module.h
		${MAME_DIR}/src/osd/modules/netdev/netdev_module.h
		${MAME_DIR}/src/osd/modules/sound/sound_module.h
		${MAME_DIR}/src/osd/modules/diagnostics/diagnostics_module.h
		${MAME_DIR}/src/osd/modules/monitor/monitor_module.h
		${MAME_DIR}/src/osd/modules/lib/osdobj_common.cpp
		${MAME_DIR}/src/osd/modules/lib/osdobj_common.h
		${MAME_DIR}/src/osd/modules/diagnostics/none.cpp
		${MAME_DIR}/src/osd/modules/diagnostics/diagnostics_win32.cpp
		${MAME_DIR}/src/osd/modules/debugger/none.cpp
		${MAME_DIR}/src/osd/modules/debugger/debugwin.cpp
		${MAME_DIR}/src/osd/modules/debugger/debugimgui.cpp
		${MAME_DIR}/src/osd/modules/debugger/debuggdbstub.cpp
		${MAME_DIR}/src/osd/modules/font/font_sdl.cpp
		${MAME_DIR}/src/osd/modules/font/font_windows.cpp
		${MAME_DIR}/src/osd/modules/font/font_dwrite.cpp
		${MAME_DIR}/src/osd/modules/font/font_osx.cpp
		${MAME_DIR}/src/osd/modules/font/font_none.cpp
		${MAME_DIR}/src/osd/modules/netdev/taptun.cpp
		${MAME_DIR}/src/osd/modules/netdev/pcap.cpp
		${MAME_DIR}/src/osd/modules/netdev/none.cpp
		${MAME_DIR}/src/osd/modules/midi/portmidi.cpp
		${MAME_DIR}/src/osd/modules/midi/none.cpp
		${MAME_DIR}/src/osd/modules/sound/js_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/direct_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/pa_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/pulse_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/coreaudio_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/sdl_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/xaudio2_sound.cpp
		${MAME_DIR}/src/osd/modules/sound/none.cpp
		${MAME_DIR}/src/osd/modules/input/input_module.h
		${MAME_DIR}/src/osd/modules/input/input_common.cpp
		${MAME_DIR}/src/osd/modules/input/input_common.h
		${MAME_DIR}/src/osd/modules/input/input_dinput.cpp
		${MAME_DIR}/src/osd/modules/input/input_dinput.h
		${MAME_DIR}/src/osd/modules/input/input_none.cpp
		${MAME_DIR}/src/osd/modules/input/input_rawinput.cpp
		${MAME_DIR}/src/osd/modules/input/input_win32.cpp
		${MAME_DIR}/src/osd/modules/input/input_sdl.cpp
		${MAME_DIR}/src/osd/modules/input/input_sdlcommon.cpp
		${MAME_DIR}/src/osd/modules/input/input_sdlcommon.h
		${MAME_DIR}/src/osd/modules/input/input_x11.cpp
		${MAME_DIR}/src/osd/modules/input/input_windows.cpp
		${MAME_DIR}/src/osd/modules/input/input_windows.h
		${MAME_DIR}/src/osd/modules/input/input_xinput.cpp
		${MAME_DIR}/src/osd/modules/input/input_xinput.h
		${MAME_DIR}/src/osd/modules/input/input_winhybrid.cpp
		${MAME_DIR}/src/osd/modules/input/input_mac.cpp
		${MAME_DIR}/src/osd/modules/output/output_module.h
		${MAME_DIR}/src/osd/modules/output/none.cpp
		${MAME_DIR}/src/osd/modules/output/console.cpp
		${MAME_DIR}/src/osd/modules/output/network.cpp
		${MAME_DIR}/src/osd/modules/output/win32_output.cpp
		${MAME_DIR}/src/osd/modules/output/win32_output.h
		${MAME_DIR}/src/osd/modules/monitor/monitor_common.h
		${MAME_DIR}/src/osd/modules/monitor/monitor_common.cpp
		${MAME_DIR}/src/osd/modules/monitor/monitor_win32.cpp
		${MAME_DIR}/src/osd/modules/monitor/monitor_dxgi.cpp
		${MAME_DIR}/src/osd/modules/monitor/monitor_sdl.cpp
		${MAME_DIR}/src/osd/modules/monitor/monitor_mac.cpp

		${MAME_DIR}/src/osd/modules/render/drawbgfx.cpp
		${MAME_DIR}/src/osd/modules/render/aviwrite.cpp
		${MAME_DIR}/src/osd/modules/render/aviwrite.h
		${MAME_DIR}/src/osd/modules/render/bgfxutil.cpp
		${MAME_DIR}/src/osd/modules/render/bgfxutil.h
		${MAME_DIR}/src/osd/modules/render/binpacker.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/blendreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/blendreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/chain.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/chain.h
		${MAME_DIR}/src/osd/modules/render/bgfx/chainentry.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/chainentry.h
		${MAME_DIR}/src/osd/modules/render/bgfx/chainentryreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/chainentryreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/chainmanager.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/chainmanager.h
		${MAME_DIR}/src/osd/modules/render/bgfx/chainreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/chainreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/clear.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/clear.h
		${MAME_DIR}/src/osd/modules/render/bgfx/clearreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/clearreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/cullreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/cullreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/depthreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/depthreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/effect.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/effect.h
		${MAME_DIR}/src/osd/modules/render/bgfx/effectmanager.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/effectmanager.h
		${MAME_DIR}/src/osd/modules/render/bgfx/effectreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/effectreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/entryuniformreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/entryuniformreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/inputpair.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/inputpair.h
		${MAME_DIR}/src/osd/modules/render/bgfx/frameparameter.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/frameparameter.h
		${MAME_DIR}/src/osd/modules/render/bgfx/timeparameter.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/timeparameter.h
		${MAME_DIR}/src/osd/modules/render/bgfx/paramreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/paramreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/paramuniform.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/paramuniform.h
		${MAME_DIR}/src/osd/modules/render/bgfx/paramuniformreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/paramuniformreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/shadermanager.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/shadermanager.h
		${MAME_DIR}/src/osd/modules/render/bgfx/slider.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/slider.h
		${MAME_DIR}/src/osd/modules/render/bgfx/sliderreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/sliderreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/slideruniform.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/slideruniform.h
		${MAME_DIR}/src/osd/modules/render/bgfx/slideruniformreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/slideruniformreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/statereader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/statereader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/suppressor.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/suppressor.h
		${MAME_DIR}/src/osd/modules/render/bgfx/suppressorreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/suppressorreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/target.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/target.h
		${MAME_DIR}/src/osd/modules/render/bgfx/targetreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/targetreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/targetmanager.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/targetmanager.h
		${MAME_DIR}/src/osd/modules/render/bgfx/texture.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/texture.h
		${MAME_DIR}/src/osd/modules/render/bgfx/texturehandleprovider.h
		${MAME_DIR}/src/osd/modules/render/bgfx/texturemanager.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/texturemanager.h
		${MAME_DIR}/src/osd/modules/render/bgfx/uniform.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/uniform.h
		${MAME_DIR}/src/osd/modules/render/bgfx/uniformreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/uniformreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/valueuniform.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/valueuniform.h
		${MAME_DIR}/src/osd/modules/render/bgfx/valueuniformreader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/valueuniformreader.h
		${MAME_DIR}/src/osd/modules/render/bgfx/view.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/view.h
		${MAME_DIR}/src/osd/modules/render/bgfx/writereader.cpp
		${MAME_DIR}/src/osd/modules/render/bgfx/writereader.h
	)
	if (NOT NO_OPENGL)
		target_sources(${_project} PRIVATE
			${MAME_DIR}/src/osd/modules/render/drawogl.cpp
			${MAME_DIR}/src/osd/modules/opengl/gl_shader_tool.cpp
			${MAME_DIR}/src/osd/modules/opengl/gl_shader_mgr.cpp
			${MAME_DIR}/src/osd/modules/opengl/gl_shader_mgr.h
			${MAME_DIR}/src/osd/modules/opengl/gl_shader_tool.h
			${MAME_DIR}/src/osd/modules/opengl/osd_opengl.h
		)
	endif()

	target_include_directories(${_project} PRIVATE ${EXT_INCLUDEDIR_ASIO})

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		target_include_directories(${_project} PRIVATE
			${MAME_DIR}/3rdparty/compat/mingw
			${MAME_DIR}/3rdparty/portaudio/include
			${MAME_DIR}/3rdparty/compat/winsdk-override
		)
	endif()

	if (NO_OPENGL)
		target_compile_definitions(${_project} PRIVATE USE_OPENGL=0)
	else()
		target_compile_definitions(${_project} PRIVATE USE_OPENGL=1)
	endif()
	if (USE_DISPATCH_GL)
		target_compile_definitions(${_project} PRIVATE USE_DISPATCH_GL=1)
	endif()

	target_compile_definitions(${_project} PRIVATE
		__STDC_LIMIT_MACROS
		__STDC_FORMAT_MACROS
		__STDC_CONSTANT_MACROS
	)

	target_include_directories(${_project} PRIVATE
		${MAME_DIR}/3rdparty/bgfx/examples/common
		${MAME_DIR}/3rdparty/bgfx/include
		${MAME_DIR}/3rdparty/bgfx/3rdparty
		${MAME_DIR}/3rdparty/bgfx/3rdparty/khronos
		${MAME_DIR}/3rdparty/bx/include
	)
	target_link_libraries(${_project} PRIVATE bgfx)

	target_include_directories(${_project} PRIVATE ${EXT_INCLUDEDIR_RAPIDJSON})

	if(NO_USE_PORTAUDIO)
		target_compile_definitions(${_project} PRIVATE NO_USE_PORTAUDIO)
	else()
		target_include_directories(${_project} PRIVATE ${EXT_INCLUDEDIR_PORTAUDIO})
		target_link_libraries(${_project} PRIVATE ${EXT_LIB_PORTAUDIO})
	endif()

	if(NO_USE_PULSEAUDIO)
		target_compile_definitions(${_project} PRIVATE NO_USE_PULSEAUDIO)
	endif()

	if(NO_USE_MIDI)
		target_compile_definitions(${_project} PRIVATE NO_USE_MIDI)
	else()
		target_include_directories(${_project} PRIVATE ${EXT_INCLUDEDIR_PORTMIDI})
		target_link_libraries(${_project} PRIVATE ${EXT_LIB_PORTMIDI})
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		target_link_libraries(${_project} PRIVATE winmm)
	endif()

	if((NOT NO_USE_MIDI) AND (NOT NO_USE_PORTAUDIO))
		if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
			find_package(ALSA REQUIRED)
			if(NOT ALSA_FOUND)
				message(FATAL_ERROR "ALSA not found")
			endif()
			target_link_libraries(${_project} PRIVATE ALSA::ALSA)
		endif()
	endif()

	if(NOT NO_USE_MIDI)
		if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
			target_link_libraries(${_project} PRIVATE "-framework CoreMIDI")
		endif()
	endif()

	if(USE_QTDEBUG)
		target_compile_definitions(${_project} PRIVATE USE_QTDEBUG=1)
	else()
		target_compile_definitions(${_project} PRIVATE USE_QTDEBUG=0)
	endif()
	target_link_libraries(${_project} PRIVATE qtdbg_${OSD})

endmacro()

########################
# qtdebuggerbuild
########################

macro(qtdebuggerbuild _projectname)
	set(QTDEBUGGER_SRCS ${MAME_DIR}/src/osd/modules/debugger/debugqt.cpp)

	if(USE_QTDEBUG)
		list(APPEND QTDEBUGGER_SRCS
			${MAME_DIR}/src/osd/modules/debugger/qt/debuggerview.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/debuggerview.h
			${MAME_DIR}/src/osd/modules/debugger/qt/windowqt.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/windowqt.h
			${MAME_DIR}/src/osd/modules/debugger/qt/logwindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/logwindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/dasmwindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/dasmwindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/mainwindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/mainwindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/memorywindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/memorywindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/breakpointswindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/breakpointswindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/deviceswindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/deviceinformationwindow.cpp
			${MAME_DIR}/src/osd/modules/debugger/qt/deviceinformationwindow.h
			${MAME_DIR}/src/osd/modules/debugger/qt/deviceswindow.h
			${GEN_DIR}/osd/modules/debugger/qt/debuggerview.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/windowqt.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/logwindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/dasmwindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/mainwindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/memorywindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/breakpointswindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/deviceswindow.moc.cpp
			${GEN_DIR}/osd/modules/debugger/qt/deviceinformationwindow.moc.cpp
		)
	endif()

	add_library(${_projectname} ${LIBTYPE} ${QTDEBUGGER_SRCS})
	add_project_to_group(libs ${_projectname})

	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_options(${_projectname} PRIVATE -Wno-inconsistent-missing-override)
	endif()

	if(USE_QTDEBUG)
		target_compile_definitions(${_projectname} PRIVATE USE_QTDEBUG=1)

		find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
		if((NOT Qt5_FOUND) OR (NOT Qt5Core_FOUND) OR (NOT Qt5Widgets_FOUND) OR (NOT Qt5Gui_FOUND))
			message(FATAL_ERROR "Qt5 or its components not found")
		endif()

		target_link_libraries(${_projectname} PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui)

		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/debuggerview.h ${GEN_DIR}/osd/modules/debugger/qt/debuggerview.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/windowqt.h ${GEN_DIR}/osd/modules/debugger/qt/windowqt.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/logwindow.h ${GEN_DIR}/osd/modules/debugger/qt/logwindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/dasmwindow.h ${GEN_DIR}/osd/modules/debugger/qt/dasmwindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/mainwindow.h ${GEN_DIR}/osd/modules/debugger/qt/mainwindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/memorywindow.h ${GEN_DIR}/osd/modules/debugger/qt/memorywindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/breakpointswindow.h ${GEN_DIR}/osd/modules/debugger/qt/breakpointswindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/deviceswindow.h ${GEN_DIR}/osd/modules/debugger/qt/deviceswindow.moc.cpp)
		qt5_generate_moc(${MAME_DIR}/src/osd/modules/debugger/qt/deviceinformationwindow.h ${GEN_DIR}/osd/modules/debugger/qt/deviceinformationwindow.moc.cpp)
	else()
		target_compile_definitions(${_projectname} PRIVATE USE_QTDEBUG=0)
	endif()

	target_include_directories(${_projectname} PRIVATE
			${MAME_DIR}/src/emu
			${MAME_DIR}/src/devices # accessing imagedev from debugger
			${MAME_DIR}/src/osd
			${MAME_DIR}/src/lib
			${MAME_DIR}/src/lib/util
			${MAME_DIR}/src/osd/modules/render
			${MAME_DIR}/3rdparty
	)
endmacro()

########################
# osdmodulestargetconf
########################

macro(osdmodulestargetconf _projectname)

	if(NOT NO_OPENGL)
		if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
			target_link_libraries(${_projectname} PRIVATE OpenGL::GL)
		elseif(NOT USE_DISPATCH_GL)
			if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
				target_link_libraries(${_projectname} PRIVATE OpenGL::GL)
			else()
				if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
					target_link_libraries(${_projectname} PRIVATE OpenGL::GL)
				else()
					target_link_libraries(${_projectname} PRIVATE GL)
				endif()
			endif()
		endif()
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		target_link_libraries(${_projectname} PRIVATE
			gdi32
			dsound
			dxguid
			oleaut32
		)
	elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		target_link_libraries(${_projectname} PRIVATE
			"-framework AudioUnit"
			"-framework AudioToolbox"
			"-framework CoreAudio"
			"-framework CoreServices"
			"-framework Cocoa"
			"-framework QuartzCore"
			"-framework OpenGL"
			"-weak_framework Metal"
		)
	endif()

	if (NOT NO_USE_PULSEAUDIO)
		target_link_libraries(${_projectname} PRIVATE pulse)
	endif()

endmacro()
