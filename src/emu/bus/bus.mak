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
#@src/emu/bus/a7800/a78_slot.h,BUSES += A7800
#-------------------------------------------------

ifneq ($(filter A7800,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/a7800
BUSOBJS += $(BUSOBJ)/a7800/a78_slot.o
BUSOBJS += $(BUSOBJ)/a7800/rom.o
BUSOBJS += $(BUSOBJ)/a7800/hiscore.o
BUSOBJS += $(BUSOBJ)/a7800/xboard.o
BUSOBJS += $(BUSOBJ)/a7800/cpuwiz.o
endif


#-------------------------------------------------
#
#@src/emu/bus/a800/a800_slot.h,BUSES += A800
#-------------------------------------------------

ifneq ($(filter A800,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/a800
BUSOBJS += $(BUSOBJ)/a800/a800_slot.o
BUSOBJS += $(BUSOBJ)/a800/rom.o
BUSOBJS += $(BUSOBJ)/a800/oss.o
BUSOBJS += $(BUSOBJ)/a800/sparta.o
endif


#-------------------------------------------------
#
#@src/emu/bus/abcbus/abcbus.h,BUSES += ABCBUS
#-------------------------------------------------

ifneq ($(filter ABCBUS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/abcbus
BUSOBJS += $(BUSOBJ)/abcbus/abcbus.o
BUSOBJS += $(BUSOBJ)/abcbus/abc890.o
BUSOBJS += $(BUSOBJ)/abcbus/dos.o
BUSOBJS += $(BUSOBJ)/abcbus/fd2.o
BUSOBJS += $(BUSOBJ)/abcbus/hdc.o
BUSOBJS += $(BUSOBJ)/abcbus/lux10828.o
BUSOBJS += $(BUSOBJ)/abcbus/lux21046.o
BUSOBJS += $(BUSOBJ)/abcbus/lux21056.o
BUSOBJS += $(BUSOBJ)/abcbus/lux4105.o
BUSOBJS += $(BUSOBJ)/abcbus/uni800.o
BUSOBJS += $(BUSOBJ)/abcbus/sio.o
BUSOBJS += $(BUSOBJ)/abcbus/slutprov.o
BUSOBJS += $(BUSOBJ)/abcbus/turbo.o
endif


#-------------------------------------------------
#
#@src/emu/bus/adam/exp.h,BUSES += ADAM
#-------------------------------------------------

ifneq ($(filter ADAM,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/adam
BUSOBJS += $(BUSOBJ)/adam/exp.o
BUSOBJS += $(BUSOBJ)/adam/adamlink.o
BUSOBJS += $(BUSOBJ)/adam/ide.o
BUSOBJS += $(BUSOBJ)/adam/ram.o
endif


#-------------------------------------------------
#
#@src/emu/bus/adamnet/adamnet.h,BUSES += ADAMNET
#-------------------------------------------------

ifneq ($(filter ADAMNET,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/adamnet
BUSOBJS += $(BUSOBJ)/adamnet/adamnet.o
BUSOBJS += $(BUSOBJ)/adamnet/ddp.o
BUSOBJS += $(BUSOBJ)/adamnet/fdc.o
BUSOBJS += $(BUSOBJ)/adamnet/kb.o
BUSOBJS += $(BUSOBJ)/adamnet/printer.o
BUSOBJS += $(BUSOBJ)/adamnet/spi.o
endif


#-------------------------------------------------
#
#@src/emu/bus/apf/slot.h,BUSES += APF
#-------------------------------------------------

ifneq ($(filter APF,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/apf
BUSOBJS += $(BUSOBJ)/apf/slot.o
BUSOBJS += $(BUSOBJ)/apf/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/arcadia/slot.h,BUSES += ARCADIA
#-------------------------------------------------

ifneq ($(filter ARCADIA,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/arcadia
BUSOBJS += $(BUSOBJ)/arcadia/slot.o
BUSOBJS += $(BUSOBJ)/arcadia/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/astrocde/slot.h,BUSES += ASTROCADE
#-------------------------------------------------

ifneq ($(filter ASTROCADE,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/astrocde
BUSOBJS += $(BUSOBJ)/astrocde/slot.o
BUSOBJS += $(BUSOBJ)/astrocde/rom.o
BUSOBJS += $(BUSOBJ)/astrocde/exp.o
BUSOBJS += $(BUSOBJ)/astrocde/ram.o
endif


#-------------------------------------------------
#
#@src/emu/bus/bw2/exp.h,BUSES += BW2
#-------------------------------------------------

ifneq ($(filter BW2,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/bw2
BUSOBJS += $(BUSOBJ)/bw2/exp.o
BUSOBJS += $(BUSOBJ)/bw2/ramcard.o
endif


#-------------------------------------------------
#
#@src/emu/bus/c64/exp.h,BUSES += C64
#@src/emu/bus/c64/user.h,BUSES += C64
#-------------------------------------------------

ifneq ($(filter C64,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/c64
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
BUSOBJS += $(BUSOBJ)/c64/4dxh.o
BUSOBJS += $(BUSOBJ)/c64/4ksa.o
BUSOBJS += $(BUSOBJ)/c64/4tba.o
BUSOBJS += $(BUSOBJ)/c64/16kb.o
BUSOBJS += $(BUSOBJ)/c64/bn1541.o
BUSOBJS += $(BUSOBJ)/c64/geocable.o
endif


#-------------------------------------------------
#
#@src/emu/bus/cbm2/exp.h,BUSES += CBM2
#@src/emu/bus/cbm2/user.h,BUSES += CBM2
#-------------------------------------------------

ifneq ($(filter CBM2,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/cbm2
BUSOBJS += $(BUSOBJ)/cbm2/exp.o
BUSOBJS += $(BUSOBJ)/cbm2/24k.o
BUSOBJS += $(BUSOBJ)/cbm2/hrg.o
BUSOBJS += $(BUSOBJ)/cbm2/std.o
BUSOBJS += $(BUSOBJ)/cbm2/user.o
endif


#-------------------------------------------------
#
#@src/emu/bus/cbmiec/cbmiec.h,BUSES += CBMIEC
#-------------------------------------------------

ifneq ($(filter CBMIEC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/cbmiec
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
#@src/emu/bus/chanf/slot.h,BUSES += CHANNELF
#-------------------------------------------------

ifneq ($(filter CHANNELF,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/chanf
BUSOBJS += $(BUSOBJ)/chanf/slot.o
BUSOBJS += $(BUSOBJ)/chanf/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/comx35/exp.h,BUSES += COMX35
#-------------------------------------------------

ifneq ($(filter COMX35,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/comx35
BUSOBJS += $(BUSOBJ)/comx35/exp.o
BUSOBJS += $(BUSOBJ)/comx35/clm.o
BUSOBJS += $(BUSOBJ)/comx35/expbox.o
BUSOBJS += $(BUSOBJ)/comx35/eprom.o
BUSOBJS += $(BUSOBJ)/comx35/fdc.o
BUSOBJS += $(BUSOBJ)/comx35/joycard.o
BUSOBJS += $(BUSOBJ)/comx35/printer.o
BUSOBJS += $(BUSOBJ)/comx35/ram.o
BUSOBJS += $(BUSOBJ)/comx35/thermal.o
endif


#-------------------------------------------------
#
#@src/emu/bus/coleco/ctrl.h,BUSES += COLECO
#-------------------------------------------------

ifneq ($(filter COLECO,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/coleco
BUSOBJS += $(BUSOBJ)/coleco/ctrl.o
BUSOBJS += $(BUSOBJ)/coleco/hand.o
BUSOBJS += $(BUSOBJ)/coleco/sac.o
BUSOBJS += $(BUSOBJ)/coleco/exp.o
BUSOBJS += $(BUSOBJ)/coleco/std.o
endif


#-------------------------------------------------
#
#@src/emu/bus/crvision/slot.h,BUSES += CRVISION
#-------------------------------------------------

ifneq ($(filter CRVISION,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/crvision
BUSOBJS += $(BUSOBJ)/crvision/slot.o
BUSOBJS += $(BUSOBJ)/crvision/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/dmv/dmv.h,BUSES += DMV
#-------------------------------------------------

ifneq ($(filter DMV,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/dmv
BUSOBJS += $(BUSOBJ)/dmv/dmvbus.o
BUSOBJS += $(BUSOBJ)/dmv/k220.o
BUSOBJS += $(BUSOBJ)/dmv/k230.o
BUSOBJS += $(BUSOBJ)/dmv/k233.o
BUSOBJS += $(BUSOBJ)/dmv/k806.o
BUSOBJS += $(BUSOBJ)/dmv/ram.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ecbbus/ecbbus.h,BUSES += ECBBUS
#-------------------------------------------------

ifneq ($(filter ECBBUS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/ecbbus
BUSOBJS += $(BUSOBJ)/ecbbus/ecbbus.o
BUSOBJS += $(BUSOBJ)/ecbbus/grip.o
endif


#-------------------------------------------------
#
#@src/emu/bus/econet/econet.h,BUSES += ECONET
#-------------------------------------------------

ifneq ($(filter ECONET,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/econet
BUSOBJS += $(BUSOBJ)/econet/econet.o
BUSOBJS += $(BUSOBJ)/econet/e01.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ep64/exp.h,BUSES += EP64
#-------------------------------------------------

ifneq ($(filter EP64,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/ep64
BUSOBJS += $(BUSOBJ)/ep64/exp.o
BUSOBJS += $(BUSOBJ)/ep64/exdos.o
endif


#-------------------------------------------------
#
#@src/emu/bus/generic/slot.h,BUSES += GENERIC
#-------------------------------------------------

ifneq ($(filter GENERIC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/generic
BUSOBJS += $(BUSOBJ)/generic/slot.o
BUSOBJS += $(BUSOBJ)/generic/carts.o
BUSOBJS += $(BUSOBJ)/generic/ram.o
BUSOBJS += $(BUSOBJ)/generic/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ieee488/ieee488.h,BUSES += IEEE488
#-------------------------------------------------

ifneq ($(filter IEEE488,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/ieee488
BUSOBJS += $(BUSOBJ)/ieee488/ieee488.o
BUSOBJS += $(BUSOBJ)/ieee488/c2031.o
BUSOBJS += $(BUSOBJ)/ieee488/c2040.o
BUSOBJS += $(BUSOBJ)/ieee488/c2040fdc.o
BUSOBJS += $(BUSOBJ)/ieee488/c8050.o
BUSOBJS += $(BUSOBJ)/ieee488/c8280.o
BUSOBJS += $(BUSOBJ)/ieee488/d9060.o
BUSOBJS += $(BUSOBJ)/ieee488/softbox.o
BUSOBJS += $(BUSOBJ)/ieee488/hardbox.o
BUSOBJS += $(BUSOBJ)/ieee488/shark.o
endif


#-------------------------------------------------
#
#@src/emu/bus/iq151/iq151.h,BUSES += IQ151
#-------------------------------------------------

ifneq ($(filter IQ151,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/iq151
BUSOBJS += $(BUSOBJ)/iq151/iq151.o
BUSOBJS += $(BUSOBJ)/iq151/disc2.o
BUSOBJS += $(BUSOBJ)/iq151/grafik.o
BUSOBJS += $(BUSOBJ)/iq151/minigraf.o
BUSOBJS += $(BUSOBJ)/iq151/ms151a.o
BUSOBJS += $(BUSOBJ)/iq151/rom.o
BUSOBJS += $(BUSOBJ)/iq151/staper.o
BUSOBJS += $(BUSOBJ)/iq151/video32.o
BUSOBJS += $(BUSOBJ)/iq151/video64.o
endif


#-------------------------------------------------
#
#@src/emu/bus/isbx/isbx.h,BUSES += IMI7000
#-------------------------------------------------

ifneq ($(filter IMI7000,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/imi7000
BUSOBJS += $(BUSOBJ)/imi7000/imi7000.o
BUSOBJS += $(BUSOBJ)/imi7000/imi5000h.o
endif


#-------------------------------------------------
#
#@src/emu/bus/intv/slot.h,BUSES += INTV
#-------------------------------------------------

ifneq ($(filter INTV,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/intv
BUSOBJS += $(BUSOBJ)/intv/slot.o
BUSOBJS += $(BUSOBJ)/intv/rom.o
BUSOBJS += $(BUSOBJ)/intv/voice.o
BUSOBJS += $(BUSOBJ)/intv/ecs.o
endif


#-------------------------------------------------
#
#@src/emu/bus/isa/isa.h,BUSES += ISA
#-------------------------------------------------

ifneq ($(filter ISA,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/isa
BUSOBJS += $(BUSOBJ)/isa/isa.o
BUSOBJS += $(BUSOBJ)/isa/isa_cards.o
BUSOBJS += $(BUSOBJ)/isa/mda.o
BUSOBJS += $(BUSOBJ)/isa/wdxt_gen.o
BUSOBJS += $(BUSOBJ)/isa/adlib.o
BUSOBJS += $(BUSOBJ)/isa/com.o
BUSOBJS += $(BUSOBJ)/isa/fdc.o
BUSOBJS += $(BUSOBJ)/isa/mufdc.o
BUSOBJS += $(BUSOBJ)/isa/finalchs.o
BUSOBJS += $(BUSOBJ)/isa/gblaster.o
BUSOBJS += $(BUSOBJ)/isa/gus.o
BUSOBJS += $(BUSOBJ)/isa/sb16.o
BUSOBJS += $(BUSOBJ)/isa/hdc.o
BUSOBJS += $(BUSOBJ)/isa/ibm_mfc.o
BUSOBJS += $(BUSOBJ)/isa/mpu401.o
BUSOBJS += $(BUSOBJ)/isa/sblaster.o
BUSOBJS += $(BUSOBJ)/isa/stereo_fx.o
BUSOBJS += $(BUSOBJ)/isa/ssi2001.o
BUSOBJS += $(BUSOBJ)/isa/ide.o
BUSOBJS += $(BUSOBJ)/isa/xtide.o
BUSOBJS += $(BUSOBJ)/isa/side116.o
BUSOBJS += $(BUSOBJ)/isa/aha1542.o
BUSOBJS += $(BUSOBJ)/isa/wd1002a_wx1.o
BUSOBJS += $(BUSOBJ)/isa/dectalk.o
BUSOBJS += $(BUSOBJ)/isa/pds.o
BUSOBJS += $(BUSOBJ)/isa/omti8621.o
BUSOBJS += $(BUSOBJ)/isa/cga.o
BUSOBJS += $(BUSOBJ)/isa/svga_cirrus.o
BUSOBJS += $(BUSOBJ)/isa/ega.o
BUSOBJS += $(BUSOBJ)/isa/vga.o
BUSOBJS += $(BUSOBJ)/isa/vga_ati.o
BUSOBJS += $(BUSOBJ)/isa/mach32.o
BUSOBJS += $(BUSOBJ)/isa/svga_tseng.o
BUSOBJS += $(BUSOBJ)/isa/svga_s3.o
BUSOBJS += $(BUSOBJ)/isa/s3virge.o
BUSOBJS += $(BUSOBJ)/isa/pc1640_iga.o
BUSOBJS += $(BUSOBJ)/isa/3c503.o
BUSOBJS += $(BUSOBJ)/isa/ne1000.o
BUSOBJS += $(BUSOBJ)/isa/ne2000.o
BUSOBJS += $(BUSOBJ)/isa/3c505.o
BUSOBJS += $(BUSOBJ)/isa/lpt.o
BUSOBJS += $(BUSOBJ)/isa/p1_fdc.o
BUSOBJS += $(BUSOBJ)/isa/p1_hdc.o
BUSOBJS += $(BUSOBJ)/isa/p1_rom.o
BUSOBJS += $(BUSOBJ)/isa/mc1502_fdc.o
BUSOBJS += $(BUSOBJ)/isa/mc1502_rom.o
BUSOBJS += $(BUSOBJ)/isa/xsu_cards.o
BUSOBJS += $(BUSOBJ)/isa/sc499.o
BUSOBJS += $(BUSOBJ)/isa/aga.o
BUSOBJS += $(BUSOBJ)/isa/svga_trident.o
endif

#-------------------------------------------------
#
#@src/emu/bus/isbx/isbx.h,BUSES += ISBX
#-------------------------------------------------

ifneq ($(filter ISBX,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/isbx
BUSOBJS += $(BUSOBJ)/isbx/isbx.o
BUSOBJS += $(BUSOBJ)/isbx/compis_fdc.o
BUSOBJS += $(BUSOBJ)/isbx/isbc_218a.o
endif


#-------------------------------------------------
#
#@src/emu/bus/msx_slot/slot.h,BUSES += MSX_SLOT
#-------------------------------------------------

ifneq ($(filter MSX_SLOT,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/msx_slot
BUSOBJS += $(BUSOBJ)/msx_slot/bunsetsu.o
BUSOBJS += $(BUSOBJ)/msx_slot/cartridge.o
BUSOBJS += $(BUSOBJ)/msx_slot/disk.o
BUSOBJS += $(BUSOBJ)/msx_slot/fs4600.o
BUSOBJS += $(BUSOBJ)/msx_slot/music.o
BUSOBJS += $(BUSOBJ)/msx_slot/panasonic08.o
BUSOBJS += $(BUSOBJ)/msx_slot/rom.o
BUSOBJS += $(BUSOBJ)/msx_slot/ram.o
BUSOBJS += $(BUSOBJ)/msx_slot/ram_mm.o
BUSOBJS += $(BUSOBJ)/msx_slot/slot.o
BUSOBJS += $(BUSOBJ)/msx_slot/sony08.o
OBJDIRS += $(BUSOBJ)/msx_cart
BUSOBJS += $(BUSOBJ)/msx_cart/arc.o
BUSOBJS += $(BUSOBJ)/msx_cart/ascii.o
BUSOBJS += $(BUSOBJ)/msx_cart/bm_012.o
BUSOBJS += $(BUSOBJ)/msx_cart/cartridge.o
BUSOBJS += $(BUSOBJ)/msx_cart/crossblaim.o
BUSOBJS += $(BUSOBJ)/msx_cart/disk.o
BUSOBJS += $(BUSOBJ)/msx_cart/dooly.o
BUSOBJS += $(BUSOBJ)/msx_cart/fmpac.o
BUSOBJS += $(BUSOBJ)/msx_cart/halnote.o
BUSOBJS += $(BUSOBJ)/msx_cart/hfox.o
BUSOBJS += $(BUSOBJ)/msx_cart/holy_quran.o
BUSOBJS += $(BUSOBJ)/msx_cart/konami.o
BUSOBJS += $(BUSOBJ)/msx_cart/korean.o
BUSOBJS += $(BUSOBJ)/msx_cart/majutsushi.o
BUSOBJS += $(BUSOBJ)/msx_cart/msx_audio.o
BUSOBJS += $(BUSOBJ)/msx_cart/msx_audio_kb.o
BUSOBJS += $(BUSOBJ)/msx_cart/msxdos2.o
BUSOBJS += $(BUSOBJ)/msx_cart/nomapper.o
BUSOBJS += $(BUSOBJ)/msx_cart/rtype.o
BUSOBJS += $(BUSOBJ)/msx_cart/superloderunner.o
BUSOBJS += $(BUSOBJ)/msx_cart/super_swangi.o
BUSOBJS += $(BUSOBJ)/msx_cart/yamaha.o
endif


#-------------------------------------------------
#
#@src/emu/bus/kc/kc.h,BUSES += KC
#-------------------------------------------------

ifneq ($(filter KC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/kc
BUSOBJS += $(BUSOBJ)/kc/kc.o
BUSOBJS += $(BUSOBJ)/kc/d002.o
BUSOBJS += $(BUSOBJ)/kc/d004.o
BUSOBJS += $(BUSOBJ)/kc/ram.o
BUSOBJS += $(BUSOBJ)/kc/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/odyssey2/slot.h,BUSES += O2
#-------------------------------------------------

ifneq ($(filter O2,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/odyssey2
BUSOBJS += $(BUSOBJ)/odyssey2/slot.o
BUSOBJS += $(BUSOBJ)/odyssey2/rom.o
BUSOBJS += $(BUSOBJ)/odyssey2/chess.o
BUSOBJS += $(BUSOBJ)/odyssey2/voice.o
endif


#-------------------------------------------------
#
#@src/emu/bus/pc_joy/pc_joy.h,BUSES += PC_JOY
#-------------------------------------------------

ifneq ($(filter PC_JOY,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/pc_joy
BUSOBJS += $(BUSOBJ)/pc_joy/pc_joy.o
BUSOBJS += $(BUSOBJ)/pc_joy/pc_joy_sw.o
endif


#-------------------------------------------------
#
#@src/emu/bus/pc_kbd/pc_kbdc.h,BUSES += PC_KBD
#-------------------------------------------------

ifneq ($(filter PC_KBD,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/pc_kbd
BUSOBJS += $(BUSOBJ)/pc_kbd/pc_kbdc.o
BUSOBJS += $(BUSOBJ)/pc_kbd/keyboards.o
BUSOBJS += $(BUSOBJ)/pc_kbd/ec1841.o
BUSOBJS += $(BUSOBJ)/pc_kbd/iskr1030.o
BUSOBJS += $(BUSOBJ)/pc_kbd/keytro.o
BUSOBJS += $(BUSOBJ)/pc_kbd/msnat.o
BUSOBJS += $(BUSOBJ)/pc_kbd/pc83.o
BUSOBJS += $(BUSOBJ)/pc_kbd/pcat84.o
BUSOBJS += $(BUSOBJ)/pc_kbd/pcxt83.o
endif


#-------------------------------------------------
#
#@src/emu/bus/pet/cass.h,BUSES += PET
#@src/emu/bus/pet/exp.h,BUSES += PET
#@src/emu/bus/pet/user.h,BUSES += PET
#-------------------------------------------------

ifneq ($(filter PET,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/pet
BUSOBJS += $(BUSOBJ)/pet/cass.o
BUSOBJS += $(BUSOBJ)/pet/c2n.o
BUSOBJS += $(BUSOBJ)/pet/diag264_lb_tape.o
BUSOBJS += $(BUSOBJ)/pet/exp.o
BUSOBJS += $(BUSOBJ)/pet/64k.o
BUSOBJS += $(BUSOBJ)/pet/superpet.o
BUSOBJS += $(BUSOBJ)/pet/user.o
BUSOBJS += $(BUSOBJ)/pet/diag.o
BUSOBJS += $(BUSOBJ)/pet/petuja.o
BUSOBJS += $(BUSOBJ)/pet/cb2snd.o
endif


#-------------------------------------------------
#
#@src/emu/bus/plus4/exp.h,BUSES += PLUS4
#@src/emu/bus/plus4/user.h,BUSES += PLUS4
#-------------------------------------------------

ifneq ($(filter PLUS4,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/plus4
BUSOBJS += $(BUSOBJ)/plus4/exp.o
BUSOBJS += $(BUSOBJ)/plus4/c1551.o
BUSOBJS += $(BUSOBJ)/plus4/sid.o
BUSOBJS += $(BUSOBJ)/plus4/std.o
BUSOBJS += $(BUSOBJ)/plus4/user.o
BUSOBJS += $(BUSOBJ)/plus4/diag264_lb_user.o
endif


#-------------------------------------------------
#
#@src/emu/bus/s100/s100.h,BUSES += S100
#-------------------------------------------------

ifneq ($(filter S100,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/s100
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
#@src/emu/bus/spc1000/exp.h,BUSES += SPC1000
#-------------------------------------------------

ifneq ($(filter SPC1000,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/spc1000
BUSOBJS += $(BUSOBJ)/spc1000/exp.o
BUSOBJS += $(BUSOBJ)/spc1000/fdd.o
BUSOBJS += $(BUSOBJ)/spc1000/vdp.o
endif


#-------------------------------------------------
#
#@src/emu/bus/tvc/tvc.h,BUSES += TVC
#-------------------------------------------------

ifneq ($(filter TVC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/tvc
BUSOBJS += $(BUSOBJ)/tvc/tvc.o
BUSOBJS += $(BUSOBJ)/tvc/hbf.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vc4000/slot.h,BUSES += VC4000
#-------------------------------------------------

ifneq ($(filter VC4000,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vc4000
BUSOBJS += $(BUSOBJ)/vc4000/slot.o
BUSOBJS += $(BUSOBJ)/vc4000/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vcs/vcs_slot.h,BUSES += VCS
#-------------------------------------------------

ifneq ($(filter VCS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vcs
BUSOBJS += $(BUSOBJ)/vcs/vcs_slot.o
BUSOBJS += $(BUSOBJ)/vcs/rom.o
BUSOBJS += $(BUSOBJ)/vcs/compumat.o
BUSOBJS += $(BUSOBJ)/vcs/dpc.o
BUSOBJS += $(BUSOBJ)/vcs/scharger.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vcs/ctrl.h,BUSES += VCS_CTRL
#-------------------------------------------------

ifneq ($(filter VCS_CTRL,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vcs_ctrl
BUSOBJS += $(BUSOBJ)/vcs_ctrl/ctrl.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/joystick.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/joybooster.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/keypad.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/lightpen.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/paddles.o
BUSOBJS += $(BUSOBJ)/vcs_ctrl/wheel.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vectrex/slot.h,BUSES += VECTREX
#-------------------------------------------------

ifneq ($(filter VECTREX,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vectrex
BUSOBJS += $(BUSOBJ)/vectrex/slot.o
BUSOBJS += $(BUSOBJ)/vectrex/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vic10/exp.h,BUSES += VIC10
#-------------------------------------------------

ifneq ($(filter VIC10,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vic10
BUSOBJS += $(BUSOBJ)/vic10/exp.o
BUSOBJS += $(BUSOBJ)/vic10/std.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vic20/exp.h,BUSES += VIC20
#@src/emu/bus/vic20/user.h,BUSES += VIC20
#-------------------------------------------------

ifneq ($(filter VIC20,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vic20
BUSOBJS += $(BUSOBJ)/vic20/exp.o
BUSOBJS += $(BUSOBJ)/vic20/fe3.o
BUSOBJS += $(BUSOBJ)/vic20/megacart.o
BUSOBJS += $(BUSOBJ)/vic20/std.o
BUSOBJS += $(BUSOBJ)/vic20/vic1010.o
BUSOBJS += $(BUSOBJ)/vic20/vic1110.o
BUSOBJS += $(BUSOBJ)/vic20/vic1111.o
BUSOBJS += $(BUSOBJ)/vic20/vic1112.o
BUSOBJS += $(BUSOBJ)/vic20/vic1210.o
BUSOBJS += $(BUSOBJ)/vic20/user.o
BUSOBJS += $(BUSOBJ)/vic20/4cga.o
BUSOBJS += $(BUSOBJ)/vic20/vic1011.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vidbrain/exp.h,BUSES += VIDBRAIN
#-------------------------------------------------

ifneq ($(filter VIDBRAIN,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vidbrain
BUSOBJS += $(BUSOBJ)/vidbrain/exp.o
BUSOBJS += $(BUSOBJ)/vidbrain/std.o
BUSOBJS += $(BUSOBJ)/vidbrain/money_minder.o
BUSOBJS += $(BUSOBJ)/vidbrain/timeshare.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vip/byteio.h,BUSES += VIP
#@src/emu/bus/vip/exp.h,BUSES += VIP
#-------------------------------------------------

ifneq ($(filter VIP,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vip
BUSOBJS += $(BUSOBJ)/vip/byteio.o
BUSOBJS += $(BUSOBJ)/vip/vp620.o
BUSOBJS += $(BUSOBJ)/vip/exp.o
BUSOBJS += $(BUSOBJ)/vip/vp550.o
BUSOBJS += $(BUSOBJ)/vip/vp570.o
BUSOBJS += $(BUSOBJ)/vip/vp575.o
BUSOBJS += $(BUSOBJ)/vip/vp585.o
BUSOBJS += $(BUSOBJ)/vip/vp590.o
BUSOBJS += $(BUSOBJ)/vip/vp595.o
BUSOBJS += $(BUSOBJ)/vip/vp700.o
endif


#-------------------------------------------------
#
#@src/emu/bus/wangpc/wangpc.h,BUSES += WANGPC
#-------------------------------------------------

ifneq ($(filter WANGPC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/wangpc
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


#-------------------------------------------------
#
#@src/emu/bus/z88/z88.h,BUSES += Z88
#-------------------------------------------------

ifneq ($(filter Z88,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/z88
BUSOBJS += $(BUSOBJ)/z88/z88.o
BUSOBJS += $(BUSOBJ)/z88/flash.o
BUSOBJS += $(BUSOBJ)/z88/ram.o
BUSOBJS += $(BUSOBJ)/z88/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/a2bus/a2bus.h,BUSES += A2BUS
#-------------------------------------------------

ifneq ($(filter A2BUS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/a2bus
BUSOBJS += $(BUSOBJ)/a2bus/a2bus.o
BUSOBJS += $(BUSOBJ)/a2bus/a2lang.o
BUSOBJS += $(BUSOBJ)/a2bus/a2diskii.o
BUSOBJS += $(BUSOBJ)/a2bus/a2mockingboard.o
BUSOBJS += $(BUSOBJ)/a2bus/a2cffa.o
BUSOBJS += $(BUSOBJ)/a2bus/a2memexp.o
BUSOBJS += $(BUSOBJ)/a2bus/a2scsi.o
BUSOBJS += $(BUSOBJ)/a2bus/a2thunderclock.o
BUSOBJS += $(BUSOBJ)/a2bus/a2softcard.o
BUSOBJS += $(BUSOBJ)/a2bus/a2videoterm.o
BUSOBJS += $(BUSOBJ)/a2bus/a2ssc.o
BUSOBJS += $(BUSOBJ)/a2bus/a2swyft.o
BUSOBJS += $(BUSOBJ)/a2bus/a2eauxslot.o
BUSOBJS += $(BUSOBJ)/a2bus/a2themill.o
BUSOBJS += $(BUSOBJ)/a2bus/a2sam.o
BUSOBJS += $(BUSOBJ)/a2bus/a2alfam2.o
BUSOBJS += $(BUSOBJ)/a2bus/laser128.o
BUSOBJS += $(BUSOBJ)/a2bus/a2echoii.o
BUSOBJS += $(BUSOBJ)/a2bus/a2arcadebd.o
BUSOBJS += $(BUSOBJ)/a2bus/a2midi.o
BUSOBJS += $(BUSOBJ)/a2bus/a2vulcan.o
BUSOBJS += $(BUSOBJ)/a2bus/a2zipdrive.o
BUSOBJS += $(BUSOBJ)/a2bus/a2applicard.o
BUSOBJS += $(BUSOBJ)/a2bus/a2hsscsi.o
BUSOBJS += $(BUSOBJ)/a2bus/a2ultraterm.o
BUSOBJS += $(BUSOBJ)/a2bus/a2pic.o
BUSOBJS += $(BUSOBJ)/a2bus/a2estd80col.o
BUSOBJS += $(BUSOBJ)/a2bus/a2eext80col.o
BUSOBJS += $(BUSOBJ)/a2bus/a2eramworks3.o
BUSOBJS += $(BUSOBJ)/a2bus/a2corvus.o
BUSOBJS += $(BUSOBJ)/a2bus/a2diskiing.o
BUSOBJS += $(BUSOBJ)/a2bus/a2mcms.o
BUSOBJS += $(BUSOBJ)/a2bus/a2dx1.o
BUSOBJS += $(BUSOBJ)/a2bus/timemasterho.o
BUSOBJS += $(BUSOBJ)/a2bus/mouse.o
BUSOBJS += $(BUSOBJ)/a2bus/corvfdc01.o
BUSOBJS += $(BUSOBJ)/a2bus/corvfdc02.o
endif

#-------------------------------------------------
#
#@src/emu/bus/nubus/nubus.h,BUSES += NUBUS
#-------------------------------------------------

ifneq ($(filter NUBUS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/nubus
BUSOBJS += $(BUSOBJ)/nubus/nubus.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_48gc.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_cb264.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_vikbw.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_specpdq.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_m2hires.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_spec8.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_radiustpd.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_m2video.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_asntmc3b.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_image.o
BUSOBJS += $(BUSOBJ)/nubus/nubus_wsportrait.o
BUSOBJS += $(BUSOBJ)/nubus/pds30_cb264.o
BUSOBJS += $(BUSOBJ)/nubus/pds30_procolor816.o
BUSOBJS += $(BUSOBJ)/nubus/pds30_sigmalview.o
BUSOBJS += $(BUSOBJ)/nubus/pds30_30hr.o
BUSOBJS += $(BUSOBJ)/nubus/pds30_mc30.o
endif

#-------------------------------------------------
#
#@src/emu/bus/centronics/ctronics.h,BUSES += CENTRONICS
#-------------------------------------------------

ifneq ($(filter CENTRONICS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/centronics
BUSOBJS += $(BUSOBJ)/centronics/ctronics.o
BUSOBJS += $(BUSOBJ)/centronics/comxpl80.o
BUSOBJS += $(BUSOBJ)/centronics/covox.o
BUSOBJS += $(BUSOBJ)/centronics/dsjoy.o
BUSOBJS += $(BUSOBJ)/centronics/epson_ex800.o
BUSOBJS += $(BUSOBJ)/centronics/epson_lx800.o
BUSOBJS += $(BUSOBJ)/centronics/printer.o
BUSOBJS += $(BUSOBJ)/centronics/digiblst.o
$(BUSOBJ)/centronics/epson_ex800.o:    $(EMUOBJ)/layout/ex800.lh
$(BUSOBJ)/centronics/epson_lx800.o:    $(EMUOBJ)/layout/lx800.lh
endif

#-------------------------------------------------
#
#@src/emu/bus/rs232/rs232.h,BUSES += RS232
#-------------------------------------------------

ifneq ($(filter RS232,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/rs232
BUSOBJS += $(BUSOBJ)/rs232/keyboard.o
BUSOBJS += $(BUSOBJ)/rs232/loopback.o
BUSOBJS += $(BUSOBJ)/rs232/null_modem.o
BUSOBJS += $(BUSOBJ)/rs232/printer.o
BUSOBJS += $(BUSOBJ)/rs232/rs232.o
BUSOBJS += $(BUSOBJ)/rs232/ser_mouse.o
BUSOBJS += $(BUSOBJ)/rs232/terminal.o
BUSOBJS += $(BUSOBJ)/rs232/xvd701.o
endif

#-------------------------------------------------
#
#@src/emu/bus/midi/midi.h,BUSES += MIDI
#-------------------------------------------------

ifneq ($(filter MIDI,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/midi
BUSOBJS += $(BUSOBJ)/midi/midi.o
BUSOBJS += $(BUSOBJ)/midi/midiinport.o
BUSOBJS += $(BUSOBJ)/midi/midioutport.o
endif

#-------------------------------------------------
#
#@src/emu/bus/pci/pci.h,BUSES += PCI
#-------------------------------------------------

ifneq ($(filter PCI,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/pci
BUSOBJS += $(BUSOBJ)/pci/pci.o
BUSOBJS += $(BUSOBJ)/pci/cirrus.o
BUSOBJS += $(BUSOBJ)/pci/i82371ab.o
BUSOBJS += $(BUSOBJ)/pci/i82371sb.o
BUSOBJS += $(BUSOBJ)/pci/i82439tx.o
BUSOBJS += $(BUSOBJ)/pci/northbridge.o
BUSOBJS += $(BUSOBJ)/pci/southbridge.o
BUSOBJS += $(BUSOBJ)/pci/mpc105.o
endif

#-------------------------------------------------
#
#@src/emu/bus/nes/nes_slot.h,BUSES += NES
#-------------------------------------------------

ifneq ($(filter NES,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/nes
BUSOBJS += $(BUSOBJ)/nes/nes_slot.o
BUSOBJS += $(BUSOBJ)/nes/nes_carts.o
BUSOBJS += $(BUSOBJ)/nes/2a03pur.o
BUSOBJS += $(BUSOBJ)/nes/act53.o
BUSOBJS += $(BUSOBJ)/nes/aladdin.o
BUSOBJS += $(BUSOBJ)/nes/ave.o
BUSOBJS += $(BUSOBJ)/nes/bandai.o
BUSOBJS += $(BUSOBJ)/nes/benshieng.o
BUSOBJS += $(BUSOBJ)/nes/bootleg.o
BUSOBJS += $(BUSOBJ)/nes/camerica.o
BUSOBJS += $(BUSOBJ)/nes/cne.o
BUSOBJS += $(BUSOBJ)/nes/cony.o
BUSOBJS += $(BUSOBJ)/nes/datach.o
BUSOBJS += $(BUSOBJ)/nes/discrete.o
BUSOBJS += $(BUSOBJ)/nes/disksys.o
BUSOBJS += $(BUSOBJ)/nes/event.o
BUSOBJS += $(BUSOBJ)/nes/ggenie.o
BUSOBJS += $(BUSOBJ)/nes/henggedianzi.o
BUSOBJS += $(BUSOBJ)/nes/hes.o
BUSOBJS += $(BUSOBJ)/nes/hosenkan.o
BUSOBJS += $(BUSOBJ)/nes/irem.o
BUSOBJS += $(BUSOBJ)/nes/jaleco.o
BUSOBJS += $(BUSOBJ)/nes/jy.o
BUSOBJS += $(BUSOBJ)/nes/kaiser.o
BUSOBJS += $(BUSOBJ)/nes/karastudio.o
BUSOBJS += $(BUSOBJ)/nes/konami.o
BUSOBJS += $(BUSOBJ)/nes/legacy.o
BUSOBJS += $(BUSOBJ)/nes/mmc1.o
BUSOBJS += $(BUSOBJ)/nes/mmc2.o
BUSOBJS += $(BUSOBJ)/nes/mmc3.o
BUSOBJS += $(BUSOBJ)/nes/mmc3_clones.o
BUSOBJS += $(BUSOBJ)/nes/mmc5.o
BUSOBJS += $(BUSOBJ)/nes/multigame.o
BUSOBJS += $(BUSOBJ)/nes/namcot.o
BUSOBJS += $(BUSOBJ)/nes/nanjing.o
BUSOBJS += $(BUSOBJ)/nes/ntdec.o
BUSOBJS += $(BUSOBJ)/nes/nxrom.o
BUSOBJS += $(BUSOBJ)/nes/pirate.o
BUSOBJS += $(BUSOBJ)/nes/pt554.o
BUSOBJS += $(BUSOBJ)/nes/racermate.o
BUSOBJS += $(BUSOBJ)/nes/rcm.o
BUSOBJS += $(BUSOBJ)/nes/rexsoft.o
BUSOBJS += $(BUSOBJ)/nes/sachen.o
BUSOBJS += $(BUSOBJ)/nes/somari.o
BUSOBJS += $(BUSOBJ)/nes/sunsoft.o
BUSOBJS += $(BUSOBJ)/nes/sunsoft_dcs.o
BUSOBJS += $(BUSOBJ)/nes/taito.o
BUSOBJS += $(BUSOBJ)/nes/tengen.o
BUSOBJS += $(BUSOBJ)/nes/txc.o
BUSOBJS += $(BUSOBJ)/nes/waixing.o
endif

#-------------------------------------------------
#
#@src/emu/bus/snes/snes_slot.h,BUSES += SNES
#-------------------------------------------------

ifneq ($(filter SNES,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/snes
BUSOBJS += $(BUSOBJ)/snes/snes_slot.o
BUSOBJS += $(BUSOBJ)/snes/snes_carts.o
BUSOBJS += $(BUSOBJ)/snes/bsx.o
BUSOBJS += $(BUSOBJ)/snes/event.o
BUSOBJS += $(BUSOBJ)/snes/rom.o
BUSOBJS += $(BUSOBJ)/snes/rom21.o
BUSOBJS += $(BUSOBJ)/snes/sa1.o
BUSOBJS += $(BUSOBJ)/snes/sdd1.o
BUSOBJS += $(BUSOBJ)/snes/sfx.o
BUSOBJS += $(BUSOBJ)/snes/sgb.o
BUSOBJS += $(BUSOBJ)/snes/spc7110.o
BUSOBJS += $(BUSOBJ)/snes/sufami.o
BUSOBJS += $(BUSOBJ)/snes/upd.o
endif

#-------------------------------------------------
#
#@src/emu/bus/vboy/slot.h,BUSES += VBOY
#-------------------------------------------------
ifneq ($(filter VBOY,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vboy
BUSOBJS += $(BUSOBJ)/vboy/slot.o
BUSOBJS += $(BUSOBJ)/vboy/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/megadrive/md_slot.h,BUSES += MEGADRIVE
#-------------------------------------------------

ifneq ($(filter MEGADRIVE,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/megadrive
BUSOBJS += $(BUSOBJ)/megadrive/md_slot.o
BUSOBJS += $(BUSOBJ)/megadrive/md_carts.o
BUSOBJS += $(BUSOBJ)/megadrive/eeprom.o
BUSOBJS += $(BUSOBJ)/megadrive/ggenie.o
BUSOBJS += $(BUSOBJ)/megadrive/jcart.o
BUSOBJS += $(BUSOBJ)/megadrive/rom.o
BUSOBJS += $(BUSOBJ)/megadrive/sk.o
BUSOBJS += $(BUSOBJ)/megadrive/stm95.o
BUSOBJS += $(BUSOBJ)/megadrive/svp.o
endif

#-------------------------------------------------
#
#@src/emu/bus/neogeo/neogeo_slot.h,BUSES += NEOGEO
#-------------------------------------------------

ifneq ($(filter NEOGEO,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/neogeo
BUSOBJS += $(BUSOBJ)/neogeo/neogeo_slot.o
BUSOBJS += $(BUSOBJ)/neogeo/neogeo_intf.o
BUSOBJS += $(BUSOBJ)/neogeo/neogeo_carts.o
BUSOBJS += $(BUSOBJ)/neogeo/neogeo_helper.o
BUSOBJS += $(BUSOBJ)/neogeo/banked_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/mslugx_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/mslugx_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/sma_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/sma_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/cmc_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/cmc_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/pcm2_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/pcm2_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/kof2002_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/kof2002_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/pvc_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/pvc_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/fatfury2_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/fatfury2_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/kof98_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/kof98_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/bootleg_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/bootleg_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/bootleg_hybrid_cart.o
BUSOBJS += $(BUSOBJ)/neogeo/sbp_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/kog_prot.o
BUSOBJS += $(BUSOBJ)/neogeo/rom.o
endif


#-------------------------------------------------
#
#@src/emu/bus/saturn/sat_slot.h,BUSES += SATURN
#-------------------------------------------------

ifneq ($(filter SATURN,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/saturn
BUSOBJS += $(BUSOBJ)/saturn/sat_slot.o
BUSOBJS += $(BUSOBJ)/saturn/bram.o
BUSOBJS += $(BUSOBJ)/saturn/dram.o
BUSOBJS += $(BUSOBJ)/saturn/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/sega8/sega8_slot.h,BUSES += SEGA8
#-------------------------------------------------

ifneq ($(filter SEGA8,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/sega8
BUSOBJS += $(BUSOBJ)/sega8/sega8_slot.o
BUSOBJS += $(BUSOBJ)/sega8/rom.o
BUSOBJS += $(BUSOBJ)/sega8/ccatch.o
BUSOBJS += $(BUSOBJ)/sega8/mgear.o
endif

#-------------------------------------------------
#
#@src/emu/bus/sms_ctrl/smsctrl.h,BUSES += SMS_CTRL
#-------------------------------------------------

ifneq ($(filter SMS_CTRL,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/sms_ctrl
BUSOBJS += $(BUSOBJ)/sms_ctrl/smsctrl.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/joypad.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/lphaser.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/paddle.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/rfu.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/sports.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/sportsjp.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/multitap.o
BUSOBJS += $(BUSOBJ)/sms_ctrl/graphic.o
endif

#-------------------------------------------------
#
#@src/emu/bus/sms_exp/smsexp.h,BUSES += SMS_EXP
#-------------------------------------------------

ifneq ($(filter SMS_EXP,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/sms_exp
BUSOBJS += $(BUSOBJ)/sms_exp/smsexp.o
BUSOBJS += $(BUSOBJ)/sms_exp/gender.o
endif

#-------------------------------------------------
#
#@src/emu/bus/ti99_peb/peribox.h,BUSES += TI99PEB
#-------------------------------------------------

ifneq ($(filter TI99PEB,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/ti99_peb
BUSOBJS += $(BUSOBJ)/ti99_peb/peribox.o
BUSOBJS += $(BUSOBJ)/ti99_peb/bwg.o
BUSOBJS += $(BUSOBJ)/ti99_peb/evpc.o
BUSOBJS += $(BUSOBJ)/ti99_peb/hfdc.o
BUSOBJS += $(BUSOBJ)/ti99_peb/horizon.o
BUSOBJS += $(BUSOBJ)/ti99_peb/hsgpl.o
BUSOBJS += $(BUSOBJ)/ti99_peb/memex.o
BUSOBJS += $(BUSOBJ)/ti99_peb/myarcmem.o
BUSOBJS += $(BUSOBJ)/ti99_peb/pcode.o
BUSOBJS += $(BUSOBJ)/ti99_peb/samsmem.o
BUSOBJS += $(BUSOBJ)/ti99_peb/spchsyn.o
BUSOBJS += $(BUSOBJ)/ti99_peb/ti_32kmem.o
BUSOBJS += $(BUSOBJ)/ti99_peb/ti_fdc.o
BUSOBJS += $(BUSOBJ)/ti99_peb/ti_rs232.o
BUSOBJS += $(BUSOBJ)/ti99_peb/tn_ide.o
BUSOBJS += $(BUSOBJ)/ti99_peb/tn_usbsm.o
endif

#-------------------------------------------------
#
#@src/emu/bus/gameboy/gb_slot.h,BUSES += GAMEBOY
#-------------------------------------------------

ifneq ($(filter GAMEBOY,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/gameboy
BUSOBJS += $(BUSOBJ)/gameboy/gb_slot.o
BUSOBJS += $(BUSOBJ)/gameboy/rom.o
BUSOBJS += $(BUSOBJ)/gameboy/mbc.o
endif

#-------------------------------------------------
#
#@src/emu/bus/gba/gba_slot.h,BUSES += GBA
#-------------------------------------------------

ifneq ($(filter GBA,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/gba
BUSOBJS += $(BUSOBJ)/gba/gba_slot.o
BUSOBJS += $(BUSOBJ)/gba/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/bml3/bml3bus.h,BUSES += BML3
#-------------------------------------------------
ifneq ($(filter BML3,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/bml3
BUSOBJS += $(BUSOBJ)/bml3/bml3bus.o
BUSOBJS += $(BUSOBJ)/bml3/bml3mp1802.o
BUSOBJS += $(BUSOBJ)/bml3/bml3mp1805.o
BUSOBJS += $(BUSOBJ)/bml3/bml3kanji.o
endif

#-------------------------------------------------
#
#@src/emu/bus/coco/cococart.h,BUSES += COCO
#-------------------------------------------------
ifneq ($(filter COCO,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/coco
BUSOBJS += $(BUSOBJ)/coco/cococart.o
BUSOBJS += $(BUSOBJ)/coco/coco_232.o
BUSOBJS += $(BUSOBJ)/coco/coco_orch90.o
BUSOBJS += $(BUSOBJ)/coco/coco_pak.o
BUSOBJS += $(BUSOBJ)/coco/coco_fdc.o
BUSOBJS += $(BUSOBJ)/coco/coco_multi.o
BUSOBJS += $(BUSOBJ)/coco/coco_dwsock.o
endif

#-------------------------------------------------
#
#@src/emu/bus/cpc/cpcexp.h,BUSES += CPC
#-------------------------------------------------
ifneq ($(filter CPC,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/cpc
BUSOBJS += $(BUSOBJ)/cpc/cpcexp.o
BUSOBJS += $(BUSOBJ)/cpc/cpc_ssa1.o
BUSOBJS += $(BUSOBJ)/cpc/cpc_rom.o
BUSOBJS += $(BUSOBJ)/cpc/cpc_pds.o
BUSOBJS += $(BUSOBJ)/cpc/cpc_rs232.o
BUSOBJS += $(BUSOBJ)/cpc/mface2.o
BUSOBJS += $(BUSOBJ)/cpc/symbfac2.o
BUSOBJS += $(BUSOBJ)/cpc/amdrum.o
BUSOBJS += $(BUSOBJ)/cpc/playcity.o
endif

#-------------------------------------------------
#
#@src/emu/bus/epson_sio/epson_sio.h,BUSES += EPSON_SIO
#-------------------------------------------------
ifneq ($(filter EPSON_SIO,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/epson_sio
BUSOBJS += $(BUSOBJ)/epson_sio/epson_sio.o
BUSOBJS += $(BUSOBJ)/epson_sio/pf10.o
BUSOBJS += $(BUSOBJ)/epson_sio/tf20.o
endif

#-------------------------------------------------
#
#@src/emu/bus/pce/pce_slot.h,BUSES += PCE
#-------------------------------------------------
ifneq ($(filter PCE,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/pce
BUSOBJS += $(BUSOBJ)/pce/pce_slot.o
BUSOBJS += $(BUSOBJ)/pce/pce_rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/scv/slot.h,BUSES += SCV
#-------------------------------------------------
ifneq ($(filter SCV,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/scv
BUSOBJS += $(BUSOBJ)/scv/slot.o
BUSOBJS += $(BUSOBJ)/scv/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/x68k/x68kexp.h,BUSES += X68K
#-------------------------------------------------
ifneq ($(filter X68K,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/x68k
BUSOBJS += $(BUSOBJ)/x68k/x68kexp.o
BUSOBJS += $(BUSOBJ)/x68k/x68k_neptunex.o
BUSOBJS += $(BUSOBJ)/x68k/x68k_scsiext.o
endif

#-------------------------------------------------
#
#@src/emu/bus/abckb/abckb.h,BUSES += ABCKB
#-------------------------------------------------
ifneq ($(filter ABCKB,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/abckb
BUSOBJS += $(BUSOBJ)/abckb/abckb.o
BUSOBJS += $(BUSOBJ)/abckb/abc77.o
BUSOBJS += $(BUSOBJ)/abckb/abc99.o
BUSOBJS += $(BUSOBJ)/abckb/abc800kb.o
endif

#-------------------------------------------------
#
#@src/emu/bus/compucolor/compclr_flp.h,BUSES += COMPUCOLOR
#-------------------------------------------------
ifneq ($(filter COMPUCOLOR,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/compucolor
BUSOBJS += $(BUSOBJ)/compucolor/floppy.o
endif

#-------------------------------------------------
#
#@src/emu/bus/scsi/scsi.h,BUSES += SCSI
#-------------------------------------------------
ifneq ($(filter SCSI,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/scsi
BUSOBJS += $(BUSOBJ)/scsi/scsi.o
BUSOBJS += $(BUSOBJ)/scsi/scsicd.o
BUSOBJS += $(BUSOBJ)/scsi/scsihd.o
BUSOBJS += $(BUSOBJ)/scsi/scsihle.o
BUSOBJS += $(BUSOBJ)/scsi/cdu76s.o
BUSOBJS += $(BUSOBJ)/scsi/acb4070.o
BUSOBJS += $(BUSOBJ)/scsi/d9060hd.o
BUSOBJS += $(BUSOBJ)/scsi/sa1403d.o
BUSOBJS += $(BUSOBJ)/scsi/s1410.o
BUSOBJS += $(BUSOBJ)/scsi/pc9801_sasi.o
endif

#-------------------------------------------------
#
#@src/emu/bus/macpds/macpds.h,BUSES += MACPDS
#-------------------------------------------------
ifneq ($(filter MACPDS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/macpds
BUSOBJS += $(BUSOBJ)/macpds/macpds.o
BUSOBJS += $(BUSOBJ)/macpds/pds_tpdfpd.o
endif

#-------------------------------------------------
#
#@src/emu/bus/oricext/oricext.h,BUSES += ORICEXT
#-------------------------------------------------
ifneq ($(filter ORICEXT,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/oricext
BUSOBJS += $(BUSOBJ)/oricext/oricext.o
BUSOBJS += $(BUSOBJ)/oricext/jasmin.o
BUSOBJS += $(BUSOBJ)/oricext/microdisc.o
endif

#-------------------------------------------------
#
#@src/emu/bus/a1bus/a1bus.h,BUSES += A1BUS
#-------------------------------------------------

ifneq ($(filter A1BUS,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/a1bus
BUSOBJS += $(BUSOBJ)/a1bus/a1bus.o
BUSOBJS += $(BUSOBJ)/a1bus/a1cassette.o
BUSOBJS += $(BUSOBJ)/a1bus/a1cffa.o
endif

#-------------------------------------------------
#
#@src/emu/bus/amiga/zorro/zorro.h,BUSES += ZORRO
#-------------------------------------------------

ifneq ($(filter ZORRO,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/amiga/zorro
BUSOBJS += $(BUSOBJ)/amiga/zorro/zorro.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/cards.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/a2052.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/a2232.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/a590.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/action_replay.o
BUSOBJS += $(BUSOBJ)/amiga/zorro/buddha.o
endif

#-------------------------------------------------
#
#@src/emu/bus/ql/exp.h,BUSES += QL
#-------------------------------------------------

ifneq ($(filter QL,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/ql
BUSOBJS += $(BUSOBJ)/ql/exp.o
BUSOBJS += $(BUSOBJ)/ql/cst_qdisc.o
BUSOBJS += $(BUSOBJ)/ql/cst_q_plus4.o
BUSOBJS += $(BUSOBJ)/ql/cumana_fdi.o
BUSOBJS += $(BUSOBJ)/ql/kempston_di.o
BUSOBJS += $(BUSOBJ)/ql/miracle_gold_card.o
BUSOBJS += $(BUSOBJ)/ql/mp_fdi.o
BUSOBJS += $(BUSOBJ)/ql/opd_basic_master.o
BUSOBJS += $(BUSOBJ)/ql/pcml_qdisk.o
BUSOBJS += $(BUSOBJ)/ql/qubide.o
BUSOBJS += $(BUSOBJ)/ql/sandy_superdisk.o
BUSOBJS += $(BUSOBJ)/ql/sandy_superqboard.o
BUSOBJS += $(BUSOBJ)/ql/trumpcard.o
BUSOBJS += $(BUSOBJ)/ql/rom.o
BUSOBJS += $(BUSOBJ)/ql/miracle_hd.o
BUSOBJS += $(BUSOBJ)/ql/std.o
endif

#-------------------------------------------------
#
#@src/emu/bus/vtech/memexp/memexp.h,BUSES += VTECH_MEMEXP
#-------------------------------------------------

ifneq ($(filter VTECH_MEMEXP,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vtech/memexp
BUSOBJS += $(BUSOBJ)/vtech/memexp/memexp.o
BUSOBJS += $(BUSOBJ)/vtech/memexp/carts.o
BUSOBJS += $(BUSOBJ)/vtech/memexp/floppy.o
BUSOBJS += $(BUSOBJ)/vtech/memexp/memory.o
BUSOBJS += $(BUSOBJ)/vtech/memexp/rs232.o
BUSOBJS += $(BUSOBJ)/vtech/memexp/wordpro.o
endif

#-------------------------------------------------
#
#@src/emu/bus/vtech/ioexp/ioexp.h,BUSES += VTECH_IOEXP
#-------------------------------------------------

ifneq ($(filter VTECH_IOEXP,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/vtech/ioexp
BUSOBJS += $(BUSOBJ)/vtech/ioexp/ioexp.o
BUSOBJS += $(BUSOBJ)/vtech/ioexp/carts.o
BUSOBJS += $(BUSOBJ)/vtech/ioexp/joystick.o
BUSOBJS += $(BUSOBJ)/vtech/ioexp/printer.o
endif

#-------------------------------------------------
#
#@src/emu/bus/wswan/slot.h,BUSES += WSWAN
#-------------------------------------------------

ifneq ($(filter WSWAN,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/wswan
BUSOBJS += $(BUSOBJ)/wswan/slot.o
BUSOBJS += $(BUSOBJ)/wswan/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/psx/ctlrport.h,BUSES += PSX_CONTROLLER
#-------------------------------------------------

ifneq ($(filter PSX_CONTROLLER,$(BUSES)),)
OBJDIRS += $(BUSOBJ)/psx
BUSOBJS += $(BUSOBJ)/psx/ctlrport.o
BUSOBJS += $(BUSOBJ)/psx/analogue.o
BUSOBJS += $(BUSOBJ)/psx/multitap.o
BUSOBJS += $(BUSOBJ)/psx/memcard.o
endif
