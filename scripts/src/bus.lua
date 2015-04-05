---------------------------------------------------------------------------
--
--   bus.lua
--
--   Rules for building bus cores
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
--
---------------------------------------------------------------------------

-------------------------------------------------
--
--@src/emu/bus/a7800/a78_slot.h,BUSES += A7800
---------------------------------------------------

if (BUSES["A7800"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a7800/a78_slot.c",
		MAME_DIR .. "src/emu/bus/a7800/rom.c",
		MAME_DIR .. "src/emu/bus/a7800/hiscore.c",
		MAME_DIR .. "src/emu/bus/a7800/xboard.c",
		MAME_DIR .. "src/emu/bus/a7800/cpuwiz.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/a800/a800_slot.h,BUSES += A800
---------------------------------------------------

if (BUSES["A800"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a800/a800_slot.c",
		MAME_DIR .. "src/emu/bus/a800/rom.c",
		MAME_DIR .. "src/emu/bus/a800/oss.c",
		MAME_DIR .. "src/emu/bus/a800/sparta.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/a8sio/a8sio.h,BUSES += A8SIO
---------------------------------------------------

if (BUSES["A8SIO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a8sio/a8sio.c",
		MAME_DIR .. "src/emu/bus/a8sio/cassette.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/abcbus/abcbus.h,BUSES += ABCBUS
---------------------------------------------------

if (BUSES["ABCBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/abcbus/abcbus.c",
		MAME_DIR .. "src/emu/bus/abcbus/abc890.c",
		MAME_DIR .. "src/emu/bus/abcbus/fd2.c",
		MAME_DIR .. "src/emu/bus/abcbus/hdc.c",
		MAME_DIR .. "src/emu/bus/abcbus/lux10828.c",
		MAME_DIR .. "src/emu/bus/abcbus/lux21046.c",
		MAME_DIR .. "src/emu/bus/abcbus/lux21056.c",
		MAME_DIR .. "src/emu/bus/abcbus/lux4105.c",
		MAME_DIR .. "src/emu/bus/abcbus/memcard.c",
		MAME_DIR .. "src/emu/bus/abcbus/uni800.c",
		MAME_DIR .. "src/emu/bus/abcbus/sio.c",
		MAME_DIR .. "src/emu/bus/abcbus/slutprov.c",
		MAME_DIR .. "src/emu/bus/abcbus/turbo.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/adam/exp.h,BUSES += ADAM
---------------------------------------------------

if (BUSES["ADAM"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/adam/exp.c",
		MAME_DIR .. "src/emu/bus/adam/adamlink.c",
		MAME_DIR .. "src/emu/bus/adam/ide.c",
		MAME_DIR .. "src/emu/bus/adam/ram.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/adamnet/adamnet.h,BUSES += ADAMNET
---------------------------------------------------

if (BUSES["ADAMNET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/adamnet/adamnet.c",
		MAME_DIR .. "src/emu/bus/adamnet/ddp.c",
		MAME_DIR .. "src/emu/bus/adamnet/fdc.c",
		MAME_DIR .. "src/emu/bus/adamnet/kb.c",
		MAME_DIR .. "src/emu/bus/adamnet/printer.c",
		MAME_DIR .. "src/emu/bus/adamnet/spi.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/apf/slot.h,BUSES += APF
---------------------------------------------------

if (BUSES["APF"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/apf/slot.c",
		MAME_DIR .. "src/emu/bus/apf/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/arcadia/slot.h,BUSES += ARCADIA
---------------------------------------------------

if (BUSES["ARCADIA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/arcadia/slot.c",
		MAME_DIR .. "src/emu/bus/arcadia/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/astrocde/slot.h,BUSES += ASTROCADE
---------------------------------------------------

if (BUSES["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/astrocde/slot.c",
		MAME_DIR .. "src/emu/bus/astrocde/rom.c",
		MAME_DIR .. "src/emu/bus/astrocde/exp.c",
		MAME_DIR .. "src/emu/bus/astrocde/ram.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/bw2/exp.h,BUSES += BW2
---------------------------------------------------

if (BUSES["BW2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/bw2/exp.c",
		MAME_DIR .. "src/emu/bus/bw2/ramcard.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/c64/exp.h,BUSES += C64
--@src/emu/bus/c64/user.h,BUSES += C64
---------------------------------------------------

if (BUSES["C64"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/c64/exp.c",
		MAME_DIR .. "src/emu/bus/c64/c128_comal80.c",
		MAME_DIR .. "src/emu/bus/c64/comal80.c",
		MAME_DIR .. "src/emu/bus/c64/cpm.c",
		MAME_DIR .. "src/emu/bus/c64/currah_speech.c",
		MAME_DIR .. "src/emu/bus/c64/dela_ep256.c",
		MAME_DIR .. "src/emu/bus/c64/dela_ep64.c",
		MAME_DIR .. "src/emu/bus/c64/dela_ep7x8.c",
		MAME_DIR .. "src/emu/bus/c64/dinamic.c",
		MAME_DIR .. "src/emu/bus/c64/dqbb.c",
		MAME_DIR .. "src/emu/bus/c64/easy_calc_result.c",
		MAME_DIR .. "src/emu/bus/c64/easyflash.c",
		MAME_DIR .. "src/emu/bus/c64/epyx_fast_load.c",
		MAME_DIR .. "src/emu/bus/c64/exos.c",
		MAME_DIR .. "src/emu/bus/c64/fcc.c",
		MAME_DIR .. "src/emu/bus/c64/final.c",
		MAME_DIR .. "src/emu/bus/c64/final3.c",
		MAME_DIR .. "src/emu/bus/c64/fun_play.c",
		MAME_DIR .. "src/emu/bus/c64/georam.c",
		MAME_DIR .. "src/emu/bus/c64/ide64.c",
		MAME_DIR .. "src/emu/bus/c64/ieee488.c",
		MAME_DIR .. "src/emu/bus/c64/kingsoft.c",
		MAME_DIR .. "src/emu/bus/c64/mach5.c",
		MAME_DIR .. "src/emu/bus/c64/magic_desk.c",
		MAME_DIR .. "src/emu/bus/c64/magic_formel.c",
		MAME_DIR .. "src/emu/bus/c64/magic_voice.c",
		MAME_DIR .. "src/emu/bus/c64/midi_maplin.c",
		MAME_DIR .. "src/emu/bus/c64/midi_namesoft.c",
		MAME_DIR .. "src/emu/bus/c64/midi_passport.c",
		MAME_DIR .. "src/emu/bus/c64/midi_sci.c",
		MAME_DIR .. "src/emu/bus/c64/midi_siel.c",
		MAME_DIR .. "src/emu/bus/c64/mikro_assembler.c",
		MAME_DIR .. "src/emu/bus/c64/multiscreen.c",
		MAME_DIR .. "src/emu/bus/c64/music64.c",
		MAME_DIR .. "src/emu/bus/c64/neoram.c",
		MAME_DIR .. "src/emu/bus/c64/ocean.c",
		MAME_DIR .. "src/emu/bus/c64/pagefox.c",
		MAME_DIR .. "src/emu/bus/c64/partner.c",
		MAME_DIR .. "src/emu/bus/c64/prophet64.c",
		MAME_DIR .. "src/emu/bus/c64/ps64.c",
		MAME_DIR .. "src/emu/bus/c64/reu.c",
		MAME_DIR .. "src/emu/bus/c64/rex.c",
		MAME_DIR .. "src/emu/bus/c64/rex_ep256.c",
		MAME_DIR .. "src/emu/bus/c64/ross.c",
		MAME_DIR .. "src/emu/bus/c64/sfx_sound_expander.c",
		MAME_DIR .. "src/emu/bus/c64/silverrock.c",
		MAME_DIR .. "src/emu/bus/c64/simons_basic.c",
		MAME_DIR .. "src/emu/bus/c64/stardos.c",
		MAME_DIR .. "src/emu/bus/c64/std.c",
		MAME_DIR .. "src/emu/bus/c64/structured_basic.c",
		MAME_DIR .. "src/emu/bus/c64/super_explode.c",
		MAME_DIR .. "src/emu/bus/c64/super_games.c",
		MAME_DIR .. "src/emu/bus/c64/supercpu.c",
		MAME_DIR .. "src/emu/bus/c64/sw8k.c",
		MAME_DIR .. "src/emu/bus/c64/swiftlink.c",
		MAME_DIR .. "src/emu/bus/c64/system3.c",
		MAME_DIR .. "src/emu/bus/c64/tdos.c",
		MAME_DIR .. "src/emu/bus/c64/turbo232.c",
		MAME_DIR .. "src/emu/bus/c64/vizastar.c",
		MAME_DIR .. "src/emu/bus/c64/vw64.c",
		MAME_DIR .. "src/emu/bus/c64/warp_speed.c",
		MAME_DIR .. "src/emu/bus/c64/westermann.c",
		MAME_DIR .. "src/emu/bus/c64/xl80.c",
		MAME_DIR .. "src/emu/bus/c64/zaxxon.c",
		MAME_DIR .. "src/emu/bus/c64/user.c",
		MAME_DIR .. "src/emu/bus/c64/4dxh.c",
		MAME_DIR .. "src/emu/bus/c64/4ksa.c",
		MAME_DIR .. "src/emu/bus/c64/4tba.c",
		MAME_DIR .. "src/emu/bus/c64/16kb.c",
		MAME_DIR .. "src/emu/bus/c64/bn1541.c",
		MAME_DIR .. "src/emu/bus/c64/geocable.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/cbm2/exp.h,BUSES += CBM2
--@src/emu/bus/cbm2/user.h,BUSES += CBM2
---------------------------------------------------

if (BUSES["CBM2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cbm2/exp.c",
		MAME_DIR .. "src/emu/bus/cbm2/24k.c",
		MAME_DIR .. "src/emu/bus/cbm2/hrg.c",
		MAME_DIR .. "src/emu/bus/cbm2/std.c",
		MAME_DIR .. "src/emu/bus/cbm2/user.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/cbmiec/cbmiec.h,BUSES += CBMIEC
---------------------------------------------------

if (BUSES["CBMIEC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cbmiec/cbmiec.c",
		MAME_DIR .. "src/emu/bus/cbmiec/c1541.c",
		MAME_DIR .. "src/emu/bus/cbmiec/c1571.c",
		MAME_DIR .. "src/emu/bus/cbmiec/c1581.c",
		MAME_DIR .. "src/emu/bus/cbmiec/c64_nl10.c",
		MAME_DIR .. "src/emu/bus/cbmiec/cmdhd.c",
		MAME_DIR .. "src/emu/bus/cbmiec/diag264_lb_iec.c",
		MAME_DIR .. "src/emu/bus/cbmiec/fd2000.c",
		MAME_DIR .. "src/emu/bus/cbmiec/interpod.c",
		MAME_DIR .. "src/emu/bus/cbmiec/serialbox.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/chanf/slot.h,BUSES += CHANNELF
---------------------------------------------------

if (BUSES["CHANNELF"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/chanf/slot.c",
		MAME_DIR .. "src/emu/bus/chanf/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/comx35/exp.h,BUSES += COMX35
---------------------------------------------------

if (BUSES["COMX35"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/comx35/exp.c",
		MAME_DIR .. "src/emu/bus/comx35/clm.c",
		MAME_DIR .. "src/emu/bus/comx35/expbox.c",
		MAME_DIR .. "src/emu/bus/comx35/eprom.c",
		MAME_DIR .. "src/emu/bus/comx35/fdc.c",
		MAME_DIR .. "src/emu/bus/comx35/joycard.c",
		MAME_DIR .. "src/emu/bus/comx35/printer.c",
		MAME_DIR .. "src/emu/bus/comx35/ram.c",
		MAME_DIR .. "src/emu/bus/comx35/thermal.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/coleco/ctrl.h,BUSES += COLECO
---------------------------------------------------

if (BUSES["COLECO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/coleco/ctrl.c",
		MAME_DIR .. "src/emu/bus/coleco/hand.c",
		MAME_DIR .. "src/emu/bus/coleco/sac.c",
		MAME_DIR .. "src/emu/bus/coleco/exp.c",
		MAME_DIR .. "src/emu/bus/coleco/std.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/crvision/slot.h,BUSES += CRVISION
---------------------------------------------------

if (BUSES["CRVISION"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/crvision/slot.c",
		MAME_DIR .. "src/emu/bus/crvision/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/dmv/dmv.h,BUSES += DMV
---------------------------------------------------

if (BUSES["DMV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/dmv/dmvbus.c",
		MAME_DIR .. "src/emu/bus/dmv/k210.c",
		MAME_DIR .. "src/emu/bus/dmv/k220.c",
		MAME_DIR .. "src/emu/bus/dmv/k230.c",
		MAME_DIR .. "src/emu/bus/dmv/k233.c",
		MAME_DIR .. "src/emu/bus/dmv/k801.c",
		MAME_DIR .. "src/emu/bus/dmv/k803.c",
		MAME_DIR .. "src/emu/bus/dmv/k806.c",
		MAME_DIR .. "src/emu/bus/dmv/ram.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ecbbus/ecbbus.h,BUSES += ECBBUS
---------------------------------------------------

if (BUSES["ECBBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ecbbus/ecbbus.c",
		MAME_DIR .. "src/emu/bus/ecbbus/grip.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/econet/econet.h,BUSES += ECONET
---------------------------------------------------

if (BUSES["ECONET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/econet/econet.c",
		MAME_DIR .. "src/emu/bus/econet/e01.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ep64/exp.h,BUSES += EP64
---------------------------------------------------

if (BUSES["EP64"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ep64/exp.c",
		MAME_DIR .. "src/emu/bus/ep64/exdos.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/generic/slot.h,BUSES += GENERIC
---------------------------------------------------

if (BUSES["GENERIC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/generic/slot.c",
		MAME_DIR .. "src/emu/bus/generic/carts.c",
		MAME_DIR .. "src/emu/bus/generic/ram.c",
		MAME_DIR .. "src/emu/bus/generic/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/ieee488/ieee488.h,BUSES += IEEE488
---------------------------------------------------

if (BUSES["IEEE488"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ieee488/ieee488.c",
		MAME_DIR .. "src/emu/bus/ieee488/c2031.c",
		MAME_DIR .. "src/emu/bus/ieee488/c2040.c",
		MAME_DIR .. "src/emu/bus/ieee488/c2040fdc.c",
		MAME_DIR .. "src/emu/bus/ieee488/c8050.c",
		MAME_DIR .. "src/emu/bus/ieee488/c8050fdc.c",
		MAME_DIR .. "src/emu/bus/ieee488/c8280.c",
		MAME_DIR .. "src/emu/bus/ieee488/d9060.c",
		MAME_DIR .. "src/emu/bus/ieee488/softbox.c",
		MAME_DIR .. "src/emu/bus/ieee488/hardbox.c",
		MAME_DIR .. "src/emu/bus/ieee488/shark.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/iq151/iq151.h,BUSES += IQ151
---------------------------------------------------

if (BUSES["IQ151"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/iq151/iq151.c",
		MAME_DIR .. "src/emu/bus/iq151/disc2.c",
		MAME_DIR .. "src/emu/bus/iq151/grafik.c",
		MAME_DIR .. "src/emu/bus/iq151/minigraf.c",
		MAME_DIR .. "src/emu/bus/iq151/ms151a.c",
		MAME_DIR .. "src/emu/bus/iq151/rom.c",
		MAME_DIR .. "src/emu/bus/iq151/staper.c",
		MAME_DIR .. "src/emu/bus/iq151/video32.c",
		MAME_DIR .. "src/emu/bus/iq151/video64.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/isbx/isbx.h,BUSES += IMI7000
---------------------------------------------------

if (BUSES["IMI7000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/imi7000/imi7000.c",
		MAME_DIR .. "src/emu/bus/imi7000/imi5000h.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/intv/slot.h,BUSES += INTV
---------------------------------------------------

if (BUSES["INTV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/intv/slot.c",
		MAME_DIR .. "src/emu/bus/intv/rom.c",
		MAME_DIR .. "src/emu/bus/intv/voice.c",
		MAME_DIR .. "src/emu/bus/intv/ecs.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/isa/isa.h,BUSES += ISA
---------------------------------------------------

if (BUSES["ISA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/isa/isa.c",
		MAME_DIR .. "src/emu/bus/isa/isa_cards.c",
		MAME_DIR .. "src/emu/bus/isa/mda.c",
		MAME_DIR .. "src/emu/bus/isa/wdxt_gen.c",
		MAME_DIR .. "src/emu/bus/isa/adlib.c",
		MAME_DIR .. "src/emu/bus/isa/com.c",
		MAME_DIR .. "src/emu/bus/isa/fdc.c",
		MAME_DIR .. "src/emu/bus/isa/mufdc.c",
		MAME_DIR .. "src/emu/bus/isa/finalchs.c",
		MAME_DIR .. "src/emu/bus/isa/gblaster.c",
		MAME_DIR .. "src/emu/bus/isa/gus.c",
		MAME_DIR .. "src/emu/bus/isa/sb16.c",
		MAME_DIR .. "src/emu/bus/isa/hdc.c",
		MAME_DIR .. "src/emu/bus/isa/ibm_mfc.c",
		MAME_DIR .. "src/emu/bus/isa/mpu401.c",
		MAME_DIR .. "src/emu/bus/isa/sblaster.c",
		MAME_DIR .. "src/emu/bus/isa/stereo_fx.c",
		MAME_DIR .. "src/emu/bus/isa/ssi2001.c",
		MAME_DIR .. "src/emu/bus/isa/ide.c",
		MAME_DIR .. "src/emu/bus/isa/xtide.c",
		MAME_DIR .. "src/emu/bus/isa/side116.c",
		MAME_DIR .. "src/emu/bus/isa/aha1542.c",
		MAME_DIR .. "src/emu/bus/isa/wd1002a_wx1.c",
		MAME_DIR .. "src/emu/bus/isa/dectalk.c",
		MAME_DIR .. "src/emu/bus/isa/pds.c",
		MAME_DIR .. "src/emu/bus/isa/omti8621.c",
		MAME_DIR .. "src/emu/bus/isa/cga.c",
		MAME_DIR .. "src/emu/bus/isa/svga_cirrus.c",
		MAME_DIR .. "src/emu/bus/isa/ega.c",
		MAME_DIR .. "src/emu/bus/isa/vga.c",
		MAME_DIR .. "src/emu/bus/isa/vga_ati.c",
		MAME_DIR .. "src/emu/bus/isa/mach32.c",
		MAME_DIR .. "src/emu/bus/isa/svga_tseng.c",
		MAME_DIR .. "src/emu/bus/isa/svga_s3.c",
		MAME_DIR .. "src/emu/bus/isa/s3virge.c",
		MAME_DIR .. "src/emu/bus/isa/pc1640_iga.c",
		MAME_DIR .. "src/emu/bus/isa/3c503.c",
		MAME_DIR .. "src/emu/bus/isa/ne1000.c",
		MAME_DIR .. "src/emu/bus/isa/ne2000.c",
		MAME_DIR .. "src/emu/bus/isa/3c505.c",
		MAME_DIR .. "src/emu/bus/isa/lpt.c",
		MAME_DIR .. "src/emu/bus/isa/p1_fdc.c",
		MAME_DIR .. "src/emu/bus/isa/p1_hdc.c",
		MAME_DIR .. "src/emu/bus/isa/p1_rom.c",
		MAME_DIR .. "src/emu/bus/isa/mc1502_fdc.c",
		MAME_DIR .. "src/emu/bus/isa/mc1502_rom.c",
		MAME_DIR .. "src/emu/bus/isa/xsu_cards.c",
		MAME_DIR .. "src/emu/bus/isa/sc499.c",
		MAME_DIR .. "src/emu/bus/isa/aga.c",
		MAME_DIR .. "src/emu/bus/isa/svga_trident.c",
		MAME_DIR .. "src/emu/bus/isa/num9rev.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/isbx/isbx.h,BUSES += ISBX
---------------------------------------------------

if (BUSES["ISBX"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/isbx/isbx.c",
		MAME_DIR .. "src/emu/bus/isbx/compis_fdc.c",
		MAME_DIR .. "src/emu/bus/isbx/isbc_218a.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/msx_slot/slot.h,BUSES += MSX_SLOT
---------------------------------------------------

if (BUSES["MSX_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/msx_slot/bunsetsu.c",
		MAME_DIR .. "src/emu/bus/msx_slot/cartridge.c",
		MAME_DIR .. "src/emu/bus/msx_slot/disk.c",
		MAME_DIR .. "src/emu/bus/msx_slot/fs4600.c",
		MAME_DIR .. "src/emu/bus/msx_slot/music.c",
		MAME_DIR .. "src/emu/bus/msx_slot/panasonic08.c",
		MAME_DIR .. "src/emu/bus/msx_slot/rom.c",
		MAME_DIR .. "src/emu/bus/msx_slot/ram.c",
		MAME_DIR .. "src/emu/bus/msx_slot/ram_mm.c",
		MAME_DIR .. "src/emu/bus/msx_slot/slot.c",
		MAME_DIR .. "src/emu/bus/msx_slot/sony08.c",
		MAME_DIR .. "src/emu/bus/msx_cart/arc.c",
		MAME_DIR .. "src/emu/bus/msx_cart/ascii.c",
		MAME_DIR .. "src/emu/bus/msx_cart/bm_012.c",
		MAME_DIR .. "src/emu/bus/msx_cart/cartridge.c",
		MAME_DIR .. "src/emu/bus/msx_cart/crossblaim.c",
		MAME_DIR .. "src/emu/bus/msx_cart/disk.c",
		MAME_DIR .. "src/emu/bus/msx_cart/dooly.c",
		MAME_DIR .. "src/emu/bus/msx_cart/fmpac.c",
		MAME_DIR .. "src/emu/bus/msx_cart/halnote.c",
		MAME_DIR .. "src/emu/bus/msx_cart/hfox.c",
		MAME_DIR .. "src/emu/bus/msx_cart/holy_quran.c",
		MAME_DIR .. "src/emu/bus/msx_cart/konami.c",
		MAME_DIR .. "src/emu/bus/msx_cart/korean.c",
		MAME_DIR .. "src/emu/bus/msx_cart/majutsushi.c",
		MAME_DIR .. "src/emu/bus/msx_cart/msx_audio.c",
		MAME_DIR .. "src/emu/bus/msx_cart/msx_audio_kb.c",
		MAME_DIR .. "src/emu/bus/msx_cart/msxdos2.c",
		MAME_DIR .. "src/emu/bus/msx_cart/nomapper.c",
		MAME_DIR .. "src/emu/bus/msx_cart/rtype.c",
		MAME_DIR .. "src/emu/bus/msx_cart/superloderunner.c",
		MAME_DIR .. "src/emu/bus/msx_cart/super_swangi.c",
		MAME_DIR .. "src/emu/bus/msx_cart/yamaha.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/kc/kc.h,BUSES += KC
---------------------------------------------------

if (BUSES["KC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/kc/kc.c",
		MAME_DIR .. "src/emu/bus/kc/d002.c",
		MAME_DIR .. "src/emu/bus/kc/d004.c",
		MAME_DIR .. "src/emu/bus/kc/ram.c",
		MAME_DIR .. "src/emu/bus/kc/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/odyssey2/slot.h,BUSES += O2
---------------------------------------------------

if (BUSES["O2"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/odyssey2/slot.c",
		MAME_DIR .. "src/emu/bus/odyssey2/rom.c",
		MAME_DIR .. "src/emu/bus/odyssey2/chess.c",
		MAME_DIR .. "src/emu/bus/odyssey2/voice.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pc_joy/pc_joy.h,BUSES += PC_JOY
---------------------------------------------------

if (BUSES["PC_JOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pc_joy/pc_joy.c",
		MAME_DIR .. "src/emu/bus/pc_joy/pc_joy_sw.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pc_kbd/pc_kbdc.h,BUSES += PC_KBD
---------------------------------------------------

if (BUSES["PC_KBD"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pc_kbd/pc_kbdc.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/keyboards.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/ec1841.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/iskr1030.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/keytro.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/msnat.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/pc83.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/pcat84.c",
		MAME_DIR .. "src/emu/bus/pc_kbd/pcxt83.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/pet/cass.h,BUSES += PET
--@src/emu/bus/pet/exp.h,BUSES += PET
--@src/emu/bus/pet/user.h,BUSES += PET
---------------------------------------------------

if (BUSES["PET"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pet/cass.c",
		MAME_DIR .. "src/emu/bus/pet/c2n.c",
		MAME_DIR .. "src/emu/bus/pet/diag264_lb_tape.c",
		MAME_DIR .. "src/emu/bus/pet/exp.c",
		MAME_DIR .. "src/emu/bus/pet/64k.c",
		MAME_DIR .. "src/emu/bus/pet/hsg.c",
		MAME_DIR .. "src/emu/bus/pet/superpet.c",
		MAME_DIR .. "src/emu/bus/pet/user.c",
		MAME_DIR .. "src/emu/bus/pet/diag.c",
		MAME_DIR .. "src/emu/bus/pet/petuja.c",
		MAME_DIR .. "src/emu/bus/pet/cb2snd.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/plus4/exp.h,BUSES += PLUS4
--@src/emu/bus/plus4/user.h,BUSES += PLUS4
---------------------------------------------------

if (BUSES["PLUS4"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/plus4/exp.c",
		MAME_DIR .. "src/emu/bus/plus4/c1551.c",
		MAME_DIR .. "src/emu/bus/plus4/sid.c",
		MAME_DIR .. "src/emu/bus/plus4/std.c",
		MAME_DIR .. "src/emu/bus/plus4/user.c",
		MAME_DIR .. "src/emu/bus/plus4/diag264_lb_user.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/s100/s100.h,BUSES += S100
---------------------------------------------------

if (BUSES["S100"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/s100/s100.c",
		MAME_DIR .. "src/emu/bus/s100/dj2db.c",
		MAME_DIR .. "src/emu/bus/s100/djdma.c",
		MAME_DIR .. "src/emu/bus/s100/mm65k16s.c",
		MAME_DIR .. "src/emu/bus/s100/nsmdsa.c",
		MAME_DIR .. "src/emu/bus/s100/nsmdsad.c",
		MAME_DIR .. "src/emu/bus/s100/wunderbus.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/spc1000/exp.h,BUSES += SPC1000
---------------------------------------------------

if (BUSES["SPC1000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/spc1000/exp.c",
		MAME_DIR .. "src/emu/bus/spc1000/fdd.c",
		MAME_DIR .. "src/emu/bus/spc1000/vdp.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/tvc/tvc.h,BUSES += TVC
---------------------------------------------------

if (BUSES["TVC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/tvc/tvc.c",
		MAME_DIR .. "src/emu/bus/tvc/hbf.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vc4000/slot.h,BUSES += VC4000
---------------------------------------------------

if (BUSES["VC4000"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vc4000/slot.c",
		MAME_DIR .. "src/emu/bus/vc4000/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vcs/vcs_slot.h,BUSES += VCS
---------------------------------------------------

if (BUSES["VCS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vcs/vcs_slot.c",
		MAME_DIR .. "src/emu/bus/vcs/rom.c",
		MAME_DIR .. "src/emu/bus/vcs/compumat.c",
		MAME_DIR .. "src/emu/bus/vcs/dpc.c",
		MAME_DIR .. "src/emu/bus/vcs/scharger.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vcs/ctrl.h,BUSES += VCS_CTRL
---------------------------------------------------

if (BUSES["VCS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vcs_ctrl/ctrl.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/joystick.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/joybooster.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/keypad.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/lightpen.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/paddles.c",
		MAME_DIR .. "src/emu/bus/vcs_ctrl/wheel.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vectrex/slot.h,BUSES += VECTREX
---------------------------------------------------

if (BUSES["VECTREX"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vectrex/slot.c",
		MAME_DIR .. "src/emu/bus/vectrex/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vic10/exp.h,BUSES += VIC10
---------------------------------------------------

if (BUSES["VIC10"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vic10/exp.c",
		MAME_DIR .. "src/emu/bus/vic10/std.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vic20/exp.h,BUSES += VIC20
--@src/emu/bus/vic20/user.h,BUSES += VIC20
---------------------------------------------------

if (BUSES["VIC20"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vic20/exp.c",
		MAME_DIR .. "src/emu/bus/vic20/fe3.c",
		MAME_DIR .. "src/emu/bus/vic20/megacart.c",
		MAME_DIR .. "src/emu/bus/vic20/std.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1010.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1110.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1111.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1112.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1210.c",
		MAME_DIR .. "src/emu/bus/vic20/user.c",
		MAME_DIR .. "src/emu/bus/vic20/4cga.c",
		MAME_DIR .. "src/emu/bus/vic20/vic1011.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vidbrain/exp.h,BUSES += VIDBRAIN
---------------------------------------------------

if (BUSES["VIDBRAIN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vidbrain/exp.c",
		MAME_DIR .. "src/emu/bus/vidbrain/std.c",
		MAME_DIR .. "src/emu/bus/vidbrain/money_minder.c",
		MAME_DIR .. "src/emu/bus/vidbrain/timeshare.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/vip/byteio.h,BUSES += VIP
--@src/emu/bus/vip/exp.h,BUSES += VIP
---------------------------------------------------

if (BUSES["VIP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vip/byteio.c",
		MAME_DIR .. "src/emu/bus/vip/vp620.c",
		MAME_DIR .. "src/emu/bus/vip/exp.c",
		MAME_DIR .. "src/emu/bus/vip/vp550.c",
		MAME_DIR .. "src/emu/bus/vip/vp570.c",
		MAME_DIR .. "src/emu/bus/vip/vp575.c",
		MAME_DIR .. "src/emu/bus/vip/vp585.c",
		MAME_DIR .. "src/emu/bus/vip/vp590.c",
		MAME_DIR .. "src/emu/bus/vip/vp595.c",
		MAME_DIR .. "src/emu/bus/vip/vp700.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/wangpc/wangpc.h,BUSES += WANGPC
---------------------------------------------------

if (BUSES["WANGPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/wangpc/wangpc.c",
		MAME_DIR .. "src/emu/bus/wangpc/emb.c",
		MAME_DIR .. "src/emu/bus/wangpc/lic.c",
		MAME_DIR .. "src/emu/bus/wangpc/lvc.c",
		MAME_DIR .. "src/emu/bus/wangpc/mcc.c",
		MAME_DIR .. "src/emu/bus/wangpc/mvc.c",
		MAME_DIR .. "src/emu/bus/wangpc/rtc.c",
		MAME_DIR .. "src/emu/bus/wangpc/tig.c",
		MAME_DIR .. "src/emu/bus/wangpc/wdc.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/z88/z88.h,BUSES += Z88
---------------------------------------------------

if (BUSES["Z88"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/z88/z88.c",
		MAME_DIR .. "src/emu/bus/z88/flash.c",
		MAME_DIR .. "src/emu/bus/z88/ram.c",
		MAME_DIR .. "src/emu/bus/z88/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/a2bus/a2bus.h,BUSES += A2BUS
---------------------------------------------------

if (BUSES["A2BUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a2bus/a2bus.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2lang.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2diskii.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2mockingboard.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2cffa.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2memexp.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2scsi.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2thunderclock.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2softcard.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2videoterm.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2ssc.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2swyft.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2eauxslot.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2themill.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2sam.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2alfam2.c",
		MAME_DIR .. "src/emu/bus/a2bus/laser128.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2echoii.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2arcadebd.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2midi.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2vulcan.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2zipdrive.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2applicard.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2hsscsi.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2ultraterm.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2pic.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2estd80col.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2eext80col.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2eramworks3.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2corvus.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2diskiing.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2mcms.c",
		MAME_DIR .. "src/emu/bus/a2bus/a2dx1.c",
		MAME_DIR .. "src/emu/bus/a2bus/timemasterho.c",
		MAME_DIR .. "src/emu/bus/a2bus/mouse.c",
		MAME_DIR .. "src/emu/bus/a2bus/corvfdc01.c",
		MAME_DIR .. "src/emu/bus/a2bus/corvfdc02.c",
		MAME_DIR .. "src/emu/bus/a2bus/ramcard16k.c",
		MAME_DIR .. "src/emu/bus/a2bus/ramcard128k.c",
		MAME_DIR .. "src/emu/bus/a2bus/ezcgi.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nubus/nubus.h,BUSES += NUBUS
---------------------------------------------------

if (BUSES["NUBUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nubus/nubus.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_48gc.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_cb264.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_vikbw.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_specpdq.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_m2hires.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_spec8.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_radiustpd.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_m2video.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_asntmc3b.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_image.c",
		MAME_DIR .. "src/emu/bus/nubus/nubus_wsportrait.c",
		MAME_DIR .. "src/emu/bus/nubus/pds30_cb264.c",
		MAME_DIR .. "src/emu/bus/nubus/pds30_procolor816.c",
		MAME_DIR .. "src/emu/bus/nubus/pds30_sigmalview.c",
		MAME_DIR .. "src/emu/bus/nubus/pds30_30hr.c",
		MAME_DIR .. "src/emu/bus/nubus/pds30_mc30.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/centronics/ctronics.h,BUSES += CENTRONICS
---------------------------------------------------

if (BUSES["CENTRONICS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/centronics/ctronics.c",
		MAME_DIR .. "src/emu/bus/centronics/comxpl80.c",
		MAME_DIR .. "src/emu/bus/centronics/covox.c",
		MAME_DIR .. "src/emu/bus/centronics/dsjoy.c",
		MAME_DIR .. "src/emu/bus/centronics/epson_ex800.c",
		MAME_DIR .. "src/emu/bus/centronics/epson_lx800.c",
		MAME_DIR .. "src/emu/bus/centronics/epson_lx810l.c",
		MAME_DIR .. "src/emu/bus/centronics/printer.c",
		MAME_DIR .. "src/emu/bus/centronics/digiblst.c",
	}
	
	dependency {
		{ MAME_DIR .. "src/emu/bus/centronics/epson_ex800.c",  GEN_DIR .. "emu/layout/ex800.lh" },
		{ MAME_DIR .. "src/emu/bus/centronics/epson_lx800.c",  GEN_DIR .. "emu/layout/lx800.lh" },
		{ MAME_DIR .. "src/emu/bus/centronics/epson_lx810l.c", GEN_DIR .. "emu/layout/lx800.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "ex800"),
		layoutbuildtask("emu/layout", "lx800"),
	}		
end

---------------------------------------------------
--
--@src/emu/bus/rs232/rs232.h,BUSES += RS232
---------------------------------------------------

if (BUSES["RS232"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/rs232/keyboard.c",
		MAME_DIR .. "src/emu/bus/rs232/loopback.c",
		MAME_DIR .. "src/emu/bus/rs232/null_modem.c",
		MAME_DIR .. "src/emu/bus/rs232/printer.c",
		MAME_DIR .. "src/emu/bus/rs232/rs232.c",
		MAME_DIR .. "src/emu/bus/rs232/ser_mouse.c",
		MAME_DIR .. "src/emu/bus/rs232/terminal.c",
		MAME_DIR .. "src/emu/bus/rs232/xvd701.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/midi/midi.h,BUSES += MIDI
---------------------------------------------------

if (BUSES["MIDI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/midi/midi.c",
		MAME_DIR .. "src/emu/bus/midi/midiinport.c",
		MAME_DIR .. "src/emu/bus/midi/midioutport.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/lpci/pci.h,BUSES += LPCI
---------------------------------------------------

if (BUSES["LPCI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/lpci/pci.c",
		MAME_DIR .. "src/emu/bus/lpci/cirrus.c",
		MAME_DIR .. "src/emu/bus/lpci/i82371ab.c",
		MAME_DIR .. "src/emu/bus/lpci/i82371sb.c",
		MAME_DIR .. "src/emu/bus/lpci/i82439tx.c",
		MAME_DIR .. "src/emu/bus/lpci/northbridge.c",
		MAME_DIR .. "src/emu/bus/lpci/southbridge.c",
		MAME_DIR .. "src/emu/bus/lpci/mpc105.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nes/nes_slot.h,BUSES += NES
---------------------------------------------------

if (BUSES["NES"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nes/nes_slot.c",
		MAME_DIR .. "src/emu/bus/nes/nes_carts.c",
		MAME_DIR .. "src/emu/bus/nes/2a03pur.c",
		MAME_DIR .. "src/emu/bus/nes/act53.c",
		MAME_DIR .. "src/emu/bus/nes/aladdin.c",
		MAME_DIR .. "src/emu/bus/nes/ave.c",
		MAME_DIR .. "src/emu/bus/nes/bandai.c",
		MAME_DIR .. "src/emu/bus/nes/benshieng.c",
		MAME_DIR .. "src/emu/bus/nes/bootleg.c",
		MAME_DIR .. "src/emu/bus/nes/camerica.c",
		MAME_DIR .. "src/emu/bus/nes/cne.c",
		MAME_DIR .. "src/emu/bus/nes/cony.c",
		MAME_DIR .. "src/emu/bus/nes/datach.c",
		MAME_DIR .. "src/emu/bus/nes/discrete.c",
		MAME_DIR .. "src/emu/bus/nes/disksys.c",
		MAME_DIR .. "src/emu/bus/nes/event.c",
		MAME_DIR .. "src/emu/bus/nes/ggenie.c",
		MAME_DIR .. "src/emu/bus/nes/henggedianzi.c",
		MAME_DIR .. "src/emu/bus/nes/hes.c",
		MAME_DIR .. "src/emu/bus/nes/hosenkan.c",
		MAME_DIR .. "src/emu/bus/nes/irem.c",
		MAME_DIR .. "src/emu/bus/nes/jaleco.c",
		MAME_DIR .. "src/emu/bus/nes/jy.c",
		MAME_DIR .. "src/emu/bus/nes/kaiser.c",
		MAME_DIR .. "src/emu/bus/nes/karastudio.c",
		MAME_DIR .. "src/emu/bus/nes/konami.c",
		MAME_DIR .. "src/emu/bus/nes/legacy.c",
		MAME_DIR .. "src/emu/bus/nes/mmc1.c",
		MAME_DIR .. "src/emu/bus/nes/mmc2.c",
		MAME_DIR .. "src/emu/bus/nes/mmc3.c",
		MAME_DIR .. "src/emu/bus/nes/mmc3_clones.c",
		MAME_DIR .. "src/emu/bus/nes/mmc5.c",
		MAME_DIR .. "src/emu/bus/nes/multigame.c",
		MAME_DIR .. "src/emu/bus/nes/namcot.c",
		MAME_DIR .. "src/emu/bus/nes/nanjing.c",
		MAME_DIR .. "src/emu/bus/nes/ntdec.c",
		MAME_DIR .. "src/emu/bus/nes/nxrom.c",
		MAME_DIR .. "src/emu/bus/nes/pirate.c",
		MAME_DIR .. "src/emu/bus/nes/pt554.c",
		MAME_DIR .. "src/emu/bus/nes/racermate.c",
		MAME_DIR .. "src/emu/bus/nes/rcm.c",
		MAME_DIR .. "src/emu/bus/nes/rexsoft.c",
		MAME_DIR .. "src/emu/bus/nes/sachen.c",
		MAME_DIR .. "src/emu/bus/nes/somari.c",
		MAME_DIR .. "src/emu/bus/nes/sunsoft.c",
		MAME_DIR .. "src/emu/bus/nes/sunsoft_dcs.c",
		MAME_DIR .. "src/emu/bus/nes/taito.c",
		MAME_DIR .. "src/emu/bus/nes/tengen.c",
		MAME_DIR .. "src/emu/bus/nes/txc.c",
		MAME_DIR .. "src/emu/bus/nes/waixing.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/nes_ctrl/ctrl.h,BUSES += NES_CTRL
---------------------------------------------------

if (BUSES["NES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/nes_ctrl/ctrl.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/joypad.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/4score.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/arkpaddle.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/bcbattle.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/ftrainer.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/fckeybrd.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/hori.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/konamihs.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/miracle.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/mjpanel.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/pachinko.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/partytap.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/powerpad.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/suborkey.c",
		MAME_DIR .. "src/emu/bus/nes_ctrl/zapper.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/snes/snes_slot.h,BUSES += SNES
---------------------------------------------------

if (BUSES["SNES"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/snes/snes_slot.c",
		MAME_DIR .. "src/emu/bus/snes/snes_carts.c",
		MAME_DIR .. "src/emu/bus/snes/bsx.c",
		MAME_DIR .. "src/emu/bus/snes/event.c",
		MAME_DIR .. "src/emu/bus/snes/rom.c",
		MAME_DIR .. "src/emu/bus/snes/rom21.c",
		MAME_DIR .. "src/emu/bus/snes/sa1.c",
		MAME_DIR .. "src/emu/bus/snes/sdd1.c",
		MAME_DIR .. "src/emu/bus/snes/sfx.c",
		MAME_DIR .. "src/emu/bus/snes/sgb.c",
		MAME_DIR .. "src/emu/bus/snes/spc7110.c",
		MAME_DIR .. "src/emu/bus/snes/sufami.c",
		MAME_DIR .. "src/emu/bus/snes/upd.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/snes_ctrl/ctrl.h,BUSES += SNES_CTRL
---------------------------------------------------

if (BUSES["SNES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/snes_ctrl/ctrl.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/bcbattle.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/joypad.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/miracle.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/mouse.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/multitap.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/pachinko.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/sscope.c",
		MAME_DIR .. "src/emu/bus/snes_ctrl/twintap.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vboy/slot.h,BUSES += VBOY
---------------------------------------------------
if (BUSES["VBOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vboy/slot.c",
		MAME_DIR .. "src/emu/bus/vboy/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/megadrive/md_slot.h,BUSES += MEGADRIVE
---------------------------------------------------

if (BUSES["MEGADRIVE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/megadrive/md_slot.c",
		MAME_DIR .. "src/emu/bus/megadrive/md_carts.c",
		MAME_DIR .. "src/emu/bus/megadrive/eeprom.c",
		MAME_DIR .. "src/emu/bus/megadrive/ggenie.c",
		MAME_DIR .. "src/emu/bus/megadrive/jcart.c",
		MAME_DIR .. "src/emu/bus/megadrive/rom.c",
		MAME_DIR .. "src/emu/bus/megadrive/sk.c",
		MAME_DIR .. "src/emu/bus/megadrive/stm95.c",
		MAME_DIR .. "src/emu/bus/megadrive/svp.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/neogeo/neogeo_slot.h,BUSES += NEOGEO
---------------------------------------------------

if (BUSES["NEOGEO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_slot.c",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_intf.c",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_carts.c",
		MAME_DIR .. "src/emu/bus/neogeo/neogeo_helper.c",
		MAME_DIR .. "src/emu/bus/neogeo/banked_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/mslugx_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/mslugx_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/sma_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/sma_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/cmc_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/cmc_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/pcm2_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/pcm2_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/kof2002_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/kof2002_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/pvc_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/pvc_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/fatfury2_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/fatfury2_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/kof98_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/kof98_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/bootleg_hybrid_cart.c",
		MAME_DIR .. "src/emu/bus/neogeo/sbp_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/kog_prot.c",
		MAME_DIR .. "src/emu/bus/neogeo/rom.c",
	}
end


---------------------------------------------------
--
--@src/emu/bus/saturn/sat_slot.h,BUSES += SATURN
---------------------------------------------------

if (BUSES["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/saturn/sat_slot.c",
		MAME_DIR .. "src/emu/bus/saturn/bram.c",
		MAME_DIR .. "src/emu/bus/saturn/dram.c",
		MAME_DIR .. "src/emu/bus/saturn/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sega8/sega8_slot.h,BUSES += SEGA8
---------------------------------------------------

if (BUSES["SEGA8"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sega8/sega8_slot.c",
		MAME_DIR .. "src/emu/bus/sega8/rom.c",
		MAME_DIR .. "src/emu/bus/sega8/ccatch.c",
		MAME_DIR .. "src/emu/bus/sega8/mgear.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sms_ctrl/smsctrl.h,BUSES += SMS_CTRL
---------------------------------------------------

if (BUSES["SMS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sms_ctrl/smsctrl.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/joypad.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/lphaser.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/paddle.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/rfu.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/sports.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/sportsjp.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/multitap.c",
		MAME_DIR .. "src/emu/bus/sms_ctrl/graphic.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/sms_exp/smsexp.h,BUSES += SMS_EXP
---------------------------------------------------

if (BUSES["SMS_EXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/sms_exp/smsexp.c",
		MAME_DIR .. "src/emu/bus/sms_exp/gender.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/ti99_peb/peribox.h,BUSES += TI99PEB
---------------------------------------------------

if (BUSES["TI99PEB"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ti99_peb/peribox.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/bwg.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/evpc.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/hfdc.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/horizon.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/hsgpl.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/memex.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/myarcmem.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/pcode.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/samsmem.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/spchsyn.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_32kmem.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_fdc.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/ti_rs232.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/tn_ide.c",
		MAME_DIR .. "src/emu/bus/ti99_peb/tn_usbsm.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gameboy/gb_slot.h,BUSES += GAMEBOY
---------------------------------------------------

if (BUSES["GAMEBOY"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gameboy/gb_slot.c",
		MAME_DIR .. "src/emu/bus/gameboy/rom.c",
		MAME_DIR .. "src/emu/bus/gameboy/mbc.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gamegear/ggext.h,BUSES += GAMEGEAR
---------------------------------------------------

if (BUSES["GAMEGEAR"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gamegear/ggext.c",
		MAME_DIR .. "src/emu/bus/gamegear/smsctrladp.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/gba/gba_slot.h,BUSES += GBA
---------------------------------------------------

if (BUSES["GBA"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/gba/gba_slot.c",
		MAME_DIR .. "src/emu/bus/gba/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/bml3/bml3bus.h,BUSES += BML3
---------------------------------------------------
if (BUSES["BML3"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/bml3/bml3bus.c",
		MAME_DIR .. "src/emu/bus/bml3/bml3mp1802.c",
		MAME_DIR .. "src/emu/bus/bml3/bml3mp1805.c",
		MAME_DIR .. "src/emu/bus/bml3/bml3kanji.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/coco/cococart.h,BUSES += COCO
---------------------------------------------------
if (BUSES["COCO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/coco/cococart.c",
		MAME_DIR .. "src/emu/bus/coco/coco_232.c",
		MAME_DIR .. "src/emu/bus/coco/coco_orch90.c",
		MAME_DIR .. "src/emu/bus/coco/coco_pak.c",
		MAME_DIR .. "src/emu/bus/coco/coco_fdc.c",
		MAME_DIR .. "src/emu/bus/coco/coco_multi.c",
		MAME_DIR .. "src/emu/bus/coco/coco_dwsock.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/cpc/cpcexp.h,BUSES += CPC
---------------------------------------------------
if (BUSES["CPC"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/cpc/cpcexp.c",
		MAME_DIR .. "src/emu/bus/cpc/cpc_ssa1.c",
		MAME_DIR .. "src/emu/bus/cpc/cpc_rom.c",
		MAME_DIR .. "src/emu/bus/cpc/cpc_pds.c",
		MAME_DIR .. "src/emu/bus/cpc/cpc_rs232.c",
		MAME_DIR .. "src/emu/bus/cpc/mface2.c",
		MAME_DIR .. "src/emu/bus/cpc/symbfac2.c",
		MAME_DIR .. "src/emu/bus/cpc/amdrum.c",
		MAME_DIR .. "src/emu/bus/cpc/playcity.c",
		MAME_DIR .. "src/emu/bus/cpc/smartwatch.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/epson_sio/epson_sio.h,BUSES += EPSON_SIO
---------------------------------------------------
if (BUSES["EPSON_SIO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/epson_sio/epson_sio.c",
		MAME_DIR .. "src/emu/bus/epson_sio/pf10.c",
		MAME_DIR .. "src/emu/bus/epson_sio/tf20.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/pce/pce_slot.h,BUSES += PCE
---------------------------------------------------
if (BUSES["PCE"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/pce/pce_slot.c",
		MAME_DIR .. "src/emu/bus/pce/pce_rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/scv/slot.h,BUSES += SCV
---------------------------------------------------
if (BUSES["SCV"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/scv/slot.c",
		MAME_DIR .. "src/emu/bus/scv/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/x68k/x68kexp.h,BUSES += X68K
---------------------------------------------------
if (BUSES["X68K"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/x68k/x68kexp.c",
		MAME_DIR .. "src/emu/bus/x68k/x68k_neptunex.c",
		MAME_DIR .. "src/emu/bus/x68k/x68k_scsiext.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/abckb/abckb.h,BUSES += ABCKB
---------------------------------------------------
if (BUSES["ABCKB"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/abckb/abckb.c",
		MAME_DIR .. "src/emu/bus/abckb/abc77.c",
		MAME_DIR .. "src/emu/bus/abckb/abc99.c",
		MAME_DIR .. "src/emu/bus/abckb/abc800kb.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/compucolor/compclr_flp.h,BUSES += COMPUCOLOR
---------------------------------------------------
if (BUSES["COMPUCOLOR"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/compucolor/floppy.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/scsi/scsi.h,BUSES += SCSI
---------------------------------------------------
if (BUSES["SCSI"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/scsi/scsi.c",
		MAME_DIR .. "src/emu/bus/scsi/scsicd.c",
		MAME_DIR .. "src/emu/bus/scsi/scsihd.c",
		MAME_DIR .. "src/emu/bus/scsi/scsihle.c",
		MAME_DIR .. "src/emu/bus/scsi/cdu76s.c",
		MAME_DIR .. "src/emu/bus/scsi/acb4070.c",
		MAME_DIR .. "src/emu/bus/scsi/d9060hd.c",
		MAME_DIR .. "src/emu/bus/scsi/sa1403d.c",
		MAME_DIR .. "src/emu/bus/scsi/s1410.c",
		MAME_DIR .. "src/emu/bus/scsi/pc9801_sasi.c",
		MAME_DIR .. "src/emu/bus/scsi/omti5100.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/macpds/macpds.h,BUSES += MACPDS
---------------------------------------------------
if (BUSES["MACPDS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/macpds/macpds.c",
		MAME_DIR .. "src/emu/bus/macpds/pds_tpdfpd.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/oricext/oricext.h,BUSES += ORICEXT
---------------------------------------------------
if (BUSES["ORICEXT"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/oricext/oricext.c",
		MAME_DIR .. "src/emu/bus/oricext/jasmin.c",
		MAME_DIR .. "src/emu/bus/oricext/microdisc.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/a1bus/a1bus.h,BUSES += A1BUS
---------------------------------------------------

if (BUSES["A1BUS"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/a1bus/a1bus.c",
		MAME_DIR .. "src/emu/bus/a1bus/a1cassette.c",
		MAME_DIR .. "src/emu/bus/a1bus/a1cffa.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/amiga/zorro/zorro.h,BUSES += ZORRO
---------------------------------------------------

if (BUSES["ZORRO"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/amiga/zorro/zorro.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/cards.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a2052.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a2232.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/a590.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/action_replay.c",
		MAME_DIR .. "src/emu/bus/amiga/zorro/buddha.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/ql/exp.h,BUSES += QL
---------------------------------------------------

if (BUSES["QL"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/ql/exp.c",
		MAME_DIR .. "src/emu/bus/ql/cst_qdisc.c",
		MAME_DIR .. "src/emu/bus/ql/cst_q_plus4.c",
		MAME_DIR .. "src/emu/bus/ql/cumana_fdi.c",
		MAME_DIR .. "src/emu/bus/ql/kempston_di.c",
		MAME_DIR .. "src/emu/bus/ql/miracle_gold_card.c",
		MAME_DIR .. "src/emu/bus/ql/mp_fdi.c",
		MAME_DIR .. "src/emu/bus/ql/opd_basic_master.c",
		MAME_DIR .. "src/emu/bus/ql/pcml_qdisk.c",
		MAME_DIR .. "src/emu/bus/ql/qubide.c",
		MAME_DIR .. "src/emu/bus/ql/sandy_superdisk.c",
		MAME_DIR .. "src/emu/bus/ql/sandy_superqboard.c",
		MAME_DIR .. "src/emu/bus/ql/trumpcard.c",
		MAME_DIR .. "src/emu/bus/ql/rom.c",
		MAME_DIR .. "src/emu/bus/ql/miracle_hd.c",
		MAME_DIR .. "src/emu/bus/ql/std.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vtech/memexp/memexp.h,BUSES += VTECH_MEMEXP
---------------------------------------------------

if (BUSES["VTECH_MEMEXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vtech/memexp/memexp.c",
		MAME_DIR .. "src/emu/bus/vtech/memexp/carts.c",
		MAME_DIR .. "src/emu/bus/vtech/memexp/floppy.c",
		MAME_DIR .. "src/emu/bus/vtech/memexp/memory.c",
		MAME_DIR .. "src/emu/bus/vtech/memexp/rs232.c",
		MAME_DIR .. "src/emu/bus/vtech/memexp/wordpro.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/vtech/ioexp/ioexp.h,BUSES += VTECH_IOEXP
---------------------------------------------------

if (BUSES["VTECH_IOEXP"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/vtech/ioexp/ioexp.c",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/carts.c",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/joystick.c",
		MAME_DIR .. "src/emu/bus/vtech/ioexp/printer.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/wswan/slot.h,BUSES += WSWAN
---------------------------------------------------

if (BUSES["WSWAN"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/wswan/slot.c",
		MAME_DIR .. "src/emu/bus/wswan/rom.c",
	}
end

---------------------------------------------------
--
--@src/emu/bus/psx/ctlrport.h,BUSES += PSX_CONTROLLER
---------------------------------------------------

if (BUSES["PSX_CONTROLLER"]~=null) then
	files {
		MAME_DIR .. "src/emu/bus/psx/ctlrport.c",
		MAME_DIR .. "src/emu/bus/psx/analogue.c",
		MAME_DIR .. "src/emu/bus/psx/multitap.c",
		MAME_DIR .. "src/emu/bus/psx/memcard.c",
	}
end
