###########################################################################
#
#   cpu.mak
#
#   Rules for building CPU cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


CPUSRC = $(EMUSRC)/cpu
CPUOBJ = $(EMUOBJ)/cpu


#-------------------------------------------------
# Dynamic recompiler objects
#-------------------------------------------------

ifdef PTR64

DRCOBJ = $(CPUOBJ)/x64drc.o $(CPUOBJ)/x86log.o $(CPUOBJ)/drcfe.o

DRCDEPS = 	$(CPUSRC)/x86emit.h \
			$(CPUSRC)/x64drc.c \
			$(CPUSRC)/x64drc.h \

$(DRCOBJ): $(DRCDEPS)

else

DRCOBJ = $(CPUOBJ)/x86drc.o $(CPUOBJ)/x86log.o $(CPUOBJ)/drcfe.o

DRCDEPS = 	$(CPUSRC)/x86emit.h \
			$(CPUSRC)/x86drc.c \
			$(CPUSRC)/x86drc.h \

$(DRCOBJ): $(DRCDEPS)

endif


#-------------------------------------------------
# Acorn ARM series
#-------------------------------------------------

CPUDEFS += -DHAS_ARM=$(if $(filter ARM,$(CPUS)),1,0)
CPUDEFS += -DHAS_ARM7=$(if $(filter ARM7,$(CPUS)),1,0)

