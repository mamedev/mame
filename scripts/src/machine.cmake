# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   machine.cmake
##
##   Rules for building machine cores
##
##########################################################################

set(MACHINE_SRCS
	${MAME_DIR}/src/devices/machine/bcreader.cpp
	${MAME_DIR}/src/devices/machine/bcreader.h
	${MAME_DIR}/src/devices/machine/buffer.cpp
	${MAME_DIR}/src/devices/machine/buffer.h
	${MAME_DIR}/src/devices/machine/clock.cpp
	${MAME_DIR}/src/devices/machine/clock.h
	${MAME_DIR}/src/devices/machine/keyboard.cpp
	${MAME_DIR}/src/devices/machine/keyboard.h
	${MAME_DIR}/src/devices/machine/keyboard.ipp
	${MAME_DIR}/src/devices/machine/laserdsc.cpp
	${MAME_DIR}/src/devices/machine/laserdsc.h
	${MAME_DIR}/src/devices/machine/nvram.cpp
	${MAME_DIR}/src/devices/machine/nvram.h
	${MAME_DIR}/src/devices/machine/ram.cpp
	${MAME_DIR}/src/devices/machine/ram.h
	${MAME_DIR}/src/devices/machine/legscsi.cpp
	${MAME_DIR}/src/devices/machine/legscsi.h
	${MAME_DIR}/src/devices/machine/sdlc.cpp
	${MAME_DIR}/src/devices/machine/sdlc.h
	${MAME_DIR}/src/devices/machine/terminal.cpp
	${MAME_DIR}/src/devices/machine/terminal.h
	${MAME_DIR}/src/devices/machine/timer.cpp
	${MAME_DIR}/src/devices/machine/timer.h

	${MAME_DIR}/src/devices/imagedev/bitbngr.cpp
	${MAME_DIR}/src/devices/imagedev/bitbngr.h
	${MAME_DIR}/src/devices/imagedev/cassette.cpp
	${MAME_DIR}/src/devices/imagedev/cassette.h
	${MAME_DIR}/src/devices/imagedev/chd_cd.cpp
	${MAME_DIR}/src/devices/imagedev/chd_cd.h
	${MAME_DIR}/src/devices/imagedev/diablo.cpp
	${MAME_DIR}/src/devices/imagedev/diablo.h
	${MAME_DIR}/src/devices/imagedev/flopdrv.cpp
	${MAME_DIR}/src/devices/imagedev/flopdrv.h
	${MAME_DIR}/src/devices/imagedev/floppy.cpp
	${MAME_DIR}/src/devices/imagedev/floppy.h
	${MAME_DIR}/src/devices/imagedev/harddriv.cpp
	${MAME_DIR}/src/devices/imagedev/harddriv.h
	${MAME_DIR}/src/devices/imagedev/mfmhd.cpp
	${MAME_DIR}/src/devices/imagedev/mfmhd.h
	${MAME_DIR}/src/devices/imagedev/microdrv.cpp
	${MAME_DIR}/src/devices/imagedev/microdrv.h
	${MAME_DIR}/src/devices/imagedev/midiin.cpp
	${MAME_DIR}/src/devices/imagedev/midiin.h
	${MAME_DIR}/src/devices/imagedev/midiout.cpp
	${MAME_DIR}/src/devices/imagedev/midiout.h
	${MAME_DIR}/src/devices/imagedev/picture.cpp
	${MAME_DIR}/src/devices/imagedev/picture.h
	${MAME_DIR}/src/devices/imagedev/printer.cpp
	${MAME_DIR}/src/devices/imagedev/printer.h
	${MAME_DIR}/src/devices/imagedev/snapquik.cpp
	${MAME_DIR}/src/devices/imagedev/snapquik.h
	${MAME_DIR}/src/devices/imagedev/wafadrive.cpp
	${MAME_DIR}/src/devices/imagedev/wafadrive.h
	${MAME_DIR}/src/devices/imagedev/avivideo.cpp
	${MAME_DIR}/src/devices/imagedev/avivideo.h
)


##################################################
##
##@src/devices/machine/acorn_ioc.h,list(APPEND MACHINES ACORN_IOC)
##################################################

if("ACORN_IOC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/acorn_ioc.cpp
		${MAME_DIR}/src/devices/machine/acorn_ioc.h
	)
endif()

##################################################
##
##@src/devices/machine/acorn_memc.h,list(APPEND MACHINES ACORN_MEMC)
##################################################

if("ACORN_MEMC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/acorn_memc.cpp
		${MAME_DIR}/src/devices/machine/acorn_memc.h
	)
endif()

##################################################
##
##@src/devices/machine/acorn_vidc.h,list(APPEND MACHINES ACORN_VIDC)
##################################################

if("ACORN_VIDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/acorn_vidc.cpp
		${MAME_DIR}/src/devices/machine/acorn_vidc.h
	)
endif()

###################################################
##
##@src/devices/machine/akiko.h,list(APPEND MACHINES AKIKO)
###################################################

if("AKIKO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/akiko.cpp
		${MAME_DIR}/src/devices/machine/akiko.h
	)
endif()

###################################################
##
##@src/devices/machine/am2901b.h,list(APPEND MACHINES AM2901B)
###################################################

if("AM2901B" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am2901b.cpp
		${MAME_DIR}/src/devices/machine/am2901b.h
	)
endif()

##################################################
##
##@src/devices/machine/arm_iomd.h,list(APPEND MACHINES ARM_IOMD)
##################################################

if("ARM_IOMD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/arm_iomd.cpp
		${MAME_DIR}/src/devices/machine/arm_iomd.h
	)
endif()

###################################################
##
##@src/devices/machine/autoconfig.h,list(APPEND MACHINES AUTOCONFIG)
###################################################

if("AUTOCONFIG" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/autoconfig.cpp
		${MAME_DIR}/src/devices/machine/autoconfig.h
	)
endif()


###################################################
##
##@src/devices/machine/cop452.h,list(APPEND MACHINES COP452)
###################################################

if("COP452" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cop452.cpp
		${MAME_DIR}/src/devices/machine/cop452.h
	)
endif()


###################################################
##
##@src/devices/machine/cr511b.h,list(APPEND MACHINES CR511B)
###################################################

if("CR511B" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cr511b.cpp
		${MAME_DIR}/src/devices/machine/cr511b.h
	)
endif()


###################################################
##
##@src/devices/machine/dmac.h,list(APPEND MACHINES DMAC)
###################################################

if("DMAC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/dmac.cpp
		${MAME_DIR}/src/devices/machine/dmac.h
	)
endif()


###################################################
##
##@src/devices/machine/gayle.h,list(APPEND MACHINES GAYLE)
###################################################

if("GAYLE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/gayle.cpp
		${MAME_DIR}/src/devices/machine/gayle.h
	)
endif()


###################################################
##
##@src/devices/machine/40105.h,list(APPEND MACHINES CMOS40105)
###################################################

if("CMOS40105" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/40105.cpp
		${MAME_DIR}/src/devices/machine/40105.h
	)
endif()


###################################################
##
##@src/devices/machine/53c7xx.h,list(APPEND MACHINES NCR53C7XX)
###################################################

if("NCR53C7XX" IN_LIST MACHINES)
	list(APPEND MACHINES "NSCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/53c7xx.cpp
		${MAME_DIR}/src/devices/machine/53c7xx.h
	)
endif()

###################################################
##
##@src/devices/machine/ncr5385.h,list(APPEND MACHINES NCR5385)
###################################################

if("NCR5385" IN_LIST MACHINES)
	list(APPEND MACHINES "NSCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ncr5385.cpp
		${MAME_DIR}/src/devices/machine/ncr5385.h
	)
endif()

###################################################
##
##@src/devices/machine/53c810.h,list(APPEND MACHINES LSI53C810)
###################################################

if("LSI53C810" IN_LIST MACHINES)
	list(APPEND MACHINES "SCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/53c810.cpp
		${MAME_DIR}/src/devices/machine/53c810.h
	)
endif()

###################################################
##
##@src/devices/machine/2812fifo.h,list(APPEND MACHINES 2812FIFO)
###################################################

if("2812FIFO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/2812fifo.cpp
		${MAME_DIR}/src/devices/machine/2812fifo.h
	)
endif()

###################################################
##
##@src/devices/machine/6522via.h,list(APPEND MACHINES 6522VIA)
###################################################

if("6522VIA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6522via.cpp
		${MAME_DIR}/src/devices/machine/6522via.h
	)
endif()

###################################################
##
##@src/devices/machine/6525tpi.h,list(APPEND MACHINES TPI6525)
###################################################

if("TPI6525" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6525tpi.cpp
		${MAME_DIR}/src/devices/machine/6525tpi.h
	)
endif()

###################################################
##
##@src/devices/machine/6532riot.h,list(APPEND MACHINES RIOT6532)
###################################################

if("RIOT6532" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6532riot.cpp
		${MAME_DIR}/src/devices/machine/6532riot.h
	)
endif()

###################################################
##
##@src/devices/machine/6821pia.h,list(APPEND MACHINES 6821PIA)
###################################################

if("6821PIA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6821pia.cpp
		${MAME_DIR}/src/devices/machine/6821pia.h
	)
endif()

###################################################
##
##@src/devices/machine/6840ptm.h,list(APPEND MACHINES 6840PTM)
###################################################

if("6840PTM" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6840ptm.cpp
		${MAME_DIR}/src/devices/machine/6840ptm.h
	)
endif()

###################################################
##
##@src/devices/machine/6850acia.h,list(APPEND MACHINES ACIA6850)
###################################################

if("ACIA6850" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/6850acia.cpp
		${MAME_DIR}/src/devices/machine/6850acia.h
	)
endif()

###################################################
##
##@src/devices/machine/68153bim.h,list(APPEND MACHINES BIM68153)
###################################################

if("BIM68153" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/68153bim.cpp
		${MAME_DIR}/src/devices/machine/68153bim.h
	)
endif()

###################################################
##
##@src/devices/machine/68230pit.h,list(APPEND MACHINES PIT68230)
###################################################

if("PIT68230" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/68230pit.cpp
		${MAME_DIR}/src/devices/machine/68230pit.h
	)
endif()

###################################################
##
##@src/devices/machine/68561mpcc.h,list(APPEND MACHINES MPCC68561)
###################################################

if("MPCC68561" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/68561mpcc.cpp
		${MAME_DIR}/src/devices/machine/68561mpcc.h
	)
endif()

###################################################
##
##@src/devices/machine/mc68681.h,list(APPEND MACHINES 68681)
###################################################

if("68681" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc68681.cpp
		${MAME_DIR}/src/devices/machine/mc68681.h
	)
endif()

###################################################
##
##@src/devices/machine/7200fifo.h,list(APPEND MACHINES 7200FIFO)
###################################################

if("7200FIFO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/7200fifo.cpp
		${MAME_DIR}/src/devices/machine/7200fifo.h
	)
endif()

###################################################
##
##@src/devices/machine/7400.h,list(APPEND MACHINES TTL7400)
###################################################

if("TTL7400" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/7400.cpp
		${MAME_DIR}/src/devices/machine/7400.h
	)
endif()

###################################################
##
##@src/devices/machine/7404.h,list(APPEND MACHINES TTL7404)
###################################################

if("TTL7404" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/7404.cpp
		${MAME_DIR}/src/devices/machine/7404.h
	)
endif()

###################################################
##
##@src/devices/machine/74123.h,list(APPEND MACHINES TTL74123)
###################################################

if("TTL74123" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74123.cpp
		${MAME_DIR}/src/devices/machine/74123.h
		${MAME_DIR}/src/devices/machine/rescap.h
	)
endif()

###################################################
##
##@src/devices/machine/74145.h,list(APPEND MACHINES TTL74145)
###################################################

if("TTL74145" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74145.cpp
		${MAME_DIR}/src/devices/machine/74145.h
	)
endif()

###################################################
##
##@src/devices/machine/74148.h,list(APPEND MACHINES TTL74148)
###################################################

if("TTL74148" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74148.cpp
		${MAME_DIR}/src/devices/machine/74148.h
	)
endif()

###################################################
##
##@src/devices/machine/74153.h,list(APPEND MACHINES TTL74153)
###################################################

if("TTL74153" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74153.cpp
		${MAME_DIR}/src/devices/machine/74153.h
	)
endif()

###################################################
##
##@src/devices/machine/74157.h,list(APPEND MACHINES TTL74157)
###################################################

if("TTL74157" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74157.cpp
		${MAME_DIR}/src/devices/machine/74157.h
	)
endif()

###################################################
##
##@src/devices/machine/74161.h,list(APPEND MACHINES TTL74161)
###################################################

if("TTL74161" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74161.cpp
		${MAME_DIR}/src/devices/machine/74161.h
	)
endif()

###################################################
##
##@src/devices/machine/74165.h,list(APPEND MACHINES TTL74165)
###################################################

if("TTL74165" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74165.cpp
		${MAME_DIR}/src/devices/machine/74165.h
	)
endif()

###################################################
##
##@src/devices/machine/74166.h,list(APPEND MACHINES TTL74166)
###################################################

if("TTL74166" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74166.cpp
		${MAME_DIR}/src/devices/machine/74166.h
	)
endif()

###################################################
##
##@src/devices/machine/74175.h,list(APPEND MACHINES TTL74175)
###################################################

