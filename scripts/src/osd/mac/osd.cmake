# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   mac.cmake
##
##   Rules for the building with SDL
##
##########################################################################

########################
# maintargetosdoptions
########################

macro(maintargetosdoptions _projectname)
	osdmodulestargetconf(${_projectname})
endmacro()

########################
# qtdbg_mac library
########################

qtdebuggerbuild(qtdbg_${OSD})
osd_cfg(qtdbg_${OSD})

########################
# osd_mac library
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
	${MAME_DIR}/src/osd/mac
)

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

	${MAME_DIR}/src/osd/mac/main.mm
	${MAME_DIR}/src/osd/mac/macmain.cpp
	${MAME_DIR}/src/osd/mac/appdelegate.mm
	${MAME_DIR}/src/osd/mac/appdelegate.h
	${MAME_DIR}/src/osd/mac/video.cpp
	${MAME_DIR}/src/osd/mac/window.cpp
	${MAME_DIR}/src/osd/mac/window.h
	${MAME_DIR}/src/osd/mac/windowcontroller.mm
	${MAME_DIR}/src/osd/mac/windowcontroller.h
	${MAME_DIR}/src/osd/mac/mamefswindow.mm
	${MAME_DIR}/src/osd/mac/mamefswindow.h
	${MAME_DIR}/src/osd/mac/oglview.mm
	${MAME_DIR}/src/osd/mac/oglview.h
	${MAME_DIR}/src/osd/modules/osdwindow.cpp
	${MAME_DIR}/src/osd/modules/osdwindow.h
)

########################
# ocore_mac library
########################

add_library(ocore_${OSD} ${LIBTYPE})
osd_cfg(ocore_${OSD})

target_include_directories(ocore_${OSD} PRIVATE
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/src/osd/mac
)

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
	${MAME_DIR}/src/osd/modules/lib/osdlib_macosx.cpp
	${MAME_DIR}/src/osd/modules/lib/osdlib.h
	${MAME_DIR}/src/osd/modules/file/posixdir.cpp
	${MAME_DIR}/src/osd/modules/file/posixdomain.cpp
	${MAME_DIR}/src/osd/modules/file/posixfile.cpp
	${MAME_DIR}/src/osd/modules/file/posixfile.h
	${MAME_DIR}/src/osd/modules/file/posixptty.cpp
	${MAME_DIR}/src/osd/modules/file/posixsocket.cpp
)

target_link_libraries(ocore_${OSD} PUBLIC
	"-framework Cocoa"
	"-framework QuartzCore"
	"-framework OpenGL"
	"-weak_framework Metal"
)
