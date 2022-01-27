# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   ci.cmake
##
##   Small driver-specific example makefile for use in CI
##   Use cmake -DSUBTARGET=ci to build
##
##########################################################################


##################################################
## Specify all the CPU cores necessary for the
## drivers referenced in ci.lst.
##################################################

list(APPEND CPUS Z80)
list(APPEND CPUS M6502)
list(APPEND CPUS MCS48)
list(APPEND CPUS MCS51)
list(APPEND CPUS M6800)
list(APPEND CPUS M6805)
list(APPEND CPUS M6809)
list(APPEND CPUS M680X0)
list(APPEND CPUS TMS9900)
list(APPEND CPUS COP400)
list(APPEND CPUS MIPS3)
list(APPEND CPUS ADSP21XX)
list(APPEND CPUS FR)

##################################################
## Specify all the sound cores necessary for the
## drivers referenced in ci.lst.
##################################################

list(APPEND SOUNDS SAMPLES)
list(APPEND SOUNDS DAC)
list(APPEND SOUNDS DISCRETE)
list(APPEND SOUNDS AY8910)
list(APPEND SOUNDS YM2151)
list(APPEND SOUNDS ASTROCADE)
list(APPEND SOUNDS TMS5220)
list(APPEND SOUNDS OKIM6295)
list(APPEND SOUNDS HC55516)
list(APPEND SOUNDS YM3812)
list(APPEND SOUNDS CEM3394)
list(APPEND SOUNDS VOTRAX)
list(APPEND SOUNDS DMADAC)
list(APPEND SOUNDS CDDA)
list(APPEND SOUNDS POKEY)

##################################################
## specify available video cores
##################################################

list(APPEND VIDEOS PS2GIF)
list(APPEND VIDEOS PS2GS)

##################################################
## specify available machine cores
##################################################

list(APPEND MACHINES 6821PIA)
list(APPEND MACHINES ADC0808)
list(APPEND MACHINES TTL74148)
list(APPEND MACHINES TTL74153)
list(APPEND MACHINES TTL74157)
list(APPEND MACHINES TTL7474)
list(APPEND MACHINES TTL74259)
list(APPEND MACHINES RIOT6532)
list(APPEND MACHINES PIT8253)
list(APPEND MACHINES Z80CTC)
list(APPEND MACHINES Z80PIO)
list(APPEND MACHINES 68681)
list(APPEND MACHINES BANKDEV)
list(APPEND MACHINES GEN_LATCH)
list(APPEND MACHINES INPUT_MERGER)
list(APPEND MACHINES OUTPUT_LATCH)
list(APPEND MACHINES TICKET)
list(APPEND MACHINES WATCHDOG)
list(APPEND MACHINES Z80DAISY)
list(APPEND MACHINES NETLIST)
list(APPEND MACHINES IDECTRL)
list(APPEND MACHINES PS2INTC)
list(APPEND MACHINES ER2055)

##################################################
## specify available bus cores
##################################################

list(APPEND BUSES CENTRONICS)
list(APPEND BUSES ATA)

##################################################
## This is the list of files that are necessary
## for building all of the drivers referenced
## in ci.lst
##################################################