if("TTL74175" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74175.cpp
		${MAME_DIR}/src/devices/machine/74175.h
	)
endif()

###################################################
##
##@src/devices/machine/74181.h,list(APPEND MACHINES TTL74181)
###################################################

if("TTL74181" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74181.cpp
		${MAME_DIR}/src/devices/machine/74181.h
	)
endif()

###################################################
##
##@src/devices/machine/74259.h,list(APPEND MACHINES TTL74259)
###################################################

if("TTL74259" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74259.cpp
		${MAME_DIR}/src/devices/machine/74259.h
	)
endif()

###################################################
##
##@src/devices/machine/74381.h,list(APPEND MACHINES TTL74381)
###################################################

if("TTL74381" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74381.cpp
		${MAME_DIR}/src/devices/machine/74381.h
	)
endif()

###################################################
##
##@src/devices/machine/74543.h,list(APPEND MACHINES TTL74543)
###################################################

if("TTL74543" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/74543.cpp
		${MAME_DIR}/src/devices/machine/74543.h
	)
endif()

###################################################
##
##@src/devices/machine/7474.h,list(APPEND MACHINES TTL7474)
###################################################

if("TTL7474" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/7474.cpp
		${MAME_DIR}/src/devices/machine/7474.h
	)
endif()

###################################################
##
##@src/devices/machine/82s129.h,list(APPEND MACHINES PROM82S129)
###################################################

if("PROM82S129" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/82s129.cpp
		${MAME_DIR}/src/devices/machine/82s129.h
	)
endif()

###################################################
##
##@src/devices/machine/8042kbdc.h,list(APPEND MACHINES KBDC8042)
###################################################

if("KBDC8042" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/8042kbdc.cpp
		${MAME_DIR}/src/devices/machine/8042kbdc.h
	)
endif()

###################################################
##
##@src/devices/machine/8530scc.h,list(APPEND MACHINES 8530SCC)
###################################################

if("8530SCC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/8530scc.cpp
		${MAME_DIR}/src/devices/machine/8530scc.h
	)
endif()

###################################################
##
##@src/devices/machine/adc0804.h,list(APPEND MACHINES ADC0804)
###################################################

if("ADC0804" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc0804.cpp
		${MAME_DIR}/src/devices/machine/adc0804.h
	)
endif()

###################################################
##
##@src/devices/machine/adc0808.h,list(APPEND MACHINES ADC0808)
###################################################

if("ADC0808" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc0808.cpp
		${MAME_DIR}/src/devices/machine/adc0808.h
	)
endif()

###################################################
##
##@src/devices/machine/adc083x.h,list(APPEND MACHINES ADC083X)
###################################################

if("ADC083X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc083x.cpp
		${MAME_DIR}/src/devices/machine/adc083x.h
	)
endif()

###################################################
##
##@src/devices/machine/adc1038.h,list(APPEND MACHINES ADC1038)
###################################################

if("ADC1038" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc1038.cpp
		${MAME_DIR}/src/devices/machine/adc1038.h
	)
endif()

###################################################
##
##@src/devices/machine/adc1213x.h,list(APPEND MACHINES ADC1213X)
###################################################

if("ADC1213X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc1213x.cpp
		${MAME_DIR}/src/devices/machine/adc1213x.h
	)
endif()

###################################################
##
##@src/devices/machine/aicartc.h,list(APPEND MACHINES AICARTC)
###################################################

if("AICARTC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/aicartc.cpp
		${MAME_DIR}/src/devices/machine/aicartc.h
	)
endif()

###################################################
##
##@src/devices/machine/am25s55x.h,list(APPEND MACHINES AM25S55X)
###################################################

if("AM25S55X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am25s55x.cpp
		${MAME_DIR}/src/devices/machine/am25s55x.h
	)
endif()

###################################################
##
##@src/devices/machine/am2847.h,list(APPEND MACHINES AM2847)
###################################################

if("AM2847" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am2847.cpp
		${MAME_DIR}/src/devices/machine/am2847.h
	)
endif()

###################################################
##
##@src/devices/machine/am2910.h,list(APPEND MACHINES AM2910)
###################################################

if("AM2910" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am2910.cpp
		${MAME_DIR}/src/devices/machine/am2910.h
	)
endif()

###################################################
##
##@src/devices/machine/am53cf96.h,list(APPEND MACHINES AM53CF96)
###################################################

if("AM53CF96" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am53cf96.cpp
		${MAME_DIR}/src/devices/machine/am53cf96.h
	)
endif()

###################################################
##
##@src/devices/machine/am79c30.h,list(APPEND MACHINES AM79C30)
###################################################

if("AM79C30" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am79c30.cpp
		${MAME_DIR}/src/devices/machine/am79c30.h
	)
endif()

###################################################
##
##@src/devices/machine/am79c90.h,list(APPEND MACHINES AM79C90)
###################################################

if("AM79C90" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am79c90.cpp
		${MAME_DIR}/src/devices/machine/am79c90.h
	)
endif()

###################################################
##
##@src/devices/machine/am9513.h,list(APPEND MACHINES AM9513)
###################################################

if("AM9513" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am9513.cpp
		${MAME_DIR}/src/devices/machine/am9513.h
	)
endif()

###################################################
##
##@src/devices/machine/am9517a.h,list(APPEND MACHINES AM9517A)
###################################################

if("AM9517A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am9517a.cpp
		${MAME_DIR}/src/devices/machine/am9517a.h
	)
endif()

###################################################
##
##@src/devices/machine/am9519.h,list(APPEND MACHINES AM9519)
###################################################

if("AM9519" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/am9519.cpp
		${MAME_DIR}/src/devices/machine/am9519.h
	)
endif()

###################################################
##
##@src/devices/machine/amigafdc.h,list(APPEND MACHINES AMIGAFDC)
###################################################

if("AMIGAFDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/amigafdc.cpp
		${MAME_DIR}/src/devices/machine/amigafdc.h
	)
endif()

###################################################
##
##@src/devices/machine/at28c16.h,list(APPEND MACHINES AT28C16)
###################################################

if("AT28C16" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/at28c16.cpp
		${MAME_DIR}/src/devices/machine/at28c16.h
	)
endif()

###################################################
##
##@src/devices/machine/at28c64b.h,list(APPEND MACHINES AT28C64B)
###################################################

if("AT28C64B" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/at28c64b.cpp
		${MAME_DIR}/src/devices/machine/at28c64b.h
	)
endif()

###################################################
##
##@src/devices/machine/at29x.h,list(APPEND MACHINES AT29X)
###################################################

if("AT29X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/at29x.cpp
		${MAME_DIR}/src/devices/machine/at29x.h
	)
endif()

###################################################
##
##@src/devices/machine/at45dbxx.h,list(APPEND MACHINES AT45DBXX)
###################################################

if("AT45DBXX" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/at45dbxx.cpp
		${MAME_DIR}/src/devices/machine/at45dbxx.h
	)
endif()

###################################################
##
##@src/devices/machine/ataflash.h,list(APPEND MACHINES ATAFLASH)
###################################################

if("ATAFLASH" IN_LIST MACHINES)
	list(APPEND MACHINES "PCCARD")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ataflash.cpp
		${MAME_DIR}/src/devices/machine/ataflash.h
	)
endif()

###################################################
##
##@src/devices/machine/atmel_arm_aic.h,list(APPEND MACHINES ARM_AIC)
###################################################

if("ARM_AIC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/atmel_arm_aic.cpp
		${MAME_DIR}/src/devices/machine/atmel_arm_aic.h
	)
endif()

###################################################
##
##@src/devices/machine/ay31015.h,list(APPEND MACHINES AY31015)
###################################################

if("AY31015" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ay31015.cpp
		${MAME_DIR}/src/devices/machine/ay31015.h
	)
endif()

###################################################
##
##@src/devices/machine/bankdev.h,list(APPEND MACHINES BANKDEV)
###################################################

if("BANKDEV" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/bankdev.cpp
		${MAME_DIR}/src/devices/machine/bankdev.h
	)
endif()

###################################################
##
##@src/devices/machine/bq4847.h,list(APPEND MACHINES BQ4847)
###################################################

if("BQ4847" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/bq4847.cpp
		${MAME_DIR}/src/devices/machine/bq4847.h
	)
endif()

###################################################
##
##@src/devices/machine/bq48x2.h,list(APPEND MACHINES BQ4852)
###################################################

if("BQ4852" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/bq48x2.cpp
		${MAME_DIR}/src/devices/machine/bq48x2.h
	)
endif()

###################################################
##
##@src/devices/machine/busmouse.h,list(APPEND MACHINES BUSMOUSE)
###################################################

if("BUSMOUSE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/busmouse.cpp
		${MAME_DIR}/src/devices/machine/busmouse.h
	)
endif()

###################################################
##
##@src/devices/machine/cdp1852.h,list(APPEND MACHINES CDP1852)
###################################################

if("CDP1852" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cdp1852.cpp
		${MAME_DIR}/src/devices/machine/cdp1852.h
	)
endif()

###################################################
##
##@src/devices/machine/cdp1871.h,list(APPEND MACHINES CDP1871)
###################################################

if("CDP1871" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cdp1871.cpp
		${MAME_DIR}/src/devices/machine/cdp1871.h
	)
endif()

###################################################
##
##@src/devices/machine/cdp1879.h,list(APPEND MACHINES CDP1879)
###################################################

if("CDP1879" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cdp1879.cpp
		${MAME_DIR}/src/devices/machine/cdp1879.h
	)
endif()


###################################################
##
##@src/devices/machine/ch376.h,list(APPEND MACHINES CH376)
###################################################

if("CH376" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ch376.cpp
		${MAME_DIR}/src/devices/machine/ch376.h
	)
endif()


###################################################
##
##@src/devices/machine/chessmachine.h,list(APPEND MACHINES CHESSMACHINE)
###################################################

if("CHESSMACHINE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/chessmachine.cpp
		${MAME_DIR}/src/devices/machine/chessmachine.h
	)
endif()


###################################################
##
##@src/devices/machine/com52c50.h,list(APPEND MACHINES COM52C50)
###################################################

if("COM52C50" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/com52c50.cpp
		${MAME_DIR}/src/devices/machine/com52c50.h
	)
endif()

###################################################
##
##@src/devices/machine/com8116.h,list(APPEND MACHINES COM8116)
###################################################

if("COM8116" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/com8116.cpp
		${MAME_DIR}/src/devices/machine/com8116.h
	)
endif()

###################################################
##
##@src/devices/machine/cs4031.h,list(APPEND MACHINES CS4031)
###################################################

if("CS4031" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cs4031.cpp
		${MAME_DIR}/src/devices/machine/cs4031.h
	)
endif()

###################################################
##
##@src/devices/machine/cs8221.h,list(APPEND MACHINES CS8221)
###################################################

if("CS8221" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cs8221.cpp
		${MAME_DIR}/src/devices/machine/cs8221.h
	)
endif()

###################################################
##
##@src/devices/machine/cs8900a.h,list(APPEND MACHINES CS8900A)
###################################################

if("CS8900A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cs8900a.cpp
		${MAME_DIR}/src/devices/machine/cs8900a.h
	)
endif()


###################################################
##
##@src/devices/machine/cxd1095.h,list(APPEND MACHINES CXD1095)
###################################################

if("CXD1095" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cxd1095.cpp
		${MAME_DIR}/src/devices/machine/cxd1095.h
	)
endif()

##@src/devices/machine/ds1204.h,list(APPEND MACHINES DS1204)
###################################################

if("DS1204" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1204.cpp
		${MAME_DIR}/src/devices/machine/ds1204.h
	)
endif()

###################################################
##
##@src/devices/machine/ds1205.h,list(APPEND MACHINES DS1205)
###################################################

if("DS1205" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1205.cpp
		${MAME_DIR}/src/devices/machine/ds1205.h
	)
endif()

###################################################
##
##@src/devices/machine/ds1302.h,list(APPEND MACHINES DS1302)
###################################################

if("DS1302" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1302.cpp
		${MAME_DIR}/src/devices/machine/ds1302.h
	)
endif()

###################################################
##
##@src/devices/machine/ds1315.h,list(APPEND MACHINES DS1315)
###################################################

if("DS1315" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1315.cpp
		${MAME_DIR}/src/devices/machine/ds1315.h
	)
endif()

###################################################
##
##@src/devices/machine/ds1386.h,list(APPEND MACHINES DS1386)
###################################################

if("DS1386" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1386.cpp
		${MAME_DIR}/src/devices/machine/ds1386.h
	)
endif()

###################################################
##
##@src/devices/machine/ds17x85.h,list(APPEND MACHINES DS17X85)
###################################################

if("DS17X85" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds17x85.cpp
		${MAME_DIR}/src/devices/machine/ds17x85.h
	)
endif()

###################################################
##
##@src/devices/machine/ds1994.h,list(APPEND MACHINES DS1994)
###################################################

if("DS1994" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds1994.cpp
		${MAME_DIR}/src/devices/machine/ds1994.h
	)
endif()

###################################################
##
##@src/devices/machine/ds2401.h,list(APPEND MACHINES DS2401)
###################################################

if("DS2401" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds2401.cpp
		${MAME_DIR}/src/devices/machine/ds2401.h
	)
