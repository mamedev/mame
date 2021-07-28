# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   devices.cmake
##
##   Rules for building device cores
##
##########################################################################

include(scripts/src/cpu.cmake)
include(scripts/src/sound.cmake)
include(scripts/src/video.cmake)
include(scripts/src/machine.cmake)
include(scripts/src/bus.cmake)

add_library(optional ${LIBTYPE} ${CPU_SRCS} ${SOUND_SRCS} ${VIDEO_SRCS} ${MACHINE_SRCS} ${BUS_SRCS})
add_project_to_group(devices optional)
addprojectflags(optional)
precompiledheaders(optional)

target_include_directories(optional PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/devices
	${MAME_DIR}/src/mame # used for sound amiga
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/3rdparty
	${GEN_DIR}/emu
	${GEN_DIR}/emu/layout
	${EXT_INCLUDEDIR_ASIO}
	${EXT_INCLUDEDIR_EXPAT}
	${EXT_INCLUDEDIR_FLAC}
)
if(NOT FORCE_DRC_C_BACKEND)
	target_link_libraries(optional PRIVATE asmjit)
endif()

if("NETLIST" IN_LIST MACHINES)
	target_link_libraries(optional PRIVATE netlist)
endif()

target_link_libraries(optional PRIVATE
	softfloat
	softfloat3
	ymfm
)

add_library(dasm ${LIBTYPE} ${DASM_SRCS})
add_project_to_group(devices dasm)
addprojectflags(dasm)
precompiledheaders(dasm)

target_include_directories(dasm PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/emu
	${MAME_DIR}/src/devices
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/3rdparty
	${GEN_DIR}/emu
	${EXT_INCLUDEDIR_ASIO}
	${EXT_INCLUDEDIR_EXPAT}
)