macro(createProjects_mame_ci _target  _subtarget)
	add_library(mame_ci ${LIBTYPE})
	addprojectflags(mame_ci)
	precompiledheaders_novs(mame_ci)
	add_dependencies(mame_ci layouts optional)

	target_include_directories(mame_ci PRIVATE
		${MAME_DIR}/src/osd
		${MAME_DIR}/src/emu
		${MAME_DIR}/src/devices
		${MAME_DIR}/src/mame
		${MAME_DIR}/src/lib
		${MAME_DIR}/src/lib/util
		${MAME_DIR}/3rdparty
		${GEN_DIR}/mame/layout
	)

	target_sources(mame_ci PRIVATE
		${MAME_DIR}/src/mame/audio/nl_carpolo.cpp
		${MAME_DIR}/src/mame/audio/nl_carpolo.h
		${MAME_DIR}/src/mame/drivers/carpolo.cpp
		${MAME_DIR}/src/mame/includes/carpolo.h
		${MAME_DIR}/src/mame/machine/carpolo.cpp
		${MAME_DIR}/src/mame/video/carpolo.cpp
		${MAME_DIR}/src/mame/drivers/circus.cpp
		${MAME_DIR}/src/mame/includes/circus.h
		${MAME_DIR}/src/mame/audio/circus.cpp
		${MAME_DIR}/src/mame/video/circus.cpp
		${MAME_DIR}/src/mame/drivers/exidy.cpp
		${MAME_DIR}/src/mame/includes/exidy.h
		${MAME_DIR}/src/mame/audio/exidy.cpp
		${MAME_DIR}/src/mame/audio/exidy.h
		${MAME_DIR}/src/mame/video/exidy.cpp
		${MAME_DIR}/src/mame/audio/exidy440.cpp
		${MAME_DIR}/src/mame/audio/exidy440.h
		${MAME_DIR}/src/mame/drivers/starfire.cpp
		${MAME_DIR}/src/mame/audio/nl_fireone.cpp
		${MAME_DIR}/src/mame/audio/nl_fireone.h
		${MAME_DIR}/src/mame/audio/nl_starfire.cpp
		${MAME_DIR}/src/mame/audio/nl_starfire.h
		${MAME_DIR}/src/mame/includes/starfire.h
		${MAME_DIR}/src/mame/video/starfire.cpp
		${MAME_DIR}/src/mame/drivers/vertigo.cpp
		${MAME_DIR}/src/mame/includes/vertigo.h
		${MAME_DIR}/src/mame/machine/vertigo.cpp
		${MAME_DIR}/src/mame/video/vertigo.cpp
		${MAME_DIR}/src/mame/drivers/victory.cpp
		${MAME_DIR}/src/mame/includes/victory.h
		${MAME_DIR}/src/mame/video/victory.cpp
		${MAME_DIR}/src/mame/drivers/astrocde.cpp
		${MAME_DIR}/src/mame/includes/astrocde.h
		${MAME_DIR}/src/mame/video/astrocde.cpp
		${MAME_DIR}/src/mame/drivers/gridlee.cpp
		${MAME_DIR}/src/mame/includes/gridlee.h
		${MAME_DIR}/src/mame/audio/gridlee.cpp
		${MAME_DIR}/src/mame/video/gridlee.cpp
		${MAME_DIR}/src/mame/drivers/williams.cpp
		${MAME_DIR}/src/mame/includes/williams.h
		${MAME_DIR}/src/mame/machine/williams.cpp
		${MAME_DIR}/src/mame/audio/williams.cpp
		${MAME_DIR}/src/mame/audio/williams.h
		${MAME_DIR}/src/mame/video/williams.cpp
		${MAME_DIR}/src/mame/audio/s11c_bg.cpp
		${MAME_DIR}/src/mame/audio/s11c_bg.h
		${MAME_DIR}/src/mame/drivers/gaelco.cpp
		${MAME_DIR}/src/mame/includes/gaelco.h
		${MAME_DIR}/src/mame/video/gaelco.cpp
		${MAME_DIR}/src/mame/machine/gaelcrpt.cpp
		${MAME_DIR}/src/mame/drivers/wrally.cpp
		${MAME_DIR}/src/mame/includes/wrally.h
		${MAME_DIR}/src/mame/video/wrally.cpp
		${MAME_DIR}/src/mame/video/gaelco_wrally_sprites.cpp
		${MAME_DIR}/src/mame/video/gaelco_wrally_sprites.h
		${MAME_DIR}/src/mame/machine/gaelco_ds5002fp.cpp
		${MAME_DIR}/src/mame/machine/gaelco_ds5002fp.h
		${MAME_DIR}/src/mame/drivers/looping.cpp
		${MAME_DIR}/src/mame/drivers/supertnk.cpp
		${MAME_DIR}/src/mame/drivers/kinst.cpp
		${MAME_DIR}/src/mame/audio/dcs.cpp
		${MAME_DIR}/src/mame/audio/dcs.h
		${MAME_DIR}/src/mame/drivers/akkaarrh.cpp
	)
	add_project_to_group(drivers mame_ci)
endmacro()

macro(linkProjects_mame_ci _target _subtarget _projectname)
	target_link_libraries(${_projectname} PRIVATE mame_ci)
endmacro()