endif()

###################################################
##
##@src/devices/machine/ds2404.h,list(APPEND MACHINES DS2404)
###################################################

if("DS2404" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds2404.cpp
		${MAME_DIR}/src/devices/machine/ds2404.h
	)
endif()

###################################################
##
##@src/devices/machine/ds6417.h,list(APPEND MACHINES DS6417)
###################################################

if("DS6417" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds6417.cpp
		${MAME_DIR}/src/devices/machine/ds6417.h
	)
endif()

###################################################
##
##@src/devices/machine/ds75160a.h,list(APPEND MACHINES DS75160A)
###################################################

if("DS75160A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds75160a.cpp
		${MAME_DIR}/src/devices/machine/ds75160a.h
	)
endif()

###################################################
##
##@src/devices/machine/ds75161a.h,list(APPEND MACHINES DS75161A)
###################################################

if("DS75161A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds75161a.cpp
		${MAME_DIR}/src/devices/machine/ds75161a.h
	)
endif()

###################################################
##
##@src/devices/machine/ds8874.h,list(APPEND MACHINES DS8874)
###################################################

if("DS8874" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ds8874.cpp
		${MAME_DIR}/src/devices/machine/ds8874.h
	)
endif()

###################################################
##
##@src/devices/machine/e0516.h,list(APPEND MACHINES E0516)
###################################################

if("E0516" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/e0516.cpp
		${MAME_DIR}/src/devices/machine/e0516.h
	)
endif()

###################################################
##
##@src/devices/machine/e05a03.h,list(APPEND MACHINES E05A03)
###################################################

if("E05A03" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/e05a03.cpp
		${MAME_DIR}/src/devices/machine/e05a03.h
	)
endif()

###################################################
##
##@src/devices/machine/e05a30.h,list(APPEND MACHINES E05A30)
###################################################

if("E05A30" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/e05a30.cpp
		${MAME_DIR}/src/devices/machine/e05a30.h
	)
endif()

###################################################
##
##@src/devices/machine/eeprom.h,list(APPEND MACHINES EEPROMDEV)
##@src/devices/machine/eepromser.h,list(APPEND MACHINES EEPROMDEV)
##@src/devices/machine/eeprompar.h,list(APPEND MACHINES EEPROMDEV)
###################################################

if("EEPROMDEV" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/eeprom.cpp
		${MAME_DIR}/src/devices/machine/eeprom.h
		${MAME_DIR}/src/devices/machine/eepromser.cpp
		${MAME_DIR}/src/devices/machine/eepromser.h
		${MAME_DIR}/src/devices/machine/eeprompar.cpp
		${MAME_DIR}/src/devices/machine/eeprompar.h
	)
endif()

###################################################
##
##@src/devices/machine/er1400.h,list(APPEND MACHINES ER1400)
###################################################

if("ER1400" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/er1400.cpp
		${MAME_DIR}/src/devices/machine/er1400.h
	)
endif()

###################################################
##
##@src/devices/machine/er2055.h,list(APPEND MACHINES ER2055)
###################################################

if("ER2055" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/er2055.cpp
		${MAME_DIR}/src/devices/machine/er2055.h
	)
endif()

###################################################
##
##@src/devices/machine/exorterm.h,list(APPEND MACHINES EXORTERM)
###################################################

if("EXORTERM" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/exorterm.cpp
		${MAME_DIR}/src/devices/machine/exorterm.h
		${GEN_DIR}/emu/layout/exorterm155.lh
	)

	layoutbuildtask("emu/layout" "exorterm155")
endif()

###################################################
##
##@src/devices/machine/f3853.h,list(APPEND MACHINES F3853)
###################################################

if("F3853" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/f3853.cpp
		${MAME_DIR}/src/devices/machine/f3853.h
	)
endif()

###################################################
##
##@src/devices/machine/f4702.h,list(APPEND MACHINES F4702)
###################################################

if("F4702" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/f4702.cpp
		${MAME_DIR}/src/devices/machine/f4702.h
	)
endif()

###################################################
##
##@src/devices/machine/fga002.h,list(APPEND MACHINES FGA002)
###################################################

if("FGA002" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/fga002.cpp
		${MAME_DIR}/src/devices/machine/fga002.h
	)
endif()

###################################################
##
##@src/devices/machine/hd63450.h,list(APPEND MACHINES HD63450)
###################################################

if("HD63450" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/hd63450.cpp
		${MAME_DIR}/src/devices/machine/hd63450.h
	)
endif()

###################################################
##
##@src/devices/machine/hd64610.h,list(APPEND MACHINES HD64610)
###################################################

if("HD64610" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/hd64610.cpp
		${MAME_DIR}/src/devices/machine/hd64610.h
	)
endif()

###################################################
##
##@src/devices/machine/hp_dc100_tape.h,list(APPEND MACHINES HP_DC100_TAPE)
###################################################

if("HP_DC100_TAPE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/hp_dc100_tape.cpp
		${MAME_DIR}/src/devices/machine/hp_dc100_tape.h
	)
endif()

###################################################
##
##@src/devices/machine/hp_taco.h,list(APPEND MACHINES HP_TACO)
###################################################

if("HP_TACO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/hp_taco.cpp
		${MAME_DIR}/src/devices/machine/hp_taco.h
	)
endif()

###################################################
##
##@src/devices/machine/1ma6.h,list(APPEND MACHINES 1MA6)
###################################################

if("1MA6" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/1ma6.cpp
		${MAME_DIR}/src/devices/machine/1ma6.h
	)
endif()

###################################################
##
##@src/devices/machine/1mb5.h,list(APPEND MACHINES 1MB5)
###################################################

if("1MB5" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/1mb5.cpp
		${MAME_DIR}/src/devices/machine/1mb5.h
	)
endif()

###################################################
##
##@src/devices/machine/i2cmem.h,list(APPEND MACHINES I2CMEM)
###################################################

if("I2CMEM" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i2cmem.cpp
		${MAME_DIR}/src/devices/machine/i2cmem.h
	)
endif()

###################################################
##
##@src/devices/machine/i7220.h,list(APPEND MACHINES I7220)
###################################################

if("I7220" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i7220.cpp
		${MAME_DIR}/src/devices/machine/i7220.h
	)
endif()

###################################################
##
##@src/devices/machine/i8087.h,list(APPEND MACHINES I8087)
###################################################

if("I8087" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8087.cpp
		${MAME_DIR}/src/devices/machine/i8087.h
	)
endif()

###################################################
##
##@src/devices/machine/i8155.h,list(APPEND MACHINES I8155)
###################################################

if("I8155" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8155.cpp
		${MAME_DIR}/src/devices/machine/i8155.h
	)
endif()

###################################################
##
##@src/devices/machine/i8212.h,list(APPEND MACHINES I8212)
###################################################

if("I8212" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8212.cpp
		${MAME_DIR}/src/devices/machine/i8212.h
	)
endif()

###################################################
##
##@src/devices/machine/i8214.h,list(APPEND MACHINES I8214)
###################################################

if("I8214" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8214.cpp
		${MAME_DIR}/src/devices/machine/i8214.h
	)
endif()

###################################################
##
##@src/devices/machine/i82355.h,list(APPEND MACHINES I82355)
###################################################

if("I82355" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i82355.cpp
		${MAME_DIR}/src/devices/machine/i82355.h
	)
endif()

###################################################
##
##@src/devices/machine/i8243.h,list(APPEND MACHINES I8243)
###################################################

if("I8243" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8243.cpp
		${MAME_DIR}/src/devices/machine/i8243.h
	)
endif()

###################################################
##
##@src/devices/machine/i8251.h,list(APPEND MACHINES I8251)
###################################################

if("I8251" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8251.cpp
		${MAME_DIR}/src/devices/machine/i8251.h
	)
endif()

###################################################
##
##@src/devices/machine/i8257.h,list(APPEND MACHINES I8257)
###################################################

if("I8257" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8257.cpp
		${MAME_DIR}/src/devices/machine/i8257.h
	)
endif()


###################################################
##
##@src/devices/machine/i8271.h,list(APPEND MACHINES I8271)
###################################################

if("I8271" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8271.cpp
		${MAME_DIR}/src/devices/machine/i8271.h
	)
endif()

###################################################
##
##@src/devices/machine/i8279.h,list(APPEND MACHINES I8279)
###################################################

if("I8279" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8279.cpp
		${MAME_DIR}/src/devices/machine/i8279.h
	)
endif()

###################################################
##
##@src/devices/machine/i8355.h,list(APPEND MACHINES I8355)
###################################################

if("I8355" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8355.cpp
		${MAME_DIR}/src/devices/machine/i8355.h
	)
endif()

###################################################
##
##@src/devices/machine/i80130.h,list(APPEND MACHINES I80130)
###################################################

if("I80130" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i80130.cpp
		${MAME_DIR}/src/devices/machine/i80130.h
	)
endif()

###################################################
##
##@src/devices/machine/icm7170.h,list(APPEND MACHINES ICM7170)
###################################################

if("ICM7170" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/icm7170.cpp
		${MAME_DIR}/src/devices/machine/icm7170.h
	)
endif()

###################################################
##
##@src/devices/machine/ibm21s850.h,list(APPEND MACHINES IBM21S850)
###################################################

if("IBM21S850" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ibm21s850.cpp
		${MAME_DIR}/src/devices/machine/ibm21s850.h
	)
endif()

###################################################
##
##@src/devices/machine/idectrl.h,list(APPEND MACHINES IDECTRL)
##@src/devices/machine/vt83c461.h,list(APPEND MACHINES IDECTRL)
###################################################

if("IDECTRL" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/idectrl.cpp
		${MAME_DIR}/src/devices/machine/idectrl.h
		${MAME_DIR}/src/devices/machine/vt83c461.cpp
		${MAME_DIR}/src/devices/machine/vt83c461.h
	)
endif()

###################################################
##
##@src/devices/machine/ie15.h,list(APPEND MACHINES IE15)
###################################################

if("IE15" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ie15.cpp
		${MAME_DIR}/src/devices/machine/ie15.h
		${MAME_DIR}/src/devices/machine/ie15_kbd.cpp
		${MAME_DIR}/src/devices/machine/ie15_kbd.h
		${GEN_DIR}/emu/layout/ie15.lh
	)

	layoutbuildtask("emu/layout" "ie15")
endif()

###################################################
##
##@src/devices/machine/im6402.h,list(APPEND MACHINES IM6402)
###################################################

if("IM6402" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/im6402.cpp
		${MAME_DIR}/src/devices/machine/im6402.h
	)
endif()

###################################################
##
##@src/devices/machine/ins8154.h,list(APPEND MACHINES INS8154)
###################################################

if("INS8154" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ins8154.cpp
		${MAME_DIR}/src/devices/machine/ins8154.h
	)
endif()

###################################################
##
##@src/devices/machine/ins8250.h,list(APPEND MACHINES INS8250)
###################################################

if("INS8250" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ins8250.cpp
		${MAME_DIR}/src/devices/machine/ins8250.h
	)
endif()

###################################################
##
##@src/devices/machine/intelfsh.h,list(APPEND MACHINES INTELFLASH)
###################################################

if("INTELFLASH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/intelfsh.cpp
		${MAME_DIR}/src/devices/machine/intelfsh.h
	)
endif()

###################################################
##
##@src/devices/machine/jvsdev.h,list(APPEND MACHINES JVS)
##@src/devices/machine/jvshost.h,list(APPEND MACHINES JVS)
###################################################

if("JVS" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/jvsdev.cpp
		${MAME_DIR}/src/devices/machine/jvsdev.h
		${MAME_DIR}/src/devices/machine/jvshost.cpp
		${MAME_DIR}/src/devices/machine/jvshost.h
	)
endif()

###################################################
##
##@src/devices/machine/k033906.h,list(APPEND MACHINES K033906)
###################################################

if("K033906" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/k033906.cpp
		${MAME_DIR}/src/devices/machine/k033906.h
	)
endif()

###################################################
##
##@src/devices/machine/k053252.h,list(APPEND MACHINES K053252)
###################################################

if("K053252" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/k053252.cpp
		${MAME_DIR}/src/devices/machine/k053252.h
	)
endif()

###################################################
##
##@src/devices/machine/k056230.h,list(APPEND MACHINES K056230)
###################################################

if("K056230" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/k056230.cpp
		${MAME_DIR}/src/devices/machine/k056230.h
	)
endif()

###################################################
##
##@src/devices/machine/m950x0.h,list(APPEND MACHINES M950X0)
###################################################

if("M950X0" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/m950x0.cpp
		${MAME_DIR}/src/devices/machine/m950x0.h
	)
endif()

###################################################
##
##@src/devices/machine/mm5740.h,list(APPEND MACHINES MM5740)
###################################################

if("MM5740" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm5740.cpp
		${MAME_DIR}/src/devices/machine/mm5740.h
	)
endif()

###################################################
##
##@src/devices/machine/kb3600.h,list(APPEND MACHINES KB3600)
###################################################

if("KB3600" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/kb3600.cpp
		${MAME_DIR}/src/devices/machine/kb3600.h
	)
endif()

###################################################
##
##@src/devices/machine/kr2376.h,list(APPEND MACHINES KR2376)
###################################################

