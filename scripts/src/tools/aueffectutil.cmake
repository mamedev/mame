# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## aueffectutil
##########################################################################

add_executable(aueffectutil)

target_link_libraries(aueffectutil PRIVATE
	"-framework AudioUnit"
	"-framework AudioToolbox"
	"-framework CoreAudio"
	"-framework CoreAudioKit"
	"-framework CoreServices"
	"-framework AppKit"
)

target_link_options(aueffectutil PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,${MAME_DIR}/src/tools/aueffectutil-Info.plist)

target_sources(aueffectutil PRIVATE
	${MAME_DIR}/src/tools/aueffectutil.mm
)

strip_executable(aueffectutil)
minimal_symbols(aueffectutil)
