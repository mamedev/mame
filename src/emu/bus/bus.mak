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
#@src/emu/bus/abcbus/abcbus.h,BUSES += ABCBUS
#-------------------------------------------------

ifneq ($(filter ABCBUS,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/abcbus/abcbus.o
BUSOBJS += $(BUSOBJ)/abcbus/abc890.o
BUSOBJS += $(BUSOBJ)/abcbus/dos.o
BUSOBJS += $(BUSOBJ)/abcbus/fd2.o
BUSOBJS += $(BUSOBJ)/abcbus/hdc.o
BUSOBJS += $(BUSOBJ)/abcbus/lux10828.o
BUSOBJS += $(BUSOBJ)/abcbus/lux21046.o
BUSOBJS += $(BUSOBJ)/abcbus/uni800.o
BUSOBJS += $(BUSOBJ)/abcbus/sio.o
BUSOBJS += $(BUSOBJ)/abcbus/slutprov.o
BUSOBJS += $(BUSOBJ)/abcbus/turbo.o
BUSOBJS += $(BUSOBJ)/abcbus/xebec.o
endif


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
#@src/emu/bus/c64/exp.h,BUSES += C64
#@src/emu/bus/c64/user.h,BUSES += C64
#-------------------------------------------------

ifneq ($(filter C64,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/c64/exp.o
BUSOBJS += $(BUSOBJ)/c64/c128_comal80.o
BUSOBJS += $(BUSOBJ)/c64/comal80.o
BUSOBJS += $(BUSOBJ)/c64/cpm.o
BUSOBJS += $(BUSOBJ)/c64/currah_speech.o
BUSOBJS += $(BUSOBJ)/c64/dela_ep256.o
BUSOBJS += $(BUSOBJ)/c64/dela_ep64.o
BUSOBJS += $(BUSOBJ)/c64/dela_ep7x8.o
BUSOBJS += $(BUSOBJ)/c64/dinamic.o
BUSOBJS += $(BUSOBJ)/c64/dqbb.o
BUSOBJS += $(BUSOBJ)/c64/easy_calc_result.o
BUSOBJS += $(BUSOBJ)/c64/easyflash.o
BUSOBJS += $(BUSOBJ)/c64/epyx_fast_load.o
BUSOBJS += $(BUSOBJ)/c64/exos.o
BUSOBJS += $(BUSOBJ)/c64/fcc.o
BUSOBJS += $(BUSOBJ)/c64/final.o
BUSOBJS += $(BUSOBJ)/c64/final3.o
BUSOBJS += $(BUSOBJ)/c64/fun_play.o
BUSOBJS += $(BUSOBJ)/c64/georam.o
BUSOBJS += $(BUSOBJ)/c64/ide64.o
BUSOBJS += $(BUSOBJ)/c64/ieee488.o
BUSOBJS += $(BUSOBJ)/c64/kingsoft.o
BUSOBJS += $(BUSOBJ)/c64/mach5.o
BUSOBJS += $(BUSOBJ)/c64/magic_desk.o
BUSOBJS += $(BUSOBJ)/c64/magic_formel.o
BUSOBJS += $(BUSOBJ)/c64/magic_voice.o
BUSOBJS += $(BUSOBJ)/c64/midi_maplin.o
BUSOBJS += $(BUSOBJ)/c64/midi_namesoft.o
BUSOBJS += $(BUSOBJ)/c64/midi_passport.o
BUSOBJS += $(BUSOBJ)/c64/midi_sci.o
BUSOBJS += $(BUSOBJ)/c64/midi_siel.o
BUSOBJS += $(BUSOBJ)/c64/mikro_assembler.o
BUSOBJS += $(BUSOBJ)/c64/multiscreen.o
BUSOBJS += $(BUSOBJ)/c64/music64.o
BUSOBJS += $(BUSOBJ)/c64/neoram.o
BUSOBJS += $(BUSOBJ)/c64/ocean.o
BUSOBJS += $(BUSOBJ)/c64/pagefox.o
BUSOBJS += $(BUSOBJ)/c64/partner.o
BUSOBJS += $(BUSOBJ)/c64/prophet64.o
BUSOBJS += $(BUSOBJ)/c64/ps64.o
BUSOBJS += $(BUSOBJ)/c64/reu.o
BUSOBJS += $(BUSOBJ)/c64/rex.o
BUSOBJS += $(BUSOBJ)/c64/rex_ep256.o
BUSOBJS += $(BUSOBJ)/c64/ross.o
BUSOBJS += $(BUSOBJ)/c64/sfx_sound_expander.o
BUSOBJS += $(BUSOBJ)/c64/silverrock.o
BUSOBJS += $(BUSOBJ)/c64/simons_basic.o
BUSOBJS += $(BUSOBJ)/c64/stardos.o
BUSOBJS += $(BUSOBJ)/c64/std.o
BUSOBJS += $(BUSOBJ)/c64/structured_basic.o
BUSOBJS += $(BUSOBJ)/c64/super_explode.o
BUSOBJS += $(BUSOBJ)/c64/super_games.o
BUSOBJS += $(BUSOBJ)/c64/supercpu.o
BUSOBJS += $(BUSOBJ)/c64/sw8k.o
BUSOBJS += $(BUSOBJ)/c64/swiftlink.o
BUSOBJS += $(BUSOBJ)/c64/system3.o
BUSOBJS += $(BUSOBJ)/c64/tdos.o
BUSOBJS += $(BUSOBJ)/c64/turbo232.o
BUSOBJS += $(BUSOBJ)/c64/vizastar.o
BUSOBJS += $(BUSOBJ)/c64/vw64.o
BUSOBJS += $(BUSOBJ)/c64/warp_speed.o
BUSOBJS += $(BUSOBJ)/c64/westermann.o
BUSOBJS += $(BUSOBJ)/c64/xl80.o
BUSOBJS += $(BUSOBJ)/c64/zaxxon.o
BUSOBJS += $(BUSOBJ)/c64/user.o
BUSOBJS += $(BUSOBJ)/c64/4cga.o
BUSOBJS += $(BUSOBJ)/c64/4dxh.o
BUSOBJS += $(BUSOBJ)/c64/4ksa.o
BUSOBJS += $(BUSOBJ)/c64/4tba.o
BUSOBJS += $(BUSOBJ)/c64/16kb.o
BUSOBJS += $(BUSOBJ)/c64/bn1541.o
BUSOBJS += $(BUSOBJ)/c64/geocable.o
BUSOBJS += $(BUSOBJ)/c64/vic1011.o
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