if("KR2376" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/kr2376.cpp
		${MAME_DIR}/src/devices/machine/kr2376.h
	)
endif()

###################################################
##
##@src/devices/machine/latch8.h,list(APPEND MACHINES LATCH8)
###################################################

if("LATCH8" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/latch8.cpp
		${MAME_DIR}/src/devices/machine/latch8.h
	)
endif()

###################################################
##
##@src/devices/machine/lc89510.h,list(APPEND MACHINES LC89510)
###################################################

if("LC89510" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/lc89510.cpp
		${MAME_DIR}/src/devices/machine/lc89510.h
	)
endif()

###################################################
##
##@src/devices/machine/ldpr8210.h,list(APPEND MACHINES LDPR8210)
###################################################

if("LDPR8210" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldpr8210.cpp
		${MAME_DIR}/src/devices/machine/ldpr8210.h
	)
endif()

###################################################
##
##@src/devices/machine/ldstub.h,list(APPEND MACHINES LDSTUB)
###################################################

if("LDSTUB" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldstub.cpp
		${MAME_DIR}/src/devices/machine/ldstub.h
	)
endif()

###################################################
##
##@src/devices/machine/ldv1000.h,list(APPEND MACHINES LDV1000)
###################################################

if("LDV1000" IN_LIST MACHINES)
	list(APPEND MACHINES "Z80CTC")
	list(APPEND MACHINES "I8255")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldv1000.cpp
		${MAME_DIR}/src/devices/machine/ldv1000.h
	)
endif()

###################################################
##
##@src/devices/machine/ldp1000.h,list(APPEND MACHINES LDP1000)
###################################################

if("LDP1000" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldp1000.cpp
		${MAME_DIR}/src/devices/machine/ldp1000.h
	)
endif()

###################################################
##
##@src/devices/machine/ldp1450.h,list(APPEND MACHINES LDP1450)
###################################################

if("LDP1450" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldp1450.cpp
		${MAME_DIR}/src/devices/machine/ldp1450.h
	)
endif()

###################################################
##
##@src/devices/machine/ldvp931.h,list(APPEND MACHINES LDVP931)
###################################################

if("LDVP931" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ldvp931.cpp
		${MAME_DIR}/src/devices/machine/ldvp931.h
	)
endif()

###################################################
##
##@src/devices/machine/lh5810.h,list(APPEND MACHINES LH5810)
###################################################

if("LH5810" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/lh5810.cpp
		${MAME_DIR}/src/devices/machine/lh5810.h
	)
endif()

###################################################
##
##@src/devices/machine/linflash.h,list(APPEND MACHINES LINFLASH)
###################################################

if("LINFLASH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/linflash.cpp
		${MAME_DIR}/src/devices/machine/linflash.h
	)
endif()

###################################################
##
##@src/devices/machine/locomo.h,list(APPEND MACHINES LOCOMO)
###################################################

if("LOCOMO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/locomo.cpp
		${MAME_DIR}/src/devices/machine/locomo.h
	)
endif()

###################################################
##
##@src/devices/machine/m3002.h,list(APPEND MACHINES M3002)
###################################################

if("M3002" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/m3002.cpp
		${MAME_DIR}/src/devices/machine/m3002.h
	)
endif()

###################################################
##
##@src/devices/machine/m68sfdc.h,list(APPEND MACHINES M68SFDC)
###################################################

if("M68SFDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/m68sfdc.cpp
		${MAME_DIR}/src/devices/machine/m68sfdc.h
	)
endif()

###################################################
##
##@src/devices/machine/m6m80011ap.h,list(APPEND MACHINES M6M80011AP)
###################################################

if("M6M80011AP" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/m6m80011ap.cpp
		${MAME_DIR}/src/devices/machine/m6m80011ap.h
	)
endif()

###################################################
##
##@src/devices/machine/mb14241.h,list(APPEND MACHINES MB14241)
###################################################

if("MB14241" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb14241.cpp
		${MAME_DIR}/src/devices/machine/mb14241.h
	)
endif()

###################################################
##
##@src/devices/machine/mb3773.h,list(APPEND MACHINES MB3773)
###################################################

if("MB3773" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb3773.cpp
		${MAME_DIR}/src/devices/machine/mb3773.h
	)
endif()

###################################################
##
##@src/devices/machine/mb8421.h,list(APPEND MACHINES MB8421)
###################################################

if("MB8421" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb8421.cpp
		${MAME_DIR}/src/devices/machine/mb8421.h
	)
endif()

###################################################
##
##@src/devices/machine/mb87030.h,list(APPEND MACHINES MB87030)
###################################################

if("MB87030" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb87030.cpp
		${MAME_DIR}/src/devices/machine/mb87030.h
	)
endif()

###################################################
##
##@src/devices/machine/mb87078.h,list(APPEND MACHINES MB87078)
###################################################

if("MB87078" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb87078.cpp
		${MAME_DIR}/src/devices/machine/mb87078.h
	)
endif()

###################################################
##
##@src/devices/machine/mb8795.h,list(APPEND MACHINES MB8795)
###################################################

if("MB8795" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb8795.cpp
		${MAME_DIR}/src/devices/machine/mb8795.h
	)
endif()

###################################################
##
##@src/devices/machine/mb89352.h,list(APPEND MACHINES MB89352)
###################################################

if("MB89352" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb89352.cpp
		${MAME_DIR}/src/devices/machine/mb89352.h
	)
endif()

###################################################
##
##@src/devices/machine/mb89371.h,list(APPEND MACHINES MB89371)
###################################################

if("MB89371" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb89371.cpp
		${MAME_DIR}/src/devices/machine/mb89371.h
	)
endif()

###################################################
##
##@src/devices/machine/mb89374.h,list(APPEND MACHINES MB89374)
###################################################

if("MB89374" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mb89374.cpp
		${MAME_DIR}/src/devices/machine/mb89374.h
	)
endif()

###################################################
##
##@src/devices/machine/mc146818.h,list(APPEND MACHINES MC146818)
###################################################

if("MC146818" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc146818.cpp
		${MAME_DIR}/src/devices/machine/mc146818.h
		${MAME_DIR}/src/devices/machine/ds128x.cpp
		${MAME_DIR}/src/devices/machine/ds128x.h
	)
endif()

###################################################
##
##@src/devices/machine/mc14411.h,list(APPEND MACHINES MC14411)
###################################################

if("MC14411" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc14411.cpp
		${MAME_DIR}/src/devices/machine/mc14411.h
	)
endif()

###################################################
##
##@src/devices/machine/mc6843.h,list(APPEND MACHINES MC6843)
###################################################

if("MC6843" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc6843.cpp
		${MAME_DIR}/src/devices/machine/mc6843.h
	)
endif()

###################################################
##
##@src/devices/machine/mc6844.h,list(APPEND MACHINES MC6844)
###################################################

if("MC6844" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc6844.cpp
		${MAME_DIR}/src/devices/machine/mc6844.h
	)
endif()

###################################################
##
##@src/devices/machine/mc6846.h,list(APPEND MACHINES MC6846)
###################################################

if("MC6846" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc6846.cpp
		${MAME_DIR}/src/devices/machine/mc6846.h
	)
endif()

###################################################
##
##@src/devices/machine/mc6852.h,list(APPEND MACHINES MC6852)
###################################################

if("MC6852" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc6852.cpp
		${MAME_DIR}/src/devices/machine/mc6852.h
	)
endif()

###################################################
##
##@src/devices/machine/mc6854.h,list(APPEND MACHINES MC6854)
###################################################

if("MC6854" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc6854.cpp
		${MAME_DIR}/src/devices/machine/mc6854.h
	)
endif()

###################################################
##
##@src/devices/machine/mc68328.h,list(APPEND MACHINES MC68328)
###################################################

if("MC68328" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc68328.cpp
		${MAME_DIR}/src/devices/machine/mc68328.h
	)
endif()

###################################################
##
##@src/devices/machine/mc68901.h,list(APPEND MACHINES MC68901)
###################################################

if("MC68901" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mc68901.cpp
		${MAME_DIR}/src/devices/machine/mc68901.h
	)
endif()

###################################################
##
##@src/devices/machine/mccs1850.h,list(APPEND MACHINES MCCS1850)
###################################################

if("MCCS1850" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mccs1850.cpp
		${MAME_DIR}/src/devices/machine/mccs1850.h
	)
endif()

###################################################
##
##@src/devices/machine/68307.h,list(APPEND MACHINES M68307)
###################################################

if("M68307" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/68307.cpp
		${MAME_DIR}/src/devices/machine/68307.h
		${MAME_DIR}/src/devices/machine/68307sim.cpp
		${MAME_DIR}/src/devices/machine/68307sim.h
		${MAME_DIR}/src/devices/machine/68307bus.cpp
		${MAME_DIR}/src/devices/machine/68307bus.h
		${MAME_DIR}/src/devices/machine/68307tmu.cpp
		${MAME_DIR}/src/devices/machine/68307tmu.h
	)
endif()

###################################################
##
##@src/devices/machine/68340.h,list(APPEND MACHINES M68340)
###################################################

if("M68340" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/68340.cpp
		${MAME_DIR}/src/devices/machine/68340.h
		${MAME_DIR}/src/devices/machine/68340sim.cpp
		${MAME_DIR}/src/devices/machine/68340sim.h
		${MAME_DIR}/src/devices/machine/68340dma.cpp
		${MAME_DIR}/src/devices/machine/68340dma.h
		${MAME_DIR}/src/devices/machine/68340ser.cpp
		${MAME_DIR}/src/devices/machine/68340ser.h
		${MAME_DIR}/src/devices/machine/68340tmu.cpp
		${MAME_DIR}/src/devices/machine/68340tmu.h
	)
endif()

###################################################
##
##@src/devices/machine/mcf5206e.h,list(APPEND MACHINES MCF5206E)
###################################################

if("MCF5206E" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mcf5206e.cpp
		${MAME_DIR}/src/devices/machine/mcf5206e.h
	)
endif()

###################################################
##
##@src/devices/machine/meters.h,list(APPEND MACHINES METERS)
###################################################

if("METERS" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/meters.cpp
		${MAME_DIR}/src/devices/machine/meters.h
	)
endif()

###################################################
##
##@src/devices/machine/microtch.h,list(APPEND MACHINES MICROTOUCH)
###################################################

if("MICROTOUCH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/microtch.cpp
		${MAME_DIR}/src/devices/machine/microtch.h
	)
endif()

###################################################
##
##@src/devices/machine/mm5307.h,list(APPEND MACHINES MM5307)
###################################################

if("MM5307" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm5307.cpp
		${MAME_DIR}/src/devices/machine/mm5307.h
	)
endif()

###################################################
##
##@src/devices/machine/mm58274c.h,list(APPEND MACHINES MM58274C)
###################################################

if("MM58274C" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm58274c.cpp
		${MAME_DIR}/src/devices/machine/mm58274c.h
	)
endif()

###################################################
##
##@src/devices/machine/mm74c922.h,list(APPEND MACHINES MM74C922)
###################################################

if("MM74C922" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm74c922.cpp
		${MAME_DIR}/src/devices/machine/mm74c922.h
	)
endif()

###################################################
##
##@src/devices/machine/mos6526.h,list(APPEND MACHINES MOS6526)
###################################################

if("MOS6526" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos6526.cpp
		${MAME_DIR}/src/devices/machine/mos6526.h
	)
endif()

###################################################
##
##@src/devices/machine/mos6529.h,list(APPEND MACHINES MOS6529)
###################################################

if("MOS6529" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos6529.cpp
		${MAME_DIR}/src/devices/machine/mos6529.h
	)
endif()

###################################################
##
##@src/devices/machine/mos6702.h,list(APPEND MACHINES MOS6702)
###################################################

if("MOS6702" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos6702.cpp
		${MAME_DIR}/src/devices/machine/mos6702.h
	)
endif()

###################################################
##
##@src/devices/machine/mos8706.h,list(APPEND MACHINES MOS8706)
###################################################

if("MOS8706" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos8706.cpp
		${MAME_DIR}/src/devices/machine/mos8706.h
	)
endif()

###################################################
##
##@src/devices/machine/mos8722.h,list(APPEND MACHINES MOS8722)
###################################################

if("MOS8722" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos8722.cpp
		${MAME_DIR}/src/devices/machine/mos8722.h
	)
endif()

###################################################
##
##@src/devices/machine/mos8726.h,list(APPEND MACHINES MOS8726)
###################################################

if("MOS8726" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos8726.cpp
		${MAME_DIR}/src/devices/machine/mos8726.h
	)
endif()

###################################################
##
##@src/devices/machine/mos6530.h,list(APPEND MACHINES MIOT6530)
##@src/devices/machine/mos6530n.h,list(APPEND MACHINES MIOT6530)
###################################################

if("MIOT6530" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos6530.cpp
		${MAME_DIR}/src/devices/machine/mos6530.h
		${MAME_DIR}/src/devices/machine/mos6530n.cpp
		${MAME_DIR}/src/devices/machine/mos6530n.h
	)
endif()

###################################################
##
##@src/devices/machine/mos6551.h,list(APPEND MACHINES MOS6551)
###################################################

if("MOS6551" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mos6551.cpp
		${MAME_DIR}/src/devices/machine/mos6551.h
	)
endif()

