# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   cpu.cmake
##
##   Rules for building CPU cores
##
##########################################################################

##################################################
## Dynamic recompiler objects
##################################################

set(DRC_CPUS "E1;SH;MIPS3;POWERPC;RSP;ARM7;ADSP21062;MB86235;DSP16;UNSP")

set(CPU_INCLUDE_DRC false)
foreach(DRC_CPU ${DRC_CPUS})
	if(${DRC_CPU} IN_LIST CPUS)
		set(CPU_INCLUDE_DRC true)
		break()
	endif()
endforeach()

if (CPU_INCLUDE_DRC)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/drcbec.cpp
		${MAME_DIR}/src/devices/cpu/drcbec.h
		${MAME_DIR}/src/devices/cpu/drcbeut.cpp
		${MAME_DIR}/src/devices/cpu/drcbeut.h
		${MAME_DIR}/src/devices/cpu/drccache.cpp
		${MAME_DIR}/src/devices/cpu/drccache.h
		${MAME_DIR}/src/devices/cpu/drcfe.cpp
		${MAME_DIR}/src/devices/cpu/drcfe.h
		${MAME_DIR}/src/devices/cpu/drcuml.cpp
		${MAME_DIR}/src/devices/cpu/drcuml.h
		${MAME_DIR}/src/devices/cpu/uml.cpp
		${MAME_DIR}/src/devices/cpu/uml.h
		${MAME_DIR}/src/devices/cpu/x86log.cpp
		${MAME_DIR}/src/devices/cpu/x86log.h
		${MAME_DIR}/src/devices/cpu/drcumlsh.h
	)
	if(NOT FORCE_DRC_C_BACKEND)
		list(APPEND CPU_SRCS
			${MAME_DIR}/src/devices/cpu/drcbex64.cpp
			${MAME_DIR}/src/devices/cpu/drcbex64.h
			${MAME_DIR}/src/devices/cpu/drcbex86.cpp
			${MAME_DIR}/src/devices/cpu/drcbex86.h
		)
	endif()
##
##  if _OPTIONS["targetos"]=="macosx" and _OPTIONS["gcc"]~=nil then
##      if string.find(_OPTIONS["gcc"], "clang) and (str_to_version(_OPTIONS["gcc_version"]) < 80000 IN_LIST CPUS) OR TOOLS)
##          defines {
##              "TARGET_OS_OSX=1
##          )
##      endif()
##  endif()
endif()

##################################################
## Signetics 8X300 / Scientific Micro Systems SMS300
##@src/devices/cpu/8x300/8x300.h,list(APPEND CPUS 8X300)
##################################################

if("8X300" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/8x300/8x300.cpp
		${MAME_DIR}/src/devices/cpu/8x300/8x300.h
	)
endif()

