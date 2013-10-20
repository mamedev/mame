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
#@src/emu/bus/adamnet/adamnet.h,BUSES += ADAMNET
#-------------------------------------------------

ifneq ($(filter ADAMNET,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/adamnet/adamnet.o
BUSOBJS += $(BUSOBJ)/adamnet/ddp.o
BUSOBJS += $(BUSOBJ)/adamnet/fdc.o
BUSOBJS += $(BUSOBJ)/adamnet/kb.o
BUSOBJS += $(BUSOBJ)/adamnet/printer.o
BUSOBJS += $(BUSOBJ)/adamnet/spi.o
endif


#-------------------------------------------------
#
#@src/emu/bus/cbmiec/cbmiec.h,BUSES += CBMIEC
#-------------------------------------------------

ifneq ($(filter CBMIEC,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/cbmiec/cbmiec.o
BUSOBJS += $(BUSOBJ)/cbmiec/c1541.o
BUSOBJS += $(BUSOBJ)/cbmiec/c1571.o
BUSOBJS += $(BUSOBJ)/cbmiec/c1581.o
BUSOBJS += $(BUSOBJ)/cbmiec/c64_nl10.o
BUSOBJS += $(BUSOBJ)/cbmiec/cmdhd.o
BUSOBJS += $(BUSOBJ)/cbmiec/diag264_lb_iec.o
BUSOBJS += $(BUSOBJ)/cbmiec/fd2000.o
BUSOBJS += $(BUSOBJ)/cbmiec/interpod.o
BUSOBJS += $(BUSOBJ)/cbmiec/serialbox.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ieee488/ieee488.h,BUSES += IEEE488
#-------------------------------------------------

ifneq ($(filter IEEE488,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/ieee488/ieee488.o
BUSOBJS += $(BUSOBJ)/ieee488/c2031.o
BUSOBJS += $(BUSOBJ)/ieee488/c2040.o
BUSOBJS += $(BUSOBJ)/ieee488/c8280.o
BUSOBJS += $(BUSOBJ)/ieee488/d9060.o
BUSOBJS += $(BUSOBJ)/ieee488/d9060hd.o
BUSOBJS += $(BUSOBJ)/ieee488/softbox.o
BUSOBJS += $(BUSOBJ)/ieee488/hardbox.o
BUSOBJS += $(BUSOBJ)/ieee488/shark.o
endif


#-------------------------------------------------
#
#@src/emu/bus/isbx/isbx.h,BUSES += ISBX
#-------------------------------------------------

ifneq ($(filter ISBX,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/isbx/isbx.o
BUSOBJS += $(BUSOBJ)/isbx/compis_fdc.o
endif


#-------------------------------------------------
#
#@src/emu/bus/s100/s100.h,BUSES += S100
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


#-------------------------------------------------
#
#@src/emu/bus/wangpc/wangpc.h,BUSES += WANGPC
#-------------------------------------------------

ifneq ($(filter WANGPC,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/wangpc/wangpc.o
BUSOBJS += $(BUSOBJ)/wangpc/emb.o
BUSOBJS += $(BUSOBJ)/wangpc/lic.o
BUSOBJS += $(BUSOBJ)/wangpc/lvc.o
BUSOBJS += $(BUSOBJ)/wangpc/mcc.o
BUSOBJS += $(BUSOBJ)/wangpc/mvc.o
BUSOBJS += $(BUSOBJ)/wangpc/rtc.o
BUSOBJS += $(BUSOBJ)/wangpc/tig.o
BUSOBJS += $(BUSOBJ)/wangpc/wdc.o
endif