###################################################
##
##@src/devices/machine/msm5832.h,list(APPEND MACHINES MSM5832)
###################################################

if("MSM5832" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/msm5832.cpp
		${MAME_DIR}/src/devices/machine/msm5832.h
	)
endif()

###################################################
##
##@src/devices/machine/msm58321.h,list(APPEND MACHINES MSM58321)
###################################################

if("MSM58321" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/msm58321.cpp
		${MAME_DIR}/src/devices/machine/msm58321.h
	)
endif()

###################################################
##
##@src/devices/machine/msm6242.h,list(APPEND MACHINES MSM6242)
###################################################

if("MSM6242" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/msm6242.cpp
		${MAME_DIR}/src/devices/machine/msm6242.h
	)
endif()

###################################################
##
##@src/devices/machine/msm6253.h,list(APPEND MACHINES MSM6253)
###################################################

if("MSM6253" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/msm6253.cpp
		${MAME_DIR}/src/devices/machine/msm6253.h
	)
endif()

###################################################
##
##@src/devices/machine/myb3k_kbd.h,list(APPEND MACHINES MYB3K_KEYBOARD)
###################################################

if("MYB3K_KEYBOARD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
	${MAME_DIR}/src/devices/machine/myb3k_kbd.cpp
	${MAME_DIR}/src/devices/machine/myb3k_kbd.h
	)
endif()

###################################################
##
##@src/devices/machine/ncr539x.h,list(APPEND MACHINES NCR539x)
###################################################

if("NCR539x" IN_LIST MACHINES)
	list(APPEND MACHINES "SCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ncr539x.cpp
		${MAME_DIR}/src/devices/machine/ncr539x.h
	)
endif()

###################################################
##
##@src/devices/machine/nmc9306.h,list(APPEND MACHINES NMC9306)
###################################################

if("NMC9306" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/nmc9306.cpp
		${MAME_DIR}/src/devices/machine/nmc9306.h
	)
endif()

###################################################
##
##@src/devices/machine/nscsi_bus.h,list(APPEND MACHINES NSCSI)
##@src/devices/machine/nscsi_cb.h,list(APPEND MACHINES NSCSI)
###################################################

if("NSCSI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/nscsi_bus.cpp
		${MAME_DIR}/src/devices/machine/nscsi_bus.h
		${MAME_DIR}/src/devices/machine/nscsi_cb.cpp
		${MAME_DIR}/src/devices/machine/nscsi_cb.h
	)
endif()

###################################################
##
##@src/devices/machine/pcf8573.h,list(APPEND MACHINES PCF8573)
###################################################

if("PCF8573" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pcf8573.cpp
		${MAME_DIR}/src/devices/machine/pcf8573.h
	)
endif()

###################################################
##
##@src/devices/machine/pcf8583.h,list(APPEND MACHINES PCF8583)
###################################################

if("PCF8583" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pcf8583.cpp
		${MAME_DIR}/src/devices/machine/pcf8583.h
	)
endif()

###################################################
##
##@src/devices/machine/pcf8584.h,list(APPEND MACHINES PCF8584)
###################################################

if("PCF8584" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pcf8584.cpp
		${MAME_DIR}/src/devices/machine/pcf8584.h
	)
endif()

###################################################
##
##@src/devices/machine/pcf8593.h,list(APPEND MACHINES PCF8593)
###################################################

if("PCF8593" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pcf8593.cpp
		${MAME_DIR}/src/devices/machine/pcf8593.h
	)
endif()

###################################################
##
##@src/devices/machine/lpci.h,list(APPEND MACHINES LPCI)
###################################################

if("LPCI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/lpci.cpp
		${MAME_DIR}/src/devices/machine/lpci.h
	)
endif()

###################################################
##
##@src/devices/machine/pci.h,list(APPEND MACHINES PCI)
###################################################

if("PCI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pci.cpp
		${MAME_DIR}/src/devices/machine/pci.h
		${MAME_DIR}/src/devices/machine/pci-usb.cpp
		${MAME_DIR}/src/devices/machine/pci-usb.h
		${MAME_DIR}/src/devices/machine/pci-sata.cpp
		${MAME_DIR}/src/devices/machine/pci-sata.h
		${MAME_DIR}/src/devices/machine/pci-ide.cpp
		${MAME_DIR}/src/devices/machine/pci-ide.h
		${MAME_DIR}/src/devices/machine/pci-apic.cpp
		${MAME_DIR}/src/devices/machine/pci-apic.h
		${MAME_DIR}/src/devices/machine/pci-smbus.cpp
		${MAME_DIR}/src/devices/machine/pci-smbus.h
		${MAME_DIR}/src/devices/machine/i82541.cpp
		${MAME_DIR}/src/devices/machine/i82541.h
		${MAME_DIR}/src/devices/machine/i82875p.cpp
		${MAME_DIR}/src/devices/machine/i82875p.h
		${MAME_DIR}/src/devices/machine/i6300esb.cpp
		${MAME_DIR}/src/devices/machine/i6300esb.h
		${MAME_DIR}/src/devices/machine/i82439hx.cpp
		${MAME_DIR}/src/devices/machine/i82439hx.h
		${MAME_DIR}/src/devices/machine/i82439tx.cpp
		${MAME_DIR}/src/devices/machine/i82439tx.h
		${MAME_DIR}/src/devices/machine/i82371sb.cpp
		${MAME_DIR}/src/devices/machine/i82371sb.h
		${MAME_DIR}/src/devices/machine/lpc.h
		${MAME_DIR}/src/devices/machine/lpc-acpi.cpp
		${MAME_DIR}/src/devices/machine/lpc-acpi.h
		${MAME_DIR}/src/devices/machine/lpc-rtc.cpp
		${MAME_DIR}/src/devices/machine/lpc-rtc.h
		${MAME_DIR}/src/devices/machine/lpc-pit.cpp
		${MAME_DIR}/src/devices/machine/lpc-pit.h
		${MAME_DIR}/src/devices/machine/vrc4373.cpp
		${MAME_DIR}/src/devices/machine/vrc4373.h
		${MAME_DIR}/src/devices/machine/vrc5074.cpp
		${MAME_DIR}/src/devices/machine/vrc5074.h
		${MAME_DIR}/src/devices/machine/gt64xxx.cpp
		${MAME_DIR}/src/devices/machine/gt64xxx.h
		${MAME_DIR}/src/devices/machine/sis85c496.cpp
		${MAME_DIR}/src/devices/machine/sis85c496.h
	)
endif()

###################################################
##
##@src/devices/machine/pckeybrd.h,list(APPEND MACHINES PCKEYBRD)
###################################################

if("PCKEYBRD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pckeybrd.cpp
		${MAME_DIR}/src/devices/machine/pckeybrd.h
	)
endif()

###################################################
##
##@src/devices/machine/phi.h,list(APPEND MACHINES PHI)
###################################################

if("PHI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/phi.cpp
		${MAME_DIR}/src/devices/machine/phi.h
	)
endif()

###################################################
##
##@src/devices/machine/pic8259.h,list(APPEND MACHINES PIC8259)
###################################################

if("PIC8259" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pic8259.cpp
		${MAME_DIR}/src/devices/machine/pic8259.h
	)
endif()

###################################################
##
##@src/devices/machine/pit8253.h,list(APPEND MACHINES PIT8253)
###################################################

if("PIT8253" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pit8253.cpp
		${MAME_DIR}/src/devices/machine/pit8253.h
	)
endif()

###################################################
##
##@src/devices/machine/pla.h,list(APPEND MACHINES PLA)
###################################################

if("PLA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pla.cpp
		${MAME_DIR}/src/devices/machine/pla.h
	)
endif()

###################################################
##
##@src/devices/machine/pxa255.h,list(APPEND MACHINES PXA255)
###################################################

if("PXA255" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pxa255.cpp
		${MAME_DIR}/src/devices/machine/pxa255.h
		${MAME_DIR}/src/devices/machine/pxa255defs.h
	)
endif()

###################################################
##
##@src/devices/machine/r10696.h,list(APPEND MACHINES R10696)
###################################################

if("R10696" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/r10696.cpp
		${MAME_DIR}/src/devices/machine/r10696.h
	)
endif()

###################################################
##
##@src/devices/machine/r10788.h,list(APPEND MACHINES R10788)
###################################################

if("R10788" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/r10788.cpp
		${MAME_DIR}/src/devices/machine/r10788.h
	)
endif()

###################################################
##
##@src/devices/machine/ra17xx.h,list(APPEND MACHINES RA17XX)
###################################################

if("RA17XX" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ra17xx.cpp
		${MAME_DIR}/src/devices/machine/ra17xx.h
	)
endif()

###################################################
##
##@src/devices/machine/rf5c296.h,list(APPEND MACHINES RF5C296)
###################################################

if("RF5C296" IN_LIST MACHINES)
	list(APPEND MACHINES "PCCARD")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rf5c296.cpp
		${MAME_DIR}/src/devices/machine/rf5c296.h
	)
endif()

###################################################
##
##@src/devices/machine/ripple_counter.h,list(APPEND MACHINES RIPPLE_COUNTER)
###################################################

if("RIPPLE_COUNTER" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ripple_counter.cpp
		${MAME_DIR}/src/devices/machine/ripple_counter.h
	)
endif()

###################################################
##
##@src/devices/machine/roc10937.h,list(APPEND MACHINES ROC10937)
###################################################

if("ROC10937" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/roc10937.cpp
		${MAME_DIR}/src/devices/machine/roc10937.h
	)
endif()

###################################################
##
##@src/devices/machine/rp5c01.h,list(APPEND MACHINES RP5C01)
###################################################

if("RP5C01" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rp5c01.cpp
		${MAME_DIR}/src/devices/machine/rp5c01.h
	)
endif()

###################################################
##
##@src/devices/machine/rp5c15.h,list(APPEND MACHINES RP5C15)
###################################################

if("RP5C15" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rp5c15.cpp
		${MAME_DIR}/src/devices/machine/rp5c15.h
	)
endif()

###################################################
##
##@src/devices/machine/rp5h01.h,list(APPEND MACHINES RP5H01)
###################################################

if("RP5H01" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rp5h01.cpp
		${MAME_DIR}/src/devices/machine/rp5h01.h
	)
endif()

###################################################
##
##@src/devices/machine/64h156.h,list(APPEND MACHINES R64H156)
###################################################

if("R64H156" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/64h156.cpp
		${MAME_DIR}/src/devices/machine/64h156.h
	)
endif()

###################################################
##
##@src/devices/machine/rstbuf.h,list(APPEND MACHINES RSTBUF)
###################################################

if("RSTBUF" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rstbuf.cpp
		${MAME_DIR}/src/devices/machine/rstbuf.h
	)
endif()

###################################################
##
##@src/devices/machine/rtc4543.h,list(APPEND MACHINES RTC4543)
###################################################

if("RTC4543" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rtc4543.cpp
		${MAME_DIR}/src/devices/machine/rtc4543.h
	)
endif()

###################################################
##
##@src/devices/machine/rtc65271.h,list(APPEND MACHINES RTC65271)
###################################################

if("RTC65271" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rtc65271.cpp
		${MAME_DIR}/src/devices/machine/rtc65271.h
	)
endif()

###################################################
##
##@src/devices/machine/rtc9701.h,list(APPEND MACHINES RTC9701)
###################################################

if("RTC9701" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/rtc9701.cpp
		${MAME_DIR}/src/devices/machine/rtc9701.h
	)
endif()

###################################################
##
##@src/devices/machine/s2636.h,list(APPEND MACHINES S2636)
###################################################

if("S2636" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/s2636.cpp
		${MAME_DIR}/src/devices/machine/s2636.h
	)
endif()

###################################################
##
##@src/devices/machine/s3520cf.h,list(APPEND MACHINES S3520CF)
###################################################

if("S3520CF" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/s3520cf.cpp
		${MAME_DIR}/src/devices/machine/s3520cf.h
	)
endif()

###################################################
##
##@src/devices/machine/s3c2400.h,list(APPEND MACHINES S3C24XX)
##@src/devices/machine/s3c2410.h,list(APPEND MACHINES S3C24XX)
##@src/devices/machine/s3c2440.h,list(APPEND MACHINES S3C24XX)
###################################################

if("S3C24XX" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/s3c2400.cpp
		${MAME_DIR}/src/devices/machine/s3c2400.h
		${MAME_DIR}/src/devices/machine/s3c2410.cpp
		${MAME_DIR}/src/devices/machine/s3c2410.h
		${MAME_DIR}/src/devices/machine/s3c2440.cpp
		${MAME_DIR}/src/devices/machine/s3c2440.h
		${MAME_DIR}/src/devices/machine/s3c24xx.cpp
		${MAME_DIR}/src/devices/machine/s3c24xx.h
		${MAME_DIR}/src/devices/machine/s3c24xx.hxx
	)
endif()

###################################################
##
##@src/devices/machine/s3c44b0.h,list(APPEND MACHINES S3C44B0)
###################################################

if("S3C44B0" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/s3c44b0.cpp
		${MAME_DIR}/src/devices/machine/s3c44b0.h
	)
endif()

###################################################
##
##@src/devices/machine/sa1110.h,list(APPEND MACHINES SA1110)
###################################################

if("SA1110" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sa1110.cpp
		${MAME_DIR}/src/devices/machine/sa1110.h
	)