ifneq ($(filter ARM,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/arm
CPUOBJS += $(CPUOBJ)/arm/arm.o
DBGOBJS += $(CPUOBJ)/arm/armdasm.o
endif

ifneq ($(filter ARM7,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/arm7
CPUOBJS += $(CPUOBJ)/arm7/arm7.o
DBGOBJS += $(CPUOBJ)/arm7/arm7dasm.o
endif

$(CPUOBJ)/arm/arm.o: 	$(CPUSRC)/arm/arm.c \
						$(CPUSRC)/arm/arm.h

$(CPUOBJ)/arm7/arm7.o:	$(CPUSRC)/arm7/arm7.c \
						$(CPUSRC)/arm7/arm7.h \
						$(CPUSRC)/arm7/arm7exec.c \
						$(CPUSRC)/arm7/arm7core.c



#-------------------------------------------------
# Advanced Digital Chips SE3208
#-------------------------------------------------

CPUDEFS += -DHAS_SE3208=$(if $(filter SE3208,$(CPUS)),1,0)

ifneq ($(filter SE3208,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/se3208
CPUOBJS += $(CPUOBJ)/se3208/se3208.o
DBGOBJS += $(CPUOBJ)/se3208/se3208dis.o
endif

$(CPUOBJ)/se3208/se3208.o: 	$(CPUSRC)/se3208/se3208.c \
							$(CPUSRC)/se3208/se3208.h



#-------------------------------------------------
# Alpha 8201
#-------------------------------------------------

CPUDEFS += -DHAS_ALPHA8201=$(if $(filter ALPHA8201,$(CPUS)),1,0)
CPUDEFS += -DHAS_ALPHA8301=$(if $(filter ALPHA8301,$(CPUS)),1,0)

ifneq ($(filter ALPHA8201 ALPHA8301,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/alph8201
CPUOBJS += $(CPUOBJ)/alph8201/alph8201.o
DBGOBJS += $(CPUOBJ)/alph8201/8201dasm.o
endif

$(CPUOBJ)/alph8201/alph8201.o:	$(CPUSRC)/alph8201/alph8201.c \
								$(CPUSRC)/alph8201/alph8201.h



#-------------------------------------------------
# Analog Devices ADSP21xx series
#-------------------------------------------------

CPUDEFS += -DHAS_ADSP2100=$(if $(filter ADSP2100,$(CPUS)),1,0)
CPUDEFS += -DHAS_ADSP2101=$(if $(filter ADSP2101,$(CPUS)),1,0)
CPUDEFS += -DHAS_ADSP2104=$(if $(filter ADSP2104,$(CPUS)),1,0)
CPUDEFS += -DHAS_ADSP2105=$(if $(filter ADSP2105,$(CPUS)),1,0)
CPUDEFS += -DHAS_ADSP2115=$(if $(filter ADSP2115,$(CPUS)),1,0)
CPUDEFS += -DHAS_ADSP2181=$(if $(filter ADSP2181,$(CPUS)),1,0)

ifneq ($(filter ADSP2100 ADSP2101 ADSP2104 ADSP2105 ADSP2115 ADSP2181,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/adsp2100
CPUOBJS += $(CPUOBJ)/adsp2100/adsp2100.o
DBGOBJS += $(CPUOBJ)/adsp2100/2100dasm.o
endif

$(CPUOBJ)/adsp2100/adsp2100.o:	$(CPUSRC)/adsp2100/adsp2100.c \
								$(CPUSRC)/adsp2100/adsp2100.h \
								$(CPUSRC)/adsp2100/2100ops.c



#-------------------------------------------------
# Analog Devices "Sharc" ADSP21062
#-------------------------------------------------

CPUDEFS += -DHAS_ADSP21062=$(if $(filter ADSP21062,$(CPUS)),1,0)

ifneq ($(filter ADSP21062,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sharc
CPUOBJS += $(CPUOBJ)/sharc/sharc.o
DBGOBJS += $(CPUOBJ)/sharc/sharcdsm.o
endif

$(CPUOBJ)/sharc/sharc.o:	$(CPUSRC)/sharc/sharc.c \
							$(CPUSRC)/sharc/sharc.h \
							$(CPUSRC)/sharc/sharcops.c \
							$(CPUSRC)/sharc/sharcops.h \
							$(CPUSRC)/sharc/sharcdsm.c \
							$(CPUSRC)/sharc/sharcdsm.h \
							$(CPUSRC)/sharc/compute.c \
							$(CPUSRC)/sharc/sharcdma.c \
							$(CPUSRC)/sharc/sharcmem.c



#-------------------------------------------------
# APEXC
#-------------------------------------------------

CPUDEFS += -DHAS_APEXC=$(if $(filter APEXC,$(CPUS)),1,0)

ifneq ($(filter APEXC,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/apexc
CPUOBJS += $(CPUOBJ)/apexc/apexc.o
DBGOBJS += $(CPUOBJ)/apexc/apexcdsm.o
endif

$(CPUOBJ)/apexc/apexc.o:	$(CPUSRC)/apexc/apexc.c \
							$(CPUSRC)/apexc/apexc.h



#-------------------------------------------------
# AT&T DSP32C
#-------------------------------------------------

CPUDEFS += -DHAS_DSP32C=$(if $(filter DSP32C,$(CPUS)),1,0)

ifneq ($(filter DSP32C,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/dsp32
CPUOBJS += $(CPUOBJ)/dsp32/dsp32.o
DBGOBJS += $(CPUOBJ)/dsp32/dsp32dis.o
endif

$(CPUOBJ)/dsp32/dsp32.o: 	$(CPUSRC)/dsp32/dsp32.c \
							$(CPUSRC)/dsp32/dsp32.h



#-------------------------------------------------
# Atari custom RISC processor
#-------------------------------------------------

CPUDEFS += -DHAS_ASAP=$(if $(filter ASAP,$(CPUS)),1,0)

ifneq ($(filter ASAP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/asap
CPUOBJS += $(CPUOBJ)/asap/asap.o
DBGOBJS += $(CPUOBJ)/asap/asapdasm.o
endif

$(CPUOBJ)/asap/asap.o:	$(CPUSRC)/asap/asap.c \
						$(CPUSRC)/asap/asap.h



#-------------------------------------------------
# Atari Jaguar custom DSPs
#-------------------------------------------------

CPUDEFS += -DHAS_JAGUAR=$(if $(filter JAGUAR,$(CPUS)),1,0)

ifneq ($(filter JAGUAR,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/jaguar
CPUOBJS += $(CPUOBJ)/jaguar/jaguar.o
DBGOBJS += $(CPUOBJ)/jaguar/jagdasm.o
endif

$(CPUOBJ)/jaguar/jaguar.o:	$(CPUSRC)/jaguar/jaguar.c \
							$(CPUSRC)/jaguar/jaguar.h



#-------------------------------------------------
# RCA CDP1802
#-------------------------------------------------

CPUDEFS += -DHAS_CDP1802=$(if $(filter CDP1802,$(CPUS)),1,0)

ifneq ($(filter CDP1802,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cdp1802
CPUOBJS += $(CPUOBJ)/cdp1802/cdp1802.o
DBGOBJS += $(CPUOBJ)/cdp1802/1802dasm.o
endif

$(CPUOBJ)/cdp1802/cdp1802.o:	$(CPUSRC)/cdp1802/cdp1802.c \
								$(CPUSRC)/cdp1802/cdp1802.h



#-------------------------------------------------
# National Semiconductor COP4xx
#-------------------------------------------------

CPUDEFS += -DHAS_COP410=$(if $(filter COP410,$(CPUS)),1,0)
CPUDEFS += -DHAS_COP411=$(if $(filter COP411,$(CPUS)),1,0)
CPUDEFS += -DHAS_COP420=$(if $(filter COP420,$(CPUS)),1,0)

ifneq ($(filter COP410 COP411,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cop400
CPUOBJS += $(CPUOBJ)/cop400/cop410.o
DBGOBJS += $(CPUOBJ)/cop400/cop410ds.o
endif

ifneq ($(filter COP420,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cop400
CPUOBJS += $(CPUOBJ)/cop400/cop420.o
DBGOBJS += $(CPUOBJ)/cop400/cop420ds.o
endif

$(CPUOBJ)/cop400/cop410.o:	$(CPUSRC)/cop400/cop410.c \
							$(CPUSRC)/cop400/cop400.h \
							$(CPUSRC)/cop400/410ops.c

$(CPUOBJ)/cop400/cop420.o:	$(CPUSRC)/cop400/cop420.c \
							$(CPUSRC)/cop400/cop400.h \
							$(CPUSRC)/cop400/410ops.c \
							$(CPUSRC)/cop400/420ops.c



#-------------------------------------------------
# CP1610
#-------------------------------------------------

CPUDEFS += -DHAS_CP1610=$(if $(filter CP1610,$(CPUS)),1,0)

ifneq ($(filter CP1610,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cp1610
CPUOBJS += $(CPUOBJ)/cp1610/cp1610.o
DBGOBJS += $(CPUOBJ)/cp1610/1610dasm.o
endif

$(CPUOBJ)/cp1610/cp1610.o:	$(CPUSRC)/cp1610/cp1610.c \
							$(CPUSRC)/cp1610/cp1610.h



#-------------------------------------------------
# Cinematronics vector "CPU"
#-------------------------------------------------

CPUDEFS += -DHAS_CCPU=$(if $(filter CCPU,$(CPUS)),1,0)

ifneq ($(filter CCPU,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/ccpu
CPUOBJS += $(CPUOBJ)/ccpu/ccpu.o
DBGOBJS += $(CPUOBJ)/ccpu/ccpudasm.o
endif

$(CPUOBJ)/ccpu/ccpu.o:	$(CPUSRC)/ccpu/ccpu.c \
						$(CPUSRC)/ccpu/ccpu.h



#-------------------------------------------------
# DEC T-11
#-------------------------------------------------

CPUDEFS += -DHAS_T11=$(if $(filter T11,$(CPUS)),1,0)

ifneq ($(filter T11,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/t11
CPUOBJS += $(CPUOBJ)/t11/t11.o
DBGOBJS += $(CPUOBJ)/t11/t11dasm.o
endif

$(CPUOBJ)/t11/t11.o:	$(CPUSRC)/t11/t11.c \
						$(CPUSRC)/t11/t11.h \
						$(CPUSRC)/t11/t11ops.c \
						$(CPUSRC)/t11/t11table.c



#-------------------------------------------------
# F8
#-------------------------------------------------

CPUDEFS += -DHAS_F8=$(if $(filter F8,$(CPUS)),1,0)

ifneq ($(filter F8,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/f8
CPUOBJS += $(CPUOBJ)/f8/f8.o
DBGOBJS += $(CPUOBJ)/f8/f8dasm.o
endif

$(CPUOBJ)/f8/f8.o:	$(CPUSRC)/f8/f8.c \
					$(CPUSRC)/f8/f8.h



#-------------------------------------------------
# G65816
#-------------------------------------------------

CPUDEFS += -DHAS_G65816=$(if $(filter G65816,$(CPUS)),1,0)

ifneq ($(filter G65816,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/g65816
CPUOBJS += \
	$(CPUOBJ)/g65816/g65816.o \
	$(CPUOBJ)/g65816/g65816o0.o \
	$(CPUOBJ)/g65816/g65816o1.o \
	$(CPUOBJ)/g65816/g65816o2.o \
	$(CPUOBJ)/g65816/g65816o3.o \
	$(CPUOBJ)/g65816/g65816o4.o
DBGOBJS += $(CPUOBJ)/g65816/g65816ds.o
endif

G65816DEPS = \
	$(CPUSRC)/g65816/g65816.h \
	$(CPUSRC)/g65816/g65816cm.h \
	$(CPUSRC)/g65816/g65816op.h

$(CPUOBJ)/g65816/g65816.o:		$(CPUSRC)/g65816/g65816.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o0.o: 	$(CPUSRC)/g65816/g65816o0.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o1.o:	$(CPUSRC)/g65816/g65816o1.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o2.o:	$(CPUSRC)/g65816/g65816o2.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o3.o:	$(CPUSRC)/g65816/g65816o3.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o4.o:	$(CPUSRC)/g65816/g65816o4.c \
								$(G65816DEPS)



#-------------------------------------------------
# Hitachi 6309
#-------------------------------------------------

CPUDEFS += -DHAS_HD6309=$(if $(filter HD6309,$(CPUS)),1,0)

ifneq ($(filter HD6309,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/hd6309
CPUOBJS += $(CPUOBJ)/hd6309/hd6309.o
DBGOBJS += $(CPUOBJ)/hd6309/6309dasm.o
endif

$(CPUOBJ)/hd6309/hd6309.o:	$(CPUSRC)/hd6309/hd6309.c \
							$(CPUSRC)/hd6309/hd6309.h \
							$(CPUSRC)/hd6309/6309ops.c \
							$(CPUSRC)/hd6309/6309tbl.c



#-------------------------------------------------
# Hitachi H8/3002
#-------------------------------------------------

CPUDEFS += -DHAS_H83002=$(if $(filter H83002,$(CPUS)),1,0)

ifneq ($(filter H83002,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/h83002
CPUOBJS += $(CPUOBJ)/h83002/h83002.o $(CPUOBJ)/h83002/h8periph.o
DBGOBJS += $(CPUOBJ)/h83002/h8disasm.o
endif

$(CPUOBJ)/h83002/h83002.o:		$(CPUSRC)/h83002/h83002.c \
								$(CPUSRC)/h83002/h83002.h \
								$(CPUSRC)/h83002/h8priv.h

$(CPUOBJ)/h83002/h8disasm.o: 	$(CPUSRC)/h83002/h8disasm.c

$(CPUOBJ)/h83002/h8periph.o:	$(CPUSRC)/h83002/h8periph.c \
								$(CPUSRC)/h83002/h8priv.h



#-------------------------------------------------
# Hitachi SH2
#-------------------------------------------------

CPUDEFS += -DHAS_SH2=$(if $(filter SH2,$(CPUS)),1,0)

ifneq ($(filter SH2,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sh2
CPUOBJS += $(CPUOBJ)/sh2/sh2.o
DBGOBJS += $(CPUOBJ)/sh2/sh2dasm.o
endif

$(CPUOBJ)/sh2/sh2.o:	$(CPUSRC)/sh2/sh2.c \
						$(CPUSRC)/sh2/sh2.h

#-------------------------------------------------
# Hitachi SH4
#-------------------------------------------------

CPUDEFS += -DHAS_SH4=$(if $(filter SH4,$(CPUS)),1,0)

ifneq ($(filter SH4,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sh4
CPUOBJS += $(CPUOBJ)/sh4/sh4.o
DBGOBJS += $(CPUOBJ)/sh4/sh4dasm.o
endif

$(CPUOBJ)/sh4/sh4.o:	$(CPUSRC)/sh4/sh4.c \
						$(CPUSRC)/sh4/sh4.h

#-------------------------------------------------
# Hudsonsoft 6280
#-------------------------------------------------

CPUDEFS += -DHAS_H6280=$(if $(filter H6280,$(CPUS)),1,0)

ifneq ($(filter H6280,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/h6280
CPUOBJS += $(CPUOBJ)/h6280/h6280.o
DBGOBJS += $(CPUOBJ)/h6280/6280dasm.o
endif

$(CPUOBJ)/h6280/h6280.o:	$(CPUSRC)/h6280/h6280.c \
							$(CPUSRC)/h6280/h6280.h \
							$(CPUSRC)/h6280/h6280ops.h \
							$(CPUSRC)/h6280/tblh6280.c



#-------------------------------------------------
# Hyperstone E1 series
#-------------------------------------------------

CPUDEFS += -DHAS_E116T=$(if $(filter E116T,$(CPUS)),1,0)
CPUDEFS += -DHAS_E116XT=$(if $(filter E116XT,$(CPUS)),1,0)
CPUDEFS += -DHAS_E116XS=$(if $(filter E116XS,$(CPUS)),1,0)
CPUDEFS += -DHAS_E116XSR=$(if $(filter E116XSR,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132N=$(if $(filter E132N,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132T=$(if $(filter E132T,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132XN=$(if $(filter E132XN,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132XT=$(if $(filter E132XT,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132XS=$(if $(filter E132XS,$(CPUS)),1,0)
CPUDEFS += -DHAS_E132XSR=$(if $(filter E132XSR,$(CPUS)),1,0)
CPUDEFS += -DHAS_GMS30C2116=$(if $(filter GMS30C2116,$(CPUS)),1,0)
CPUDEFS += -DHAS_GMS30C2132=$(if $(filter GMS30C2132,$(CPUS)),1,0)
CPUDEFS += -DHAS_GMS30C2216=$(if $(filter GMS30C2216,$(CPUS)),1,0)
CPUDEFS += -DHAS_GMS30C2232=$(if $(filter GMS30C2232,$(CPUS)),1,0)

ifneq ($(filter E116T E116XT E116XS E116XSR E132N E132T E132XN E132XT E132XS E132XSR GMS30C2116 GMS30C2132 GMS30C2216 GMS30C2232,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/e132xs
CPUOBJS += $(CPUOBJ)/e132xs/e132xs.o
DBGOBJS += $(CPUOBJ)/e132xs/32xsdasm.o
endif

$(CPUOBJ)/e132xs/e132xs.o:	$(CPUSRC)/e132xs/e132xs.c \
							$(CPUSRC)/e132xs/e132xs.h \
							$(CPUSRC)/e132xs/e132xsop.c



#-------------------------------------------------
# Intel 8080/8085A
#-------------------------------------------------

CPUDEFS += -DHAS_8080=$(if $(filter 8080,$(CPUS)),1,0)
CPUDEFS += -DHAS_8085A=$(if $(filter 8085A,$(CPUS)),1,0)

ifneq ($(filter 8080 8085A,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8085
CPUOBJS += $(CPUOBJ)/i8085/i8085.o
DBGOBJS += $(CPUOBJ)/i8085/8085dasm.o
endif

$(CPUOBJ)/i8085/i8085.o:	$(CPUSRC)/i8085/i8085.c \
							$(CPUSRC)/i8085/i8085.h \
							$(CPUSRC)/i8085/i8085cpu.h \
							$(CPUSRC)/i8085/i8085daa.h



#-------------------------------------------------
# Intel MCS-48 (8039 and derivatives)
#-------------------------------------------------

CPUDEFS += -DHAS_I8035=$(if $(filter I8035,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8039=$(if $(filter I8039,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8048=$(if $(filter I8048,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8749=$(if $(filter I8749,$(CPUS)),1,0)
CPUDEFS += -DHAS_N7751=$(if $(filter N7751,$(CPUS)),1,0)
CPUDEFS += -DHAS_MB8884=$(if $(filter MB8884,$(CPUS)),1,0)
CPUDEFS += -DHAS_M58715=$(if $(filter M58715,$(CPUS)),1,0)

ifneq ($(filter I8035 I8039 I8048 I8749 N7751 MB8884 M58715,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8039
CPUOBJS += $(CPUOBJ)/i8039/i8039.o
DBGOBJS += $(CPUOBJ)/i8039/8039dasm.o
endif

$(CPUOBJ)/i8039/i8039.o:	$(CPUSRC)/i8039/i8039.c \
							$(CPUSRC)/i8039/i8039.h



#-------------------------------------------------
# Intel 8x41
#-------------------------------------------------

CPUDEFS += -DHAS_I8X41=$(if $(filter I8X41,$(CPUS)),1,0)

ifneq ($(filter I8X41,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8x41
CPUOBJS += $(CPUOBJ)/i8x41/i8x41.o
DBGOBJS += $(CPUOBJ)/i8x41/8x41dasm.o
endif

$(CPUOBJ)/i8x41/i8x41.o:	$(CPUSRC)/i8x41/i8x41.c \
							$(CPUSRC)/i8x41/i8x41.h



#-------------------------------------------------
# Intel 8051 and derivatives
#-------------------------------------------------

CPUDEFS += -DHAS_I8051=$(if $(filter I8051,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8052=$(if $(filter I8052,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8751=$(if $(filter I8751,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8752=$(if $(filter I8752,$(CPUS)),1,0)

ifneq ($(filter I8051 I8052 I8751 I8752,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8051
CPUOBJS += $(CPUOBJ)/i8051/i8051.o
DBGOBJS += $(CPUOBJ)/i8051/8051dasm.o
endif

$(CPUOBJ)/i8051/i8051.o:	$(CPUSRC)/i8051/i8051.c \
							$(CPUSRC)/i8051/i8051.h \
							$(CPUSRC)/i8051/i8051ops.c



#-------------------------------------------------
# Intel 80x86 series
#-------------------------------------------------

CPUDEFS += -DHAS_I8086=$(if $(filter I8086,$(CPUS)),1,0)
CPUDEFS += -DHAS_I8088=$(if $(filter I8088,$(CPUS)),1,0)
CPUDEFS += -DHAS_I80186=$(if $(filter I80186,$(CPUS)),1,0)
CPUDEFS += -DHAS_I80188=$(if $(filter I80188,$(CPUS)),1,0)
CPUDEFS += -DHAS_I80286=$(if $(filter I80286,$(CPUS)),1,0)
CPUDEFS += -DHAS_I386=$(if $(filter I386,$(CPUS)),1,0)
CPUDEFS += -DHAS_I486=$(if $(filter I486,$(CPUS)),1,0)
CPUDEFS += -DHAS_PENTIUM=$(if $(filter PENTIUM,$(CPUS)),1,0)
CPUDEFS += -DHAS_MEDIAGX=$(if $(filter MEDIAGX,$(CPUS)),1,0)

ifneq ($(filter I8086 I8088 I80186 I80188,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i86 $(CPUOBJ)/i386
CPUOBJS += $(CPUOBJ)/i86/i86.o
DBGOBJS += $(CPUOBJ)/i386/i386dasm.o
endif


ifneq ($(filter I80286,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i86 $(CPUOBJ)/i386
CPUOBJS += $(CPUOBJ)/i86/i286.o
DBGOBJS += $(CPUOBJ)/i386/i386dasm.o
endif


ifneq ($(filter I386 I486 PENTIUM MEDIAGX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i386
CPUOBJS += $(CPUOBJ)/i386/i386.o
DBGOBJS += $(CPUOBJ)/i386/i386dasm.o
endif

I86DEPS = \
	$(CPUSRC)/i86/i86.h \
	$(CPUSRC)/i86/ea.h \
	$(CPUSRC)/i86/host.h \
	$(CPUSRC)/i86/modrm.h

$(CPUOBJ)/i86/i86.o:	$(CPUSRC)/i86/i86.c \
						$(CPUSRC)/i86/instr86.c \
						$(CPUSRC)/i86/instr186.c \
						$(CPUSRC)/i86/i86intf.h \
						$(CPUSRC)/i86/i186intf.h \
						$(I86DEPS)

$(CPUOBJ)/i86/i286.o:	$(CPUSRC)/i86/i86.c \
						$(CPUSRC)/i86/instr286.c \
						$(CPUSRC)/i86/i286intf.h \
						$(I86DEPS)

$(CPUOBJ)/i386/i386.o:	$(CPUSRC)/i386/i386.c \
						$(CPUSRC)/i386/i386.h \
						$(CPUSRC)/i386/i386intf.h \
						$(CPUSRC)/i386/i386op16.c \
						$(CPUSRC)/i386/i386op32.c \
						$(CPUSRC)/i386/i386ops.c \
						$(CPUSRC)/i386/i486ops.c \
						$(CPUSRC)/i386/pentops.c \
						$(CPUSRC)/i386/x87ops.c \
						$(CPUSRC)/i386/i386ops.h \
						$(CPUSRC)/i386/cycles.h



#-------------------------------------------------
# Intel i960
#-------------------------------------------------

CPUDEFS += -DHAS_I960=$(if $(filter I960,$(CPUS)),1,0)

ifneq ($(filter I960,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i960
CPUOBJS += $(CPUOBJ)/i960/i960.o
DBGOBJS += $(CPUOBJ)/i960/i960dis.o
endif

$(CPUOBJ)/i960/i960.o:	$(CPUSRC)/i960/i960.c \
						$(CPUSRC)/i960/i960.h



#-------------------------------------------------
# Konami custom CPU (6809-based)
#-------------------------------------------------

CPUDEFS += -DHAS_KONAMI=$(if $(filter KONAMI,$(CPUS)),1,0)

ifneq ($(filter KONAMI,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/konami
CPUOBJS += $(CPUOBJ)/konami/konami.o
DBGOBJS += $(CPUOBJ)/konami/knmidasm.o
endif

$(CPUOBJ)/konami/konami.o:	$(CPUSRC)/konami/konami.c \
							$(CPUSRC)/konami/konami.h \
							$(CPUSRC)/konami/konamops.c \
							$(CPUSRC)/konami/konamtbl.c



#-------------------------------------------------
# LH5801
#-------------------------------------------------

CPUDEFS += -DHAS_LH5801=$(if $(filter LH5801,$(CPUS)),1,0)

ifneq ($(filter LH5801,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/lh5801
CPUOBJS += $(CPUOBJ)/lh5801/lh5801.o
DBGOBJS += $(CPUOBJ)/lh5801/5801dasm.o
endif

$(CPUOBJ)/lh5801/lh5801.o:	$(CPUSRC)/lh5801/lh5801.c \
							$(CPUSRC)/lh5801/5801tbl.c \
							$(CPUSRC)/lh5801/lh5801.h



#-------------------------------------------------
# Fujitsu MB88xx
#-------------------------------------------------

CPUDEFS += -DHAS_MB8841=$(if $(filter MB8841,$(CPUS)),1,0)
CPUDEFS += -DHAS_MB8842=$(if $(filter MB8842,$(CPUS)),1,0)
CPUDEFS += -DHAS_MB8843=$(if $(filter MB8843,$(CPUS)),1,0)
CPUDEFS += -DHAS_MB8844=$(if $(filter MB8844,$(CPUS)),1,0)

ifneq ($(filter MB8841 MB8842 MB8843 MB8844,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mb88xx
CPUOBJS += $(CPUOBJ)/mb88xx/mb88xx.o
DBGOBJS += $(CPUOBJ)/mb88xx/mb88dasm.o
endif

$(CPUOBJ)/mb88xx/mb88xx.o:	$(CPUSRC)/mb88xx/mb88xx.c \
							$(CPUSRC)/mb88xx/mb88xx.h



#-------------------------------------------------
# Fujitsu MB86233
#-------------------------------------------------

CPUDEFS += -DHAS_MB86233=$(if $(filter MB86233,$(CPUS)),1,0)

ifneq ($(filter MB86233,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mb86233
CPUOBJS += $(CPUOBJ)/mb86233/mb86233.o
DBGOBJS += $(CPUOBJ)/mb86233/mb86233d.o
endif

$(CPUOBJ)/mb86233/mb86233.o:	$(CPUSRC)/mb86233/mb86233.c \
								$(CPUSRC)/mb86233/mb86233.h



#-------------------------------------------------
# Microchip PIC16C5x
#-------------------------------------------------

CPUDEFS += -DHAS_PIC16C54=$(if $(filter PIC16C54,$(CPUS)),1,0)
CPUDEFS += -DHAS_PIC16C55=$(if $(filter PIC16C55,$(CPUS)),1,0)
CPUDEFS += -DHAS_PIC16C56=$(if $(filter PIC16C56,$(CPUS)),1,0)
CPUDEFS += -DHAS_PIC16C57=$(if $(filter PIC16C57,$(CPUS)),1,0)
CPUDEFS += -DHAS_PIC16C58=$(if $(filter PIC16C58,$(CPUS)),1,0)

ifneq ($(filter PIC16C54 PIC16C55 PIC16C56 PIC16C57 PIC16C58,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pic16c5x
CPUOBJS += $(CPUOBJ)/pic16c5x/pic16c5x.o
DBGOBJS += $(CPUOBJ)/pic16c5x/16c5xdsm.o
endif

$(CPUOBJ)/pic16c5x/pic16c5x.o:	$(CPUSRC)/pic16c5x/pic16c5x.c \
								$(CPUSRC)/pic16c5x/pic16c5x.h



#-------------------------------------------------
# MIPS R3000 (MIPS I/II) series
#-------------------------------------------------

CPUDEFS += -DHAS_R3000=$(if $(filter R3000,$(CPUS)),1,0)
CPUDEFS += -DHAS_R3041=$(if $(filter R3041,$(CPUS)),1,0)

ifneq ($(filter R3000 R3041,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mips
CPUOBJS += $(CPUOBJ)/mips/r3000.o
DBGOBJS += $(CPUOBJ)/mips/r3kdasm.o
endif

$(CPUOBJ)/mips/r3000.o:	$(CPUSRC)/mips/r3000.c \
						$(CPUSRC)/mips/r3000.h



#-------------------------------------------------
# MIPS R4000 (MIPS III/IV) series
#-------------------------------------------------

CPUDEFS += -DHAS_R4600=$(if $(filter R4600,$(CPUS)),1,0)
CPUDEFS += -DHAS_R4650=$(if $(filter R4650,$(CPUS)),1,0)
CPUDEFS += -DHAS_R4700=$(if $(filter R4700,$(CPUS)),1,0)
CPUDEFS += -DHAS_R5000=$(if $(filter R5000,$(CPUS)),1,0)
CPUDEFS += -DHAS_QED5271=$(if $(filter QED5271,$(CPUS)),1,0)
CPUDEFS += -DHAS_RM7000=$(if $(filter RM7000,$(CPUS)),1,0)

ifneq ($(filter R4600 R4650 R4700 R5000 QED5271 RM7000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mips
CPUOBJS += $(CPUOBJ)/mips/mips3com.o
DBGOBJS += $(CPUOBJ)/mips/mips3dsm.o

ifdef X86_MIPS3_DRC
CPUOBJS += $(CPUOBJ)/mips/mips3drc.o $(CPUOBJ)/mips/mips3fe.o $(DRCOBJ)
else
CPUOBJS += $(CPUOBJ)/mips/mips3.o
endif
endif

$(CPUOBJ)/mips/mips3.o:		$(CPUSRC)/mips/mips3.c \
							$(CPUSRC)/mips/mips3.h \
							$(CPUSRC)/mips/mips3com.h

$(CPUOBJ)/mips/mips3drc.o:	$(CPUSRC)/mips/mips3drc.c \
							$(CPUSRC)/mips/mdrcold.c \
							$(CPUSRC)/mips/mdrc64.c \
							$(CPUSRC)/mips/mips3.h \
							$(CPUSRC)/mips/mips3com.h \
							$(CPUSRC)/mips/mips3fe.h \
							$(DRCDEPS)



#-------------------------------------------------
# Mitsubishi M37702 and M37710 (based on 65C816)
#-------------------------------------------------

CPUDEFS += -DHAS_M37702=$(if $(filter M37702,$(CPUS)),1,0)
CPUDEFS += -DHAS_M37710=$(if $(filter M37710,$(CPUS)),1,0)

ifneq ($(filter M37702 M37710,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m37710
CPUOBJS += \
	$(CPUOBJ)/m37710/m37710.o \
	$(CPUOBJ)/m37710/m37710o0.o \
	$(CPUOBJ)/m37710/m37710o1.o \
	$(CPUOBJ)/m37710/m37710o2.o \
	$(CPUOBJ)/m37710/m37710o3.o
DBGOBJS += $(CPUOBJ)/m37710/m7700ds.o
endif

M37710DEPS = \
	$(CPUSRC)/m37710/m37710.h \
	$(CPUSRC)/m37710/m37710op.h \
	$(CPUSRC)/m37710/m7700ds.h

$(CPUOBJ)/m37710/m37710.o:		$(CPUSRC)/m37710/m37710.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o0.o:	$(CPUSRC)/m37710/m37710o0.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o1.o:	$(CPUSRC)/m37710/m37710o1.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o2.o:	$(CPUSRC)/m37710/m37710o2.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o3.o:	$(CPUSRC)/m37710/m37710o3.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m7700ds.o:		$(CPUSRC)/m37710/m7700ds.c \
								$(CPUSRC)/m37710/m7700ds.h



#-------------------------------------------------
# Mostek 6502 and its many derivatives
#-------------------------------------------------

CPUDEFS += -DHAS_M6502=$(if $(filter M6502,$(CPUS)),1,0)
CPUDEFS += -DHAS_M65C02=$(if $(filter M65C02,$(CPUS)),1,0)
CPUDEFS += -DHAS_M65SC02=$(if $(filter M65SC02,$(CPUS)),1,0)
CPUDEFS += -DHAS_M65CE02=$(if $(filter M65CE02,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6509=$(if $(filter M6509,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6510=$(if $(filter M6510 M6510T M7501 M8502,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6510T=$(if $(filter M6510T,$(CPUS)),1,0)
CPUDEFS += -DHAS_M7501=$(if $(filter M7501,$(CPUS)),1,0)
CPUDEFS += -DHAS_M8502=$(if $(filter M8502,$(CPUS)),1,0)
CPUDEFS += -DHAS_N2A03=$(if $(filter N2A03,$(CPUS)),1,0)
CPUDEFS += -DHAS_DECO16=$(if $(filter DECO16,$(CPUS)),1,0)
CPUDEFS += -DHAS_M4510=$(if $(filter M4510,$(CPUS)),1,0)

ifneq ($(filter M6502 M65C02 M65SC02 M6510 M6510T M7501 M8502 N2A03 DECO16,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6502
CPUOBJS += $(CPUOBJ)/m6502/m6502.o
DBGOBJS += $(CPUOBJ)/m6502/6502dasm.o
endif

ifneq ($(filter M65CE02,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6502
CPUOBJS += $(CPUOBJ)/m6502/m65ce02.o
DBGOBJS += $(CPUOBJ)/m6502/6502dasm.o
endif

ifneq ($(filter M6509,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6502
CPUOBJS += $(CPUOBJ)/m6502/m6509.o
DBGOBJS += $(CPUOBJ)/m6502/6502dasm.o
endif

ifneq ($(filter M4510,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6502
CPUOBJS += $(CPUOBJ)/m6502/m4510.o
DBGOBJS += $(CPUOBJ)/m6502/6502dasm.o
endif

$(CPUOBJ)/m6502/m6502.o:	$(CPUSRC)/m6502/m6502.c \
							$(CPUSRC)/m6502/m6502.h \
							$(CPUSRC)/m6502/ops02.h \
							$(CPUSRC)/m6502/t6502.c \
							$(CPUSRC)/m6502/t65c02.c \
							$(CPUSRC)/m6502/t65sc02.c \
							$(CPUSRC)/m6502/t6510.c \
							$(CPUSRC)/m6502/tdeco16.c

$(CPUOBJ)/m6502/m65ce02.o:	$(CPUSRC)/m6502/m65ce02.c \
							$(CPUSRC)/m6502/m65ce02.h \
							$(CPUSRC)/m6502/opsce02.h \
							$(CPUSRC)/m6502/t65ce02.c

$(CPUOBJ)/m6502/m6509.o:	$(CPUSRC)/m6502/m6509.c \
							$(CPUSRC)/m6502/m6509.h \
							$(CPUSRC)/m6502/ops09.h \
							$(CPUSRC)/m6502/t6509.c



#-------------------------------------------------
# Motorola 680x
#-------------------------------------------------

CPUDEFS += -DHAS_M6800=$(if $(filter M6800,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6801=$(if $(filter M6801,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6802=$(if $(filter M6802,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6803=$(if $(filter M6803,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6808=$(if $(filter M6808,$(CPUS)),1,0)
CPUDEFS += -DHAS_HD63701=$(if $(filter HD63701,$(CPUS)),1,0)
CPUDEFS += -DHAS_NSC8105=$(if $(filter NSC8105,$(CPUS)),1,0)

ifneq ($(filter M6800 M6801 M6802 M6803 M6808 HD63701 NSC8105,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6800
CPUOBJS += $(CPUOBJ)/m6800/m6800.o
DBGOBJS += $(CPUOBJ)/m6800/6800dasm.o
endif

$(CPUOBJ)/m6800/m6800.o:	$(CPUSRC)/m6800/m6800.c \
							$(CPUSRC)/m6800/m6800.h \
							$(CPUSRC)/m6800/6800ops.c \
							$(CPUSRC)/m6800/6800tbl.c



#-------------------------------------------------
# Motorola 6805
#-------------------------------------------------

CPUDEFS += -DHAS_M6805=$(if $(filter M6805,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68705=$(if $(filter M68705,$(CPUS)),1,0)
CPUDEFS += -DHAS_HD63705=$(if $(filter HD63705,$(CPUS)),1,0)

ifneq ($(filter M6805 M68705 HD63705,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6805
CPUOBJS += $(CPUOBJ)/m6805/m6805.o
DBGOBJS += $(CPUOBJ)/m6805/6805dasm.o
endif

$(CPUOBJ)/m6805/m6805.o:	$(CPUSRC)/m6805/m6805.c \
							$(CPUSRC)/m6805/m6805.h \
							$(CPUSRC)/m6805/6805ops.c



#-------------------------------------------------
# Motorola 6809
#-------------------------------------------------

CPUDEFS += -DHAS_M6809=$(if $(filter M6809,$(CPUS)),1,0)
CPUDEFS += -DHAS_M6809E=$(if $(filter M6809E,$(CPUS)),1,0)

ifneq ($(filter M6809 M6809E,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6809
CPUOBJS += $(CPUOBJ)/m6809/m6809.o
DBGOBJS += $(CPUOBJ)/m6809/6809dasm.o
endif

$(CPUOBJ)/m6809/m6809.o:	$(CPUSRC)/m6809/m6809.c \
							$(CPUSRC)/m6809/m6809.h \
							$(CPUSRC)/m6809/6809ops.c \
							$(CPUSRC)/m6809/6809tbl.c



#-------------------------------------------------
# Motorola 68HC11
#-------------------------------------------------

CPUDEFS += -DHAS_MC68HC11=$(if $(filter MC68HC11,$(CPUS)),1,0)

ifneq ($(filter MC68HC11,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mc68hc11
CPUOBJS += $(CPUOBJ)/mc68hc11/mc68hc11.o
DBGOBJS += $(CPUOBJ)/mc68hc11/hc11dasm.o
endif

$(CPUOBJ)/mc68hc11/mc68hc11.o:	$(CPUSRC)/mc68hc11/mc68hc11.c \
								$(CPUSRC)/mc68hc11/hc11dasm.c



#-------------------------------------------------
# Motorola 68000 series
#-------------------------------------------------

CPUDEFS += -DHAS_M68000=$(if $(filter M68000,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68008=$(if $(filter M68008,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68010=$(if $(filter M68010,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68EC020=$(if $(filter M68EC020,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68020=$(if $(filter M68020,$(CPUS)),1,0)
CPUDEFS += -DHAS_M68040=$(if $(filter M68040,$(CPUS)),1,0)

ifneq ($(filter M68000 M68008 M68010 M68EC020 M68020 M68040,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m68000
CPUOBJS += $(CPUOBJ)/m68000/m68kcpu.o $(CPUOBJ)/m68000/m68kmame.o $(CPUOBJ)/m68000/m68kops.o
DBGOBJS += $(CPUOBJ)/m68000/m68kdasm.o
M68KMAKE = $(BUILDOUT)/m68kmake$(BUILD_EXE)
endif

# when we compile source files we need to include generated files from the OBJ directory
$(CPUOBJ)/m68000/%.o: $(CPUSRC)/m68000/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUOBJ)/m68000 -c $< -o $@

# when we compile generated files we need to include stuff from the src directory
$(CPUOBJ)/m68000/%.o: $(CPUOBJ)/m68000/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUSRC)/m68000 -c $< -o $@

# rule to generate the C files
$(CPUOBJ)/m68000/m68kops.c: $(M68KMAKE) $(CPUSRC)/m68000/m68k_in.c
	@echo Generating M68K source files...
	$(M68KMAKE) $(CPUOBJ)/m68000 $(CPUSRC)/m68000/m68k_in.c

# rule to build the generator
ifneq ($(CROSS_BUILD),1)

BUILD += $(M68KMAKE)

$(M68KMAKE): $(CPUOBJ)/m68000/m68kmake.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@
endif

# rule to ensure we build the header before building the core CPU file
$(CPUOBJ)/m68000/m68kcpu.o: $(CPUOBJ)/m68000/m68kops.c



#-------------------------------------------------
# Motorola/Freescale dsp56k
#-------------------------------------------------

CPUDEFS += -DHAS_DSP56156=$(if $(filter DSP56156,$(CPUS)),1,0)

ifneq ($(filter DSP56156,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/dsp56k
CPUOBJS += $(CPUOBJ)/dsp56k/dsp56k.o
DBGOBJS += $(CPUOBJ)/dsp56k/dsp56dsm.o
endif

$(CPUOBJ)/dsp56k/dsp56k.o:	$(CPUSRC)/dsp56k/dsp56k.c \
							$(CPUSRC)/dsp56k/dsp56ops.c \
							$(CPUSRC)/dsp56k/dsp56k.h



#-------------------------------------------------
# PDP-1
#-------------------------------------------------

CPUDEFS += -DHAS_PDP1=$(if $(filter PDP1,$(CPUS)),1,0)

ifneq ($(filter PDP1,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pdp1
CPUOBJS += $(CPUOBJ)/pdp1/pdp1.o
DBGOBJS += $(CPUOBJ)/pdp1/pdp1dasm.o
endif

$(CPUOBJ)/pdp1/pdp1.o:	$(CPUSRC)/pdp1/pdp1.c \
						$(CPUSRC)/pdp1/pdp1.h



#-------------------------------------------------
# Motorola PowerPC series
#-------------------------------------------------

CPUDEFS += -DHAS_PPC403=$(if $(filter PPC403,$(CPUS)),1,0)
CPUDEFS += -DHAS_PPC601=$(if $(filter PPC601,$(CPUS)),1,0)
CPUDEFS += -DHAS_PPC602=$(if $(filter PPC602,$(CPUS)),1,0)
CPUDEFS += -DHAS_PPC603=$(if $(filter PPC603,$(CPUS)),1,0)
CPUDEFS += -DHAS_PPC604=$(if $(filter PPC604,$(CPUS)),1,0)
CPUDEFS += -DHAS_MPC8240=$(if $(filter MPC8240,$(CPUS)),1,0)

ifneq ($(filter PPC403 PPC601 PPC602 PPC603 PPC604 MPC8240,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/powerpc
DBGOBJS += $(CPUOBJ)/powerpc/ppc_dasm.o

ifdef X86_PPC_DRC
CPUOBJS += $(CPUOBJ)/powerpc/ppcdrc.o $(DRCOBJ)
else
CPUOBJS += $(CPUOBJ)/powerpc/ppc.o
endif
endif

$(CPUOBJ)/powerpc/ppc.o:	$(CPUSRC)/powerpc/ppc.c \
							$(CPUSRC)/powerpc/ppc.h \
							$(CPUSRC)/powerpc/ppc_ops.c \
							$(CPUSRC)/powerpc/ppc_mem.c \
							$(CPUSRC)/powerpc/ppc403.c \
							$(CPUSRC)/powerpc/ppc602.c \
							$(CPUSRC)/powerpc/ppc603.c

$(CPUOBJ)/powerpc/ppcdrc.o:	$(CPUSRC)/powerpc/ppcdrc.c \
							$(CPUSRC)/powerpc/ppc.h \
							$(CPUSRC)/powerpc/drc_ops.c \
							$(CPUSRC)/powerpc/drc_ops.h \
							$(CPUSRC)/powerpc/ppc_ops.c \
							$(CPUSRC)/powerpc/ppc_mem.c \
							$(CPUSRC)/powerpc/ppc403.c \
							$(CPUSRC)/powerpc/ppc602.c \
							$(CPUSRC)/powerpc/ppc603.c \
							$(DRCDEPS)



#-------------------------------------------------
# NEC V-series Intel-compatible
#-------------------------------------------------

CPUDEFS += -DHAS_V20=$(if $(filter V20,$(CPUS)),1,0)
CPUDEFS += -DHAS_V25=$(if $(filter V25,$(CPUS)),1,0)
CPUDEFS += -DHAS_V30=$(if $(filter V30,$(CPUS)),1,0)
CPUDEFS += -DHAS_V30MZ=$(if $(filter V30MZ,$(CPUS)),1,0)
CPUDEFS += -DHAS_V33=$(if $(filter V33,$(CPUS)),1,0)
CPUDEFS += -DHAS_V35=$(if $(filter V35,$(CPUS)),1,0)

ifneq ($(filter V20 V25 V30 V33 V35,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/nec
CPUOBJS += $(CPUOBJ)/nec/nec.o
DBGOBJS += $(CPUOBJ)/nec/necdasm.o
endif

ifneq ($(filter V30MZ,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v30mz $(CPUOBJ)/nec
CPUOBJS += $(CPUOBJ)/v30mz/v30mz.o
DBGOBJS += $(CPUOBJ)/nec/necdasm.o
endif

$(CPUOBJ)/nec/nec.o:	$(CPUSRC)/nec/nec.c \
						$(CPUSRC)/nec/nec.h \
						$(CPUSRC)/nec/necintrf.h \
						$(CPUSRC)/nec/necea.h \
						$(CPUSRC)/nec/nechost.h \
						$(CPUSRC)/nec/necinstr.h \
						$(CPUSRC)/nec/necmodrm.h

$(CPUOBJ)/v30mz/v30mz.o:	$(CPUSRC)/v30mz/v30mz.c \
							$(CPUSRC)/v30mz/v30mz.h \
							$(CPUSRC)/v30mz/necmodrm.h \
							$(CPUSRC)/v30mz/necinstr.h \
							$(CPUSRC)/v30mz/necea.h \
							$(CPUSRC)/v30mz/nechost.h \
							$(CPUSRC)/v30mz/necintrf.h



#-------------------------------------------------
# NEC V60/V70
#-------------------------------------------------

CPUDEFS += -DHAS_V60=$(if $(filter V60,$(CPUS)),1,0)
CPUDEFS += -DHAS_V70=$(if $(filter V70,$(CPUS)),1,0)

ifneq ($(filter V60 V70,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v60
CPUOBJS += $(CPUOBJ)/v60/v60.o
DBGOBJS += $(CPUOBJ)/v60/v60d.o
endif

$(CPUOBJ)/v60/v60.o:	$(CPUSRC)/v60/am.c \
						$(CPUSRC)/v60/am1.c \
						$(CPUSRC)/v60/am2.c \
						$(CPUSRC)/v60/am3.c \
						$(CPUSRC)/v60/op12.c \
						$(CPUSRC)/v60/op2.c \
						$(CPUSRC)/v60/op3.c \
						$(CPUSRC)/v60/op4.c \
						$(CPUSRC)/v60/op5.c \
						$(CPUSRC)/v60/op6.c \
						$(CPUSRC)/v60/op7a.c \
						$(CPUSRC)/v60/optable.c \
						$(CPUSRC)/v60/v60.c \
						$(CPUSRC)/v60/v60.h \
						$(CPUSRC)/v60/v60d.c



#-------------------------------------------------
# NEC V810 (uPD70732)
#-------------------------------------------------

CPUDEFS += -DHAS_V810=$(if $(filter V810,$(CPUS)),1,0)

ifneq ($(filter V810,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v810
CPUOBJS += $(CPUOBJ)/v810/v810.o
DBGOBJS += $(CPUOBJ)/v810/v810dasm.o
endif

$(CPUOBJ)/v810/v810.o:	$(CPUSRC)/v810/v810.c \
						$(CPUSRC)/v810/v810.h



#-------------------------------------------------
# NEC uPD7810 series
#-------------------------------------------------

CPUDEFS += -DHAS_UPD7810=$(if $(filter UPD7810,$(CPUS)),1,0)
CPUDEFS += -DHAS_UPD7807=$(if $(filter UPD7807,$(CPUS)),1,0)
CPUDEFS += -DHAS_UPD7801=$(if $(filter UPD7801,$(CPUS)),1,0)

ifneq ($(filter UPD7810 UPD7807 UPD7801,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/upd7810
CPUOBJS += $(CPUOBJ)/upd7810/upd7810.o
DBGOBJS += $(CPUOBJ)/upd7810/7810dasm.o
endif

$(CPUOBJ)/upd7810/upd7810.o:	$(CPUSRC)/upd7810/upd7810.c \
								$(CPUSRC)/upd7810/7810tbl.c \
								$(CPUSRC)/upd7810/7810ops.c \
								$(CPUSRC)/upd7810/upd7810.h



#-------------------------------------------------
# Nintendo Minx
#-------------------------------------------------

CPUDEFS += -DHAS_MINX=$(if $(filter MINX,$(CPUS)),1,0)

ifneq ($(filter MINX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/minx
CPUOBJS += $(CPUOBJ)/minx/minx.o
DBGOBJS += $(CPUOBJ)/minx/minxd.o
endif

$(CPUOBJ)/minx/minx.o:		$(CPUSRC)/minx/minx.c \
							$(CPUSRC)/minx/minx.h \
							$(CPUSRC)/minx/minxd.c \
							$(CPUSRC)/minx/minxopce.h \
							$(CPUSRC)/minx/minxopcf.h \
							$(CPUSRC)/minx/minxops.h \
							$(CPUSRC)/minx/minxfunc.h



#-------------------------------------------------
# Nintendo/SGI RSP (R3000-based + vector processing)
#-------------------------------------------------

CPUDEFS += -DHAS_RSP=$(if $(filter RSP,$(CPUS)),1,0)

ifneq ($(filter RSP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/rsp
CPUOBJS += $(CPUOBJ)/rsp/rsp.o
DBGOBJS += $(CPUOBJ)/rsp/rsp_dasm.o
endif

$(CPUOBJ)/rsp/rsp.o:	$(CPUSRC)/rsp/rsp.c \
						$(CPUSRC)/rsp/rsp.h



#-------------------------------------------------
# Saturn
#-------------------------------------------------

CPUDEFS += -DHAS_SATURN=$(if $(filter SATURN,$(CPUS)),1,0)

ifneq ($(filter SATURN,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/saturn
CPUOBJS += $(CPUOBJ)/saturn/saturn.o
DBGOBJS += $(CPUOBJ)/saturn/saturnds.o
endif

$(CPUOBJ)/saturn/saturn.o:	$(CPUSRC)/saturn/saturn.c \
							$(CPUSRC)/saturn/sattable.c \
							$(CPUSRC)/saturn/satops.c \
							$(CPUSRC)/saturn/saturn.h \
							$(CPUSRC)/saturn/sat.h



#-------------------------------------------------
# Signetics 2650
#-------------------------------------------------

CPUDEFS += -DHAS_S2650=$(if $(filter S2650,$(CPUS)),1,0)

ifneq ($(filter S2650,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/s2650
CPUOBJS += $(CPUOBJ)/s2650/s2650.o
DBGOBJS += $(CPUOBJ)/s2650/2650dasm.o
endif

$(CPUOBJ)/s2650/s2650.o:	$(CPUSRC)/s2650/s2650.c \
							$(CPUSRC)/s2650/s2650.h \
							$(CPUSRC)/s2650/s2650cpu.h



#-------------------------------------------------
# SC61860
#-------------------------------------------------

CPUDEFS += -DHAS_SC61860=$(if $(filter SC61860,$(CPUS)),1,0)

ifneq ($(filter SC61860,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sc61860
CPUOBJS += $(CPUOBJ)/sc61860/sc61860.o
DBGOBJS += $(CPUOBJ)/sc61860/scdasm.o
endif

$(CPUOBJ)/sc61860/sc61860.o:	$(CPUSRC)/sc61860/sc61860.h \
								$(CPUSRC)/sc61860/sc.h \
								$(CPUSRC)/sc61860/scops.c \
								$(CPUSRC)/sc61860/sctable.c



#-------------------------------------------------
# SM8500
#-------------------------------------------------

CPUDEFS += -DHAS_SM8500=$(if $(filter SM8500,$(CPUS)),1,0)

ifneq ($(filter SM8500,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sm8500
CPUOBJS += $(CPUOBJ)/sm8500/sm8500.o
DBGOBJS += $(CPUOBJ)/sm8500/sm8500d.o
endif

$(CPUOBJ)/sm8500/sm8500.o:	$(CPUSRC)/sm8500/sm8500.c \
							$(CPUSRC)/sm8500/sm8500.h \
							$(CPUSRC)/sm8500/sm85ops.h



#-------------------------------------------------
# Sony/Nintendo SPC700
#-------------------------------------------------

CPUDEFS += -DHAS_SPC700=$(if $(filter SPC700,$(CPUS)),1,0)

ifneq ($(filter SPC700,$(CPUS)),)
SPCD = cpu/spc700
OBJDIRS += $(CPUOBJ)/spc700
CPUOBJS += $(CPUOBJ)/spc700/spc700.o
DBGOBJS += $(CPUOBJ)/spc700/spc700ds.o
endif

$(CPUOBJ)/spc700/spc700.o:	$(CPUSRC)/spc700/spc700.c \
							$(CPUSRC)/spc700/spc700.h



#-------------------------------------------------
# Sony PlayStation CPU (R3000-based + GTE)
#-------------------------------------------------

CPUDEFS += -DHAS_PSXCPU=$(if $(filter PSXCPU,$(CPUS)),1,0)
CPUDEFS += -DHAS_CXD8661R=$(if $(filter CXD8661R,$(CPUS)),1,0)

ifneq ($(filter PSXCPU CXD8661R,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mips
CPUOBJS += $(CPUOBJ)/mips/psx.o
DBGOBJS += $(CPUOBJ)/mips/mipsdasm.o
endif

$(CPUOBJ)/mips/psx.o:	$(CPUSRC)/mips/psx.c \
						$(CPUSRC)/mips/psx.h



#-------------------------------------------------
# SSP1601
#-------------------------------------------------

CPUDEFS += -DHAS_SSP1601=$(if $(filter SSP1601,$(CPUS)),1,0)

ifneq ($(filter SSP1601,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/ssp1601
CPUOBJS += $(CPUOBJ)/ssp1601/ssp1601.o
DBGOBJS += $(CPUOBJ)/ssp1601/ssp1601d.o
endif

$(CPUOBJ)/ssp1610/ssp1601.o:	$(CPUSRC)/ssp1601/ssp1601.c \
								$(CPUSRC)/ssp1610/ssp1601.h



#-------------------------------------------------
# Texas Instruments TMS7000 series
#-------------------------------------------------

CPUDEFS += -DHAS_TMS7000=$(if $(filter TMS7000,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS7000_EXL=$(if $(filter TMS7000_EXL,$(CPUS)),1,0)

ifneq ($(filter TMS7000 TMS7000_EXL,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms7000
CPUOBJS += $(CPUOBJ)/tms7000/tms7000.o
DBGOBJS += $(CPUOBJ)/tms7000/7000dasm.o
endif

$(CPUOBJ)/tms7000/tms7000.o:	$(CPUSRC)/tms7000/tms7000.h \
								$(CPUSRC)/tms7000/tms7000.c

$(CPUOBJ)/tms7000/7000dasm.o:	$(CPUSRC)/tms7000/tms7000.h \
								$(CPUSRC)/tms7000/7000dasm.c



#-------------------------------------------------
# Texas Instruments TMS99xx series
#-------------------------------------------------

CPUDEFS += -DHAS_TMS9900=$(if $(filter TMS9900,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS9940=$(if $(filter TMS9940,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS9980=$(if $(filter TMS9980,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS9985=$(if $(filter TMS9985,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS9989=$(if $(filter TMS9989,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS9995=$(if $(filter TMS9995,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS99105A=$(if $(filter TMS99105A,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS99110A=$(if $(filter TMS99110A,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS99000=$(if $(filter TMS99000,$(CPUS)),1,0)
CPUDEFS += -DHAS_TI990_10=$(if $(filter TMS99010,$(CPUS)),1,0)

ifneq ($(filter TMS9900 TMS9940 TMS99000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms9900
CPUOBJS += $(CPUOBJ)/tms9900/tms9900.o
DBGOBJS += $(CPUOBJ)/tms9900/9900dasm.o
endif

ifneq ($(filter TMS9980 TMS9985 TMS9989,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms9900
CPUOBJS += $(CPUOBJ)/tms9900/tms9980a.o
DBGOBJS += $(CPUOBJ)/tms9900/9900dasm.o
endif

ifneq ($(filter TMS9995 TMS99105A TMS99110A,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms9900
CPUOBJS += $(CPUOBJ)/tms9900/tms9995.o
DBGOBJS += $(CPUOBJ)/tms9900/9900dasm.o
endif

ifneq ($(filter TMS99010,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms9900
CPUOBJS += $(CPUOBJ)/tms9900/ti990_10.o
DBGOBJS += $(CPUOBJ)/tms9900/9900dasm.o
endif

$(CPUOBJ)/tms9900/tms9900.o:	$(CPUSRC)/tms9900/tms9900.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/tms9980a.o:	$(CPUSRC)/tms9900/tms9980a.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/tms9995.o:	$(CPUSRC)/tms9900/tms9995.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/ti990_10.o:	$(CPUSRC)/tms9900/ti990_10.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h



#-------------------------------------------------
# Texas Instruments TMS340x0 graphics controllers
#-------------------------------------------------

CPUDEFS += -DHAS_TMS34010=$(if $(filter TMS34010,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS34020=$(if $(filter TMS34020,$(CPUS)),1,0)

ifneq ($(filter TMS34010 TMS34020,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms34010
CPUOBJS += $(CPUOBJ)/tms34010/tms34010.o $(CPUOBJ)/tms34010/34010fld.o
DBGOBJS += $(CPUOBJ)/tms34010/34010dsm.o
endif

$(CPUOBJ)/tms34010/34010fld.o:  $(CPUSRC)/tms34010/34010fld.c

$(CPUOBJ)/tms34010/tms34010.o:	$(CPUSRC)/tms34010/tms34010.c \
								$(CPUSRC)/tms34010/tms34010.h \
								$(CPUSRC)/tms34010/34010ops.c \
								$(CPUSRC)/tms34010/34010gfx.c \
								$(CPUSRC)/tms34010/34010tbl.c



#-------------------------------------------------
# Texas Instruments TMS3201x DSP
#-------------------------------------------------

CPUDEFS += -DHAS_TMS32010=$(if $(filter TMS32010,$(CPUS)),1,0)

ifneq ($(filter TMS32010,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32010
CPUOBJS += $(CPUOBJ)/tms32010/tms32010.o
DBGOBJS += $(CPUOBJ)/tms32010/32010dsm.o
endif

$(CPUOBJ)/tms32010/tms32010.o:	$(CPUSRC)/tms32010/tms32010.c \
								$(CPUSRC)/tms32010/tms32010.h



#-------------------------------------------------
# Texas Instruments TMS3202x DSP
#-------------------------------------------------

CPUDEFS += -DHAS_TMS32025=$(if $(filter TMS32025,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS32026=$(if $(filter TMS32026,$(CPUS)),1,0)

ifneq ($(filter TMS32025 TMS32026,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32025
CPUOBJS += $(CPUOBJ)/tms32025/tms32025.o
DBGOBJS += $(CPUOBJ)/tms32025/32025dsm.o
endif

$(CPUOBJ)/tms32025/tms32025.o:	$(CPUSRC)/tms32025/tms32025.c \
								$(CPUSRC)/tms32025/tms32025.h



#-------------------------------------------------
# Texas Instruments TMS3203x DSP
#-------------------------------------------------

CPUDEFS += -DHAS_TMS32031=$(if $(filter TMS32031,$(CPUS)),1,0)
CPUDEFS += -DHAS_TMS32032=$(if $(filter TMS32032,$(CPUS)),1,0)

ifneq ($(filter TMS32031 TMS32032,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32031
CPUOBJS += $(CPUOBJ)/tms32031/tms32031.o
DBGOBJS += $(CPUOBJ)/tms32031/dis32031.o
endif

$(CPUOBJ)/tms32031/tms32031.o:	$(CPUSRC)/tms32031/tms32031.c \
								$(CPUSRC)/tms32031/tms32031.h \
								$(CPUSRC)/tms32031/32031ops.c



#-------------------------------------------------
# Texas Instruments TMS3205x DSP
#-------------------------------------------------

CPUDEFS += -DHAS_TMS32051=$(if $(filter TMS32051,$(CPUS)),1,0)

ifneq ($(filter TMS32051,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32051
CPUOBJS += $(CPUOBJ)/tms32051/tms32051.o
DBGOBJS += $(CPUOBJ)/tms32051/dis32051.o
endif

$(CPUOBJ)/tms32051/tms32051.o:	$(CPUSRC)/tms32051/tms32051.c \
								$(CPUSRC)/tms32051/tms32051.h



#-------------------------------------------------
# Toshiba TLCS-90 Series
#-------------------------------------------------

CPUDEFS += -DHAS_TLCS90=$(if $(filter TLCS90,$(CPUS)),1,0)

ifneq ($(filter TLCS90,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tlcs90
CPUOBJS += $(CPUOBJ)/tlcs90/tlcs90.o
#DBGOBJS += $(CPUOBJ)/tlcs90/tlcs90.o
endif

$(CPUOBJ)/tlcs90/tlcs90.o:	$(CPUSRC)/tlcs90/tlcs90.c \
							$(CPUSRC)/tlcs90/tlcs90.h



#-------------------------------------------------
# TX0
#-------------------------------------------------

CPUDEFS += -DHAS_TX0_64KW=$(if $(filter TX0,$(CPUS)),1,0)
CPUDEFS += -DHAS_TX0_8KW=$(if $(filter TX0,$(CPUS)),1,0)

ifneq ($(filter TX0,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pdp1
CPUOBJS += $(CPUOBJ)/pdp1/tx0.o
DBGOBJS += $(CPUOBJ)/pdp1/tx0dasm.o
endif

$(CPUOBJ)/pdp1/tx0.o:		$(CPUSRC)/pdp1/tx0.h \
							$(CPUSRC)/pdp1/tx0.c

$(CPUOBJ)/pdp1/tx0dasm.o:	$(CPUSRC)/pdp1/tx0.h \
							$(CPUSRC)/pdp1/tx0dasm.c



#-------------------------------------------------
# Zilog Z80
#-------------------------------------------------

CPUDEFS += -DHAS_Z80=$(if $(filter Z80,$(CPUS)),1,0)

ifneq ($(filter Z80,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z80
CPUOBJS += $(CPUOBJ)/z80/z80.o $(CPUOBJ)/z80/z80daisy.o
DBGOBJS += $(CPUOBJ)/z80/z80dasm.o
endif

$(CPUOBJ)/z80/z80.o:	$(CPUSRC)/z80/z80.c \
						$(CPUSRC)/z80/z80.h



#-------------------------------------------------
# Game Boy Z-80
#-------------------------------------------------

CPUDEFS += -DHAS_Z80GB=$(if $(filter Z80GB,$(CPUS)),1,0)

ifneq ($(filter Z80GB,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z80gb
CPUOBJS += $(CPUOBJ)/z80gb/z80gb.o
DBGOBJS += $(CPUOBJ)/z80gb/z80gbd.o
endif

$(CPUOBJ)/z80gb/z80gb.o:	$(CPUSRC)/z80gb/z80gb.c \
							$(CPUSRC)/z80gb/z80gb.h \
							$(CPUSRC)/z80gb/opc_cb.h \
							$(CPUSRC)/z80gb/opc_main.h



#-------------------------------------------------
# Zilog Z180
#-------------------------------------------------

CPUDEFS += -DHAS_Z180=$(if $(filter Z180,$(CPUS)),1,0)

ifneq ($(filter Z180,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z180 $(CPUOBJ)/z80
CPUOBJS += $(CPUOBJ)/z180/z180.o $(CPUOBJ)/z80/z80daisy.o
DBGOBJS += $(CPUOBJ)/z180/z180dasm.o
endif

$(CPUOBJ)/z180/z180.o:	$(CPUSRC)/z180/z180.c \
						$(CPUSRC)/z180/z180.h \
						$(CPUSRC)/z180/z180daa.h \
						$(CPUSRC)/z180/z180op.c \
						$(CPUSRC)/z180/z180ops.h \
						$(CPUSRC)/z180/z180tbl.h \
						$(CPUSRC)/z180/z180cb.c \
						$(CPUSRC)/z180/z180dd.c \
						$(CPUSRC)/z180/z180ed.c \
						$(CPUSRC)/z180/z180fd.c \
						$(CPUSRC)/z180/z180xy.c



#-------------------------------------------------
# Zilog Z8000
#-------------------------------------------------

CPUDEFS += -DHAS_Z8000=$(if $(filter Z8000,$(CPUS)),1,0)

ifneq ($(filter Z8000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z8000
CPUOBJS += $(CPUOBJ)/z8000/z8000.o
DBGOBJS += $(CPUOBJ)/z8000/8000dasm.o
endif

$(CPUOBJ)/z8000/z8000.o:	$(CPUSRC)/z8000/z8000.c \
							$(CPUSRC)/z8000/z8000.h \
							$(CPUSRC)/z8000/z8000cpu.h \
							$(CPUSRC)/z8000/z8000dab.h \
							$(CPUSRC)/z8000/z8000ops.c \
							$(CPUSRC)/z8000/z8000tbl.c