if(("8X300" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/8x300/8x300dasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/8x300/8x300dasm.cpp)
endif()

##################################################
## 3DO Don's Super Performing Processor (DSPP)
##@src/devices/cpu/dspp/dspp.h,list(APPEND CPUS DSPP)
##################################################

if("DSPP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/dspp/dspp.cpp
		${MAME_DIR}/src/devices/cpu/dspp/dspp.h
		${MAME_DIR}/src/devices/cpu/dspp/dsppdrc.cpp
		${MAME_DIR}/src/devices/cpu/dspp/dsppfe.cpp
		${MAME_DIR}/src/devices/cpu/dspp/dsppfe.h
	)
endif()

if(("DSPP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dspp/dsppdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dspp/dsppdasm.h)
endif()

##################################################
## ARCangent A4
##@src/devices/cpu/arc/arc.h,list(APPEND CPUS ARC)
##################################################

if("ARC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/arc/arc.cpp
		${MAME_DIR}/src/devices/cpu/arc/arc.h
	)
endif()

if(("ARC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arc/arcdasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arc/arcdasm.cpp)
endif()

##################################################
## ARcompact (ARCtangent-A5, ARC 600, ARC 700)
##@src/devices/cpu/arcompact/arcompact.h,list(APPEND CPUS ARCOMPACT)
##################################################

if("ARCOMPACT" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/arcompact/arcompact.cpp
		${MAME_DIR}/src/devices/cpu/arcompact/arcompact.h
		${MAME_DIR}/src/devices/cpu/arcompact/arcompact_execute.cpp
	)
endif()

if(("ARCOMPACT" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arcompact/arcompactdasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arcompact/arcompactdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arcompact/arcompactdasm_dispatch.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arcompact/arcompactdasm_ops.cpp)
endif()

##################################################
## Acorn ARM series
##
##@src/devices/cpu/arm/arm.h,list(APPEND CPUS ARM)
##@src/devices/cpu/arm7/arm7.h,list(APPEND CPUS ARM7)
##################################################

if("ARM" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/arm/arm.cpp
		${MAME_DIR}/src/devices/cpu/arm/arm.h
	)
endif()

if(("ARM" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arm/armdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arm/armdasm.h)
endif()

if("ARM7" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/arm7/arm7.cpp
		${MAME_DIR}/src/devices/cpu/arm7/arm7.h
		${MAME_DIR}/src/devices/cpu/arm7/arm7thmb.cpp
		${MAME_DIR}/src/devices/cpu/arm7/arm7ops.cpp
		${MAME_DIR}/src/devices/cpu/arm7/lpc210x.cpp
		${MAME_DIR}/src/devices/cpu/arm7/lpc210x.h
		${MAME_DIR}/src/devices/cpu/arm7/arm7core.h
		${MAME_DIR}/src/devices/cpu/arm7/arm7core.hxx
		${MAME_DIR}/src/devices/cpu/arm7/arm7drc.hxx
		${MAME_DIR}/src/devices/cpu/arm7/arm7help.h
		${MAME_DIR}/src/devices/cpu/arm7/arm7tdrc.hxx
		${MAME_DIR}/src/devices/cpu/arm7/cecalls.hxx
	)
endif()

if(("ARM7" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arm7/arm7dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/arm7/arm7dasm.h)
endif()

##################################################
## Advanced Digital Chips SE3208
##@src/devices/cpu/se3208/se3208.h,list(APPEND CPUS SE3208)
##################################################

if("SE3208" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/se3208/se3208.cpp
		${MAME_DIR}/src/devices/cpu/se3208/se3208.h
	)
endif()

if(("SE3208" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/se3208/se3208dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/se3208/se3208dis.h)
endif()

##################################################
## American Microsystems, Inc.(AMI) S2000 series
##@src/devices/cpu/amis2000/amis2000.h,list(APPEND CPUS AMIS2000)
##################################################

if("AMIS2000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/amis2000/amis2000.cpp
		${MAME_DIR}/src/devices/cpu/amis2000/amis2000.h
		${MAME_DIR}/src/devices/cpu/amis2000/amis2000op.cpp
	)
endif()

if(("AMIS2000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/amis2000/amis2000d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/amis2000/amis2000d.h)
endif()

##################################################
## Analog Devices ADSP21xx series
##@src/devices/cpu/adsp2100/adsp2100.h,list(APPEND CPUS ADSP21XX)
##################################################

if("ADSP21XX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/adsp2100/adsp2100.cpp
		${MAME_DIR}/src/devices/cpu/adsp2100/adsp2100.h
		${MAME_DIR}/src/devices/cpu/adsp2100/2100ops.hxx
	)
endif()

if(("ADSP21XX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/adsp2100/2100dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/adsp2100/2100dasm.h)
endif()

##################################################
## Analog Devices "Sharc" ADSP21062
##@src/devices/cpu/sharc/sharc.h,list(APPEND CPUS ADSP21062)
##################################################

if("ADSP21062" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sharc/sharc.cpp
		${MAME_DIR}/src/devices/cpu/sharc/sharc.h
		${MAME_DIR}/src/devices/cpu/sharc/compute.hxx
		${MAME_DIR}/src/devices/cpu/sharc/sharcdma.hxx
		${MAME_DIR}/src/devices/cpu/sharc/sharcmem.hxx
		${MAME_DIR}/src/devices/cpu/sharc/sharcops.h
		${MAME_DIR}/src/devices/cpu/sharc/sharcops.hxx
		${MAME_DIR}/src/devices/cpu/sharc/sharcdrc.cpp
		${MAME_DIR}/src/devices/cpu/sharc/sharcfe.cpp
		${MAME_DIR}/src/devices/cpu/sharc/sharcfe.h
	)
endif()

if(("ADSP21062" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sharc/sharcdsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sharc/sharcdsm.h)
endif()

##################################################
## APEXC
##@src/devices/cpu/apexc/apexc.h,list(APPEND CPUS APEXC)
##################################################

if("APEXC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/apexc/apexc.cpp
		${MAME_DIR}/src/devices/cpu/apexc/apexc.h
	)
endif()

if(("APEXC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/apexc/apexcdsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/apexc/apexcdsm.h)
endif()

##################################################
## WE|AT&T DSP16
##@src/devices/cpu/dsp16/dsp16.h,list(APPEND CPUS DSP16)
##################################################

if("DSP16" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16.cpp
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16.h
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16core.cpp
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16core.h
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16core.ipp
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16fe.cpp
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16fe.h
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16rc.cpp
		${MAME_DIR}/src/devices/cpu/dsp16/dsp16rc.h
	)
endif()

if(("DSP16" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp16/dsp16dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp16/dsp16dis.h)
endif()

##################################################
## AT&T DSP32C
##@src/devices/cpu/dsp32/dsp32.h,list(APPEND CPUS DSP32C)
##################################################

if("DSP32C" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/dsp32/dsp32.cpp
		${MAME_DIR}/src/devices/cpu/dsp32/dsp32.h
		${MAME_DIR}/src/devices/cpu/dsp32/dsp32ops.hxx
	)
endif()

if(("DSP32C" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp32/dsp32dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp32/dsp32dis.h)
endif()

##################################################
## Atari custom RISC processor
##@src/devices/cpu/asap/asap.h,list(APPEND CPUS ASAP)
##################################################

if("ASAP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/asap/asap.cpp
		${MAME_DIR}/src/devices/cpu/asap/asap.h
	)
endif()

if(("ASAP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/asap/asapdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/asap/asapdasm.h)
endif()

##################################################
## AMD Am29000
##@src/devices/cpu/am29000/am29000.h,list(APPEND CPUS AM29000)
##################################################

if("AM29000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/am29000/am29000.cpp
		${MAME_DIR}/src/devices/cpu/am29000/am29000.h
		${MAME_DIR}/src/devices/cpu/am29000/am29ops.h
	)
endif()

if(("AM29000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/am29000/am29dasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/am29000/am29dasm.cpp)
endif()

##################################################
## Atari Jaguar custom DSPs
##@src/devices/cpu/jaguar/jaguar.h,list(APPEND CPUS JAGUAR)
##################################################

if("JAGUAR" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/jaguar/jaguar.cpp
		${MAME_DIR}/src/devices/cpu/jaguar/jaguar.h
	)
endif()

if(("JAGUAR" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/jaguar/jagdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/jaguar/jagdasm.h)
endif()

##################################################
## Simutrek Cube Quest bit-sliced CPUs
##@src/devices/cpu/cubeqcpu/cubeqcpu.h,list(APPEND CPUS CUBEQCPU)
##################################################

if("CUBEQCPU" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cubeqcpu/cubeqcpu.cpp
		${MAME_DIR}/src/devices/cpu/cubeqcpu/cubeqcpu.h
	)
endif()

if(("CUBEQCPU" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cubeqcpu/cubedasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cubeqcpu/cubedasm.h)
endif()

##################################################
## Ensoniq ES5510 ('ESP') DSP
##@src/devices/cpu/es5510/es5510.h,list(APPEND CPUS ES5510)
##################################################

if("ES5510" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/es5510/es5510.cpp
		${MAME_DIR}/src/devices/cpu/es5510/es5510.h
	)
endif()

if(("ES5510" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/es5510/es5510d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/es5510/es5510d.h)
endif()

##################################################
## Entertainment Sciences AM29116-based RIP
##@src/devices/cpu/esrip/esrip.h,list(APPEND CPUS ESRIP)
##################################################

if("ESRIP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/esrip/esrip.cpp
		${MAME_DIR}/src/devices/cpu/esrip/esrip.h
	)
endif()

if(("ESRIP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/esrip/esripdsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/esrip/esripdsm.h)
endif()

##################################################
## Seiko Epson E0C6200 series
##@src/devices/cpu/e0c6200/e0c6200.h,list(APPEND CPUS E0C6200)
##################################################

if("E0C6200" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/e0c6200/e0c6200.cpp
		${MAME_DIR}/src/devices/cpu/e0c6200/e0c6200.h
		${MAME_DIR}/src/devices/cpu/e0c6200/e0c6s46.cpp
		${MAME_DIR}/src/devices/cpu/e0c6200/e0c6s46.h
		${MAME_DIR}/src/devices/cpu/e0c6200/e0c6200op.cpp
	)
endif()

if(("E0C6200" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/e0c6200/e0c6200d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/e0c6200/e0c6200d.h)
endif()

##################################################
## RCA COSMAC
##@src/devices/cpu/cosmac/cosmac.h,list(APPEND CPUS COSMAC)
##################################################

if("COSMAC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cosmac/cosmac.cpp
		${MAME_DIR}/src/devices/cpu/cosmac/cosmac.h
	)
endif()

if(("COSMAC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cosmac/cosdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cosmac/cosdasm.h)
endif()

##################################################
## National Semiconductor COPS(MM57) family
##@src/devices/cpu/cops1/mm5799.h,list(APPEND CPUS COPS1)
##################################################

if("COPS1" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cops1/cops1base.cpp
		${MAME_DIR}/src/devices/cpu/cops1/cops1base.h
		${MAME_DIR}/src/devices/cpu/cops1/mm5799.cpp
		${MAME_DIR}/src/devices/cpu/cops1/mm5799.h
		${MAME_DIR}/src/devices/cpu/cops1/mm5799op.cpp
	)
endif()

if(("COPS1" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cops1/cops1d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cops1/cops1d.h)
endif()

##################################################
## National Semiconductor COPS(COP400) family
##@src/devices/cpu/cop400/cop400.h,list(APPEND CPUS COP400)
##################################################

if("COP400" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cop400/cop400.cpp
		${MAME_DIR}/src/devices/cpu/cop400/cop400.h
		${MAME_DIR}/src/devices/cpu/cop400/cop400op.hxx
	)
endif()

if(("COP400" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop410ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop410ds.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop420ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop420ds.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop444ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop444ds.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop424ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cop400/cop424ds.h)
endif()

##################################################
## CP1610
##@src/devices/cpu/cp1610/cp1610.h,list(APPEND CPUS CP1610)
##################################################

if("CP1610" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cp1610/cp1610.cpp
		${MAME_DIR}/src/devices/cpu/cp1610/cp1610.h
	)
endif()

if(("CP1610" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cp1610/1610dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cp1610/1610dasm.h)
endif()

##################################################
## Cinematronics vector "CPU"
##@src/devices/cpu/ccpu/ccpu.h,list(APPEND CPUS CCPU)
##################################################

if("CCPU" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ccpu/ccpu.cpp
		${MAME_DIR}/src/devices/cpu/ccpu/ccpu.h
	)
endif()

if(("CCPU" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ccpu/ccpudasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ccpu/ccpudasm.cpp)
endif()

##################################################
## DEC T-11
##@src/devices/cpu/t11/t11.h,list(APPEND CPUS T11)
##################################################

if("T11" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/t11/t11.cpp
		${MAME_DIR}/src/devices/cpu/t11/t11.h
		${MAME_DIR}/src/devices/cpu/t11/t11ops.hxx
		${MAME_DIR}/src/devices/cpu/t11/t11table.hxx
	)
endif()

if(("T11" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/t11/t11dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/t11/t11dasm.h)
endif()

##################################################
## DEC PDP-8
##@src/devices/cpu/pdp8/pdp8.h,list(APPEND CPUS PDP8)
##################################################

if("PDP8" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pdp8/hd6120.cpp
		${MAME_DIR}/src/devices/cpu/pdp8/hd6120.h
		${MAME_DIR}/src/devices/cpu/pdp8/pdp8.cpp
		${MAME_DIR}/src/devices/cpu/pdp8/pdp8.h
	)
endif()

if(("PDP8" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pdp8/pdp8dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pdp8/pdp8dasm.h)
endif()

##################################################
## F8
##@src/devices/cpu/f8/f8.h,list(APPEND CPUS F8)
##################################################

if("F8" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/f8/f8.cpp
		${MAME_DIR}/src/devices/cpu/f8/f8.h
	)
endif()

if(("F8" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/f8/f8dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/f8/f8dasm.h)
endif()

##################################################
## Fujitsu FR
##@src/devices/cpu/fr/fr.h,list(APPEND CPUS FR)
##################################################

if("FR" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/fr/fr.cpp
		${MAME_DIR}/src/devices/cpu/fr/fr.h
	)
endif()

if(("FR" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/fr/frdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/fr/frdasm.h)
endif()

##################################################
## G65816
##@src/devices/cpu/g65816/g65816.h,list(APPEND CPUS G65816)
##################################################

if("G65816" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/g65816/g65816.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816.h
		${MAME_DIR}/src/devices/cpu/g65816/g65816o0.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816o1.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816o2.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816o3.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816o4.cpp
		${MAME_DIR}/src/devices/cpu/g65816/g65816cm.h
		${MAME_DIR}/src/devices/cpu/g65816/g65816ds.h
		${MAME_DIR}/src/devices/cpu/g65816/g65816op.h
	)
endif()

if(("G65816" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/g65816/g65816ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/g65816/g65816ds.h)
endif()

##################################################
## Hitachi H16
##@src/devices/cpu/h16/hd641016.h,list(APPEND CPUS H16)
##################################################

if("H16" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/h16/hd641016.cpp
		${MAME_DIR}/src/devices/cpu/h16/hd641016.h
	)
endif()

if(("H16" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h16/h16dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h16/h16dasm.h)
endif()

##################################################
## Hitachi H8 (16/32-bit H8/300, H8/300H, H8S2000 and H8S2600 series)
##@src/devices/cpu/h8/h8.h,list(APPEND CPUS H8)
##################################################

if("H8" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/h8/h8.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8.h
		${MAME_DIR}/src/devices/cpu/h8/h8h.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8h.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2000.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2000.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2600.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2600.h
		${MAME_DIR}/src/devices/cpu/h8/h83337.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83337.h
		${MAME_DIR}/src/devices/cpu/h8/h83002.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83002.h
		${MAME_DIR}/src/devices/cpu/h8/h83003.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83003.h
		${MAME_DIR}/src/devices/cpu/h8/h83006.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83006.h
		${MAME_DIR}/src/devices/cpu/h8/h83008.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83008.h
		${MAME_DIR}/src/devices/cpu/h8/h83032.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83032.h
		${MAME_DIR}/src/devices/cpu/h8/h83042.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83042.h
		${MAME_DIR}/src/devices/cpu/h8/h83048.cpp
		${MAME_DIR}/src/devices/cpu/h8/h83048.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2245.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2245.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2320.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2320.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2357.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2357.h
		${MAME_DIR}/src/devices/cpu/h8/h8s2655.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8s2655.h
		${MAME_DIR}/src/devices/cpu/h8/h8_adc.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_adc.h
		${MAME_DIR}/src/devices/cpu/h8/h8_dma.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_dma.h
		${MAME_DIR}/src/devices/cpu/h8/h8_dtc.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_dtc.h
		${MAME_DIR}/src/devices/cpu/h8/h8_intc.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_intc.h
		${MAME_DIR}/src/devices/cpu/h8/h8_port.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_port.h
		${MAME_DIR}/src/devices/cpu/h8/h8_timer8.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_timer8.h
		${MAME_DIR}/src/devices/cpu/h8/h8_timer16.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_timer16.h
		${MAME_DIR}/src/devices/cpu/h8/h8_sci.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_sci.h
		${MAME_DIR}/src/devices/cpu/h8/h8_watchdog.cpp
		${MAME_DIR}/src/devices/cpu/h8/h8_watchdog.h
		${GEN_DIR}/emu/cpu/h8/h8.hxx
		${GEN_DIR}/emu/cpu/h8/h8h.hxx
		${GEN_DIR}/emu/cpu/h8/h8s2000.hxx
		${GEN_DIR}/emu/cpu/h8/h8s2600.hxx
	)

	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst s o   ${GEN_DIR}/emu/cpu/h8/h8.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8.hxx
		COMMENT "Generating H8-300 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst s h   ${GEN_DIR}/emu/cpu/h8/h8h.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8h.hxx
		COMMENT "Generating H8-300H source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst s s20 ${GEN_DIR}/emu/cpu/h8/h8s2000.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8s2000.hxx
		COMMENT "Generating H8S/2000 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst s s26 ${GEN_DIR}/emu/cpu/h8/h8s2600.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8s2600.hxx
		COMMENT "Generating H8S/2600 source file..."
	)
endif()

if(("H8" IN_LIST CPUS) OR TOOLS)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst d o   ${GEN_DIR}/emu/cpu/h8/h8d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8d.hxx
		COMMENT "Generating H8-300 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst d h   ${GEN_DIR}/emu/cpu/h8/h8hd.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8hd.hxx
		COMMENT "Generating H8-300H disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst d s20 ${GEN_DIR}/emu/cpu/h8/h8s2000d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8s2000d.hxx
		COMMENT "Generating H8S/2000 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/h8/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst d s26 ${GEN_DIR}/emu/cpu/h8/h8s2600d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/h8/h8make.py ${MAME_DIR}/src/devices/cpu/h8/h8.lst
		OUTPUT ${GEN_DIR}/emu/cpu/h8/h8s2600d.hxx
		COMMENT "Generating H8S/2600 disassembler source file..."
	)

	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/h8/h8d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/h8/h8hd.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/h8/h8s2000d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/h8/h8s2600d.hxx)

	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8hd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8hd.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8s2000d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8s2000d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8s2600d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8/h8s2600d.h)
endif()

##################################################
## Hitachi H8/500 series
##@src/devices/cpu/h8500/h8500.h,list(APPEND CPUS H8500)
##################################################

if("H8500" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/h8500/h8500.cpp
		${MAME_DIR}/src/devices/cpu/h8500/h8500.h
		${MAME_DIR}/src/devices/cpu/h8500/h8510.cpp
		${MAME_DIR}/src/devices/cpu/h8500/h8510.h
		${MAME_DIR}/src/devices/cpu/h8500/h8520.cpp
		${MAME_DIR}/src/devices/cpu/h8500/h8520.h
		${MAME_DIR}/src/devices/cpu/h8500/h8532.cpp
		${MAME_DIR}/src/devices/cpu/h8500/h8532.h
		${MAME_DIR}/src/devices/cpu/h8500/h8534.cpp
		${MAME_DIR}/src/devices/cpu/h8500/h8534.h
	)
endif()

if(("H8500" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8500/h8500dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h8500/h8500dasm.h)
endif()

##################################################
## Hitachi HCD62121
##@src/devices/cpu/hcd62121/hcd62121.h,list(APPEND CPUS HCD62121)
##################################################

if("HCD62121" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/hcd62121/hcd62121.cpp
		${MAME_DIR}/src/devices/cpu/hcd62121/hcd62121.h
	)
endif()

if(("HCD62121" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hcd62121/hcd62121d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hcd62121/hcd62121d.h)
endif()

##################################################
## Hitachi HMCS40 series
##@src/devices/cpu/hmcs40/hmcs40.h,list(APPEND CPUS HMCS40)
##################################################

if("HMCS40" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/hmcs40/hmcs40.cpp
		${MAME_DIR}/src/devices/cpu/hmcs40/hmcs40.h
		${MAME_DIR}/src/devices/cpu/hmcs40/hmcs40op.cpp
	)
endif()

if(("HMCS40" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hmcs40/hmcs40d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hmcs40/hmcs40d.h)
endif()

##################################################
## Hitachi SuperH series (SH1/SH2/SH3/SH4)
##@src/devices/cpu/sh/sh2.h,list(APPEND CPUS SH)
##@src/devices/cpu/sh/sh4.h,list(APPEND CPUS SH)
##################################################

if("SH" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sh/sh.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh.h
		${MAME_DIR}/src/devices/cpu/sh/sh2.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh2.h
		${MAME_DIR}/src/devices/cpu/sh/sh2comn.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh2comn.h
		${MAME_DIR}/src/devices/cpu/sh/sh_fe.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh2fe.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh4fe.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh7604_bus.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh7604_bus.h
		${MAME_DIR}/src/devices/cpu/sh/sh7604_sci.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh7604_sci.h
		${MAME_DIR}/src/devices/cpu/sh/sh7604_wdt.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh7604_wdt.h
		${MAME_DIR}/src/devices/cpu/sh/sh4.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh4.h
		${MAME_DIR}/src/devices/cpu/sh/sh4comn.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh4comn.h
		${MAME_DIR}/src/devices/cpu/sh/sh3comn.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh3comn.h
		${MAME_DIR}/src/devices/cpu/sh/sh4tmu.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh4tmu.h
		${MAME_DIR}/src/devices/cpu/sh/sh4dmac.cpp
		${MAME_DIR}/src/devices/cpu/sh/sh4dmac.h
		${MAME_DIR}/src/devices/cpu/sh/sh4regs.h
	)
endif()

if(("SH" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sh/sh_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sh/sh_dasm.h)
endif()

##################################################
## STmicro ST62xx
##@src/devices/cpu/st62xx/st62xx.h,list(APPEND CPUS ST62XX)
##################################################

if("ST62XX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/st62xx/st62xx.cpp
		${MAME_DIR}/src/devices/cpu/st62xx/st62xx.h
	)
endif()

if(("ST62XX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/st62xx/st62xx_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/st62xx/st62xx_dasm.h)
endif()

##################################################
## HP Hybrid processor
##@src/devices/cpu/hphybrid/hphybrid.h,list(APPEND CPUS HPHYBRID)
##################################################

if("HPHYBRID" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/hphybrid/hphybrid.cpp
		${MAME_DIR}/src/devices/cpu/hphybrid/hphybrid.h
	)
endif()

if(("HPHYBRID" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hphybrid/hphybrid_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hphybrid/hphybrid_dasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hphybrid/hphybrid_defs.h)
endif()

##################################################
## HP Nanoprocessor
##@src/devices/cpu/nanoprocessor/nanoprocessor.h,list(APPEND CPUS NANOPROCESSOR)
##################################################

if("NANOPROCESSOR" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/nanoprocessor/nanoprocessor.cpp
		${MAME_DIR}/src/devices/cpu/nanoprocessor/nanoprocessor.h
	)
endif()

if(("NANOPROCESSOR" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nanoprocessor/nanoprocessor_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nanoprocessor/nanoprocessor_dasm.h)
endif()

##################################################
## HP Capricorn
##@src/devices/cpu/capricorn/capricorn.h,list(APPEND CPUS CAPRICORN)
##################################################

if("CAPRICORN" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/capricorn/capricorn.cpp
		${MAME_DIR}/src/devices/cpu/capricorn/capricorn.h
	)
endif()

if(("CAPRICORN" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/capricorn/capricorn_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/capricorn/capricorn_dasm.h)
endif()

##################################################
## Hudsonsoft 6280
##@src/devices/cpu/h6280/h6280.h,list(APPEND CPUS H6280)
##################################################

if("H6280" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/h6280/h6280.cpp
		${MAME_DIR}/src/devices/cpu/h6280/h6280.h
	)
endif()

if(("H6280" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h6280/6280dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/h6280/6280dasm.h)
endif()

##################################################
## Hyperstone E1 series
##@src/devices/cpu/e132xs/e132xs.h,list(APPEND CPUS E1)
##################################################

if("E1" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/e132xs/e132xs.cpp
		${MAME_DIR}/src/devices/cpu/e132xs/e132xs.h
		${MAME_DIR}/src/devices/cpu/e132xs/32xsdefs.h
		${MAME_DIR}/src/devices/cpu/e132xs/e132xsop.hxx
		${MAME_DIR}/src/devices/cpu/e132xs/e132xsfe.cpp
		${MAME_DIR}/src/devices/cpu/e132xs/e132xsdrc.cpp
		${MAME_DIR}/src/devices/cpu/e132xs/e132xsdrc_ops.hxx
	)
endif()

if(("E1" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/e132xs/32xsdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/e132xs/32xsdasm.h)
endif()

##################################################
## 15IE-00-013 CPU ("Microprogrammed Control Device)
##@src/devices/cpu/ie15/ie15.h,list(APPEND CPUS IE15)
##################################################

if("IE15" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ie15/ie15.cpp
		${MAME_DIR}/src/devices/cpu/ie15/ie15.h
	)
endif()

if(("IE15" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ie15/ie15dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ie15/ie15dasm.h)
endif()

##################################################
## Intel MCS-40
##@src/devices/cpu/mcs40/mcs40.h,list(APPEND CPUS MCS40)
##################################################

if("MCS40" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mcs40/mcs40.cpp
		${MAME_DIR}/src/devices/cpu/mcs40/mcs40.h
	)
endif()

if(("MCS40" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs40/mcs40dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs40/mcs40dasm.h)
endif()

##################################################
## Intel 8008
##@src/devices/cpu/i8008/i8008.h,list(APPEND CPUS I8008)
##################################################

if("I8008" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i8008/i8008.cpp
		${MAME_DIR}/src/devices/cpu/i8008/i8008.h
	)
endif()

if(("I8008" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8008/8008dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8008/8008dasm.h)
endif()

##################################################
##  National Semiconductor SC/MP
##@src/devices/cpu/scmp/scmp.h,list(APPEND CPUS SCMP)
##################################################

if("SCMP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/scmp/scmp.cpp
		${MAME_DIR}/src/devices/cpu/scmp/scmp.h
	)
endif()

if(("SCMP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/scmp/scmpdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/scmp/scmpdasm.h)
endif()

##################################################
## Intel 8080/8085A
##@src/devices/cpu/i8085/i8085.h,list(APPEND CPUS I8085)
##################################################

if("I8085" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i8085/i8085.cpp
		${MAME_DIR}/src/devices/cpu/i8085/i8085.h
	)
endif()

if(("I8085" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8085/8085dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8085/8085dasm.h)
endif()

##################################################
## Intel 8089
##@src/devices/cpu/i8089/i8089.h,list(APPEND CPUS I8089)
##################################################

if("I8089" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i8089/i8089.cpp
		${MAME_DIR}/src/devices/cpu/i8089/i8089.h
		${MAME_DIR}/src/devices/cpu/i8089/i8089_channel.cpp
		${MAME_DIR}/src/devices/cpu/i8089/i8089_channel.h
		${MAME_DIR}/src/devices/cpu/i8089/i8089_ops.cpp
	)
endif()

if(("I8089" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8089/i8089_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i8089/i8089_dasm.h)
endif()

##################################################
## Intel MCS-48 (8039 and derivatives)
##@src/devices/cpu/mcs48/mcs48.h,list(APPEND CPUS MCS48)
##################################################

if("MCS48" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mcs48/mcs48.cpp
		${MAME_DIR}/src/devices/cpu/mcs48/mcs48.h
	)
endif()

if(("MCS48" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs48/mcs48dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs48/mcs48dsm.h)
endif()

##################################################
## Intel 8051 and derivatives
##@src/devices/cpu/mcs51/mcs51.h,list(APPEND CPUS MCS51)
##################################################

if("MCS51" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mcs51/mcs51.cpp
		${MAME_DIR}/src/devices/cpu/mcs51/mcs51.h
		${MAME_DIR}/src/devices/cpu/mcs51/mcs51ops.hxx
		${MAME_DIR}/src/devices/cpu/mcs51/axc51-core.cpp
		${MAME_DIR}/src/devices/cpu/mcs51/axc51-core.h
	)
endif()

if(("MCS51" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs51/mcs51dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs51/mcs51dasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs51/axc51-core_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs51/axc51-core_dasm.h)
endif()

##################################################
## Intel MCS-96
##@src/devices/cpu/mcs96/mcs96.h,list(APPEND CPUS MCS96)
##################################################

if("MCS96" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mcs96/mcs96.cpp
		${MAME_DIR}/src/devices/cpu/mcs96/mcs96.h
		${MAME_DIR}/src/devices/cpu/mcs96/i8x9x.cpp
		${MAME_DIR}/src/devices/cpu/mcs96/i8x9x.h
		${MAME_DIR}/src/devices/cpu/mcs96/i8xc196.cpp
		${MAME_DIR}/src/devices/cpu/mcs96/i8xc196.h
		${GEN_DIR}/emu/cpu/mcs96/mcs96.hxx
		${GEN_DIR}/emu/cpu/mcs96/i8x9x.hxx
		${GEN_DIR}/emu/cpu/mcs96/i8xc196.hxx
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/mcs96/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py s mcs96 ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst  ${GEN_DIR}/emu/cpu/mcs96/mcs96.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst
		OUTPUT ${GEN_DIR}/emu/cpu/mcs96/mcs96.hxx
		COMMENT "Generating mcs96 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/mcs96/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py s i8x9x ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst  ${GEN_DIR}/emu/cpu/mcs96/i8x9x.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst
		OUTPUT ${GEN_DIR}/emu/cpu/mcs96/i8x9x.hxx
		COMMENT "Generating i8x9x source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/mcs96/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py s i8xc196 ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst  ${GEN_DIR}/emu/cpu/mcs96/i8xc196.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst
		OUTPUT ${GEN_DIR}/emu/cpu/mcs96/i8xc196.hxx
		COMMENT "Generating i8xc196 source file..."
	)
endif()

if(("MCS96" IN_LIST CPUS) OR TOOLS)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/mcs96/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py d i8x9x ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst  ${GEN_DIR}/emu/cpu/mcs96/i8x9xd.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst
		OUTPUT ${GEN_DIR}/emu/cpu/mcs96/i8x9xd.hxx
		COMMENT "Generating i8x9x source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/mcs96/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py d i8xc196 ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst  ${GEN_DIR}/emu/cpu/mcs96/i8xc196d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96make.py ${MAME_DIR}/src/devices/cpu/mcs96/mcs96ops.lst
		OUTPUT ${GEN_DIR}/emu/cpu/mcs96/i8xc196d.hxx
		COMMENT "Generating i8xc196 source file..."
	)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/mcs96/i8x9xd.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/mcs96/i8xc196d.hxx)

	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs96/mcs96d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs96/i8x9xd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mcs96/i8xc196d.cpp)
 endif()

##################################################
## Intel 80x86 series (also a dynamic recompiler target)
##@src/devices/cpu/i86/i86.h,list(APPEND CPUS I86)
##@src/devices/cpu/i86/i286.h,list(APPEND CPUS I86)
##@src/devices/cpu/i386/i386.h,list(APPEND CPUS I386)
##################################################

if("I86" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i86/i86.cpp
		${MAME_DIR}/src/devices/cpu/i86/i86.h
		${MAME_DIR}/src/devices/cpu/i86/i186.cpp
		${MAME_DIR}/src/devices/cpu/i86/i186.h
		${MAME_DIR}/src/devices/cpu/i86/i286.cpp
		${MAME_DIR}/src/devices/cpu/i86/i286.h
		${MAME_DIR}/src/devices/cpu/i86/i86inline.h
	)
endif()

if(("I86" IN_LIST CPUS) OR ("I386" IN_LIST CPUS) OR TOOLS OR CPU_INCLUDE_DRC)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i386/i386dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i386/i386dasm.h)
endif()

if("I386" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i386/i386.cpp
		${MAME_DIR}/src/devices/cpu/i386/i386.h
		${MAME_DIR}/src/devices/cpu/i386/athlon.cpp
		${MAME_DIR}/src/devices/cpu/i386/athlon.h
		${MAME_DIR}/src/devices/cpu/i386/cache.h
		${MAME_DIR}/src/devices/cpu/i386/cycles.h
		${MAME_DIR}/src/devices/cpu/i386/i386op16.hxx
		${MAME_DIR}/src/devices/cpu/i386/i386op32.hxx
		${MAME_DIR}/src/devices/cpu/i386/i386ops.h
		${MAME_DIR}/src/devices/cpu/i386/i386ops.hxx
		${MAME_DIR}/src/devices/cpu/i386/i386priv.h
		${MAME_DIR}/src/devices/cpu/i386/i386segs.hxx
		${MAME_DIR}/src/devices/cpu/i386/i486ops.hxx
		${MAME_DIR}/src/devices/cpu/i386/pentops.hxx
		${MAME_DIR}/src/devices/cpu/i386/x87ops.hxx
		${MAME_DIR}/src/devices/cpu/i386/x87priv.h
		${MAME_DIR}/src/devices/cpu/i386/cpuidmsrs.hxx
	)
endif()

##################################################
## Intel i860
##@src/devices/cpu/i860/i860.h,list(APPEND CPUS I860)
##################################################

if("I860" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i860/i860.cpp
		${MAME_DIR}/src/devices/cpu/i860/i860.h
		${MAME_DIR}/src/devices/cpu/i860/i860dec.hxx
	)
endif()

if(("I860" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i860/i860dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i860/i860dis.h)
endif()

##################################################
## Intel i960
##@src/devices/cpu/i960/i960.h,list(APPEND CPUS I960)
##################################################

if("I960" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/i960/i960.cpp
		${MAME_DIR}/src/devices/cpu/i960/i960.h
	)
endif()

if(("I960" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i960/i960dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/i960/i960dis.h)
endif()

##################################################
## LH5801
##@src/devices/cpu/lh5801/lh5801.h,list(APPEND CPUS LH5801)
##################################################

if("LH5801" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/lh5801/lh5801.cpp
		${MAME_DIR}/src/devices/cpu/lh5801/lh5801.h
		${MAME_DIR}/src/devices/cpu/lh5801/5801tbl.hxx
	)
endif()

if(("LH5801" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lh5801/5801dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lh5801/5801dasm.h)
endif()
########
##########################################
## Manchester Small-Scale Experimental Machine
##@src/devices/cpu/ssem/ssem.h,list(APPEND CPUS SSEM)
##################################################

if("SSEM" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ssem/ssem.cpp
		${MAME_DIR}/src/devices/cpu/ssem/ssem.h
	)
endif()

if(("SSEM" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ssem/ssemdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ssem/ssemdasm.h)
endif()

##########################################
## Diablo Systems printer CPU
##@src/devices/cpu/diablo/diablo1300.h,list(APPEND CPUS DIABLO)
##################################################

if("DIABLO" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/diablo/diablo1300.cpp
		${MAME_DIR}/src/devices/cpu/diablo/diablo1300.h
	)
endif()

if(("DIABLO" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/diablo/diablo1300dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/diablo/diablo1300dasm.h)
endif()

##################################################
## Fujitsu MB88xx
##@src/devices/cpu/mb88xx/mb88xx.h,list(APPEND CPUS MB88XX)
##################################################

if("MB88XX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mb88xx/mb88xx.cpp
		${MAME_DIR}/src/devices/cpu/mb88xx/mb88xx.h
	)
endif()

if(("MB88XX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb88xx/mb88dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb88xx/mb88dasm.h)
endif()

##################################################
## Fujitsu MB86233
##@src/devices/cpu/mb86233/mb86233.h,list(APPEND CPUS MB86233)
##################################################

if("MB86233" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mb86233/mb86233.cpp
		${MAME_DIR}/src/devices/cpu/mb86233/mb86233.h
	)
endif()

if(("MB86233" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb86233/mb86233d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb86233/mb86233d.h)
endif()

##################################################
## Fujitsu MB86235
##@src/devices/cpu/mb86235/mb86235.h,list(APPEND CPUS MB86235)
##################################################

if("MB86235" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235.cpp
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235.h
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235drc.cpp
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235fe.cpp
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235fe.h
		${MAME_DIR}/src/devices/cpu/mb86235/mb86235ops.cpp
	)
endif()

if(("MB86235" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb86235/mb86235d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mb86235/mb86235d.h)
endif()

##################################################
## Microchip PIC16C5x
##@src/devices/cpu/pic16c5x/pic16c5x.h,list(APPEND CPUS PIC16C5X)
##################################################

if("PIC16C5X" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pic16c5x/pic16c5x.cpp
		${MAME_DIR}/src/devices/cpu/pic16c5x/pic16c5x.h
	)
endif()

if(("PIC16C5X" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16c5x/16c5xdsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16c5x/16c5xdsm.h)
endif()

##################################################
## PIC1670 - Disassembler only temporarily
##@src/devices/cpu/pic1670/pic1670.h,list(APPEND CPUS PIC1670)
##################################################

if(("PIC1670" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic1670/pic1670d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic1670/pic1670d.h)
endif()

##################################################
## Microchip PIC16C62x
##@src/devices/cpu/pic16c62x/pic16c62x.h,list(APPEND CPUS PIC16C62X)
##################################################

if("PIC16C62X" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pic16c62x/pic16c62x.cpp
		${MAME_DIR}/src/devices/cpu/pic16c62x/pic16c62x.h
	)
endif()

if(("PIC16C62X" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16c62x/16c62xdsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16c62x/16c62xdsm.h)
endif()

##################################################
## Generic PIC16 - Disassembler only
##@src/devices/cpu/pic16/pic16.h,list(APPEND CPUS PIC16)
##################################################

if(("PIC16" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16/pic16d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic16/pic16d.h)
endif()

##################################################
## Microchip PIC17
##@src/devices/cpu/pic17/pic17.h,list(APPEND CPUS PIC17)
##################################################

if("PIC17" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pic17/pic17.cpp
		${MAME_DIR}/src/devices/cpu/pic17/pic17.h
		${MAME_DIR}/src/devices/cpu/pic17/pic17c4x.cpp
		${MAME_DIR}/src/devices/cpu/pic17/pic17c4x.h
	)
endif()

if(("PIC17" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic17/pic17d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pic17/pic17d.h)
endif()

##################################################
## MIPS R3000 (MIPS I/II) series
##@src/devices/cpu/mips/mips1.h,list(APPEND CPUS MIPS1)
##################################################

if("MIPS1" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mips/mips1.cpp
		${MAME_DIR}/src/devices/cpu/mips/mips1.h
	)
endif()

if(("MIPS1" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/mips1dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/mips1dsm.h)
endif()

##################################################
## MIPS R4000 (MIPS III/IV) series
##@src/devices/cpu/mips/mips3.h,list(APPEND CPUS MIPS3)
##@src/devices/cpu/mips/r4000.h,list(APPEND CPUS MIPS3)
##################################################

if("MIPS3" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mips/mips3com.cpp
		${MAME_DIR}/src/devices/cpu/mips/mips3com.h
		${MAME_DIR}/src/devices/cpu/mips/mips3.cpp
		${MAME_DIR}/src/devices/cpu/mips/mips3.h
		${MAME_DIR}/src/devices/cpu/mips/mips3fe.cpp
		${MAME_DIR}/src/devices/cpu/mips/mips3fe.h
		${MAME_DIR}/src/devices/cpu/mips/mips3drc.cpp
		${MAME_DIR}/src/devices/cpu/mips/o2dprintf.hxx
		${MAME_DIR}/src/devices/cpu/mips/ps2vu.cpp
		${MAME_DIR}/src/devices/cpu/mips/ps2vu.h
		${MAME_DIR}/src/devices/cpu/mips/ps2vif1.cpp
		${MAME_DIR}/src/devices/cpu/mips/ps2vif1.h
		${MAME_DIR}/src/devices/cpu/mips/r4000.cpp
		${MAME_DIR}/src/devices/cpu/mips/r4000.h
	)
endif()

if(("MIPS3" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/mips3dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/mips3dsm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/vudasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mips/vudasm.h)
endif()

##################################################
## Sony PlayStation CPU (R3000-based + GTE)
##@src/devices/cpu/psx/psx.h,list(APPEND CPUS PSX)
##################################################

if("PSX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/psx/psx.cpp
		${MAME_DIR}/src/devices/cpu/psx/psx.h
		${MAME_DIR}/src/devices/cpu/psx/psxdefs.h
		${MAME_DIR}/src/devices/cpu/psx/gte.cpp
		${MAME_DIR}/src/devices/cpu/psx/gte.h
		${MAME_DIR}/src/devices/cpu/psx/dma.cpp
		${MAME_DIR}/src/devices/cpu/psx/dma.h
		${MAME_DIR}/src/devices/cpu/psx/irq.cpp
		${MAME_DIR}/src/devices/cpu/psx/irq.h
		${MAME_DIR}/src/devices/cpu/psx/mdec.cpp
		${MAME_DIR}/src/devices/cpu/psx/mdec.h
		${MAME_DIR}/src/devices/cpu/psx/rcnt.cpp
		${MAME_DIR}/src/devices/cpu/psx/rcnt.h
		${MAME_DIR}/src/devices/cpu/psx/sio.cpp
		${MAME_DIR}/src/devices/cpu/psx/sio.h
	)
endif()

if(("PSX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/psx/psxdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/psx/psxdasm.h)
endif()

##################################################
## Mitsubishi MELPS 4 series
##@src/devices/cpu/melps4/melps4.h,list(APPEND CPUS MELPS4)
##################################################

if("MELPS4" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/melps4/melps4.cpp
		${MAME_DIR}/src/devices/cpu/melps4/melps4.h
		${MAME_DIR}/src/devices/cpu/melps4/melps4op.cpp
		${MAME_DIR}/src/devices/cpu/melps4/m58846.cpp
		${MAME_DIR}/src/devices/cpu/melps4/m58846.h
	)
endif()

if(("MELPS4" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/melps4/melps4d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/melps4/melps4d.h)
endif()

##################################################
## Mitsubishi M32C, disassembler only
##################################################

if(("M23C" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m32c/m32cdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m32c/m32cdasm.h)
endif()

##################################################
## Mitsubishi M37702 and M37710 (based on 65C816)
##@src/devices/cpu/m37710/m37710.h,list(APPEND CPUS M37710)
##################################################

if("M37710" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m37710/m37710.cpp
		${MAME_DIR}/src/devices/cpu/m37710/m37710.h
		${MAME_DIR}/src/devices/cpu/m37710/m37710o0.cpp
		${MAME_DIR}/src/devices/cpu/m37710/m37710o1.cpp
		${MAME_DIR}/src/devices/cpu/m37710/m37710o2.cpp
		${MAME_DIR}/src/devices/cpu/m37710/m37710o3.cpp
		${MAME_DIR}/src/devices/cpu/m37710/m37710cm.h
		${MAME_DIR}/src/devices/cpu/m37710/m37710il.h
		${MAME_DIR}/src/devices/cpu/m37710/m37710op.h
	)
endif()

if(("M37710" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m37710/m7700ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m37710/m7700ds.h)
endif()

##################################################
## Mostek 6502 and its many derivatives
##@src/devices/cpu/m6502/m6502.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/deco16.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m4510.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m65ce02.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m65c02.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/r65c02.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/r65c19.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m65sc02.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6500_1.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6504.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6507.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6509.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6510.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m6510t.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m7501.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m8502.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/n2a03.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m740.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m3745x.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/m5074x.h,list(APPEND CPUS M6502)
##@src/devices/cpu/m6502/st2xxx.h,list(APPEND CPUS ST2XXX)
##@src/devices/cpu/m6502/st2204.h,list(APPEND CPUS ST2XXX)
##@src/devices/cpu/m6502/st2205u.h,list(APPEND CPUS ST2XXX)
##@src/devices/cpu/m6502/xavix.h,list(APPEND CPUS XAVIX)
##@src/devices/cpu/m6502/xavix.h,list(APPEND CPUS XAVIX2000)

##################################################

if("M6502" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6502/deco16.cpp
		${MAME_DIR}/src/devices/cpu/m6502/deco16.h
		${MAME_DIR}/src/devices/cpu/m6502/m4510.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m4510.h
		${MAME_DIR}/src/devices/cpu/m6502/m6502.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6502.h
		${MAME_DIR}/src/devices/cpu/m6502/m65c02.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m65c02.h
		${MAME_DIR}/src/devices/cpu/m6502/m65ce02.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m65ce02.h
		${MAME_DIR}/src/devices/cpu/m6502/m65sc02.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m65sc02.h
		${MAME_DIR}/src/devices/cpu/m6502/m6500_1.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6500_1.h
		${MAME_DIR}/src/devices/cpu/m6502/m6504.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6504.h
		${MAME_DIR}/src/devices/cpu/m6502/m6507.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6507.h
		${MAME_DIR}/src/devices/cpu/m6502/m6509.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6509.h
		${MAME_DIR}/src/devices/cpu/m6502/m6510.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6510.h
		${MAME_DIR}/src/devices/cpu/m6502/m6510t.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m6510t.h
		${MAME_DIR}/src/devices/cpu/m6502/m7501.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m7501.h
		${MAME_DIR}/src/devices/cpu/m6502/m8502.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m8502.h
		${MAME_DIR}/src/devices/cpu/m6502/n2a03.cpp
		${MAME_DIR}/src/devices/cpu/m6502/n2a03.h
		${MAME_DIR}/src/devices/cpu/m6502/r65c02.cpp
		${MAME_DIR}/src/devices/cpu/m6502/r65c02.h
		${MAME_DIR}/src/devices/cpu/m6502/r65c19.cpp
		${MAME_DIR}/src/devices/cpu/m6502/r65c19.h
		${MAME_DIR}/src/devices/cpu/m6502/m740.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m740.h
		${MAME_DIR}/src/devices/cpu/m6502/m3745x.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m3745x.h
		${MAME_DIR}/src/devices/cpu/m6502/m5074x.cpp
		${MAME_DIR}/src/devices/cpu/m6502/m5074x.h

		${GEN_DIR}/emu/cpu/m6502/deco16.hxx
		${GEN_DIR}/emu/cpu/m6502/m4510.hxx
		${GEN_DIR}/emu/cpu/m6502/m6502.hxx
		${GEN_DIR}/emu/cpu/m6502/m65c02.hxx
		${GEN_DIR}/emu/cpu/m6502/m65ce02.hxx
		${GEN_DIR}/emu/cpu/m6502/m6509.hxx
		${GEN_DIR}/emu/cpu/m6502/m6510.hxx
		${GEN_DIR}/emu/cpu/m6502/n2a03.hxx
		${GEN_DIR}/emu/cpu/m6502/r65c02.hxx
		${GEN_DIR}/emu/cpu/m6502/r65c19.hxx
		${GEN_DIR}/emu/cpu/m6502/m740.hxx
	)

	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s deco16 ${MAME_DIR}/src/devices/cpu/m6502/odeco16.lst ${MAME_DIR}/src/devices/cpu/m6502/ddeco16.lst ${GEN_DIR}/emu/cpu/m6502/deco16.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/odeco16.lst ${MAME_DIR}/src/devices/cpu/m6502/ddeco16.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/deco16.hxx
		COMMENT "Generating deco16 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m4510 ${MAME_DIR}/src/devices/cpu/m6502/om4510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm4510.lst ${GEN_DIR}/emu/cpu/m6502/m4510.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om4510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm4510.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m4510.hxx
		COMMENT "Generating m4510 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m6502 ${MAME_DIR}/src/devices/cpu/m6502/om6502.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6502.lst ${GEN_DIR}/emu/cpu/m6502/m6502.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6502.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6502.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6502.hxx
		COMMENT "Generating m6502 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m65c02 ${MAME_DIR}/src/devices/cpu/m6502/om65c02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65c02.lst ${GEN_DIR}/emu/cpu/m6502/m65c02.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om65c02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65c02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m65c02.hxx
		COMMENT "Generating m65c02 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m65ce02 ${MAME_DIR}/src/devices/cpu/m6502/om65ce02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65ce02.lst ${GEN_DIR}/emu/cpu/m6502/m65ce02.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om65ce02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65ce02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m65ce02.hxx
		COMMENT "Generating m65ce02 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m6509 ${MAME_DIR}/src/devices/cpu/m6502/om6509.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6509.lst ${GEN_DIR}/emu/cpu/m6502/m6509.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6509.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6509.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6509.hxx
		COMMENT "Generating m6509 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m6510 ${MAME_DIR}/src/devices/cpu/m6502/om6510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6510.lst ${GEN_DIR}/emu/cpu/m6502/m6510.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6510.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6510.hxx
		COMMENT "Generating m6510 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s n2a03_core ${MAME_DIR}/src/devices/cpu/m6502/on2a03.lst ${MAME_DIR}/src/devices/cpu/m6502/dn2a03.lst ${GEN_DIR}/emu/cpu/m6502/n2a03.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/on2a03.lst ${MAME_DIR}/src/devices/cpu/m6502/dn2a03.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/n2a03.hxx
		COMMENT "Generating n2a03 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s m740 ${MAME_DIR}/src/devices/cpu/m6502/om740.lst ${MAME_DIR}/src/devices/cpu/m6502/dm740.lst ${GEN_DIR}/emu/cpu/m6502/m740.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om740.lst ${MAME_DIR}/src/devices/cpu/m6502/dm740.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m740.hxx
		COMMENT "Generating m740 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s r65c02 - ${MAME_DIR}/src/devices/cpu/m6502/dr65c02.lst ${GEN_DIR}/emu/cpu/m6502/r65c02.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/dr65c02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/r65c02.hxx
		COMMENT "Generating r65c02 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s r65c19 ${MAME_DIR}/src/devices/cpu/m6502/or65c19.lst ${MAME_DIR}/src/devices/cpu/m6502/dr65c19.lst ${GEN_DIR}/emu/cpu/m6502/r65c19.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/or65c19.lst ${MAME_DIR}/src/devices/cpu/m6502/dr65c19.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/r65c19.hxx
		COMMENT "Generating r65c19 source file..."
	)
endif()

if("ST2XXX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6502/st2xxx.cpp
		${MAME_DIR}/src/devices/cpu/m6502/st2xxx.h
		${MAME_DIR}/src/devices/cpu/m6502/st2204.cpp
		${MAME_DIR}/src/devices/cpu/m6502/st2204.h
		${MAME_DIR}/src/devices/cpu/m6502/st2205u.cpp
		${MAME_DIR}/src/devices/cpu/m6502/st2205u.h
		${GEN_DIR}/emu/cpu/m6502/st2xxx.hxx
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s st2xxx ${MAME_DIR}/src/devices/cpu/m6502/ost2xxx.lst ${MAME_DIR}/src/devices/cpu/m6502/dst2xxx.lst ${GEN_DIR}/emu/cpu/m6502/st2xxx.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/ost2xxx.lst ${MAME_DIR}/src/devices/cpu/m6502/dst2xxx.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/st2xxx.hxx
		COMMENT "Generating st2xxx source file..."
	)
endif()

if("XAVIX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6502/xavix.cpp
		${GEN_DIR}/emu/cpu/m6502/xavix.hxx
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s xavix ${MAME_DIR}/src/devices/cpu/m6502/oxavix.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix.lst ${GEN_DIR}/emu/cpu/m6502/xavix.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/oxavix.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/xavix.hxx
		COMMENT "Generating st2xxx source file..."
	)
endif()

if("XAVIX2000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6502/xavix2000.cpp
		${GEN_DIR}/emu/cpu/m6502/xavix2000.hxx
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py s xavix2000 ${MAME_DIR}/src/devices/cpu/m6502/oxavix2000.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix2000.lst ${GEN_DIR}/emu/cpu/m6502/xavix2000.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/oxavix2000.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix2000.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/xavix2000.hxx
		COMMENT "Generating xavix2000 source file..."
	)
endif()

if(("M6502" IN_LIST CPUS) OR TOOLS)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d deco16 ${MAME_DIR}/src/devices/cpu/m6502/odeco16.lst ${MAME_DIR}/src/devices/cpu/m6502/ddeco16.lst ${GEN_DIR}/emu/cpu/m6502/deco16d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/odeco16.lst ${MAME_DIR}/src/devices/cpu/m6502/ddeco16.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/deco16d.hxx
		COMMENT "Generating deco16 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m4510 ${MAME_DIR}/src/devices/cpu/m6502/om4510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm4510.lst ${GEN_DIR}/emu/cpu/m6502/m4510d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om4510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm4510.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m4510d.hxx
		COMMENT "Generating m4510 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m6502 ${MAME_DIR}/src/devices/cpu/m6502/om6502.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6502.lst ${GEN_DIR}/emu/cpu/m6502/m6502d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6502.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6502.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6502d.hxx
		COMMENT "Generating m6502 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m65c02 ${MAME_DIR}/src/devices/cpu/m6502/om65c02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65c02.lst ${GEN_DIR}/emu/cpu/m6502/m65c02d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om65c02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65c02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m65c02d.hxx
		COMMENT "Generating m65c02 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m65ce02 ${MAME_DIR}/src/devices/cpu/m6502/om65ce02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65ce02.lst ${GEN_DIR}/emu/cpu/m6502/m65ce02d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om65ce02.lst ${MAME_DIR}/src/devices/cpu/m6502/dm65ce02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m65ce02d.hxx
		COMMENT "Generating m65ce02 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m6509 ${MAME_DIR}/src/devices/cpu/m6502/om6509.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6509.lst ${GEN_DIR}/emu/cpu/m6502/m6509d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6509.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6509.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6509d.hxx
		COMMENT "Generating m6509 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m6510 ${MAME_DIR}/src/devices/cpu/m6502/om6510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6510.lst ${GEN_DIR}/emu/cpu/m6502/m6510d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om6510.lst ${MAME_DIR}/src/devices/cpu/m6502/dm6510.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m6510d.hxx
		COMMENT "Generating m6510 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d n2a03 ${MAME_DIR}/src/devices/cpu/m6502/on2a03.lst ${MAME_DIR}/src/devices/cpu/m6502/dn2a03.lst ${GEN_DIR}/emu/cpu/m6502/n2a03d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/on2a03.lst ${MAME_DIR}/src/devices/cpu/m6502/dn2a03.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/n2a03d.hxx
		COMMENT "Generating n2a03 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d m740 ${MAME_DIR}/src/devices/cpu/m6502/om740.lst ${MAME_DIR}/src/devices/cpu/m6502/dm740.lst ${GEN_DIR}/emu/cpu/m6502/m740d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/om740.lst ${MAME_DIR}/src/devices/cpu/m6502/dm740.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/m740d.hxx
		COMMENT "Generating m740 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d r65c02 - ${MAME_DIR}/src/devices/cpu/m6502/dr65c02.lst ${GEN_DIR}/emu/cpu/m6502/r65c02d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/dr65c02.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/r65c02d.hxx
		COMMENT "Generating r65c02 disassembler source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d r65c19 ${MAME_DIR}/src/devices/cpu/m6502/or65c19.lst ${MAME_DIR}/src/devices/cpu/m6502/dr65c19.lst ${GEN_DIR}/emu/cpu/m6502/r65c19d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/or65c19.lst ${MAME_DIR}/src/devices/cpu/m6502/dr65c19.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/r65c19d.hxx
		COMMENT "Generating r65c19 disassembler source file..."
	)

	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/deco16d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m4510d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m6502d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m65c02d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m65ce02d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m6509d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m6510d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/n2a03d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/r65c02d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/r65c19d.hxx)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/m740d.hxx)

	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/deco16d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/deco16d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m4510d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m4510d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6502d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6502d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6509d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6509d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6510d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m6510d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m65c02d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m65c02d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m65ce02d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m65ce02d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m740d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/m740d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/n2a03d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/n2a03d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/r65c02d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/r65c02d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/r65c19d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/r65c19d.h)
endif()

if(("XAVIX" IN_LIST CPUS) OR TOOLS)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d xavix ${MAME_DIR}/src/devices/cpu/m6502/oxavix.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix.lst ${GEN_DIR}/emu/cpu/m6502/xavixd.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/oxavix.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/xavixd.hxx
		COMMENT "Generating xavix disassembler source file..."
	)

	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/xavixd.hxx)

	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/xavixd.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/xavixd.cpp)
endif()

if(("XAVIX2000" IN_LIST CPUS) OR TOOLS)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6502/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py d xavix2000 ${MAME_DIR}/src/devices/cpu/m6502/oxavix2000.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix2000.lst ${GEN_DIR}/emu/cpu/m6502/xavix2000d.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6502/m6502make.py ${MAME_DIR}/src/devices/cpu/m6502/oxavix2000.lst ${MAME_DIR}/src/devices/cpu/m6502/dxavix2000.lst
		OUTPUT ${GEN_DIR}/emu/cpu/m6502/xavix2000d.hxx
		COMMENT "Generating xavix2000 disassembler source file..."
	)

	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/m6502/xavix2000d.hxx)

	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/xavix2000d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6502/xavix2000d.cpp)
endif()

##################################################
## Motorola 680x
##@src/devices/cpu/m6800/m6800.h,list(APPEND CPUS M6800)
##@src/devices/cpu/m6800/m6801.h,list(APPEND CPUS M6800)
##################################################

if("M6800" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6800/m6800.cpp
		${MAME_DIR}/src/devices/cpu/m6800/m6800.h
		${MAME_DIR}/src/devices/cpu/m6800/m6801.cpp
		${MAME_DIR}/src/devices/cpu/m6800/m6801.h
		${MAME_DIR}/src/devices/cpu/m6800/6800ops.hxx
	)
endif()

if(("M6800" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6800/6800dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6800/6800dasm.h)
endif()

##################################################
## Motorola 6805
##@src/devices/cpu/m6805/m6805.h,list(APPEND CPUS M6805)
##################################################

if("M6805" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6805/m6805.cpp
		${MAME_DIR}/src/devices/cpu/m6805/m6805.h
		${MAME_DIR}/src/devices/cpu/m6805/m6805defs.h
		${MAME_DIR}/src/devices/cpu/m6805/6805ops.hxx
		${MAME_DIR}/src/devices/cpu/m6805/m68705.cpp
		${MAME_DIR}/src/devices/cpu/m6805/m68705.h
		${MAME_DIR}/src/devices/cpu/m6805/m68hc05.cpp
		${MAME_DIR}/src/devices/cpu/m6805/m68hc05.h
	)
endif()

if(("M6805" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6805/6805dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6805/6805dasm.h)
endif()

##################################################
## Motorola 6809
##@src/devices/cpu/m6809/m6809.h,list(APPEND CPUS M6809)
##@src/devices/cpu/m6809/hd6309.h,list(APPEND CPUS M6809)
##@src/devices/cpu/m6809/konami.h,list(APPEND CPUS M6809)
##################################################

if("M6809" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m6809/m6809.cpp
		${MAME_DIR}/src/devices/cpu/m6809/m6809.h
		${MAME_DIR}/src/devices/cpu/m6809/hd6309.cpp
		${MAME_DIR}/src/devices/cpu/m6809/hd6309.h
		${MAME_DIR}/src/devices/cpu/m6809/konami.cpp
		${MAME_DIR}/src/devices/cpu/m6809/konami.h
		${MAME_DIR}/src/devices/cpu/m6809/m6809inl.h

		${GEN_DIR}/emu/cpu/m6809/m6809.hxx
		${GEN_DIR}/emu/cpu/m6809/hd6309.hxx
		${GEN_DIR}/emu/cpu/m6809/konami.hxx
	)

	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6809/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/m6809.ops > ${GEN_DIR}/emu/cpu/m6809/m6809.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/base6x09.ops ${MAME_DIR}/src/devices/cpu/m6809/m6809.ops
		OUTPUT ${GEN_DIR}/emu/cpu/m6809/m6809.hxx
		COMMENT "Generating m6809 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6809/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/hd6309.ops > ${GEN_DIR}/emu/cpu/m6809/hd6309.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/base6x09.ops ${MAME_DIR}/src/devices/cpu/m6809/hd6309.ops
		OUTPUT ${GEN_DIR}/emu/cpu/m6809/hd6309.hxx
		COMMENT "Generating hd6309 source file..."
	)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/m6809/
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/konami.ops > ${GEN_DIR}/emu/cpu/m6809/konami.hxx
		DEPENDS ${MAME_DIR}/src/devices/cpu/m6809/m6809make.py ${MAME_DIR}/src/devices/cpu/m6809/base6x09.ops ${MAME_DIR}/src/devices/cpu/m6809/konami.ops
		OUTPUT ${GEN_DIR}/emu/cpu/m6809/konami.hxx
		COMMENT "Generating konami source file..."
	)
endif()

if(("M6809" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6809/6x09dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m6809/6x09dasm.h)
endif()

##################################################
## Motorola 68HC11
##@src/devices/cpu/mc68hc11/mc68hc11.h,list(APPEND CPUS MC68HC11)
##################################################

if("MC68HC11" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mc68hc11/mc68hc11.cpp
		${MAME_DIR}/src/devices/cpu/mc68hc11/mc68hc11.h
		${MAME_DIR}/src/devices/cpu/mc68hc11/hc11ops.h
		${MAME_DIR}/src/devices/cpu/mc68hc11/hc11ops.hxx
	)
endif()

if(("MC68HC11" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mc68hc11/hc11dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mc68hc11/hc11dasm.h)
endif()

##################################################
## Motorola 68000 series
##@src/devices/cpu/m68000/m68000.h,list(APPEND CPUS M680X0)
##################################################

if("M680X0" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m68000/m68kcpu.cpp
		${MAME_DIR}/src/devices/cpu/m68000/m68kcpu.h
		${MAME_DIR}/src/devices/cpu/m68000/m68kops.cpp
		${MAME_DIR}/src/devices/cpu/m68000/m68kops.h
		${MAME_DIR}/src/devices/cpu/m68000/m68000.h
		${MAME_DIR}/src/devices/cpu/m68000/m68kfpu.cpp
		${MAME_DIR}/src/devices/cpu/m68000/m68kmmu.h
	)
endif()

if(("M680X0" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m68000/m68kdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m68000/m68kdasm.h)
endif()

##################################################
## Motorola/Freescale DSP56156
##@src/devices/cpu/dsp56156/dsp56156.h,list(APPEND CPUS DSP56156)
##################################################

if("DSP56156" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56156.cpp
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56156.h
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56mem.cpp
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56mem.h
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56pcu.cpp
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56pcu.h
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56def.h
		${MAME_DIR}/src/devices/cpu/dsp56156/dsp56ops.hxx
	)
endif()

if(("DSP56156" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/dsp56dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/dsp56dsm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/opcode.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/opcode.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/inst.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/inst.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/pmove.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/pmove.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/tables.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56156/tables.h)
endif()

##################################################
## PDP-1
##@src/devices/cpu/pdp1/pdp1.h,list(APPEND CPUS PDP1)
##################################################

if("PDP1" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pdp1/pdp1.cpp
		${MAME_DIR}/src/devices/cpu/pdp1/pdp1.h
	)
endif()

if(("PDP1" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pdp1/pdp1dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pdp1/pdp1dasm.h)
endif()

##################################################
## PATINHO FEIO - Escola Politecnica - USP (Brazil)
##@src/devices/cpu/patinhofeio/patinhofeio_cpu.h,list(APPEND CPUS PATINHOFEIO)
##################################################

if("PATINHOFEIO" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/patinhofeio/patinho_feio.cpp
		${MAME_DIR}/src/devices/cpu/patinhofeio/patinhofeio_cpu.h
	)
endif()

if(("PATINHOFEIO" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/patinhofeio/patinho_feio_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/patinhofeio/patinho_feio_dasm.h)
endif()

##################################################
## Motorola PowerPC series
##@src/devices/cpu/powerpc/ppc.h,list(APPEND CPUS POWERPC)
##################################################

if("POWERPC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/powerpc/ppccom.cpp
		${MAME_DIR}/src/devices/cpu/powerpc/ppccom.h
		${MAME_DIR}/src/devices/cpu/powerpc/ppcfe.cpp
		${MAME_DIR}/src/devices/cpu/powerpc/ppcfe.h
		${MAME_DIR}/src/devices/cpu/powerpc/ppcdrc.cpp
		${MAME_DIR}/src/devices/cpu/powerpc/ppc.h
	)
endif()

if(("POWERPC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/powerpc/ppc_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/powerpc/ppc_dasm.h)
endif()

##################################################
## NEC V-series Intel-compatible
##@src/devices/cpu/nec/nec.h,list(APPEND CPUS NEC)
##@src/devices/cpu/nec/v25.h,list(APPEND CPUS NEC)
##@src/devices/cpu/nec/v5x.h,list(APPEND CPUS NEC)
##@src/devices/cpu/v30mz/v30mz.h,list(APPEND CPUS V30MZ)
##################################################

if("NEC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/nec/nec.cpp
		${MAME_DIR}/src/devices/cpu/nec/nec.h
		${MAME_DIR}/src/devices/cpu/nec/necea.h
		${MAME_DIR}/src/devices/cpu/nec/necinstr.h
		${MAME_DIR}/src/devices/cpu/nec/necinstr.hxx
		${MAME_DIR}/src/devices/cpu/nec/nec80inst.hxx
		${MAME_DIR}/src/devices/cpu/nec/necmacro.h
		${MAME_DIR}/src/devices/cpu/nec/necmodrm.h
		${MAME_DIR}/src/devices/cpu/nec/necpriv.ipp
		${MAME_DIR}/src/devices/cpu/nec/v25instr.h
		${MAME_DIR}/src/devices/cpu/nec/v25instr.hxx
		${MAME_DIR}/src/devices/cpu/nec/v25priv.ipp
		${MAME_DIR}/src/devices/cpu/nec/v25.cpp
		${MAME_DIR}/src/devices/cpu/nec/v25.h
		${MAME_DIR}/src/devices/cpu/nec/v25sfr.cpp
		${MAME_DIR}/src/devices/cpu/nec/v5x.cpp
		${MAME_DIR}/src/devices/cpu/nec/v5x.h
	)
endif()

if(("NEC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nec/necdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nec/necdasm.h)
endif()

if("V30MZ" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/v30mz/v30mz.cpp
		${MAME_DIR}/src/devices/cpu/v30mz/v30mz.h
	)
endif()

if(("V30MZ" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nec/necdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nec/necdasm.h)
endif()

##################################################
## NEC V60/V70
##@src/devices/cpu/v60/v60.h,list(APPEND CPUS V60)
##################################################

if("V60" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/v60/v60.cpp
		${MAME_DIR}/src/devices/cpu/v60/v60.h
		${MAME_DIR}/src/devices/cpu/v60/am.hxx
		${MAME_DIR}/src/devices/cpu/v60/am1.hxx
		${MAME_DIR}/src/devices/cpu/v60/am2.hxx
		${MAME_DIR}/src/devices/cpu/v60/am3.hxx
		${MAME_DIR}/src/devices/cpu/v60/op12.hxx
		${MAME_DIR}/src/devices/cpu/v60/op2.hxx
		${MAME_DIR}/src/devices/cpu/v60/op3.hxx
		${MAME_DIR}/src/devices/cpu/v60/op4.hxx
		${MAME_DIR}/src/devices/cpu/v60/op5.hxx
		${MAME_DIR}/src/devices/cpu/v60/op6.hxx
		${MAME_DIR}/src/devices/cpu/v60/op7a.hxx
		${MAME_DIR}/src/devices/cpu/v60/optable.hxx
	)
endif()

if(("V60" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v60/v60d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v60/v60d.h)
endif()

##################################################
## NEC V810 (uPD70732)
##@src/devices/cpu/v810/v810.h,list(APPEND CPUS V810)
##################################################

if("V810" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/v810/v810.cpp
		${MAME_DIR}/src/devices/cpu/v810/v810.h
	)
endif()

if(("V810" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v810/v810dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v810/v810dasm.h)
endif()

##################################################
## NEC V850, disassembler only
##################################################

if(("V850" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v850/v850dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/v850/v850dasm.h)
endif()

##################################################
## NEC uPD7725
##@src/devices/cpu/upd7725/upd7725.h,list(APPEND CPUS UPD7725)
##################################################

if("UPD7725" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/upd7725/upd7725.cpp
		${MAME_DIR}/src/devices/cpu/upd7725/upd7725.h
	)
endif()

if(("UPD7725" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd7725/dasm7725.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd7725/dasm7725.h)
endif()

##################################################
## NEC uPD7810 series
##@src/devices/cpu/upd7810/upd7810.h,list(APPEND CPUS UPD7810)
##################################################

if("UPD7810" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/upd7810/upd7810.cpp
		${MAME_DIR}/src/devices/cpu/upd7810/upd7810.h
		${MAME_DIR}/src/devices/cpu/upd7810/upd7810_opcodes.cpp
		${MAME_DIR}/src/devices/cpu/upd7810/upd7810_table.cpp
		${MAME_DIR}/src/devices/cpu/upd7810/upd7810_macros.h
		${MAME_DIR}/src/devices/cpu/upd7810/upd7811.cpp
		${MAME_DIR}/src/devices/cpu/upd7810/upd7811.h
	)
endif()

if(("UPD7810" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd7810/upd7810_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd7810/upd7810_dasm.h)
endif()

##################################################
## NEC uCOM-4 series
##@src/devices/cpu/ucom4/ucom4.h,list(APPEND CPUS UCOM4)
##################################################

if("UCOM4" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ucom4/ucom4.cpp
		${MAME_DIR}/src/devices/cpu/ucom4/ucom4.h
		${MAME_DIR}/src/devices/cpu/ucom4/ucom4op.cpp
	)
endif()

if(("UCOM4" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ucom4/ucom4d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ucom4/ucom4d.h)
endif()

##################################################
## Nintendif()o Minx
##@src/devices/cpu/minx/minx.h,list(APPEND CPUS MINX)
##################################################

if("MINX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/minx/minx.cpp
		${MAME_DIR}/src/devices/cpu/minx/minx.h
		${MAME_DIR}/src/devices/cpu/minx/minxfunc.h
		${MAME_DIR}/src/devices/cpu/minx/minxopce.h
		${MAME_DIR}/src/devices/cpu/minx/minxopcf.h
		${MAME_DIR}/src/devices/cpu/minx/minxops.h
	)
endif()

if(("MINX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/minx/minxd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/minx/minxd.h)
endif()

##################################################
## Nintendif()o/SGI RSP (R3000-based + vector processing)
##@src/devices/cpu/rsp/rsp.h,list(APPEND CPUS RSP)
##################################################

if("RSP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/rsp/rsp.cpp
		${MAME_DIR}/src/devices/cpu/rsp/rsp.h
		${MAME_DIR}/src/devices/cpu/rsp/rspdefs.h
		${MAME_DIR}/src/devices/cpu/rsp/rspdrc.cpp
		${MAME_DIR}/src/devices/cpu/rsp/rspfe.cpp
		${MAME_DIR}/src/devices/cpu/rsp/rspfe.h
		${MAME_DIR}/src/devices/cpu/rsp/rspcp2.cpp
		${MAME_DIR}/src/devices/cpu/rsp/rspcp2.h
		${MAME_DIR}/src/devices/cpu/rsp/rspcp2d.cpp
		${MAME_DIR}/src/devices/cpu/rsp/rspcp2d.h
		${MAME_DIR}/src/devices/cpu/rsp/clamp.h
		${MAME_DIR}/src/devices/cpu/rsp/vabs.h
		${MAME_DIR}/src/devices/cpu/rsp/vadd.h
		${MAME_DIR}/src/devices/cpu/rsp/vaddc.h
		${MAME_DIR}/src/devices/cpu/rsp/vand.h
		${MAME_DIR}/src/devices/cpu/rsp/vch.h
		${MAME_DIR}/src/devices/cpu/rsp/vcl.h
		${MAME_DIR}/src/devices/cpu/rsp/vcmp.h
		${MAME_DIR}/src/devices/cpu/rsp/vcr.h
		${MAME_DIR}/src/devices/cpu/rsp/vdivh.h
		${MAME_DIR}/src/devices/cpu/rsp/vldst.h
		${MAME_DIR}/src/devices/cpu/rsp/vmac.h
		${MAME_DIR}/src/devices/cpu/rsp/vmov.h
		${MAME_DIR}/src/devices/cpu/rsp/vmrg.h
		${MAME_DIR}/src/devices/cpu/rsp/vmudh.h
		${MAME_DIR}/src/devices/cpu/rsp/vmul.h
		${MAME_DIR}/src/devices/cpu/rsp/vmulh.h
		${MAME_DIR}/src/devices/cpu/rsp/vmull.h
		${MAME_DIR}/src/devices/cpu/rsp/vmulm.h
		${MAME_DIR}/src/devices/cpu/rsp/vmuln.h
		${MAME_DIR}/src/devices/cpu/rsp/vor.h
		${MAME_DIR}/src/devices/cpu/rsp/vrcpsq.h
		${MAME_DIR}/src/devices/cpu/rsp/vrsq.h
		${MAME_DIR}/src/devices/cpu/rsp/vsub.h
		${MAME_DIR}/src/devices/cpu/rsp/vsubc.h
		${MAME_DIR}/src/devices/cpu/rsp/vxor.h
		${MAME_DIR}/src/devices/cpu/rsp/rspdiv.h
	)
endif()

if(("RSP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rsp/rsp_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rsp/rsp_dasm.h)
endif()

##################################################
## Panasonic MN1880
##@src/devices/cpu/mn1880/mn1880.h,list(APPEND CPUS MN1880)
##################################################

if("MN1880" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mn1880/mn1880.cpp
		${MAME_DIR}/src/devices/cpu/mn1880/mn1880.h
	)
endif()

if(("MN1880" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mn1880/mn1880d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mn1880/mn1880d.h)
endif()

##################################################
## Panasonic MN10200
##@src/devices/cpu/mn10200/mn10200.h,list(APPEND CPUS MN10200)
##################################################

if("MN10200" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mn10200/mn10200.cpp
		${MAME_DIR}/src/devices/cpu/mn10200/mn10200.h
	)
endif()

if(("MN10200" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mn10200/mn102dis.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mn10200/mn102dis.h)
endif()

##################################################
## Saturn
##@src/devices/cpu/saturn/saturn.h,list(APPEND CPUS SATURN)
##################################################

if("SATURN" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/saturn/saturn.cpp
		${MAME_DIR}/src/devices/cpu/saturn/saturn.h
		${MAME_DIR}/src/devices/cpu/saturn/satops.ipp
		${MAME_DIR}/src/devices/cpu/saturn/sattable.ipp
	)
endif()

if(("SATURN" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/saturn/saturnds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/saturn/saturnds.h)
endif()

##################################################
## Sharp SM510 series
##@src/devices/cpu/sm510/sm510.h,list(APPEND CPUS SM510)
##################################################

if("SM510" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sm510/sm510base.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm510.h
		${MAME_DIR}/src/devices/cpu/sm510/sm510op.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm510core.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm511core.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm530.h
		${MAME_DIR}/src/devices/cpu/sm510/sm530op.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm530core.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm500.h
		${MAME_DIR}/src/devices/cpu/sm510/sm500op.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm500core.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm5acore.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm590.h
		${MAME_DIR}/src/devices/cpu/sm510/sm590op.cpp
		${MAME_DIR}/src/devices/cpu/sm510/sm590core.cpp
	)
endif()

if(("SM510" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sm510/sm510d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sm510/sm510d.h)
endif()

##################################################
## Sharp SM8500
##@src/devices/cpu/sm8500/sm8500.h,list(APPEND CPUS SM8500)
##################################################

if("SM8500" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sm8500/sm8500.cpp
		${MAME_DIR}/src/devices/cpu/sm8500/sm8500.h
		${MAME_DIR}/src/devices/cpu/sm8500/sm85ops.h
	)
endif()

if(("SM8500" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sm8500/sm8500d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sm8500/sm8500d.h)
endif()

##################################################
## Signetics 2650
##@src/devices/cpu/s2650/s2650.h,list(APPEND CPUS S2650)
##################################################

if("S2650" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/s2650/s2650.cpp
		${MAME_DIR}/src/devices/cpu/s2650/s2650.h
		${MAME_DIR}/src/devices/cpu/s2650/s2650cpu.h
	)
endif()

if(("S2650" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/s2650/2650dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/s2650/2650dasm.h)
endif()

##################################################
## SC61860
##@src/devices/cpu/sc61860/sc61860.h,list(APPEND CPUS SC61860)
##################################################

if("SC61860" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sc61860/sc61860.cpp
		${MAME_DIR}/src/devices/cpu/sc61860/sc61860.h
		##${MAME_DIR}/src/devices/cpu/sc61860/readpc.cpp
		${MAME_DIR}/src/devices/cpu/sc61860/scops.hxx
		${MAME_DIR}/src/devices/cpu/sc61860/sctable.hxx
	)
endif()

if(("SC61860" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sc61860/scdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sc61860/scdasm.h)
endif()

##################################################
## Sony/Nintendif()o SPC700
##@src/devices/cpu/spc700/spc700.h,list(APPEND CPUS SPC700)
##################################################

if("SPC700" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/spc700/spc700.cpp
		${MAME_DIR}/src/devices/cpu/spc700/spc700.h
		${MAME_DIR}/src/devices/cpu/spc700/spc700ds.h
	)
endif()

if(("SPC700" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/spc700/spc700ds.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/spc700/spc700ds.h)
endif()

##################################################
## SSP1601
##@src/devices/cpu/ssp1601/ssp1601.h,list(APPEND CPUS SSP1601)
##################################################

if("SSP1601" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ssp1601/ssp1601.cpp
		${MAME_DIR}/src/devices/cpu/ssp1601/ssp1601.h
	)
endif()

if(("SSP1601" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ssp1601/ssp1601d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ssp1601/ssp1601d.h)
endif()

##################################################
## SunPlus u'nSP
##@src/devices/cpu/unsp/unsp.h,list(APPEND CPUS UNSP)
##################################################

if("UNSP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/unsp/unsp.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unsp.h
		${MAME_DIR}/src/devices/cpu/unsp/unsp_extended.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unsp_jumps.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unsp_exxx.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unsp_fxxx.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unsp_other.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unspdefs.h
		${MAME_DIR}/src/devices/cpu/unsp/unspdrc.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unspfe.cpp
		${MAME_DIR}/src/devices/cpu/unsp/unspfe.h
	)
endif()

if(("UNSP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm_extended.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm_jumps.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm_exxx.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm_fxxx.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/unsp/unspdasm_other.cpp)
endif()

##################################################
## Atmel 8-bit AVR
##@src/devices/cpu/avr8/avr8.h,list(APPEND CPUS AVR8)
##################################################

if("AVR8" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/avr8/avr8.cpp
		${MAME_DIR}/src/devices/cpu/avr8/avr8.h
	)
endif()

if(("AVR8" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/avr8/avr8dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/avr8/avr8dasm.h)
endif()

##################################################
## Texas Instruments TMS1000 series
##@src/devices/cpu/tms1000/tms1000.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms1000c.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms1100.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms1400.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms0970.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms0980.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tms0270.h,list(APPEND CPUS TMS1000)
##@src/devices/cpu/tms1000/tp0320.h,list(APPEND CPUS TMS1000)
##################################################

if("TMS1000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms1000/tms1k_base.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms1k_base.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms1000.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms1000.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms1000c.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms1000c.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms1100.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms1100.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms1400.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms1400.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms0970.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms0970.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms0980.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms0980.h
		${MAME_DIR}/src/devices/cpu/tms1000/tms0270.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tms0270.h
		${MAME_DIR}/src/devices/cpu/tms1000/tp0320.cpp
		${MAME_DIR}/src/devices/cpu/tms1000/tp0320.h
	)
endif()

if(("TMS1000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms1000/tms1k_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms1000/tms1k_dasm.h)
endif()

##################################################
## Texas Instruments TMS7000 series
##@src/devices/cpu/tms7000/tms7000.h,list(APPEND CPUS TMS7000)
##################################################

if("TMS7000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms7000/tms7000.cpp
		${MAME_DIR}/src/devices/cpu/tms7000/tms7000.h
		${MAME_DIR}/src/devices/cpu/tms7000/tms7000op.cpp
	)
endif()

if(("TMS7000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms7000/7000dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms7000/7000dasm.h)
endif()

##################################################
## Texas Instruments TMS99xx series
##@src/devices/cpu/tms9900/tms9900.h,list(APPEND CPUS TMS9900)
##@src/devices/cpu/tms9900/tms9980a.h,list(APPEND CPUS TMS9900)
##@src/devices/cpu/tms9900/tms9995.h,list(APPEND CPUS TMS9900)
##@src/devices/cpu/tms9900/ti990_10.h,list(APPEND CPUS TMS9900)
##################################################

if("TMS9900" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms9900/tms9900.cpp
		${MAME_DIR}/src/devices/cpu/tms9900/tms9900.h
		${MAME_DIR}/src/devices/cpu/tms9900/tms9980a.cpp
		${MAME_DIR}/src/devices/cpu/tms9900/tms9980a.h
		${MAME_DIR}/src/devices/cpu/tms9900/tms9995.cpp
		${MAME_DIR}/src/devices/cpu/tms9900/tms9995.h
		${MAME_DIR}/src/devices/cpu/tms9900/ti990_10.cpp
		${MAME_DIR}/src/devices/cpu/tms9900/ti990_10.h
		${MAME_DIR}/src/devices/cpu/tms9900/tms99com.h
	)
endif()

if(("TMS9900" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms9900/9900dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms9900/9900dasm.h)
endif()

##################################################
## Texas Instruments TMS340x0 graphics controllers
##@src/devices/cpu/tms34010/tms34010.h,list(APPEND CPUS TMS340X0)
##################################################

if("TMS340X0" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms34010/tms34010.cpp
		${MAME_DIR}/src/devices/cpu/tms34010/tms34010.h
		${MAME_DIR}/src/devices/cpu/tms34010/34010fld.hxx
		${MAME_DIR}/src/devices/cpu/tms34010/34010gfx.hxx
		${MAME_DIR}/src/devices/cpu/tms34010/34010ops.h
		${MAME_DIR}/src/devices/cpu/tms34010/34010ops.hxx
		${MAME_DIR}/src/devices/cpu/tms34010/34010tbl.hxx
	)
endif()

if(("TMS340X0" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms34010/34010dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms34010/34010dsm.h)
endif()

##################################################
## Texas Instruments TMS3201x DSP
##@src/devices/cpu/tms32010/tms32010.h,list(APPEND CPUS TMS32010)
##################################################

if("TMS32010" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms32010/tms32010.cpp
		${MAME_DIR}/src/devices/cpu/tms32010/tms32010.h
	)
endif()

if(("TMS32010" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32010/32010dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32010/32010dsm.h)
endif()

##################################################
## Texas Instruments TMS3202x DSP
##@src/devices/cpu/tms32025/tms32025.h,list(APPEND CPUS TMS32025)
##################################################

if("TMS32025" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms32025/tms32025.cpp
		${MAME_DIR}/src/devices/cpu/tms32025/tms32025.h
	)
endif()

if(("TMS32025" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32025/32025dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32025/32025dsm.h)
endif()

##################################################
## Texas Instruments TMS3203x DSP
##@src/devices/cpu/tms32031/tms32031.h,list(APPEND CPUS TMS32031)
##################################################

if("TMS32031" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms32031/tms32031.cpp
		${MAME_DIR}/src/devices/cpu/tms32031/tms32031.h
		${MAME_DIR}/src/devices/cpu/tms32031/32031ops.hxx
	)
endif()

if(("TMS32031" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32031/dis32031.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32031/dis32031.h)
endif()

##################################################
## Texas Instruments TMS3205x DSP
##@src/devices/cpu/tms32051/tms32051.h,list(APPEND CPUS TMS32051)
##################################################

if("TMS32051" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms32051/tms32051.cpp
		${MAME_DIR}/src/devices/cpu/tms32051/tms32051.h
		${MAME_DIR}/src/devices/cpu/tms32051/32051ops.h
		${MAME_DIR}/src/devices/cpu/tms32051/32051ops.hxx
	)
endif()

if(("TMS32051" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32051/dis32051.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32051/dis32051.h)
endif()

##################################################
## Texas Instruments TMS3208x DSP
##@src/devices/cpu/tms32082/tms32082.h,list(APPEND CPUS TMS32082)
##################################################

if("TMS32082" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms32082/tms32082.cpp
		${MAME_DIR}/src/devices/cpu/tms32082/tms32082.h
		${MAME_DIR}/src/devices/cpu/tms32082/mp_ops.cpp
	)
endif()

if(("TMS32082" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32082/dis_mp.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32082/dis_mp.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32082/dis_pp.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms32082/dis_pp.h)
endif()

##################################################
## Texas Instruments TMS57002 DSP
##@src/devices/cpu/tms57002/tms57002.h,list(APPEND CPUS TMS57002)
##################################################

if("TMS57002" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tms57002/tms57002.cpp
		${MAME_DIR}/src/devices/cpu/tms57002/tms57002.h
		${MAME_DIR}/src/devices/cpu/tms57002/tmsops.cpp
		${MAME_DIR}/src/devices/cpu/tms57002/tms57kdec.cpp
		${GEN_DIR}/emu/cpu/tms57002/tms57002.hxx
	)
endif()

add_custom_command(
	COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/emu/cpu/tms57002/
	COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/src/devices/cpu/tms57002/tmsmake.py ${MAME_DIR}/src/devices/cpu/tms57002/tmsinstr.lst ${GEN_DIR}/emu/cpu/tms57002/tms57002.hxx
	DEPENDS ${MAME_DIR}/src/devices/cpu/tms57002/tmsmake.py ${MAME_DIR}/src/devices/cpu/tms57002/tmsinstr.lst
	OUTPUT ${GEN_DIR}/emu/cpu/tms57002/tms57002.hxx
	COMMENT "Generating TMS57002 source file..."
)

if(("TMS57002" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms57002/57002dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tms57002/57002dsm.h)
	list(APPEND DASM_SRCS ${GEN_DIR}/emu/cpu/tms57002/tms57002.hxx)
endif()

##################################################
## Toshiba TLCS-90 Series
##@src/devices/cpu/tlcs90/tlcs90.h,list(APPEND CPUS TLCS90)
##################################################

if("TLCS90" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tlcs90/tlcs90.cpp
		${MAME_DIR}/src/devices/cpu/tlcs90/tlcs90.h
	)
endif()

if(("TLCS90" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs90/tlcs90d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs90/tlcs90d.h)
endif()

##################################################
## Toshiba TLCS-870 Series
##@src/devices/cpu/tlcs870/tlcs870.h,list(APPEND CPUS TLCS870)
##################################################

if("TLCS870" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870_ops.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870_ops_reg.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870_ops_src.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870_ops_dst.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870_ops_helper.cpp
		${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870.h
	)
endif()

if(("TLCS870" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs870/tlcs870d.h)
endif()

##################################################
## Toshiba TLCS-900 Series
##@src/devices/cpu/tlcs900/tlcs900.h,list(APPEND CPUS TLCS900)
##################################################

if("TLCS900" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tlcs900/tlcs900.cpp
		${MAME_DIR}/src/devices/cpu/tlcs900/tlcs900.h
		${MAME_DIR}/src/devices/cpu/tlcs900/900tbl.hxx
		${MAME_DIR}/src/devices/cpu/tlcs900/900htbl.hxx
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp95c061.cpp
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp95c061.h
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp95c063.cpp
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp95c063.h
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp96c141.cpp
		${MAME_DIR}/src/devices/cpu/tlcs900/tmp96c141.h
	)
endif()

if(("TLCS900" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs900/dasm900.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tlcs900/dasm900.h)
endif()

##################################################
## TX0
##@src/devices/cpu/tx0/tx0.h,list(APPEND CPUS TX0)
##################################################

if("TX0" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/tx0/tx0.cpp
		${MAME_DIR}/src/devices/cpu/tx0/tx0.h
	)
endif()

if(("TX0" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tx0/tx0dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/tx0/tx0dasm.h)
endif()

##################################################
## Zilog Z80
##@src/devices/cpu/z80/z80.h,list(APPEND CPUS Z80)
##@src/devices/cpu/z80/kc82.h,list(APPEND CPUS KC80)
##@src/devices/cpu/z80/kl5c80a12.h,list(APPEND CPUS KC80)
##@src/devices/cpu/z80/kl5c80a16.h,list(APPEND CPUS KC80)
##@src/devices/cpu/z80/ky80.h,list(APPEND CPUS KC80)
##################################################

if (("Z80" IN_LIST CPUS) OR ("KC80" IN_LIST CPUS) OR TOOLS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/z80/z80.cpp
		${MAME_DIR}/src/devices/cpu/z80/z80.h
		${MAME_DIR}/src/devices/cpu/z80/tmpz84c011.cpp
		${MAME_DIR}/src/devices/cpu/z80/tmpz84c011.h
		${MAME_DIR}/src/devices/cpu/z80/tmpz84c015.cpp
		${MAME_DIR}/src/devices/cpu/z80/tmpz84c015.h
		${MAME_DIR}/src/devices/cpu/z80/lz8420m.cpp
		${MAME_DIR}/src/devices/cpu/z80/lz8420m.h
	)
endif()

if("KC80" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/z80/kc82.cpp
		${MAME_DIR}/src/devices/cpu/z80/kc82.h
		${MAME_DIR}/src/devices/cpu/z80/kl5c80a12.cpp
		${MAME_DIR}/src/devices/cpu/z80/kl5c80a12.h
		${MAME_DIR}/src/devices/cpu/z80/kl5c80a16.cpp
		${MAME_DIR}/src/devices/cpu/z80/kl5c80a16.h
		${MAME_DIR}/src/devices/cpu/z80/kp63.cpp
		${MAME_DIR}/src/devices/cpu/z80/kp63.h
		${MAME_DIR}/src/devices/cpu/z80/kp69.cpp
		${MAME_DIR}/src/devices/cpu/z80/kp69.h
		${MAME_DIR}/src/devices/cpu/z80/ky80.cpp
		${MAME_DIR}/src/devices/cpu/z80/ky80.h
	)
endif()

if (("Z80" IN_LIST CPUS) OR ("KC80" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z80/z80dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z80/z80dasm.h)
endif()

##################################################
## Sharp LR35902 (Game Boy CPU)
##@src/devices/cpu/lr35902/lr35902.h,list(APPEND CPUS LR35902)
##################################################

if("LR35902" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/lr35902/lr35902.cpp
		${MAME_DIR}/src/devices/cpu/lr35902/lr35902.h
		${MAME_DIR}/src/devices/cpu/lr35902/opc_cb.hxx
		${MAME_DIR}/src/devices/cpu/lr35902/opc_main.hxx
	)
endif()

if(("LR35902" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lr35902/lr35902d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lr35902/lr35902d.h)
endif()

##################################################
## Zilog Z180
##@src/devices/cpu/z180/z180.h,list(APPEND CPUS Z180)
##################################################

if("Z180" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/z180/hd647180x.cpp
		${MAME_DIR}/src/devices/cpu/z180/hd647180x.h
		${MAME_DIR}/src/devices/cpu/z180/z180.cpp
		${MAME_DIR}/src/devices/cpu/z180/z180.h
		${MAME_DIR}/src/devices/cpu/z180/z180cb.hxx
		${MAME_DIR}/src/devices/cpu/z180/z180dd.hxx
		${MAME_DIR}/src/devices/cpu/z180/z180ed.hxx
		${MAME_DIR}/src/devices/cpu/z180/z180fd.hxx
		${MAME_DIR}/src/devices/cpu/z180/z180op.hxx
		${MAME_DIR}/src/devices/cpu/z180/z180ops.h
		${MAME_DIR}/src/devices/cpu/z180/z180tbl.h
		${MAME_DIR}/src/devices/cpu/z180/z180xy.hxx
	)
endif()

if(("Z180" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z180/z180dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z180/z180dasm.h)
endif()

##################################################
## Zilog Z8000
##@src/devices/cpu/z8000/z8000.h,list(APPEND CPUS Z8000)
##################################################

if("Z8000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/z8000/z8000.cpp
		${MAME_DIR}/src/devices/cpu/z8000/z8000.h
		##${MAME_DIR}/src/devices/cpu/z8000/makedab.cpp
		${MAME_DIR}/src/devices/cpu/z8000/z8000cpu.h
		${MAME_DIR}/src/devices/cpu/z8000/z8000dab.h
		${MAME_DIR}/src/devices/cpu/z8000/z8000ops.hxx
		${MAME_DIR}/src/devices/cpu/z8000/z8000tbl.hxx
	)
endif()

if(("Z8000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z8000/8000dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z8000/8000dasm.h)
endif()

##################################################
## Zilog Z8
##@src/devices/cpu/z8/z8.h,list(APPEND CPUS Z8)
##################################################

if("Z8" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/z8/z8.cpp
		${MAME_DIR}/src/devices/cpu/z8/z8.h
		${MAME_DIR}/src/devices/cpu/z8/z8ops.hxx
	)
endif()

if(("Z8" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z8/z8dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/z8/z8dasm.h)
endif()

##################################################
## Argonaut SuperFX
##@src/devices/cpu/superfx/superfx.h,list(APPEND CPUS SUPERFX)
##################################################

if("SUPERFX" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/superfx/superfx.cpp
		${MAME_DIR}/src/devices/cpu/superfx/superfx.h
	)
endif()

if(("SUPERFX" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/superfx/sfx_dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/superfx/sfx_dasm.h)
endif()

##################################################
## Rockwell PPS-4
##@src/devices/cpu/pps4/pps4.h,list(APPEND CPUS PPS4)
##################################################

if("PPS4" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pps4/pps4.cpp
		${MAME_DIR}/src/devices/cpu/pps4/pps4.h
	)
endif()

if(("PPS4" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pps4/pps4dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pps4/pps4dasm.h)
endif()

##################################################
## Rockwell PPS-4/1
##@src/devices/cpu/pps41/mm75.h,list(APPEND CPUS PPS41)
##@src/devices/cpu/pps41/mm76.h,list(APPEND CPUS PPS41)
##@src/devices/cpu/pps41/mm78.h,list(APPEND CPUS PPS41)
##@src/devices/cpu/pps41/mm78la.h,list(APPEND CPUS PPS41)
##################################################

if("PPS41" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pps41/pps41base.cpp
		${MAME_DIR}/src/devices/cpu/pps41/pps41base.h
		${MAME_DIR}/src/devices/cpu/pps41/mm75.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm75.h
		${MAME_DIR}/src/devices/cpu/pps41/mm75op.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm76.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm76.h
		${MAME_DIR}/src/devices/cpu/pps41/mm76op.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm78.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm78.h
		${MAME_DIR}/src/devices/cpu/pps41/mm78op.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm78la.cpp
		${MAME_DIR}/src/devices/cpu/pps41/mm78la.h
		${MAME_DIR}/src/devices/cpu/pps41/mm78laop.cpp
	)
endif()

if(("PPS41" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pps41/pps41d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pps41/pps41d.h)
endif()

##################################################
## Hitachi HD61700
##@src/devices/cpu/hd61700/hd61700.h,list(APPEND CPUS HD61700)
##################################################

if("HD61700" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/hd61700/hd61700.cpp
		${MAME_DIR}/src/devices/cpu/hd61700/hd61700.h
	)
endif()

if(("HD61700" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hd61700/hd61700d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hd61700/hd61700d.h)
endif()

##################################################
## Sanyo LC8670
##@src/devices/cpu/lc8670/lc8670.h,list(APPEND CPUS LC8670)
##################################################

if("LC8670" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/lc8670/lc8670.cpp
		${MAME_DIR}/src/devices/cpu/lc8670/lc8670.h
	)
endif()

if(("LC8670" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc8670/lc8670dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc8670/lc8670dsm.h)
endif()

##################################################
## Sega SCU DSP
##@src/devices/cpu/scudsp/scudsp.h,list(APPEND CPUS SCUDSP)
##################################################

if("SCUDSP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/scudsp/scudsp.cpp
		${MAME_DIR}/src/devices/cpu/scudsp/scudsp.h
	)
endif()

if(("SCUDSP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/scudsp/scudspdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/scudsp/scudspdasm.h)
endif()

##################################################
## Sunplus Technology S+core
##@src/devices/cpu/score/score.h,list(APPEND CPUS SCORE)
##################################################

if("SCORE" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/score/score.cpp
		${MAME_DIR}/src/devices/cpu/score/score.h
		${MAME_DIR}/src/devices/cpu/score/scorem.h
	)
endif()

if(("SCORE" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/score/scoredsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/score/scoredsm.h)
endif()

##################################################
## Xerox Alto-II
##@src/devices/cpu/alto2/alto2cpu.h,list(APPEND CPUS ALTO2)
##################################################

if("ALTO2" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/alto2/alto2cpu.cpp
		${MAME_DIR}/src/devices/cpu/alto2/alto2cpu.h
		${MAME_DIR}/src/devices/cpu/alto2/a2disk.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2disk.h
		${MAME_DIR}/src/devices/cpu/alto2/a2disp.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2disp.h
		${MAME_DIR}/src/devices/cpu/alto2/a2curt.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2curt.h
		${MAME_DIR}/src/devices/cpu/alto2/a2dht.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2dht.h
		${MAME_DIR}/src/devices/cpu/alto2/a2dvt.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2dvt.h
		${MAME_DIR}/src/devices/cpu/alto2/a2dwt.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2dwt.h
		${MAME_DIR}/src/devices/cpu/alto2/a2emu.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2emu.h
		${MAME_DIR}/src/devices/cpu/alto2/a2ether.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2ether.h
		${MAME_DIR}/src/devices/cpu/alto2/a2hw.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2hw.h
		${MAME_DIR}/src/devices/cpu/alto2/a2kbd.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2kbd.h
		${MAME_DIR}/src/devices/cpu/alto2/a2ksec.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2ksec.h
		${MAME_DIR}/src/devices/cpu/alto2/a2kwd.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2kwd.h
		${MAME_DIR}/src/devices/cpu/alto2/a2mem.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2mem.h
		${MAME_DIR}/src/devices/cpu/alto2/a2mouse.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2mouse.h
		${MAME_DIR}/src/devices/cpu/alto2/a2mrt.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2mrt.h
		${MAME_DIR}/src/devices/cpu/alto2/a2part.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2part.h
		${MAME_DIR}/src/devices/cpu/alto2/a2ram.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2ram.h
		${MAME_DIR}/src/devices/cpu/alto2/a2roms.cpp
		${MAME_DIR}/src/devices/cpu/alto2/a2roms.h
		${MAME_DIR}/src/devices/cpu/alto2/a2jkff.h
	)
endif()

if(("ALTO2" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/alto2/alto2dsm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/alto2/alto2dsm.h)
endif()

##########################################
## Sun SPARCv7, SPARCv8 implementation
##@src/devices/cpu/sparc/sparc.h,list(APPEND CPUS SPARC)
##################################################

if("SPARC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/sparc/sparc.cpp
		${MAME_DIR}/src/devices/cpu/sparc/sparcdefs.h
		${MAME_DIR}/src/devices/cpu/sparc/sparc_intf.h
		${MAME_DIR}/src/devices/cpu/sparc/sparc.h
	)
endif()

if(("SPARC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sparc/sparcdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/sparc/sparcdasm.h)
endif()

##################################################
## Intergraph CLIPPER (C100/C300/C400) series
##@src/devices/cpu/clipper/clipper.h,list(APPEND CPUS CLIPPER)
##################################################

if("CLIPPER" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/clipper/clipper.cpp
		${MAME_DIR}/src/devices/cpu/clipper/clipper.h
	)
endif()

if(("CLIPPER" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/clipper/clipperd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/clipper/clipperd.h)
endif()


##################################################
## VM Labs Nuon, disassembler only
##################################################

if(("NUON" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nuon/nuondasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/nuon/nuondasm.h)
endif()

##################################################
## DEC Alpha (EV4/EV5/EV6/EV7) series
##@src/devices/cpu/alpha/alpha.h,list(APPEND CPUS ALPHA)
##################################################

if("ALPHA" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/alpha/alpha.cpp
		${MAME_DIR}/src/devices/cpu/alpha/alpha.h
	)
endif()

if(("ALPHA" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/alpha/alphad.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/alpha/alphad.h)
endif()

##################################################
## National Semiconductor HPC
##@src/devices/cpu/hpc/hpc.h,list(APPEND CPUS HPC)
##################################################

if("HPC" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/hpc/hpc.cpp
		${MAME_DIR}/src/devices/cpu/hpc/hpc.h
	)
endif()

if(("HPC" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hpc/hpcdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/hpc/hpcdasm.h)
endif()

##################################################
## Yamaha Multiple Effects Generator
##@src/devices/sound/meg.h,list(APPEND CPUS MEG)
##################################################

if("MEG" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/sound/meg.cpp
		${MAME_DIR}/src/devices/sound/meg.h
	)
endif()

if(("MEG" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/sound/megd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/sound/megd.h)
endif()

##################################################
## Yamaha DSPV
##@src/devices/sound/dspv.h,list(APPEND CPUS DSPV)
##################################################

if("DSPV" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/sound/dspv.cpp
		${MAME_DIR}/src/devices/sound/dspv.h
	)
endif()

if(("DSPV" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/sound/dspvd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/sound/dspvd.h)
endif()

##################################################
##  National Semiconductor NS32000 series
##@src/devices/cpu/ns32000/ns32000.h,list(APPEND CPUS NS32000)
##################################################

if("NS32000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ns32000/ns32000.cpp
		${MAME_DIR}/src/devices/cpu/ns32000/ns32000.h
		${MAME_DIR}/src/devices/cpu/ns32000/slave.h
	)
endif()

if(("NS32000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ns32000/ns32000dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ns32000/ns32000dasm.h)
endif()

##################################################
## Elan RISC II series
##@src/devices/cpu/rii/riscii.h,list(APPEND CPUS RII)
##################################################

if("RII" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/rii/riscii.cpp
		${MAME_DIR}/src/devices/cpu/rii/riscii.h
	)
endif()

if(("RII" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rii/riidasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rii/riidasm.h)
endif()

##################################################
## National Semiconductor BCP
##@src/devices/cpu/bcp/dp8344.h,list(APPEND CPUS BCP)
##################################################

if("BCP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/bcp/dp8344.cpp
		${MAME_DIR}/src/devices/cpu/bcp/dp8344.h
	)
endif()

if(("BCP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/bcp/bcpdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/bcp/bcpdasm.h)
endif()

##################################################
## Fujitsu F2MC-16 series
##@src/devices/cpu/f2mc16/f2mc16.h,list(APPEND CPUS F2MC16)
##################################################

if("F2MC16" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/f2mc16/f2mc16.cpp
		${MAME_DIR}/src/devices/cpu/f2mc16/f2mc16.h
		${MAME_DIR}/src/devices/cpu/f2mc16/mb9061x.cpp
		${MAME_DIR}/src/devices/cpu/f2mc16/mb9061x.h
	)
endif()

if(("F2MC16" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/f2mc16/f2mc16d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/f2mc16/f2mc16d.h)
endif()

##################################################
## National Semiconductor CR16B
##@src/devices/cpu/cr16b/cr16b.h,list(APPEND CPUS CR16B)
##################################################

if("CR16B" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/cr16b/cr16b.cpp
		${MAME_DIR}/src/devices/cpu/cr16b/cr16b.h
	)
endif()

if(("CR16B" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cr16b/cr16bdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cr16b/cr16bdasm.h)
endif()

##################################################
## National Semiconductor CR16C, disassembler only
##################################################

if(("CR16C" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cr16c/cr16cdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/cr16c/cr16cdasm.h)
endif()

##################################################
## Gigatron
##@src/devices/cpu/gigatron/gigatron.h,list(APPEND CPUS GTRON)
##################################################

if("GTRON" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/gigatron/gigatron.cpp
		${MAME_DIR}/src/devices/cpu/gigatron/gigatron.h
	)
endif()

if(("GTRON" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/gigatron/gigatrondasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/gigatron/gigatrondasm.h)
endif()

##################################################
## Motorola DSP56000
##@src/devices/cpu/dsp56000/dsp56000.h,list(APPEND CPUS DSP56000)
##################################################

if("DSP56000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/dsp56000/dsp56000.cpp
		${MAME_DIR}/src/devices/cpu/dsp56000/dsp56000.h
	)
endif()

if(("DSP56000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56000/dsp56000d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/dsp56000/dsp56000d.h)
endif()

##################################################
## DEC VT50/VT52
##@src/devices/cpu/vt50/vt50.h,list(APPEND CPUS VT50)
##################################################

if("VT50" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/vt50/vt50.cpp
		${MAME_DIR}/src/devices/cpu/vt50/vt50.h
	)
endif()

if(("VT50" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/vt50/vt50dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/vt50/vt50dasm.h)
endif()

##################################################
## DEC VT61
##@src/devices/cpu/vt61/vt61.h,list(APPEND CPUS VT61)
##################################################

if("VT61" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/vt61/vt61.cpp
		${MAME_DIR}/src/devices/cpu/vt61/vt61.h
	)
endif()

if(("VT61" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/vt61/vt61dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/vt61/vt61dasm.h)
endif()

##################################################
## National Semiconductor PACE/INS8900
##@src/devices/cpu/pace/pace.h,list(APPEND CPUS PACE)
##################################################

if("PACE" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/pace/pace.cpp
		${MAME_DIR}/src/devices/cpu/pace/pace.h
	)
endif()

if(("PACE" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pace/pacedasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/pace/pacedasm.h)
endif()

##################################################
## AT&T WE32000/WE32100/WE32200
##@src/devices/cpu/we32000/we32100.h,list(APPEND CPUS WE32000)
##################################################

if("WE32000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/we32000/we32100.cpp
		${MAME_DIR}/src/devices/cpu/we32000/we32100.h
	)
endif()

if(("WE32000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/we32000/we32100d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/we32000/we32100d.h)
endif()

##################################################
## DEC RX01
##@src/devices/cpu/rx01/rx01.h,list(APPEND CPUS RX01)
##################################################

if("RX01" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/rx01/rx01.cpp
		${MAME_DIR}/src/devices/cpu/rx01/rx01.h
	)
endif()

if(("RX01" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rx01/rx01dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/rx01/rx01dasm.h)
endif()

##################################################
## Motorola M88000
##@src/devices/cpu/m88000/m88000.h,list(APPEND CPUS M88000)
##################################################

if("M88000" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/m88000/m88000.cpp
		${MAME_DIR}/src/devices/cpu/m88000/m88000.h
	)
endif()

if(("M88000" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m88000/m88000d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m88000/m88000d.h)
endif()

##################################################
## XAVIX2
##@src/devices/cpu/xavix2/xavix2.h,list(APPEND CPUS XAVIX2)
##################################################

if("XAVIX2" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/xavix2/xavix2.cpp
		${MAME_DIR}/src/devices/cpu/xavix2/xavix2.h
	)
endif()

if(("XAVIX2" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/xavix2/xavix2d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/xavix2/xavix2d.h)
endif()

##################################################
## NEC 78K
##@src/devices/cpu/upd78k/upd78k0.h,list(APPEND CPUS UPD78K)
##@src/devices/cpu/upd78k/upd78k2.h,list(APPEND CPUS UPD78K)
##@src/devices/cpu/upd78k/upd78k3.h,list(APPEND CPUS UPD78K)
##################################################

if("UPD78K" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k0.cpp
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k0.h
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k2.cpp
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k2.h
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k3.cpp
		${MAME_DIR}/src/devices/cpu/upd78k/upd78k3.h
	)
endif()

if(("UPD78K" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78kd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78kd.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k0d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k0d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k1d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k1d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k2d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k2d.h)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k3d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd78k/upd78k3d.h)
endif()

##################################################
## IBM ROMP
##@src/devices/cpu/romp/romp.h,list(APPEND CPUS ROMP)
##################################################

if("ROMP" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/romp/romp.cpp
		${MAME_DIR}/src/devices/cpu/romp/romp.h
		${MAME_DIR}/src/devices/cpu/romp/rsc.h
	)
endif()

if(("ROMP" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/romp/rompdasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/romp/rompdasm.h)
endif()

##################################################
## KS0164
##@src/devices/cpu/ks0164/ks0164.h,list(APPEND CPUS KS0164)
##################################################

if("KS0164" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/ks0164/ks0164.cpp
		${MAME_DIR}/src/devices/cpu/ks0164/ks0164.h
	)
endif()

if(("KS0164" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ks0164/ks0164d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/ks0164/ks0164d.h)
endif()

##################################################
## uPD177x - Disassembler only
##@src/devices/cpu/upd177x/upd177x.h,list(APPEND CPUS UPD177X)
##################################################

if(("UPD177X" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd177x/upd177xd.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/upd177x/upd177xd.h)
endif()

##################################################
## Sanyo LC58 - Disassembler only
##@src/devices/cpu/lc58/lc58.h,list(APPEND CPUS LC58)
##################################################

if(("LC58" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc58/lc58d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc58/lc58d.h)
endif()

##################################################
## OKI MSM6502/6512 - Disassembler only
##@src/devices/cpu/msm65x2/msm65x2.h,list(APPEND CPUS MSM65X2)
##################################################

if(("MSM65X2" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/msm65x2/msm65x2d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/msm65x2/msm65x2d.h)
endif()

##################################################
## Sanyo LC57 - Disassembler only
##@src/devices/cpu/lc57/lc57.h,list(APPEND CPUS LC57)
##################################################

if(("LC57" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc57/lc57d.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/lc57/lc57d.h)
endif()

##################################################
## Mark I (Andrew Holme)
##@src/devices/cpu/mk1/mk1.h,list(APPEND CPUS MK1)
##################################################

if("MK1" IN_LIST CPUS)
	list(APPEND CPU_SRCS
		${MAME_DIR}/src/devices/cpu/mk1/mk1.cpp
		${MAME_DIR}/src/devices/cpu/mk1/mk1.h
	)
endif()

if(("MK1" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mk1/mk1dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/mk1/mk1dasm.h)
endif()

##################################################
## Motorola M68HC16 (CPU16) - Disassembler only
##@src/devices/cpu/m68hc16/m68hc16.h,list(APPEND CPUS M68HC16)
##################################################

if(("M68HC16" IN_LIST CPUS) OR TOOLS)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m68hc16/cpu16dasm.cpp)
	list(APPEND DASM_SRCS ${MAME_DIR}/src/devices/cpu/m68hc16/cpu16dasm.h)
endif()