endif()

###################################################
##
##@src/devices/machine/sa1111.h,list(APPEND MACHINES SA1111)
###################################################

if("SA1111" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sa1111.cpp
		${MAME_DIR}/src/devices/machine/sa1111.h
	)
endif()

###################################################
##
##@src/devices/machine/saa1043.h,list(APPEND MACHINES SAA1043)
###################################################

if("SAA1043" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/saa1043.cpp
		${MAME_DIR}/src/devices/machine/saa1043.h
	)
endif()

###################################################
##
##@src/devices/machine/scc68070.h,list(APPEND MACHINES SCC68070)
###################################################
if("SCC68070" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scc68070.cpp
		${MAME_DIR}/src/devices/machine/scc68070.h
	)
endif()

###################################################
##
##@src/devices/machine/scn_pci.h,list(APPEND MACHINES SCN_PCI)
###################################################

if("SCN_PCI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scn_pci.cpp
		${MAME_DIR}/src/devices/machine/scn_pci.h
	)
endif()

###################################################
##
##@src/devices/machine/scoop.h,list(APPEND MACHINES SCOOP)
###################################################
if("SCOOP" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scoop.cpp
		${MAME_DIR}/src/devices/machine/scoop.h
	)
endif()

###################################################
##
##@src/devices/machine/scnxx562.h,list(APPEND MACHINES DUSCC)
###################################################

if("DUSCC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scnxx562.cpp
		${MAME_DIR}/src/devices/machine/scnxx562.h
	)
endif()

###################################################
##
##@src/devices/machine/sda2006.h,list(APPEND MACHINES SDA2006)
###################################################

if("SDA2006" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sda2006.cpp
		${MAME_DIR}/src/devices/machine/sda2006.h
	)
endif()

###################################################
##
##@src/devices/machine/sensorboard.h,list(APPEND MACHINES SENSORBOARD)
###################################################

if("SENSORBOARD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sensorboard.cpp
		${MAME_DIR}/src/devices/machine/sensorboard.h
	)
endif()

###################################################
##
##@src/devices/machine/serflash.h,list(APPEND MACHINES SERFLASH)
###################################################

if("SERFLASH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/serflash.cpp
		${MAME_DIR}/src/devices/machine/serflash.h
	)
endif()

###################################################
##
##@src/devices/machine/smc91c9x.h,list(APPEND MACHINES SMC91C9X)
###################################################

if("SMC91C9X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/smc91c9x.cpp
		${MAME_DIR}/src/devices/machine/smc91c9x.h
	)
endif()

###################################################
##
##@src/devices/machine/smpc.h,list(APPEND MACHINES SMPC)
###################################################

if("SMPC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/smpc.cpp
		${MAME_DIR}/src/devices/machine/smpc.h
	)
endif()

###################################################
##
##@src/devices/machine/sega_scu.h,list(APPEND MACHINES SEGA_SCU)
###################################################

if("SEGA_SCU" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sega_scu.cpp
		${MAME_DIR}/src/devices/machine/sega_scu.h
	)
endif()

###################################################
##
##@src/devices/machine/spg2xx.h,list(APPEND MACHINES SPG2XX)
##@src/devices/machine/spg110.h,list(APPEND MACHINES SPG2XX)
##@src/devices/machine/generalplus_gpl16250soc.h,list(APPEND MACHINES SPG2XX)
###################################################

if("SPG2XX" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/spg2xx.cpp
		${MAME_DIR}/src/devices/machine/spg2xx.h
		${MAME_DIR}/src/devices/machine/spg2xx_audio.cpp
		${MAME_DIR}/src/devices/machine/spg2xx_audio.h
		${MAME_DIR}/src/devices/machine/spg2xx_io.cpp
		${MAME_DIR}/src/devices/machine/spg2xx_io.h
		${MAME_DIR}/src/devices/machine/spg2xx_sysdma.cpp
		${MAME_DIR}/src/devices/machine/spg2xx_sysdma.h
		${MAME_DIR}/src/devices/machine/spg2xx_video.cpp
		${MAME_DIR}/src/devices/machine/spg2xx_video.h
		${MAME_DIR}/src/devices/machine/spg110.cpp
		${MAME_DIR}/src/devices/machine/spg110.h
		${MAME_DIR}/src/devices/machine/spg110_video.cpp
		${MAME_DIR}/src/devices/machine/spg110_video.h
		${MAME_DIR}/src/devices/machine/generalplus_gpl16250soc.cpp
		${MAME_DIR}/src/devices/machine/generalplus_gpl16250soc.h
		${MAME_DIR}/src/devices/machine/generalplus_gpl16250soc_video.cpp
		${MAME_DIR}/src/devices/machine/generalplus_gpl16250soc_video.h
		${MAME_DIR}/src/devices/machine/spg_renderer.cpp
		${MAME_DIR}/src/devices/machine/spg_renderer.h
	)
endif()

###################################################
##
##@src/devices/machine/spg290_cdservo.h,list(APPEND MACHINES SPG290)
##@src/devices/machine/spg290_timer.h,list(APPEND MACHINES SPG290)
##@src/devices/machine/spg290_i2c.h,list(APPEND MACHINES SPG290)
##@src/devices/machine/spg290_ppu.h,list(APPEND MACHINES SPG290)
###################################################

if("SPG290" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/spg290_cdservo.cpp
		${MAME_DIR}/src/devices/machine/spg290_cdservo.h
		${MAME_DIR}/src/devices/machine/spg290_timer.cpp
		${MAME_DIR}/src/devices/machine/spg290_timer.h
		${MAME_DIR}/src/devices/machine/spg290_i2c.cpp
		${MAME_DIR}/src/devices/machine/spg290_i2c.h
		${MAME_DIR}/src/devices/machine/spg290_ppu.cpp
		${MAME_DIR}/src/devices/machine/spg290_ppu.h
	)
endif()
###################################################
##
##@src/devices/machine/stvcd.h,list(APPEND MACHINES STVCD)
###################################################

if("STVCD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/stvcd.cpp
		${MAME_DIR}/src/devices/machine/stvcd.h
	)
endif()

###################################################
##
##@src/devices/machine/swtpc8212.h,list(APPEND MACHINES SWTPC8212)
###################################################

if("SWTPC8212" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/swtpc8212.cpp
		${MAME_DIR}/src/devices/machine/swtpc8212.h
	)
endif()

###################################################
##
##
###################################################

if(("ATA" IN_LIST BUSES) OR ("SCSI" IN_LIST BUSES))
	list(APPEND MACHINES "T10")
endif()

if("T10" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/t10mmc.cpp
		${MAME_DIR}/src/devices/machine/t10mmc.h
		${MAME_DIR}/src/devices/machine/t10sbc.cpp
		${MAME_DIR}/src/devices/machine/t10sbc.h
		${MAME_DIR}/src/devices/machine/t10spc.cpp
		${MAME_DIR}/src/devices/machine/t10spc.h
	)
endif()


###################################################
##
##@src/devices/machine/smartboard.h,list(APPEND MACHINES TASC_SB30)
###################################################

if("TASC_SB30" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/smartboard.cpp
		${MAME_DIR}/src/devices/machine/smartboard.h
	)
endif()

###################################################
##
##@src/devices/machine/tc009xlvc.h,list(APPEND MACHINES TC0091LVC)
###################################################

if("TC0091LVC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tc009xlvc.cpp
		${MAME_DIR}/src/devices/machine/tc009xlvc.h
	)
endif()

###################################################
##
##@src/devices/machine/tdc1008.h,list(APPEND MACHINES TDC1008)
###################################################

if("TDC1008" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tdc1008.cpp
		${MAME_DIR}/src/devices/machine/tdc1008.h
	)
endif()

###################################################
##
##@src/devices/machine/te7750.h,list(APPEND MACHINES TE7750)
###################################################

if("TE7750" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/te7750.cpp
		${MAME_DIR}/src/devices/machine/te7750.h
	)
endif()

###################################################
##
##@src/devices/machine/ticket.h,list(APPEND MACHINES TICKET)
###################################################

if("TICKET" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ticket.cpp
		${MAME_DIR}/src/devices/machine/ticket.h
	)
endif()

###################################################
##
##@src/devices/machine/timekpr.h,list(APPEND MACHINES TIMEKPR)
###################################################

if("TIMEKPR" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/timekpr.cpp
		${MAME_DIR}/src/devices/machine/timekpr.h
	)
endif()

###################################################
##
##@src/devices/machine/tmc0430.h,list(APPEND MACHINES TMC0430)
###################################################

if("TMC0430" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tmc0430.cpp
		${MAME_DIR}/src/devices/machine/tmc0430.h
	)
endif()

###################################################
##
##@src/devices/machine/tmc0999.h,list(APPEND MACHINES TMC0999)
###################################################

if("TMC0999" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tmc0999.cpp
		${MAME_DIR}/src/devices/machine/tmc0999.h
	)
endif()

###################################################
##
##@src/devices/machine/tmc208k.h,list(APPEND MACHINES TMC208K)
###################################################

if("TMC208K" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tmc208k.cpp
		${MAME_DIR}/src/devices/machine/tmc208k.h
	)
endif()

###################################################
##
##@src/devices/machine/tmp68301.h,list(APPEND MACHINES TMP68301)
###################################################

if("TMP68301" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tmp68301.cpp
		${MAME_DIR}/src/devices/machine/tmp68301.h
	)
endif()

###################################################
##
##@src/devices/machine/tms1024.h,list(APPEND MACHINES TMS1024)
###################################################

if("TMS1024" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms1024.cpp
		${MAME_DIR}/src/devices/machine/tms1024.h
	)
endif()

###################################################
##
##@src/devices/machine/tms5501.h,list(APPEND MACHINES TMS5501)
###################################################

if("TMS5501" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms5501.cpp
		${MAME_DIR}/src/devices/machine/tms5501.h
	)
endif()

###################################################
##
##@src/devices/machine/tms6100.h,list(APPEND MACHINES TMS6100)
###################################################

if("TMS6100" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms6100.cpp
		${MAME_DIR}/src/devices/machine/tms6100.h
	)
endif()

###################################################
##
##@src/devices/machine/tms9901.h,list(APPEND MACHINES TMS9901)
###################################################

if("TMS9901" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms9901.cpp
		${MAME_DIR}/src/devices/machine/tms9901.h
	)
endif()

###################################################
##
##@src/devices/machine/tms9902.h,list(APPEND MACHINES TMS9902)
###################################################

if("TMS9902" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms9902.cpp
		${MAME_DIR}/src/devices/machine/tms9902.h
	)
endif()

###################################################
##
##@src/devices/machine/tms9914.h,list(APPEND MACHINES TMS9914)
###################################################

if("TMS9914" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tms9914.cpp
		${MAME_DIR}/src/devices/machine/tms9914.h
	)
endif()

###################################################
##
##@src/devices/machine/tsb12lv01a.h,list(APPEND MACHINES TSB12LV01A)
###################################################

if("TSB12LV01A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tsb12lv01a.cpp
		${MAME_DIR}/src/devices/machine/tsb12lv01a.h
	)
endif()

###################################################
##
##@src/devices/machine/tube.h,list(APPEND MACHINES TUBE)
###################################################

if("TUBE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/tube.cpp
		${MAME_DIR}/src/devices/machine/tube.h
	)
endif()

###################################################
##
##@src/devices/machine/ucb1200.h,list(APPEND MACHINES UCB1200)
###################################################

if("UCB1200" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ucb1200.cpp
		${MAME_DIR}/src/devices/machine/ucb1200.h
	)
endif()

###################################################
##
##@src/devices/machine/upd1990a.h,list(APPEND MACHINES UPD1990A)
###################################################

if("UPD1990A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd1990a.cpp
		${MAME_DIR}/src/devices/machine/upd1990a.h
	)
endif()

###################################################
##
##@src/devices/machine/upd4992.h,list(APPEND MACHINES UPD4992)
###################################################

if("UPD4992" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd4992.cpp
		${MAME_DIR}/src/devices/machine/upd4992.h
	)
endif()


###################################################
##
##@src/devices/machine/upd4701.h,list(APPEND MACHINES UPD4701)
###################################################

if("UPD4701" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd4701.cpp
		${MAME_DIR}/src/devices/machine/upd4701.h
	)
endif()

###################################################
##
##@src/devices/machine/upd7001.h,list(APPEND MACHINES UPD7001)
###################################################

if("UPD7001" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd7001.cpp
		${MAME_DIR}/src/devices/machine/upd7001.h
	)
endif()

###################################################
##
##@src/devices/machine/upd7002.h,list(APPEND MACHINES UPD7002)
###################################################

if("UPD7002" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd7002.cpp
		${MAME_DIR}/src/devices/machine/upd7002.h
	)
endif()

###################################################
##
##@src/devices/machine/upd7004.h,list(APPEND MACHINES UPD7004)
###################################################

if("UPD7004" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd7004.cpp
		${MAME_DIR}/src/devices/machine/upd7004.h
	)
endif()

###################################################
##
##@src/devices/machine/upd71071.h,list(APPEND MACHINES UPD71071)
###################################################

if("UPD71071" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd71071.cpp
		${MAME_DIR}/src/devices/machine/upd71071.h
	)
endif()

