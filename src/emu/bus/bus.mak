###########################################################################
#
#   bus.mak
#
#   Rules for building bus cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


BUSSRC = $(EMUSRC)/bus
BUSOBJ = $(EMUOBJ)/bus


#-------------------------------------------------
#
#@src/emu/bus/isbx.h,BUSES += ISBX
#-------------------------------------------------

ifneq ($(filter ISBX,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/isbx/isbx.o
BUSOBJS += $(BUSOBJ)/isbx/compis_fdc.o
endif


#-------------------------------------------------
#
#@src/emu/bus/isbx.h,BUSES += S100
#-------------------------------------------------

ifneq ($(filter S100,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/s100/s100.o
BUSOBJS += $(BUSOBJ)/s100/dj2db.o
BUSOBJS += $(BUSOBJ)/s100/djdma.o
BUSOBJS += $(BUSOBJ)/s100/mm65k16s.o
BUSOBJS += $(BUSOBJ)/s100/nsmdsa.o
BUSOBJS += $(BUSOBJ)/s100/nsmdsad.o
BUSOBJS += $(BUSOBJ)/s100/wunderbus.o
endif
