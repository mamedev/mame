# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   virtual.cmake
##
##   Virtual target makefile
##
##########################################################################

##################################################
## specify available CPU cores
##################################################

list(APPEND CPUS DSP16) ## for qsound
list(APPEND CPUS H6280)
list(APPEND CPUS KS0164)
list(APPEND CPUS M6502)
list(APPEND CPUS M680X0)
list(APPEND CPUS MCS48)
list(APPEND CPUS SH)
list(APPEND CPUS Z80)

##################################################
## specify available sound cores; some of these are
## only for MAME and so aren't included
##################################################

list(APPEND SOUNDS AY8910)
list(APPEND SOUNDS C140)
list(APPEND SOUNDS C352)
list(APPEND SOUNDS C6280)
list(APPEND SOUNDS GB_SOUND)
list(APPEND SOUNDS ES5503)
list(APPEND SOUNDS ES5505)
list(APPEND SOUNDS IREMGA20)
list(APPEND SOUNDS K051649)
list(APPEND SOUNDS K053260)
list(APPEND SOUNDS K054539)
list(APPEND SOUNDS KS0164)
list(APPEND SOUNDS MULTIPCM)
list(APPEND SOUNDS NES_APU)
list(APPEND SOUNDS OKIM6258)
list(APPEND SOUNDS OKIM6295)
list(APPEND SOUNDS POKEY)
list(APPEND SOUNDS QSOUND)
list(APPEND SOUNDS RF5C68)
list(APPEND SOUNDS SAA1099)
list(APPEND SOUNDS SCSP)
list(APPEND SOUNDS DAC)
list(APPEND SOUNDS SEGAPCM)
list(APPEND SOUNDS SN76496)
list(APPEND SOUNDS UPD7759)
list(APPEND SOUNDS VGMVIZ)
list(APPEND SOUNDS WAVE)
list(APPEND SOUNDS X1_010)
list(APPEND SOUNDS Y8950)
list(APPEND SOUNDS YM2154)
list(APPEND SOUNDS YM2151)
list(APPEND SOUNDS YM2414)
list(APPEND SOUNDS YM3806)
list(APPEND SOUNDS YM2203)
list(APPEND SOUNDS YM2413)
list(APPEND SOUNDS YM2608)
list(APPEND SOUNDS YM2610)
list(APPEND SOUNDS YM2612)
list(APPEND SOUNDS YM3526)
list(APPEND SOUNDS YM3812)
list(APPEND SOUNDS YMF262)
list(APPEND SOUNDS YMF271)
list(APPEND SOUNDS YMF278)
list(APPEND SOUNDS YMF278B)
list(APPEND SOUNDS YMZ280B)

##################################################
## specify available video cores
##################################################


##################################################
## specify available machine cores
##################################################
list(APPEND MACHINES LDV1000)
list(APPEND MACHINES LDPR8210)
list(APPEND MACHINES Z80DAISY)

##################################################
## specify available bus cores
##################################################
list(APPEND BUSES MIDI)


##################################################
## this is the driver library that
## comprise the virtual drivers
##################################################
macro(linkProjects_mame_virtual _target _subtarget _projectname)
	target_link_libraries(${_projectname} PRIVATE virtual)
endmacro()

function(createVirtualProjects _target  _subtarget _name)
	add_library(${_name} ${LIBTYPE})
	addprojectflags(${_name})
	precompiledheaders_novs(${_name})
	add_dependencies(${_name} layouts optional)

	target_include_directories(${_name} PRIVATE
		${MAME_DIR}/src/osd
		${MAME_DIR}/src/emu
		${MAME_DIR}/src/devices
		${MAME_DIR}/src/mame
		${MAME_DIR}/src/lib
		${MAME_DIR}/src/lib/util
		${MAME_DIR}/3rdparty
		${GEN_DIR}/mame/layout
		${EXT_INCLUDEDIR_ZLIB}
		${EXT_INCLUDEDIR_FLAC}
	)
	target_sources(${_name} PRIVATE ${ARGN})
	add_project_to_group(drivers ${_name})
endfunction()

macro(createProjects_mame_virtual _target  _subtarget)
	createVirtualProjects(_target _subtarget "virtual"
		${MAME_DIR}/src/mame/drivers/vgmplay.cpp
		${MAME_DIR}/src/mame/drivers/wavesynth.cpp
		${MAME_DIR}/src/mame/drivers/ldplayer.cpp
		${MAME_DIR}/src/mame/machine/mega32x.cpp
		${MAME_DIR}/src/mame/machine/mega32x.h
		${MAME_DIR}/src/mame/audio/vboy.cpp
		${MAME_DIR}/src/mame/audio/vboy.h
		${MAME_DIR}/src/mame/audio/wswan.cpp
		${MAME_DIR}/src/mame/audio/wswan.h
	)
endmacro()