###################################################
##
##@src/devices/machine/upd765.h,list(APPEND MACHINES UPD765)
###################################################

if("UPD765" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/upd765.cpp
		${MAME_DIR}/src/devices/machine/upd765.h
	)
endif()

###################################################
##
##@src/devices/machine/v3021.h,list(APPEND MACHINES V3021)
###################################################

if("V3021" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/v3021.cpp
		${MAME_DIR}/src/devices/machine/v3021.h
	)
endif()

###################################################
##
##@src/devices/machine/vic_pl192.h,list(APPEND MACHINES VIC_PL192)
###################################################

if("VIC_PL192" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/vic_pl192.cpp
		${MAME_DIR}/src/devices/machine/vic_pl192.h
	)
endif()

###################################################
##
##@src/devices/machine/wd_fdc.h,list(APPEND MACHINES WD_FDC)
###################################################

if("WD_FDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd_fdc.cpp
		${MAME_DIR}/src/devices/machine/wd_fdc.h
	)
endif()

###################################################
##
##@src/devices/machine/wd1000.h,list(APPEND MACHINES WD1000)
###################################################

if("WD1000" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd1000.cpp
		${MAME_DIR}/src/devices/machine/wd1000.h
	)
endif()

###################################################
##
##@src/devices/machine/wd1010.h,list(APPEND MACHINES WD1010)
###################################################

if("WD1010" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd1010.cpp
		${MAME_DIR}/src/devices/machine/wd1010.h
	)
endif()

###################################################
##
##@src/devices/machine/wd11c00_17.h,list(APPEND MACHINES WD11C00_17)
###################################################

if("WD11C00_17" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd11c00_17.cpp
		${MAME_DIR}/src/devices/machine/wd11c00_17.h
	)
endif()

###################################################
##
##@src/devices/machine/wd2010.h,list(APPEND MACHINES WD2010)
###################################################

if("WD2010" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd2010.cpp
		${MAME_DIR}/src/devices/machine/wd2010.h
	)
endif()

###################################################
##
##@src/devices/machine/wd33c9x.h,list(APPEND MACHINES WD33C9X)
###################################################

if("WD33C9X" IN_LIST MACHINES)
	list(APPEND MACHINES "SCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd33c9x.cpp
		${MAME_DIR}/src/devices/machine/wd33c9x.h
	)
endif()

###################################################
##
##@src/devices/machine/wd7600.h,list(APPEND MACHINES WD7600)
###################################################

if("WD7600" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wd7600.cpp
		${MAME_DIR}/src/devices/machine/wd7600.h
	)
endif()

###################################################
##
##@src/devices/machine/x2201.h,list(APPEND MACHINES X2201)
###################################################

if("X2201" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/x2201.cpp
		${MAME_DIR}/src/devices/machine/x2201.h
	)
endif()

###################################################
##
##@src/devices/machine/x2212.h,list(APPEND MACHINES X2212)
###################################################

if("X2212" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/x2212.cpp
		${MAME_DIR}/src/devices/machine/x2212.h
	)
endif()

###################################################
##
##@src/devices/machine/x76f041.h,list(APPEND MACHINES X76F041)
###################################################

if("X76F041" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/x76f041.cpp
		${MAME_DIR}/src/devices/machine/x76f041.h
	)
endif()

###################################################
##
##@src/devices/machine/x76f100.h,list(APPEND MACHINES X76F100)
###################################################

if("X76F100" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/x76f100.cpp
		${MAME_DIR}/src/devices/machine/x76f100.h
	)
endif()

###################################################
##
##@src/devices/machine/ym2148.h,list(APPEND MACHINES YM2148)
###################################################

if("YM2148" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ym2148.cpp
		${MAME_DIR}/src/devices/machine/ym2148.h
	)
endif()

###################################################
##
##@src/devices/machine/ym3802.h,list(APPEND MACHINES YM3802)
###################################################

if("YM3802" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ym3802.cpp
		${MAME_DIR}/src/devices/machine/ym3802.h
	)
endif()

###################################################
##
##@src/devices/machine/z80ctc.h,list(APPEND MACHINES Z80CTC)
###################################################

if("Z80CTC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80ctc.cpp
		${MAME_DIR}/src/devices/machine/z80ctc.h
	)
endif()

###################################################
##
##@src/devices/machine/z80sio.h,list(APPEND MACHINES Z80SIO)
###################################################

if("Z80SIO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80sio.cpp
		${MAME_DIR}/src/devices/machine/z80sio.h
	)
endif()

###################################################
##
##@src/devices/machine/z80scc.h,list(APPEND MACHINES Z80SCC)
###################################################

if("Z80SCC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80scc.cpp
		${MAME_DIR}/src/devices/machine/z80scc.h
	)
endif()

###################################################
##
##@src/devices/machine/z80dma.h,list(APPEND MACHINES Z80DMA)
###################################################

if("Z80DMA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80dma.cpp
		${MAME_DIR}/src/devices/machine/z80dma.h
	)
endif()

###################################################
##
##@src/devices/machine/z80pio.h,list(APPEND MACHINES Z80PIO)
###################################################

if("Z80PIO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80pio.cpp
		${MAME_DIR}/src/devices/machine/z80pio.h
	)
endif()

###################################################
##
##@src/devices/machine/z80sti.h,list(APPEND MACHINES Z80STI)
###################################################

if("Z80STI" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80sti.cpp
		${MAME_DIR}/src/devices/machine/z80sti.h
	)
endif()

###################################################
##
##@src/devices/machine/z8536.h,list(APPEND MACHINES Z8536)
###################################################

if("Z8536" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z8536.cpp
		${MAME_DIR}/src/devices/machine/z8536.h
	)
endif()

###################################################
##
##@src/devices/machine/pccard.h,list(APPEND MACHINES PCCARD)
###################################################

if("PCCARD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pccard.cpp
		${MAME_DIR}/src/devices/machine/pccard.h
	)
endif()

###################################################
##
##@src/devices/machine/i8255.h,list(APPEND MACHINES I8255)
###################################################

if("I8255" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8255.cpp
		${MAME_DIR}/src/devices/machine/i8255.h
		${MAME_DIR}/src/devices/machine/mb89363b.cpp
		${MAME_DIR}/src/devices/machine/mb89363b.h
	)
endif()

###################################################
##
##@src/devices/machine/ncr5380.h,list(APPEND MACHINES NCR5380)
###################################################

if("NCR5380" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ncr5380.cpp
		${MAME_DIR}/src/devices/machine/ncr5380.h
	)
endif()

###################################################
##
##@src/devices/machine/ncr5390.h,list(APPEND MACHINES NCR5390)
###################################################

if("NCR5390" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ncr5390.cpp
		${MAME_DIR}/src/devices/machine/ncr5390.h
	)
endif()

###################################################
##
##@src/devices/machine/mm58167.h,list(APPEND MACHINES MM58167)
###################################################

if("MM58167" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm58167.cpp
		${MAME_DIR}/src/devices/machine/mm58167.h
	)
endif()

###################################################
##
##@src/devices/machine/mm58174.h,list(APPEND MACHINES MM58174)
###################################################

if("MM58174" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mm58174.cpp
		${MAME_DIR}/src/devices/machine/mm58174.h
	)
endif()


###################################################
##
##@src/devices/machine/dp8390.h,list(APPEND MACHINES DP8390)
###################################################

if("DP8390" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/dp8390.cpp
		${MAME_DIR}/src/devices/machine/dp8390.h
	)
endif()

###################################################
##
##@src/devices/machine/dp83932c.h,list(APPEND MACHINES DP83932C)
###################################################

if("DP83932C" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/dp83932c.cpp
		${MAME_DIR}/src/devices/machine/dp83932c.h
	)
endif()

###################################################
##
##@src/devices/machine/dp8573.h,list(APPEND MACHINES DP8573)
###################################################

if("DP8573" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/dp8573.cpp
		${MAME_DIR}/src/devices/machine/dp8573.h
	)
endif()

###################################################
##
##@src/devices/machine/pc_lpt.h,list(APPEND MACHINES PC_LPT)
###################################################

if("PC_LPT" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pc_lpt.cpp
		${MAME_DIR}/src/devices/machine/pc_lpt.h
	)
endif()

###################################################
##
##@src/devices/machine/pc_fdc.h,list(APPEND MACHINES PC_FDC)
###################################################

if("PC_FDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pc_fdc.cpp
		${MAME_DIR}/src/devices/machine/pc_fdc.h
	)
endif()

###################################################
##
##@src/devices/machine/mpu401.h,list(APPEND MACHINES MPU401)
###################################################

if("MPU401" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mpu401.cpp
		${MAME_DIR}/src/devices/machine/mpu401.h
	)
endif()

###################################################
##
##@src/devices/machine/at_keybc.h,list(APPEND MACHINES AT_KEYBC)
###################################################

if("AT_KEYBC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/at_keybc.cpp
		${MAME_DIR}/src/devices/machine/at_keybc.h
	)
endif()


###################################################
##
##@src/devices/machine/hdc92x4.h,list(APPEND MACHINES HDC9234)
###################################################

if("HDC9234" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/hdc92x4.cpp
		${MAME_DIR}/src/devices/machine/hdc92x4.h
	)
endif()

###################################################
##
##@src/devices/machine/strata.h,list(APPEND MACHINES STRATA)
###################################################

if("STRATA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/strata.cpp
		${MAME_DIR}/src/devices/machine/strata.h
	)
endif()

###################################################
##
##@src/devices/machine/steppers.h,list(APPEND MACHINES STEPPERS)
###################################################

if("STEPPERS" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/steppers.cpp
		${MAME_DIR}/src/devices/machine/steppers.h
	)
endif()

###################################################
##
##@src/devices/machine/corvushd.h,list(APPEND MACHINES CORVUSHD)
###################################################
if("CORVUSHD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/corvushd.cpp
		${MAME_DIR}/src/devices/machine/corvushd.h
	)
endif()

###################################################
##
##@src/devices/machine/wozfdc.h,list(APPEND MACHINES WOZFDC)
###################################################
if("WOZFDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wozfdc.cpp
		${MAME_DIR}/src/devices/machine/wozfdc.h
	)
endif()

###################################################
##
##@src/devices/machine/diablo_hd.h,list(APPEND MACHINES DIABLO_HD)
###################################################
if("DIABLO_HD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/diablo_hd.cpp
		${MAME_DIR}/src/devices/machine/diablo_hd.h
	)
endif()

###################################################
##
##@src/devices/machine/fdc37c665gt.h,list(APPEND MACHINES FDC37C665GT)
###################################################

if("FDC37C665GT" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/fdc37c665gt.cpp
		${MAME_DIR}/src/devices/machine/fdc37c665gt.h
	)
endif()

###################################################
##
##@src/devices/machine/pci9050.h,list(APPEND MACHINES PCI9050)
###################################################

if("PCI9050" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pci9050.cpp
		${MAME_DIR}/src/devices/machine/pci9050.h
	)
endif()

###################################################
##
##@src/devices/machine/netlist.h,list(APPEND MACHINES NETLIST)
###################################################

if("NETLIST" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/netlist.cpp
		${MAME_DIR}/src/devices/machine/netlist.h
	)
endif()

###################################################
##
##@src/devices/machine/nsc810.h,list(APPEND MACHINES NSC810)
###################################################

if("NSC810" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/nsc810.cpp
		${MAME_DIR}/src/devices/machine/nsc810.h
	)
endif()

###################################################
##
##@src/devices/machine/vt82c496.h,list(APPEND MACHINES VT82C496)
###################################################

if("VT82C496" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/vt82c496.cpp
		${MAME_DIR}/src/devices/machine/vt82c496.h
	)
endif()

###################################################
##
##@src/devices/machine/fdc37c93x.h,list(APPEND MACHINES FDC37C93X)
###################################################

if("FDC37C93X" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/fdc37c93x.cpp
		${MAME_DIR}/src/devices/machine/fdc37c93x.h
	)
endif()

###################################################
##
##@src/devices/machine/pdc.h,list(APPEND MACHINES PDC)
###################################################

if("PDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/pdc.cpp
		${MAME_DIR}/src/devices/machine/pdc.h
	)
endif()

###################################################
##
##@src/devices/machine/genpc.h,list(APPEND MACHINES GENPC)
###################################################

if("GENPC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/genpc.cpp
		${MAME_DIR}/src/devices/machine/genpc.h
	)
endif()

###################################################
##
##@src/devices/machine/gen_latch.h,list(APPEND MACHINES GEN_LATCH)
###################################################

if("GEN_LATCH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/gen_latch.cpp
		${MAME_DIR}/src/devices/machine/gen_latch.h
	)
endif()

###################################################
##
##@src/devices/machine/fdc_pll.h,list(APPEND MACHINES FDC_PLL)
###################################################

if("FDC_PLL" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/fdc_pll.cpp
		${MAME_DIR}/src/devices/machine/fdc_pll.h
	)
endif()

###################################################
##
##@src/devices/machine/watchdog.h,list(APPEND MACHINES WATCHDOG)
###################################################

if("WATCHDOG" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/watchdog.cpp
		${MAME_DIR}/src/devices/machine/watchdog.h
	)
endif()


###################################################
##
##@src/devices/machine/smartmed.h,list(APPEND MACHINES SMARTMEDIA)
###################################################
if("SMARTMEDIA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/smartmed.cpp
		${MAME_DIR}/src/devices/machine/smartmed.h
	)
