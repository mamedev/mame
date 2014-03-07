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
BUSOBJS += $(BUSOBJ)/adamnet/adamnet.o
BUSOBJS += $(BUSOBJ)/adamnet/ddp.o
BUSOBJS += $(BUSOBJ)/adamnet/fdc.o
BUSOBJS += $(BUSOBJ)/adamnet/kb.o
BUSOBJS += $(BUSOBJ)/adamnet/printer.o
BUSOBJS += $(BUSOBJ)/adamnet/spi.o
endif


#-------------------------------------------------
#
#@src/emu/bus/bw2/exp.h,BUSES += BW2
#-------------------------------------------------

ifneq ($(filter BW2,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/bw2/exp.o
BUSOBJS += $(BUSOBJ)/bw2/ramcard.o
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
#@src/emu/bus/comx35/exp.h,BUSES += COMX35
#-------------------------------------------------

ifneq ($(filter COMX35,$(BUSES)),)
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
#@src/emu/bus/ecbbus/ecbbus.h,BUSES += ECBBUS
#-------------------------------------------------

ifneq ($(filter ECBBUS,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/ecbbus/ecbbus.o
BUSOBJS += $(BUSOBJ)/ecbbus/grip.o
endif


#-------------------------------------------------
#
#@src/emu/bus/econet/econet.h,BUSES += ECONET
#-------------------------------------------------

ifneq ($(filter ECONET,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/econet/econet.o
BUSOBJS += $(BUSOBJ)/econet/e01.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ep64/exp.h,BUSES += EP64
#-------------------------------------------------

ifneq ($(filter EP64,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/ep64/exp.o
BUSOBJS += $(BUSOBJ)/ep64/exdos.o
endif


#-------------------------------------------------
#
#@src/emu/bus/ieee488/ieee488.h,BUSES += IEEE488
#-------------------------------------------------

ifneq ($(filter IEEE488,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/ieee488/ieee488.o
BUSOBJS += $(BUSOBJ)/ieee488/c2031.o
BUSOBJS += $(BUSOBJ)/ieee488/c2040.o
BUSOBJS += $(BUSOBJ)/ieee488/c2040fdc.o
BUSOBJS += $(BUSOBJ)/ieee488/c8050.o
BUSOBJS += $(BUSOBJ)/ieee488/c8280.o
BUSOBJS += $(BUSOBJ)/ieee488/d9060.o
BUSOBJS += $(BUSOBJ)/ieee488/d9060hd.o
BUSOBJS += $(BUSOBJ)/ieee488/softbox.o
BUSOBJS += $(BUSOBJ)/ieee488/hardbox.o
BUSOBJS += $(BUSOBJ)/ieee488/shark.o
endif


#-------------------------------------------------
#
#@src/emu/bus/iq151/iq151.h,BUSES += IQ151
#-------------------------------------------------

ifneq ($(filter IQ151,$(BUSES)),)
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
BUSOBJS += $(BUSOBJ)/imi7000/imi7000.o
BUSOBJS += $(BUSOBJ)/imi7000/imi5000h.o
endif


#-------------------------------------------------
#
#@src/emu/bus/isa/isa.h,BUSES += ISA
#-------------------------------------------------

ifneq ($(filter ISA,$(BUSES)),)
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
BUSOBJS += $(BUSOBJ)/isa/svga_tseng.o
BUSOBJS += $(BUSOBJ)/isa/svga_s3.o
BUSOBJS += $(BUSOBJ)/isa/s3virge.o
BUSOBJS += $(BUSOBJ)/isa/pc1640_iga.o
BUSOBJS += $(BUSOBJ)/isa/3c503.o
BUSOBJS += $(BUSOBJ)/isa/ne1000.o
BUSOBJS += $(BUSOBJ)/isa/ne2000.o
BUSOBJS += $(BUSOBJ)/isa/lpt.o
BUSOBJS += $(BUSOBJ)/isa/p1_fdc.o
BUSOBJS += $(BUSOBJ)/isa/p1_hdc.o
BUSOBJS += $(BUSOBJ)/isa/p1_rom.o
BUSOBJS += $(BUSOBJ)/isa/mc1502_fdc.o
BUSOBJS += $(BUSOBJ)/isa/mc1502_rom.o
BUSOBJS += $(BUSOBJ)/isa/xsu_cards.o
endif

#-------------------------------------------------
#
#@src/emu/bus/isbx/isbx.h,BUSES += ISBX
#-------------------------------------------------

ifneq ($(filter ISBX,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/isbx/isbx.o
BUSOBJS += $(BUSOBJ)/isbx/compis_fdc.o
BUSOBJS += $(BUSOBJ)/isbx/isbc_218a.o
endif


#-------------------------------------------------
#
#@src/emu/bus/kc/kc.h,BUSES += KC
#-------------------------------------------------

ifneq ($(filter KC,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/kc/kc.o
BUSOBJS += $(BUSOBJ)/kc/d002.o
BUSOBJS += $(BUSOBJ)/kc/d004.o
BUSOBJS += $(BUSOBJ)/kc/ram.o
BUSOBJS += $(BUSOBJ)/kc/rom.o
endif

#-------------------------------------------------
#
#@src/emu/bus/pc_joy/pc_joy.h,BUSES += PC_JOY
#-------------------------------------------------

ifneq ($(filter PC_JOY,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/pc_joy/pc_joy.o
BUSOBJS += $(BUSOBJ)/pc_joy/pc_joy_sw.o
endif


#-------------------------------------------------
#
#@src/emu/bus/pc_kbd/pc_kbdc.h,BUSES += PC_KBD
#-------------------------------------------------

ifneq ($(filter PC_KBD,$(BUSES)),)
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
BUSOBJS += $(BUSOBJ)/pet/cass.o
BUSOBJS += $(BUSOBJ)/pet/c2n.o
BUSOBJS += $(BUSOBJ)/pet/diag264_lb_tape.o
BUSOBJS += $(BUSOBJ)/pet/exp.o
BUSOBJS += $(BUSOBJ)/pet/64k.o
BUSOBJS += $(BUSOBJ)/pet/superpet.o
BUSOBJS += $(BUSOBJ)/pet/user.o
BUSOBJS += $(BUSOBJ)/pet/petuja.o
endif


#-------------------------------------------------
#
#@src/emu/bus/plus4/exp.h,BUSES += PLUS4
#@src/emu/bus/plus4/user.h,BUSES += PLUS4
#-------------------------------------------------

ifneq ($(filter PLUS4,$(BUSES)),)
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
#@src/emu/bus/tvc/tvc.h,BUSES += TVC
#-------------------------------------------------

ifneq ($(filter TVC,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/tvc/tvc.o
BUSOBJS += $(BUSOBJ)/tvc/hbf.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vcs/ctrl.h,BUSES += VCS
#-------------------------------------------------

ifneq ($(filter VCS,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/vcs/ctrl.o
BUSOBJS += $(BUSOBJ)/vcs/joystick.o
BUSOBJS += $(BUSOBJ)/vcs/joybooster.o
BUSOBJS += $(BUSOBJ)/vcs/keypad.o
BUSOBJS += $(BUSOBJ)/vcs/lightpen.o
BUSOBJS += $(BUSOBJ)/vcs/paddles.o
BUSOBJS += $(BUSOBJ)/vcs/wheel.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vic10/exp.h,BUSES += VIC10
#-------------------------------------------------

ifneq ($(filter VIC10,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/vic10/exp.o
BUSOBJS += $(BUSOBJ)/vic10/std.o
endif


#-------------------------------------------------
#
#@src/emu/bus/vic20/exp.h,BUSES += VIC20
#@src/emu/bus/vic20/user.h,BUSES += VIC20
#-------------------------------------------------

ifneq ($(filter VIC20,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/vic20/exp.o
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
endif

#-------------------------------------------------
#
#@src/emu/bus/nubus/nubus.h,BUSES += NUBUS
#-------------------------------------------------

ifneq ($(filter NUBUS,$(BUSES)),)
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
BUSOBJS += $(BUSOBJ)/centronics/ctronics.o
BUSOBJS += $(BUSOBJ)/centronics/comxpl80.o
BUSOBJS += $(BUSOBJ)/centronics/covox.o
BUSOBJS += $(BUSOBJ)/centronics/dsjoy.o
BUSOBJS += $(BUSOBJ)/centronics/image.o
endif

#-------------------------------------------------
#
#@src/emu/bus/rs232/rs232.h,BUSES += RS232
#-------------------------------------------------

ifneq ($(filter RS232,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/rs232/keyboard.o
BUSOBJS += $(BUSOBJ)/rs232/null_modem.o
BUSOBJS += $(BUSOBJ)/rs232/rs232.o
BUSOBJS += $(BUSOBJ)/rs232/ser_mouse.o
BUSOBJS += $(BUSOBJ)/rs232/terminal.o
endif

#-------------------------------------------------
#
#@src/emu/bus/midi/midi.h,BUSES += MIDI
#-------------------------------------------------

ifneq ($(filter MIDI,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/midi/midi.o
BUSOBJS += $(BUSOBJ)/midi/midiinport.o
BUSOBJS += $(BUSOBJ)/midi/midioutport.o
endif

#-------------------------------------------------
#
#@src/emu/bus/pci/pci.h,BUSES += PCI
#-------------------------------------------------

ifneq ($(filter PCI,$(BUSES)),)
BUSOBJS += $(BUSOBJ)/pci/pci.o
BUSOBJS += $(BUSOBJ)/pci/i82371ab.o
BUSOBJS += $(BUSOBJ)/pci/i82371sb.o
BUSOBJS += $(BUSOBJ)/pci/i82439tx.o
BUSOBJS += $(BUSOBJ)/pci/northbridge.o
BUSOBJS += $(BUSOBJ)/pci/southbridge.o
BUSOBJS += $(BUSOBJ)/pci/mpc105.o
endif