endif()

###################################################
##
##@src/devices/machine/appldriv.h,list(APPEND MACHINES APPLE_DRIVE)
###################################################
if("APPLE_DRIVE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/appldriv.cpp
		${MAME_DIR}/src/devices/machine/appldriv.h
	)
endif()

###################################################
##
##@src/devices/machine/applefdc.h,list(APPEND MACHINES APPLE_FDC)
###################################################
if("APPLE_FDC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/applefdc.cpp
		${MAME_DIR}/src/devices/machine/applefdc.h
	)
endif()

###################################################
##
##@src/devices/machine/sonydriv.h,list(APPEND MACHINES SONY_DRIVE)
###################################################
if("SONY_DRIVE" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sonydriv.cpp
		${MAME_DIR}/src/devices/machine/sonydriv.h
	)
endif()

###################################################
##
##@src/devices/machine/scnxx562.h,list(APPEND MACHINES SCNXX562)
###################################################
if("SCNXX562" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scnxx562.cpp
		${MAME_DIR}/src/devices/machine/scnxx562.h
	)
endif()

###################################################
##
##@src/devices/machine/input_merger.h,list(APPEND MACHINES INPUT_MERGER)
###################################################
if("INPUT_MERGER" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/input_merger.cpp
		${MAME_DIR}/src/devices/machine/input_merger.h
	)
endif()

###################################################
##
##@src/devices/machine/k054321.h,list(APPEND MACHINES K054321)
###################################################
if("K054321" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/k054321.cpp
		${MAME_DIR}/src/devices/machine/k054321.h
	)
endif()

###################################################
##
##@src/devices/machine/smioc.h,list(APPEND MACHINES SMIOC)
###################################################

if("SMIOC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/smioc.cpp
		${MAME_DIR}/src/devices/machine/smioc.h
	)
endif()

###################################################
##
##@src/devices/machine/i82586.h,list(APPEND MACHINES I82586)
###################################################

if("I82586" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i82586.cpp
		${MAME_DIR}/src/devices/machine/i82586.h
	)
endif()

###################################################
##
##@src/devices/machine/adc0844.h,list(APPEND MACHINES ADC0844)
###################################################

if("ADC0844" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/adc0844.cpp
		${MAME_DIR}/src/devices/machine/adc0844.h
	)
endif()

###################################################
##
##@src/devices/machine/28fxxx.h,list(APPEND MACHINES 28FXXX)
###################################################

if("28FXXX" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/28fxxx.cpp
		${MAME_DIR}/src/devices/machine/28fxxx.h
	)
endif()

###################################################
##
##@src/devices/machine/gen_fifo.h,list(APPEND MACHINES GEN_FIFO)
###################################################

if("GEN_FIFO" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/gen_fifo.cpp
		${MAME_DIR}/src/devices/machine/gen_fifo.h
	)
endif()

###################################################
##
##@src/devices/machine/output_latch.h,list(APPEND MACHINES OUTPUT_LATCH)
###################################################

if("OUTPUT_LATCH" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/output_latch.cpp
		${MAME_DIR}/src/devices/machine/output_latch.h
	)
endif()

###################################################
##
##@src/devices/machine/z80daisy.h,list(APPEND MACHINES Z80DAISY)
###################################################

if("Z80DAISY" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z80daisy.cpp
		${MAME_DIR}/src/devices/machine/z80daisy.h
		${MAME_DIR}/src/devices/machine/z80daisy_generic.cpp
		${MAME_DIR}/src/devices/machine/z80daisy_generic.h
	)
endif()

###################################################
##
##@src/devices/machine/i8291a.h,list(APPEND MACHINES I8291A)
###################################################

if("I8291A" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i8291a.cpp
		${MAME_DIR}/src/devices/machine/i8291a.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2dma.h,list(APPEND MACHINES PS2DMAC)
###################################################

if("PS2DMAC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2dma.cpp
		${MAME_DIR}/src/devices/machine/ps2dma.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2intc.h,list(APPEND MACHINES PS2INTC)
###################################################

if("PS2INTC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2intc.cpp
		${MAME_DIR}/src/devices/machine/ps2intc.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2mc.h,list(APPEND MACHINES PS2MC)
###################################################

if("PS2MC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2mc.cpp
		${MAME_DIR}/src/devices/machine/ps2mc.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2pad.h,list(APPEND MACHINES PS2PAD)
###################################################

if("PS2PAD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2pad.cpp
		${MAME_DIR}/src/devices/machine/ps2pad.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2sif.h,list(APPEND MACHINES PS2SIF)
###################################################

if("PS2SIF" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2sif.cpp
		${MAME_DIR}/src/devices/machine/ps2sif.h
	)
endif()

###################################################
##
##@src/devices/machine/ps2timer.h,list(APPEND MACHINES PS2TIMER)
###################################################

if("PS2TIMER" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ps2timer.cpp
		${MAME_DIR}/src/devices/machine/ps2timer.h
	)
endif()

###################################################
##
##@src/devices/machine/iopcdvd.h,list(APPEND MACHINES IOPCDVD)
###################################################

if("IOPCDVD" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/iopcdvd.cpp
		${MAME_DIR}/src/devices/machine/iopcdvd.h
	)
endif()

###################################################
##
##@src/devices/machine/iopdma.h,list(APPEND MACHINES IOPDMA)
###################################################

if("IOPDMA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/iopdma.cpp
		${MAME_DIR}/src/devices/machine/iopdma.h
	)
endif()

###################################################
##
##@src/devices/machine/iopintc.h,list(APPEND MACHINES IOPINTC)
###################################################

if("IOPINTC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/iopintc.cpp
		${MAME_DIR}/src/devices/machine/iopintc.h
	)
endif()

###################################################
##
##@src/devices/machine/iopsio2.h,list(APPEND MACHINES IOPSIO2)
###################################################

if("IOPSIO2" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/iopsio2.cpp
		${MAME_DIR}/src/devices/machine/iopsio2.h
	)
endif()

###################################################
##
##@src/devices/machine/ioptimer.h,list(APPEND MACHINES IOPTIMER)
###################################################

if("IOPTIMER" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ioptimer.cpp
		${MAME_DIR}/src/devices/machine/ioptimer.h
	)
endif()

###################################################
##
##@src/devices/machine/sun4c_mmu.h,list(APPEND MACHINES SUN4C_MMU)
###################################################

if("SUN4C_MMU" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/sun4c_mmu.cpp
		${MAME_DIR}/src/devices/machine/sun4c_mmu.h
	)
endif()

###################################################
##
##@src/devices/machine/z8038.h,list(APPEND MACHINES Z8038)
###################################################

if("Z8038" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/z8038.cpp
		${MAME_DIR}/src/devices/machine/z8038.h
	)
endif()

###################################################
##
##@src/devices/machine/scc2698b.h,list(APPEND MACHINES SCC2698B)
###################################################

if("SCC2698B" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/scc2698b.cpp
		${MAME_DIR}/src/devices/machine/scc2698b.h
	)
endif()

###################################################
##
##@src/devices/machine/aic565.h,list(APPEND MACHINES AIC565)
###################################################

if("AIC565" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/aic565.cpp
		${MAME_DIR}/src/devices/machine/aic565.h
	)
endif()

###################################################
##
##@src/devices/machine/aic580.h,list(APPEND MACHINES AIC580)
###################################################

if("AIC580" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/aic580.cpp
		${MAME_DIR}/src/devices/machine/aic580.h
	)
endif()

###################################################
##
##@src/devices/machine/aic6250.h,list(APPEND MACHINES AIC6250)
###################################################

if("AIC6250" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/aic6250.cpp
		${MAME_DIR}/src/devices/machine/aic6250.h
	)
endif()

###################################################
##
##@src/devices/machine/dc7085.h,list(APPEND MACHINES DC7085)
###################################################

if("DC7085" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/dc7085.cpp
		${MAME_DIR}/src/devices/machine/dc7085.h
	)
endif()

###################################################
##
##@src/devices/machine/i82357.h,list(APPEND MACHINES I82357)
###################################################

if("I82357" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i82357.cpp
		${MAME_DIR}/src/devices/machine/i82357.h
	)
endif()

###################################################
##
##@src/devices/machine/xc1700e.h,list(APPEND MACHINES XC1700E)
###################################################

if("XC1700E" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/xc1700e.cpp
		${MAME_DIR}/src/devices/machine/xc1700e.h
	)
endif()

###################################################
##
##@src/devices/machine/edlc.h,list(APPEND MACHINES EDLC)
###################################################

if("EDLC" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/edlc.cpp
		${MAME_DIR}/src/devices/machine/edlc.h
	)
endif()

###################################################
##
##@src/devices/machine/wtl3132.h,list(APPEND MACHINES WTL3132)
###################################################

if("WTL3132" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/wtl3132.cpp
		${MAME_DIR}/src/devices/machine/wtl3132.h
	)
endif()

###################################################
##
##@src/devices/machine/vrender0,list(APPEND MACHINES VRENDER0)
###################################################

if("VRENDER0" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/vrender0.cpp
		${MAME_DIR}/src/devices/machine/vr0uart.cpp
		${MAME_DIR}/src/devices/machine/vrender0.h
	)
endif()

###################################################
##
##@src/devices/machine/i3001.h,list(APPEND MACHINES I3001)
###################################################

if("I3001" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i3001.cpp
		${MAME_DIR}/src/devices/machine/i3001.h
	)
endif()

###################################################
##
##@src/devices/machine/i3002.h,list(APPEND MACHINES I3002)
###################################################

if("I3002" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/i3002.cpp
		${MAME_DIR}/src/devices/machine/i3002.h
	)
endif()

###################################################
##
##@src/devices/machine/s_smp.h,list(APPEND MACHINES S_SMP)
###################################################

if("S_SMP" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/s_smp.cpp
		${MAME_DIR}/src/devices/machine/s_smp.h
	)
endif()

###################################################
##
##@src/devices/machine/cxd1185.h,list(APPEND MACHINES CXD1185)
###################################################

if("CXD1185" IN_LIST MACHINES)
	list(APPEND MACHINES "NSCSI")
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/cxd1185.cpp
		${MAME_DIR}/src/devices/machine/cxd1185.h
	)
endif()

###################################################
##
##@src/devices/machine/applefdintf.h,list(APPEND MACHINES APPLE_FDINTF)
###################################################
if("APPLE_FDINTF" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/applefdintf.cpp
		${MAME_DIR}/src/devices/machine/applefdintf.h
	)
endif()

###################################################
##
##@src/devices/machine/iwm.h,list(APPEND MACHINES IWM)
###################################################
if("IWM" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/iwm.cpp
		${MAME_DIR}/src/devices/machine/iwm.h
	)
endif()

###################################################
##
##@src/devices/machine/swim1.h,list(APPEND MACHINES SWIM1)
###################################################
if("SWIM1" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/swim1.cpp
		${MAME_DIR}/src/devices/machine/swim1.h
	)
endif()

###################################################
##
##@src/devices/machine/swim2.h,list(APPEND MACHINES SWIM2)
###################################################
if("SWIM2" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/swim2.cpp
		${MAME_DIR}/src/devices/machine/swim2.h
	)
endif()

###################################################
##
##@src/devices/machine/swim3.h,list(APPEND MACHINES SWIM3)
###################################################
if("SWIM3" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/swim3.cpp
		${MAME_DIR}/src/devices/machine/swim3.h
	)
endif()

###################################################
##
##@src/devices/machine/mv_sonora.h,list(APPEND MACHINES MAC_VIDEO_SONORA)
###################################################
if("MAC_VIDEO_SONORA" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/mv_sonora.cpp
		${MAME_DIR}/src/devices/machine/mv_sonora.h
	)
endif()

###################################################
##
##@src/devices/machine/alpha_8921.h,list(APPEND MACHINES ALPHA_8921)
###################################################
if("ALPHA_8921" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/alpha_8921.cpp
		${MAME_DIR}/src/devices/machine/alpha_8921.h
	)
endif()

###################################################
##
##@src/devices/machine/bl_handhelds_menucontrol.h,list(APPEND MACHINES BL_HANDHELDS_MENUCONTROL)
###################################################
if("BL_HANDHELDS_MENUCONTROL" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/bl_handhelds_menucontrol.cpp
		${MAME_DIR}/src/devices/machine/bl_handhelds_menucontrol.h
	)
endif()

###################################################
##
##@src/devices/machine/ns32081.h,list(APPEND MACHINES NS32081)
###################################################
if("NS32081" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ns32081.cpp
		${MAME_DIR}/src/devices/machine/ns32081.h
	)
endif()

###################################################
##
##@src/devices/machine/ns32202.h,list(APPEND MACHINES NS32202)
###################################################
if("NS32202" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ns32202.cpp
		${MAME_DIR}/src/devices/machine/ns32202.h
	)
endif()

###################################################
##
##@src/devices/machine/ns32082.h,list(APPEND MACHINES NS32082)
###################################################
if("NS32082" IN_LIST MACHINES)
	list(APPEND MACHINE_SRCS
		${MAME_DIR}/src/devices/machine/ns32082.cpp
		${MAME_DIR}/src/devices/machine/ns32082.h
	)
endif()
