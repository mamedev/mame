-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   bus.lua
--
--   Rules for building bus cores
--
---------------------------------------------------------------------------

-------------------------------------------------
--
--@src/devices/bus/a7800/a78_slot.h,BUSES["A7800"] = true
---------------------------------------------------

if (BUSES["A7800"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a7800/a78_slot.c",
		MAME_DIR .. "src/devices/bus/a7800/a78_slot.h",
		MAME_DIR .. "src/devices/bus/a7800/a78_carts.h",
		MAME_DIR .. "src/devices/bus/a7800/rom.c",
		MAME_DIR .. "src/devices/bus/a7800/rom.h",
		MAME_DIR .. "src/devices/bus/a7800/hiscore.c",
		MAME_DIR .. "src/devices/bus/a7800/hiscore.h",
		MAME_DIR .. "src/devices/bus/a7800/xboard.c",
		MAME_DIR .. "src/devices/bus/a7800/xboard.h",
		MAME_DIR .. "src/devices/bus/a7800/cpuwiz.c",
		MAME_DIR .. "src/devices/bus/a7800/cpuwiz.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/a800/a800_slot.h,BUSES["A800"] = true
---------------------------------------------------

if (BUSES["A800"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a800/a8sio.c",
		MAME_DIR .. "src/devices/bus/a800/a8sio.h",
		MAME_DIR .. "src/devices/bus/a800/cassette.c",
		MAME_DIR .. "src/devices/bus/a800/cassette.h",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.c",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.h",
		MAME_DIR .. "src/devices/bus/a800/a800_carts.h",
		MAME_DIR .. "src/devices/bus/a800/rom.c",
		MAME_DIR .. "src/devices/bus/a800/rom.h",
		MAME_DIR .. "src/devices/bus/a800/oss.c",
		MAME_DIR .. "src/devices/bus/a800/oss.h",
		MAME_DIR .. "src/devices/bus/a800/sparta.c",
		MAME_DIR .. "src/devices/bus/a800/sparta.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/abcbus/abcbus.h,BUSES["ABCBUS"] = true
---------------------------------------------------

if (BUSES["ABCBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/abcbus/abcbus.c",
		MAME_DIR .. "src/devices/bus/abcbus/abcbus.h",
		MAME_DIR .. "src/devices/bus/abcbus/abc890.c",
		MAME_DIR .. "src/devices/bus/abcbus/abc890.h",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.c",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.h",
		MAME_DIR .. "src/devices/bus/abcbus/hdc.c",
		MAME_DIR .. "src/devices/bus/abcbus/hdc.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux10828.c",
		MAME_DIR .. "src/devices/bus/abcbus/lux10828.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux21046.c",
		MAME_DIR .. "src/devices/bus/abcbus/lux21046.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux21056.c",
		MAME_DIR .. "src/devices/bus/abcbus/lux21056.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux4105.c",
		MAME_DIR .. "src/devices/bus/abcbus/lux4105.h",
		MAME_DIR .. "src/devices/bus/abcbus/memcard.c",
		MAME_DIR .. "src/devices/bus/abcbus/memcard.h",
		MAME_DIR .. "src/devices/bus/abcbus/ram.c",
		MAME_DIR .. "src/devices/bus/abcbus/ram.h",
		MAME_DIR .. "src/devices/bus/abcbus/sio.c",
		MAME_DIR .. "src/devices/bus/abcbus/sio.h",
		MAME_DIR .. "src/devices/bus/abcbus/slutprov.c",
		MAME_DIR .. "src/devices/bus/abcbus/slutprov.h",
		MAME_DIR .. "src/devices/bus/abcbus/turbo.c",
		MAME_DIR .. "src/devices/bus/abcbus/turbo.h",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.c",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/adam/exp.h,BUSES["ADAM"] = true
---------------------------------------------------

if (BUSES["ADAM"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/adam/exp.c",
		MAME_DIR .. "src/devices/bus/adam/exp.h",
		MAME_DIR .. "src/devices/bus/adam/adamlink.c",
		MAME_DIR .. "src/devices/bus/adam/adamlink.h",
		MAME_DIR .. "src/devices/bus/adam/ide.c",
		MAME_DIR .. "src/devices/bus/adam/ide.h",
		MAME_DIR .. "src/devices/bus/adam/ram.c",
		MAME_DIR .. "src/devices/bus/adam/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/adamnet/adamnet.h,BUSES["ADAMNET"] = true
---------------------------------------------------

if (BUSES["ADAMNET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/adamnet/adamnet.c",
		MAME_DIR .. "src/devices/bus/adamnet/adamnet.h",
		MAME_DIR .. "src/devices/bus/adamnet/ddp.c",
		MAME_DIR .. "src/devices/bus/adamnet/ddp.h",
		MAME_DIR .. "src/devices/bus/adamnet/fdc.c",
		MAME_DIR .. "src/devices/bus/adamnet/fdc.h",
		MAME_DIR .. "src/devices/bus/adamnet/kb.c",
		MAME_DIR .. "src/devices/bus/adamnet/kb.h",
		MAME_DIR .. "src/devices/bus/adamnet/printer.c",
		MAME_DIR .. "src/devices/bus/adamnet/printer.h",
		MAME_DIR .. "src/devices/bus/adamnet/spi.c",
		MAME_DIR .. "src/devices/bus/adamnet/spi.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apf/slot.h,BUSES["APF"] = true
---------------------------------------------------

if (BUSES["APF"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apf/slot.c",
		MAME_DIR .. "src/devices/bus/apf/slot.h",
		MAME_DIR .. "src/devices/bus/apf/rom.c",
		MAME_DIR .. "src/devices/bus/apf/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apricot/expansion.h,BUSES["APRICOT_EXPANSION"] = true
---------------------------------------------------

if (BUSES["APRICOT_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apricot/expansion.c",
		MAME_DIR .. "src/devices/bus/apricot/expansion.h",
		MAME_DIR .. "src/devices/bus/apricot/cards.c",
		MAME_DIR .. "src/devices/bus/apricot/cards.h",
		MAME_DIR .. "src/devices/bus/apricot/ram.c",
		MAME_DIR .. "src/devices/bus/apricot/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/arcadia/slot.h,BUSES["ARCADIA"] = true
---------------------------------------------------

if (BUSES["ARCADIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/arcadia/slot.c",
		MAME_DIR .. "src/devices/bus/arcadia/slot.h",
		MAME_DIR .. "src/devices/bus/arcadia/rom.c",
		MAME_DIR .. "src/devices/bus/arcadia/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/astrocde/slot.h,BUSES["ASTROCADE"] = true
---------------------------------------------------

if (BUSES["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/astrocde/slot.c",
		MAME_DIR .. "src/devices/bus/astrocde/slot.h",
		MAME_DIR .. "src/devices/bus/astrocde/rom.c",
		MAME_DIR .. "src/devices/bus/astrocde/rom.h",
		MAME_DIR .. "src/devices/bus/astrocde/exp.c",
		MAME_DIR .. "src/devices/bus/astrocde/exp.h",
		MAME_DIR .. "src/devices/bus/astrocde/ram.c",
		MAME_DIR .. "src/devices/bus/astrocde/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bw2/exp.h,BUSES["BW2"] = true
---------------------------------------------------

if (BUSES["BW2"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bw2/exp.c",
		MAME_DIR .. "src/devices/bus/bw2/exp.h",
		MAME_DIR .. "src/devices/bus/bw2/ramcard.c",
		MAME_DIR .. "src/devices/bus/bw2/ramcard.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/c64/exp.h,BUSES["C64"] = true
--@src/devices/bus/c64/user.h,BUSES["C64"] = true
---------------------------------------------------

if (BUSES["C64"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/c64/exp.c",
		MAME_DIR .. "src/devices/bus/c64/exp.h",
		MAME_DIR .. "src/devices/bus/c64/c128_comal80.c",
		MAME_DIR .. "src/devices/bus/c64/c128_comal80.h",
		MAME_DIR .. "src/devices/bus/c64/c128_partner.c",
		MAME_DIR .. "src/devices/bus/c64/c128_partner.h",
		MAME_DIR .. "src/devices/bus/c64/comal80.c",
		MAME_DIR .. "src/devices/bus/c64/comal80.h",
		MAME_DIR .. "src/devices/bus/c64/cpm.c",
		MAME_DIR .. "src/devices/bus/c64/cpm.h",
		MAME_DIR .. "src/devices/bus/c64/currah_speech.c",
		MAME_DIR .. "src/devices/bus/c64/currah_speech.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep256.c",
		MAME_DIR .. "src/devices/bus/c64/dela_ep256.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep64.c",
		MAME_DIR .. "src/devices/bus/c64/dela_ep64.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep7x8.c",
		MAME_DIR .. "src/devices/bus/c64/dela_ep7x8.h",
		MAME_DIR .. "src/devices/bus/c64/dinamic.c",
		MAME_DIR .. "src/devices/bus/c64/dinamic.h",
		MAME_DIR .. "src/devices/bus/c64/dqbb.c",
		MAME_DIR .. "src/devices/bus/c64/dqbb.h",
		MAME_DIR .. "src/devices/bus/c64/easy_calc_result.c",
		MAME_DIR .. "src/devices/bus/c64/easy_calc_result.h",
		MAME_DIR .. "src/devices/bus/c64/easyflash.c",
		MAME_DIR .. "src/devices/bus/c64/easyflash.h",
		MAME_DIR .. "src/devices/bus/c64/epyx_fast_load.c",
		MAME_DIR .. "src/devices/bus/c64/epyx_fast_load.h",
		MAME_DIR .. "src/devices/bus/c64/exos.c",
		MAME_DIR .. "src/devices/bus/c64/exos.h",
		MAME_DIR .. "src/devices/bus/c64/fcc.c",
		MAME_DIR .. "src/devices/bus/c64/fcc.h",
		MAME_DIR .. "src/devices/bus/c64/final.c",
		MAME_DIR .. "src/devices/bus/c64/final.h",
		MAME_DIR .. "src/devices/bus/c64/final3.c",
		MAME_DIR .. "src/devices/bus/c64/final3.h",
		MAME_DIR .. "src/devices/bus/c64/fun_play.c",
		MAME_DIR .. "src/devices/bus/c64/fun_play.h",
		MAME_DIR .. "src/devices/bus/c64/georam.c",
		MAME_DIR .. "src/devices/bus/c64/georam.h",
		MAME_DIR .. "src/devices/bus/c64/ide64.c",
		MAME_DIR .. "src/devices/bus/c64/ide64.h",
		MAME_DIR .. "src/devices/bus/c64/ieee488.c",
		MAME_DIR .. "src/devices/bus/c64/ieee488.h",
		MAME_DIR .. "src/devices/bus/c64/kingsoft.c",
		MAME_DIR .. "src/devices/bus/c64/kingsoft.h",
		MAME_DIR .. "src/devices/bus/c64/mach5.c",
		MAME_DIR .. "src/devices/bus/c64/mach5.h",
		MAME_DIR .. "src/devices/bus/c64/magic_desk.c",
		MAME_DIR .. "src/devices/bus/c64/magic_desk.h",
		MAME_DIR .. "src/devices/bus/c64/magic_formel.c",
		MAME_DIR .. "src/devices/bus/c64/magic_formel.h",
		MAME_DIR .. "src/devices/bus/c64/magic_voice.c",
		MAME_DIR .. "src/devices/bus/c64/magic_voice.h",
		MAME_DIR .. "src/devices/bus/c64/midi_maplin.c",
		MAME_DIR .. "src/devices/bus/c64/midi_maplin.h",
		MAME_DIR .. "src/devices/bus/c64/midi_namesoft.c",
		MAME_DIR .. "src/devices/bus/c64/midi_namesoft.h",
		MAME_DIR .. "src/devices/bus/c64/midi_passport.c",
		MAME_DIR .. "src/devices/bus/c64/midi_passport.h",
		MAME_DIR .. "src/devices/bus/c64/midi_sci.c",
		MAME_DIR .. "src/devices/bus/c64/midi_sci.h",
		MAME_DIR .. "src/devices/bus/c64/midi_siel.c",
		MAME_DIR .. "src/devices/bus/c64/midi_siel.h",
		MAME_DIR .. "src/devices/bus/c64/mikro_assembler.c",
		MAME_DIR .. "src/devices/bus/c64/mikro_assembler.h",
		MAME_DIR .. "src/devices/bus/c64/multiscreen.c",
		MAME_DIR .. "src/devices/bus/c64/multiscreen.h",
		MAME_DIR .. "src/devices/bus/c64/music64.c",
		MAME_DIR .. "src/devices/bus/c64/music64.h",
		MAME_DIR .. "src/devices/bus/c64/neoram.c",
		MAME_DIR .. "src/devices/bus/c64/neoram.h",
		MAME_DIR .. "src/devices/bus/c64/ocean.c",
		MAME_DIR .. "src/devices/bus/c64/ocean.h",
		MAME_DIR .. "src/devices/bus/c64/pagefox.c",
		MAME_DIR .. "src/devices/bus/c64/pagefox.h",
		MAME_DIR .. "src/devices/bus/c64/partner.c",
		MAME_DIR .. "src/devices/bus/c64/partner.h",
		MAME_DIR .. "src/devices/bus/c64/prophet64.c",
		MAME_DIR .. "src/devices/bus/c64/prophet64.h",
		MAME_DIR .. "src/devices/bus/c64/ps64.c",
		MAME_DIR .. "src/devices/bus/c64/ps64.h",
		MAME_DIR .. "src/devices/bus/c64/reu.c",
		MAME_DIR .. "src/devices/bus/c64/reu.h",
		MAME_DIR .. "src/devices/bus/c64/rex.c",
		MAME_DIR .. "src/devices/bus/c64/rex.h",
		MAME_DIR .. "src/devices/bus/c64/rex_ep256.c",
		MAME_DIR .. "src/devices/bus/c64/rex_ep256.h",
		MAME_DIR .. "src/devices/bus/c64/ross.c",
		MAME_DIR .. "src/devices/bus/c64/ross.h",
		MAME_DIR .. "src/devices/bus/c64/sfx_sound_expander.c",
		MAME_DIR .. "src/devices/bus/c64/sfx_sound_expander.h",
		MAME_DIR .. "src/devices/bus/c64/silverrock.c",
		MAME_DIR .. "src/devices/bus/c64/silverrock.h",
		MAME_DIR .. "src/devices/bus/c64/simons_basic.c",
		MAME_DIR .. "src/devices/bus/c64/simons_basic.h",
		MAME_DIR .. "src/devices/bus/c64/stardos.c",
		MAME_DIR .. "src/devices/bus/c64/stardos.h",
		MAME_DIR .. "src/devices/bus/c64/std.c",
		MAME_DIR .. "src/devices/bus/c64/std.h",
		MAME_DIR .. "src/devices/bus/c64/structured_basic.c",
		MAME_DIR .. "src/devices/bus/c64/structured_basic.h",
		MAME_DIR .. "src/devices/bus/c64/super_explode.c",
		MAME_DIR .. "src/devices/bus/c64/super_explode.h",
		MAME_DIR .. "src/devices/bus/c64/super_games.c",
		MAME_DIR .. "src/devices/bus/c64/super_games.h",
		MAME_DIR .. "src/devices/bus/c64/supercpu.c",
		MAME_DIR .. "src/devices/bus/c64/supercpu.h",
		MAME_DIR .. "src/devices/bus/c64/sw8k.c",
		MAME_DIR .. "src/devices/bus/c64/sw8k.h",
		MAME_DIR .. "src/devices/bus/c64/swiftlink.c",
		MAME_DIR .. "src/devices/bus/c64/swiftlink.h",
		MAME_DIR .. "src/devices/bus/c64/system3.c",
		MAME_DIR .. "src/devices/bus/c64/system3.h",
		MAME_DIR .. "src/devices/bus/c64/tdos.c",
		MAME_DIR .. "src/devices/bus/c64/tdos.h",
		MAME_DIR .. "src/devices/bus/c64/turbo232.c",
		MAME_DIR .. "src/devices/bus/c64/turbo232.h",
		MAME_DIR .. "src/devices/bus/c64/vizastar.c",
		MAME_DIR .. "src/devices/bus/c64/vizastar.h",
		MAME_DIR .. "src/devices/bus/c64/vw64.c",
		MAME_DIR .. "src/devices/bus/c64/vw64.h",
		MAME_DIR .. "src/devices/bus/c64/warp_speed.c",
		MAME_DIR .. "src/devices/bus/c64/warp_speed.h",
		MAME_DIR .. "src/devices/bus/c64/westermann.c",
		MAME_DIR .. "src/devices/bus/c64/westermann.h",
		MAME_DIR .. "src/devices/bus/c64/xl80.c",
		MAME_DIR .. "src/devices/bus/c64/xl80.h",
		MAME_DIR .. "src/devices/bus/c64/zaxxon.c",
		MAME_DIR .. "src/devices/bus/c64/zaxxon.h",
		MAME_DIR .. "src/devices/bus/c64/user.c",
		MAME_DIR .. "src/devices/bus/c64/user.h",
		MAME_DIR .. "src/devices/bus/c64/4dxh.c",
		MAME_DIR .. "src/devices/bus/c64/4dxh.h",
		MAME_DIR .. "src/devices/bus/c64/4ksa.c",
		MAME_DIR .. "src/devices/bus/c64/4ksa.h",
		MAME_DIR .. "src/devices/bus/c64/4tba.c",
		MAME_DIR .. "src/devices/bus/c64/4tba.h",
		MAME_DIR .. "src/devices/bus/c64/16kb.c",
		MAME_DIR .. "src/devices/bus/c64/16kb.h",
		MAME_DIR .. "src/devices/bus/c64/bn1541.c",
		MAME_DIR .. "src/devices/bus/c64/bn1541.h",
		MAME_DIR .. "src/devices/bus/c64/geocable.c",
		MAME_DIR .. "src/devices/bus/c64/geocable.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/cbm2/exp.h,BUSES["CBM2"] = true
--@src/devices/bus/cbm2/user.h,BUSES["CBM2"] = true
---------------------------------------------------

if (BUSES["CBM2"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cbm2/exp.c",
		MAME_DIR .. "src/devices/bus/cbm2/exp.h",
		MAME_DIR .. "src/devices/bus/cbm2/24k.c",
		MAME_DIR .. "src/devices/bus/cbm2/24k.h",
		MAME_DIR .. "src/devices/bus/cbm2/hrg.c",
		MAME_DIR .. "src/devices/bus/cbm2/hrg.h",
		MAME_DIR .. "src/devices/bus/cbm2/std.c",
		MAME_DIR .. "src/devices/bus/cbm2/std.h",
		MAME_DIR .. "src/devices/bus/cbm2/user.c",
		MAME_DIR .. "src/devices/bus/cbm2/user.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/cbmiec/cbmiec.h,BUSES["CBMIEC"] = true
---------------------------------------------------

if (BUSES["CBMIEC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cbmiec/cbmiec.c",
		MAME_DIR .. "src/devices/bus/cbmiec/cbmiec.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1541.c",
		MAME_DIR .. "src/devices/bus/cbmiec/c1541.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1571.c",
		MAME_DIR .. "src/devices/bus/cbmiec/c1571.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1581.c",
		MAME_DIR .. "src/devices/bus/cbmiec/c1581.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c64_nl10.c",
		MAME_DIR .. "src/devices/bus/cbmiec/c64_nl10.h",
		MAME_DIR .. "src/devices/bus/cbmiec/cmdhd.c",
		MAME_DIR .. "src/devices/bus/cbmiec/cmdhd.h",
		MAME_DIR .. "src/devices/bus/cbmiec/diag264_lb_iec.c",
		MAME_DIR .. "src/devices/bus/cbmiec/diag264_lb_iec.h",
		MAME_DIR .. "src/devices/bus/cbmiec/fd2000.c",
		MAME_DIR .. "src/devices/bus/cbmiec/fd2000.h",
		MAME_DIR .. "src/devices/bus/cbmiec/interpod.c",
		MAME_DIR .. "src/devices/bus/cbmiec/interpod.h",
		MAME_DIR .. "src/devices/bus/cbmiec/serialbox.c",
		MAME_DIR .. "src/devices/bus/cbmiec/serialbox.h",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1515.c",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1515.h",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1520.c",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1520.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1526.c",
		MAME_DIR .. "src/devices/bus/cbmiec/c1526.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/chanf/slot.h,BUSES["CHANNELF"] = true
---------------------------------------------------

if (BUSES["CHANNELF"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/chanf/slot.c",
		MAME_DIR .. "src/devices/bus/chanf/slot.h",
		MAME_DIR .. "src/devices/bus/chanf/rom.c",
		MAME_DIR .. "src/devices/bus/chanf/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/comx35/exp.h,BUSES["COMX35"] = true
---------------------------------------------------

if (BUSES["COMX35"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/comx35/exp.c",
		MAME_DIR .. "src/devices/bus/comx35/exp.h",
		MAME_DIR .. "src/devices/bus/comx35/clm.c",
		MAME_DIR .. "src/devices/bus/comx35/clm.h",
		MAME_DIR .. "src/devices/bus/comx35/expbox.c",
		MAME_DIR .. "src/devices/bus/comx35/expbox.h",
		MAME_DIR .. "src/devices/bus/comx35/eprom.c",
		MAME_DIR .. "src/devices/bus/comx35/eprom.h",
		MAME_DIR .. "src/devices/bus/comx35/fdc.c",
		MAME_DIR .. "src/devices/bus/comx35/fdc.h",
		MAME_DIR .. "src/devices/bus/comx35/joycard.c",
		MAME_DIR .. "src/devices/bus/comx35/joycard.h",
		MAME_DIR .. "src/devices/bus/comx35/printer.c",
		MAME_DIR .. "src/devices/bus/comx35/printer.h",
		MAME_DIR .. "src/devices/bus/comx35/ram.c",
		MAME_DIR .. "src/devices/bus/comx35/ram.h",
		MAME_DIR .. "src/devices/bus/comx35/thermal.c",
		MAME_DIR .. "src/devices/bus/comx35/thermal.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/coleco/ctrl.h,BUSES["COLECO"] = true
---------------------------------------------------

if (BUSES["COLECO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coleco/ctrl.c",
		MAME_DIR .. "src/devices/bus/coleco/ctrl.h",
		MAME_DIR .. "src/devices/bus/coleco/hand.c",
		MAME_DIR .. "src/devices/bus/coleco/hand.h",
		MAME_DIR .. "src/devices/bus/coleco/sac.c",
		MAME_DIR .. "src/devices/bus/coleco/sac.h",
		MAME_DIR .. "src/devices/bus/coleco/exp.c",
		MAME_DIR .. "src/devices/bus/coleco/exp.h",
		MAME_DIR .. "src/devices/bus/coleco/std.c",
		MAME_DIR .. "src/devices/bus/coleco/std.h",
		MAME_DIR .. "src/devices/bus/coleco/xin1.h",
		MAME_DIR .. "src/devices/bus/coleco/xin1.c",
	}
end


---------------------------------------------------
--
--@src/devices/bus/crvision/slot.h,BUSES["CRVISION"] = true
---------------------------------------------------

if (BUSES["CRVISION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/crvision/slot.c",
		MAME_DIR .. "src/devices/bus/crvision/slot.h",
		MAME_DIR .. "src/devices/bus/crvision/rom.c",
		MAME_DIR .. "src/devices/bus/crvision/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/dmv/dmv.h,BUSES["DMV"] = true
---------------------------------------------------

if (BUSES["DMV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.c",
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.h",
		MAME_DIR .. "src/devices/bus/dmv/k210.c",
		MAME_DIR .. "src/devices/bus/dmv/k210.h",
		MAME_DIR .. "src/devices/bus/dmv/k220.c",
		MAME_DIR .. "src/devices/bus/dmv/k220.h",
		MAME_DIR .. "src/devices/bus/dmv/k230.c",
		MAME_DIR .. "src/devices/bus/dmv/k230.h",
		MAME_DIR .. "src/devices/bus/dmv/k233.c",
		MAME_DIR .. "src/devices/bus/dmv/k233.h",
		MAME_DIR .. "src/devices/bus/dmv/k801.c",
		MAME_DIR .. "src/devices/bus/dmv/k801.h",
		MAME_DIR .. "src/devices/bus/dmv/k803.c",
		MAME_DIR .. "src/devices/bus/dmv/k803.h",
		MAME_DIR .. "src/devices/bus/dmv/k806.c",
		MAME_DIR .. "src/devices/bus/dmv/k806.h",
		MAME_DIR .. "src/devices/bus/dmv/ram.c",
		MAME_DIR .. "src/devices/bus/dmv/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ecbbus/ecbbus.h,BUSES["ECBBUS"] = true
---------------------------------------------------

if (BUSES["ECBBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ecbbus/ecbbus.c",
		MAME_DIR .. "src/devices/bus/ecbbus/ecbbus.h",
		MAME_DIR .. "src/devices/bus/ecbbus/grip.c",
		MAME_DIR .. "src/devices/bus/ecbbus/grip.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/econet/econet.h,BUSES["ECONET"] = true
---------------------------------------------------

if (BUSES["ECONET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/econet/econet.c",
		MAME_DIR .. "src/devices/bus/econet/econet.h",
		MAME_DIR .. "src/devices/bus/econet/e01.c",
		MAME_DIR .. "src/devices/bus/econet/e01.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ep64/exp.h,BUSES["EP64"] = true
---------------------------------------------------

if (BUSES["EP64"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ep64/exp.c",
		MAME_DIR .. "src/devices/bus/ep64/exp.h",
		MAME_DIR .. "src/devices/bus/ep64/exdos.c",
		MAME_DIR .. "src/devices/bus/ep64/exdos.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/generic/slot.h,BUSES["GENERIC"] = true
---------------------------------------------------

if (BUSES["GENERIC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/generic/slot.c",
		MAME_DIR .. "src/devices/bus/generic/slot.h",
		MAME_DIR .. "src/devices/bus/generic/carts.c",
		MAME_DIR .. "src/devices/bus/generic/carts.h",
		MAME_DIR .. "src/devices/bus/generic/ram.c",
		MAME_DIR .. "src/devices/bus/generic/ram.h",
		MAME_DIR .. "src/devices/bus/generic/rom.c",
		MAME_DIR .. "src/devices/bus/generic/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ieee488/ieee488.h,BUSES["IEEE488"] = true
---------------------------------------------------

if (BUSES["IEEE488"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ieee488/ieee488.c",
		MAME_DIR .. "src/devices/bus/ieee488/ieee488.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2031.c",
		MAME_DIR .. "src/devices/bus/ieee488/c2031.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2040.c",
		MAME_DIR .. "src/devices/bus/ieee488/c2040.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2040fdc.c",
		MAME_DIR .. "src/devices/bus/ieee488/c2040fdc.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8050.c",
		MAME_DIR .. "src/devices/bus/ieee488/c8050.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8050fdc.c",
		MAME_DIR .. "src/devices/bus/ieee488/c8050fdc.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8280.c",
		MAME_DIR .. "src/devices/bus/ieee488/c8280.h",
		MAME_DIR .. "src/devices/bus/ieee488/d9060.c",
		MAME_DIR .. "src/devices/bus/ieee488/d9060.h",
		MAME_DIR .. "src/devices/bus/ieee488/softbox.c",
		MAME_DIR .. "src/devices/bus/ieee488/softbox.h",
		MAME_DIR .. "src/devices/bus/ieee488/hardbox.c",
		MAME_DIR .. "src/devices/bus/ieee488/hardbox.h",
		MAME_DIR .. "src/devices/bus/ieee488/shark.c",
		MAME_DIR .. "src/devices/bus/ieee488/shark.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/iq151/iq151.h,BUSES["IQ151"] = true
---------------------------------------------------

if (BUSES["IQ151"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/iq151/iq151.c",
		MAME_DIR .. "src/devices/bus/iq151/iq151.h",
		MAME_DIR .. "src/devices/bus/iq151/disc2.c",
		MAME_DIR .. "src/devices/bus/iq151/disc2.h",
		MAME_DIR .. "src/devices/bus/iq151/grafik.c",
		MAME_DIR .. "src/devices/bus/iq151/grafik.h",
		MAME_DIR .. "src/devices/bus/iq151/minigraf.c",
		MAME_DIR .. "src/devices/bus/iq151/minigraf.h",
		MAME_DIR .. "src/devices/bus/iq151/ms151a.c",
		MAME_DIR .. "src/devices/bus/iq151/ms151a.h",
		MAME_DIR .. "src/devices/bus/iq151/rom.c",
		MAME_DIR .. "src/devices/bus/iq151/rom.h",
		MAME_DIR .. "src/devices/bus/iq151/staper.c",
		MAME_DIR .. "src/devices/bus/iq151/staper.h",
		MAME_DIR .. "src/devices/bus/iq151/video32.c",
		MAME_DIR .. "src/devices/bus/iq151/video32.h",
		MAME_DIR .. "src/devices/bus/iq151/video64.c",
		MAME_DIR .. "src/devices/bus/iq151/video64.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/isbx/isbx.h,BUSES["IMI7000"] = true
---------------------------------------------------

if (BUSES["IMI7000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/imi7000/imi7000.c",
		MAME_DIR .. "src/devices/bus/imi7000/imi7000.h",
		MAME_DIR .. "src/devices/bus/imi7000/imi5000h.c",
		MAME_DIR .. "src/devices/bus/imi7000/imi5000h.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/intv/slot.h,BUSES["INTV"] = true
---------------------------------------------------

if (BUSES["INTV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/intv/slot.c",
		MAME_DIR .. "src/devices/bus/intv/slot.h",
		MAME_DIR .. "src/devices/bus/intv/rom.c",
		MAME_DIR .. "src/devices/bus/intv/rom.h",
		MAME_DIR .. "src/devices/bus/intv/voice.c",
		MAME_DIR .. "src/devices/bus/intv/voice.h",
		MAME_DIR .. "src/devices/bus/intv/ecs.c",
		MAME_DIR .. "src/devices/bus/intv/ecs.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/isa/isa.h,BUSES["ISA"] = true
---------------------------------------------------

if (BUSES["ISA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/isa/isa.c",
		MAME_DIR .. "src/devices/bus/isa/isa.h",
		MAME_DIR .. "src/devices/bus/isa/isa_cards.c",
		MAME_DIR .. "src/devices/bus/isa/isa_cards.h",
		MAME_DIR .. "src/devices/bus/isa/mda.c",
		MAME_DIR .. "src/devices/bus/isa/mda.h",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.c",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.h",
		MAME_DIR .. "src/devices/bus/isa/adlib.c",
		MAME_DIR .. "src/devices/bus/isa/adlib.h",
		MAME_DIR .. "src/devices/bus/isa/com.c",
		MAME_DIR .. "src/devices/bus/isa/com.h",
		MAME_DIR .. "src/devices/bus/isa/fdc.c",
		MAME_DIR .. "src/devices/bus/isa/fdc.h",
		MAME_DIR .. "src/devices/bus/isa/mufdc.c",
		MAME_DIR .. "src/devices/bus/isa/mufdc.h",
		MAME_DIR .. "src/devices/bus/isa/finalchs.c",
		MAME_DIR .. "src/devices/bus/isa/finalchs.h",
		MAME_DIR .. "src/devices/bus/isa/gblaster.c",
		MAME_DIR .. "src/devices/bus/isa/gblaster.h",
		MAME_DIR .. "src/devices/bus/isa/gus.c",
		MAME_DIR .. "src/devices/bus/isa/gus.h",
		MAME_DIR .. "src/devices/bus/isa/sb16.c",
		MAME_DIR .. "src/devices/bus/isa/sb16.h",
		MAME_DIR .. "src/devices/bus/isa/hdc.c",
		MAME_DIR .. "src/devices/bus/isa/hdc.h",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.c",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.h",
		MAME_DIR .. "src/devices/bus/isa/mpu401.c",
		MAME_DIR .. "src/devices/bus/isa/mpu401.h",
		MAME_DIR .. "src/devices/bus/isa/sblaster.c",
		MAME_DIR .. "src/devices/bus/isa/sblaster.h",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.c",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.h",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.c",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.h",
		MAME_DIR .. "src/devices/bus/isa/ide.c",
		MAME_DIR .. "src/devices/bus/isa/ide.h",
		MAME_DIR .. "src/devices/bus/isa/xtide.c",
		MAME_DIR .. "src/devices/bus/isa/xtide.h",
		MAME_DIR .. "src/devices/bus/isa/side116.c",
		MAME_DIR .. "src/devices/bus/isa/side116.h",
		MAME_DIR .. "src/devices/bus/isa/aha1542.c",
		MAME_DIR .. "src/devices/bus/isa/aha1542.h",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.c",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.h",
		MAME_DIR .. "src/devices/bus/isa/dectalk.c",
		MAME_DIR .. "src/devices/bus/isa/dectalk.h",
		MAME_DIR .. "src/devices/bus/isa/pds.c",
		MAME_DIR .. "src/devices/bus/isa/pds.h",
		MAME_DIR .. "src/devices/bus/isa/omti8621.c",
		MAME_DIR .. "src/devices/bus/isa/omti8621.h",
		MAME_DIR .. "src/devices/bus/isa/cga.c",
		MAME_DIR .. "src/devices/bus/isa/cga.h",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.c",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.h",
		MAME_DIR .. "src/devices/bus/isa/ega.c",
		MAME_DIR .. "src/devices/bus/isa/ega.h",
		MAME_DIR .. "src/devices/bus/isa/pgc.c",
		MAME_DIR .. "src/devices/bus/isa/pgc.h",
		MAME_DIR .. "src/devices/bus/isa/vga.c",
		MAME_DIR .. "src/devices/bus/isa/vga.h",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.c",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.h",
		MAME_DIR .. "src/devices/bus/isa/mach32.c",
		MAME_DIR .. "src/devices/bus/isa/mach32.h",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.c",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.h",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.c",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.h",
		MAME_DIR .. "src/devices/bus/isa/s3virge.c",
		MAME_DIR .. "src/devices/bus/isa/s3virge.h",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.c",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.h",
		MAME_DIR .. "src/devices/bus/isa/3c503.c",
		MAME_DIR .. "src/devices/bus/isa/3c503.h",
		MAME_DIR .. "src/devices/bus/isa/ne1000.c",
		MAME_DIR .. "src/devices/bus/isa/ne1000.h",
		MAME_DIR .. "src/devices/bus/isa/ne2000.c",
		MAME_DIR .. "src/devices/bus/isa/ne2000.h",
		MAME_DIR .. "src/devices/bus/isa/3c505.c",
		MAME_DIR .. "src/devices/bus/isa/3c505.h",
		MAME_DIR .. "src/devices/bus/isa/lpt.c",
		MAME_DIR .. "src/devices/bus/isa/lpt.h",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.c",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.c",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.c",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.c",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.c",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.h",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.c",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.h",
		MAME_DIR .. "src/devices/bus/isa/sc499.c",
		MAME_DIR .. "src/devices/bus/isa/sc499.h",
		MAME_DIR .. "src/devices/bus/isa/aga.c",
		MAME_DIR .. "src/devices/bus/isa/aga.h",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.c",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.h",
		MAME_DIR .. "src/devices/bus/isa/num9rev.c",
		MAME_DIR .. "src/devices/bus/isa/num9rev.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/isbx/isbx.h,BUSES["ISBX"] = true
---------------------------------------------------

if (BUSES["ISBX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/isbx/isbx.c",
		MAME_DIR .. "src/devices/bus/isbx/isbx.h",
		MAME_DIR .. "src/devices/bus/isbx/compis_fdc.c",
		MAME_DIR .. "src/devices/bus/isbx/compis_fdc.h",
		MAME_DIR .. "src/devices/bus/isbx/isbc_218a.c",
		MAME_DIR .. "src/devices/bus/isbx/isbc_218a.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/msx_slot/slot.h,BUSES["MSX_SLOT"] = true
---------------------------------------------------

if (BUSES["MSX_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/msx_slot/bunsetsu.c",
		MAME_DIR .. "src/devices/bus/msx_slot/bunsetsu.h",
		MAME_DIR .. "src/devices/bus/msx_slot/cartridge.c",
		MAME_DIR .. "src/devices/bus/msx_slot/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx_slot/disk.c",
		MAME_DIR .. "src/devices/bus/msx_slot/disk.h",
		MAME_DIR .. "src/devices/bus/msx_slot/fs4600.c",
		MAME_DIR .. "src/devices/bus/msx_slot/fs4600.h",
		MAME_DIR .. "src/devices/bus/msx_slot/music.c",
		MAME_DIR .. "src/devices/bus/msx_slot/music.h",
		MAME_DIR .. "src/devices/bus/msx_slot/panasonic08.c",
		MAME_DIR .. "src/devices/bus/msx_slot/panasonic08.h",
		MAME_DIR .. "src/devices/bus/msx_slot/rom.c",
		MAME_DIR .. "src/devices/bus/msx_slot/rom.h",
		MAME_DIR .. "src/devices/bus/msx_slot/ram.c",
		MAME_DIR .. "src/devices/bus/msx_slot/ram.h",
		MAME_DIR .. "src/devices/bus/msx_slot/ram_mm.c",
		MAME_DIR .. "src/devices/bus/msx_slot/ram_mm.h",
		MAME_DIR .. "src/devices/bus/msx_slot/slot.c",
		MAME_DIR .. "src/devices/bus/msx_slot/slot.h",
		MAME_DIR .. "src/devices/bus/msx_slot/sony08.c",
		MAME_DIR .. "src/devices/bus/msx_slot/sony08.h",
		MAME_DIR .. "src/devices/bus/msx_cart/arc.c",
		MAME_DIR .. "src/devices/bus/msx_cart/arc.h",
		MAME_DIR .. "src/devices/bus/msx_cart/ascii.c",
		MAME_DIR .. "src/devices/bus/msx_cart/ascii.h",
		MAME_DIR .. "src/devices/bus/msx_cart/bm_012.c",
		MAME_DIR .. "src/devices/bus/msx_cart/bm_012.h",
		MAME_DIR .. "src/devices/bus/msx_cart/cartridge.c",
		MAME_DIR .. "src/devices/bus/msx_cart/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx_cart/crossblaim.c",
		MAME_DIR .. "src/devices/bus/msx_cart/crossblaim.h",
		MAME_DIR .. "src/devices/bus/msx_cart/disk.c",
		MAME_DIR .. "src/devices/bus/msx_cart/disk.h",
		MAME_DIR .. "src/devices/bus/msx_cart/dooly.c",
		MAME_DIR .. "src/devices/bus/msx_cart/dooly.h",
		MAME_DIR .. "src/devices/bus/msx_cart/fmpac.c",
		MAME_DIR .. "src/devices/bus/msx_cart/fmpac.h",
		MAME_DIR .. "src/devices/bus/msx_cart/fs_sr022.c",
		MAME_DIR .. "src/devices/bus/msx_cart/fs_sr022.h",
		MAME_DIR .. "src/devices/bus/msx_cart/halnote.c",
		MAME_DIR .. "src/devices/bus/msx_cart/halnote.h",
		MAME_DIR .. "src/devices/bus/msx_cart/hfox.c",
		MAME_DIR .. "src/devices/bus/msx_cart/hfox.h",
		MAME_DIR .. "src/devices/bus/msx_cart/holy_quran.c",
		MAME_DIR .. "src/devices/bus/msx_cart/holy_quran.h",
		MAME_DIR .. "src/devices/bus/msx_cart/konami.c",
		MAME_DIR .. "src/devices/bus/msx_cart/konami.h",
		MAME_DIR .. "src/devices/bus/msx_cart/korean.c",
		MAME_DIR .. "src/devices/bus/msx_cart/korean.h",
		MAME_DIR .. "src/devices/bus/msx_cart/majutsushi.c",
		MAME_DIR .. "src/devices/bus/msx_cart/majutsushi.h",
		MAME_DIR .. "src/devices/bus/msx_cart/moonsound.h",
		MAME_DIR .. "src/devices/bus/msx_cart/moonsound.c",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio.c",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio.h",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio_kb.c",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio_kb.h",
		MAME_DIR .. "src/devices/bus/msx_cart/msxdos2.c",
		MAME_DIR .. "src/devices/bus/msx_cart/msxdos2.h",
		MAME_DIR .. "src/devices/bus/msx_cart/nomapper.c",
		MAME_DIR .. "src/devices/bus/msx_cart/nomapper.h",
		MAME_DIR .. "src/devices/bus/msx_cart/rtype.c",
		MAME_DIR .. "src/devices/bus/msx_cart/rtype.h",
		MAME_DIR .. "src/devices/bus/msx_cart/superloderunner.c",
		MAME_DIR .. "src/devices/bus/msx_cart/superloderunner.h",
		MAME_DIR .. "src/devices/bus/msx_cart/super_swangi.c",
		MAME_DIR .. "src/devices/bus/msx_cart/super_swangi.h",
		MAME_DIR .. "src/devices/bus/msx_cart/yamaha.c",
		MAME_DIR .. "src/devices/bus/msx_cart/yamaha.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/kc/kc.h,BUSES["KC"] = true
---------------------------------------------------

if (BUSES["KC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/kc/kc.c",
		MAME_DIR .. "src/devices/bus/kc/kc.h",
		MAME_DIR .. "src/devices/bus/kc/d002.c",
		MAME_DIR .. "src/devices/bus/kc/d002.h",
		MAME_DIR .. "src/devices/bus/kc/d004.c",
		MAME_DIR .. "src/devices/bus/kc/d004.h",
		MAME_DIR .. "src/devices/bus/kc/ram.c",
		MAME_DIR .. "src/devices/bus/kc/ram.h",
		MAME_DIR .. "src/devices/bus/kc/rom.c",
		MAME_DIR .. "src/devices/bus/kc/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/odyssey2/slot.h,BUSES["O2"] = true
---------------------------------------------------

if (BUSES["O2"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/odyssey2/slot.c",
		MAME_DIR .. "src/devices/bus/odyssey2/slot.h",
		MAME_DIR .. "src/devices/bus/odyssey2/rom.c",
		MAME_DIR .. "src/devices/bus/odyssey2/rom.h",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.c",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.h",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.c",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pc_joy/pc_joy.h,BUSES["PC_JOY"] = true
---------------------------------------------------

if (BUSES["PC_JOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy.c",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy.h",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.c",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pc_kbd/pc_kbdc.h,BUSES["PC_KBD"] = true
---------------------------------------------------

if (BUSES["PC_KBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc_kbd/pc_kbdc.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc_kbdc.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/keyboards.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/keyboards.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/ec1841.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/ec1841.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/iskr1030.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/iskr1030.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/keytro.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/keytro.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/msnat.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/msnat.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc83.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc83.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat84.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat84.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcxt83.c",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcxt83.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pet/cass.h,BUSES["PET"] = true
--@src/devices/bus/pet/exp.h,BUSES["PET"] = true
--@src/devices/bus/pet/user.h,BUSES["PET"] = true
---------------------------------------------------

if (BUSES["PET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pet/cass.c",
		MAME_DIR .. "src/devices/bus/pet/cass.h",
		MAME_DIR .. "src/devices/bus/pet/c2n.c",
		MAME_DIR .. "src/devices/bus/pet/c2n.h",
		MAME_DIR .. "src/devices/bus/pet/diag264_lb_tape.c",
		MAME_DIR .. "src/devices/bus/pet/diag264_lb_tape.h",
		MAME_DIR .. "src/devices/bus/pet/exp.c",
		MAME_DIR .. "src/devices/bus/pet/exp.h",
		MAME_DIR .. "src/devices/bus/pet/64k.c",
		MAME_DIR .. "src/devices/bus/pet/64k.h",
		MAME_DIR .. "src/devices/bus/pet/hsg.c",
		MAME_DIR .. "src/devices/bus/pet/hsg.h",
		MAME_DIR .. "src/devices/bus/pet/superpet.c",
		MAME_DIR .. "src/devices/bus/pet/superpet.h",
		MAME_DIR .. "src/devices/bus/pet/user.c",
		MAME_DIR .. "src/devices/bus/pet/user.h",
		MAME_DIR .. "src/devices/bus/pet/diag.c",
		MAME_DIR .. "src/devices/bus/pet/diag.h",
		MAME_DIR .. "src/devices/bus/pet/petuja.c",
		MAME_DIR .. "src/devices/bus/pet/petuja.h",
		MAME_DIR .. "src/devices/bus/pet/cb2snd.c",
		MAME_DIR .. "src/devices/bus/pet/cb2snd.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/plus4/exp.h,BUSES["PLUS4"] = true
--@src/devices/bus/plus4/user.h,BUSES["PLUS4"] = true
---------------------------------------------------

if (BUSES["PLUS4"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/plus4/exp.c",
		MAME_DIR .. "src/devices/bus/plus4/exp.h",
		MAME_DIR .. "src/devices/bus/plus4/c1551.c",
		MAME_DIR .. "src/devices/bus/plus4/c1551.h",
		MAME_DIR .. "src/devices/bus/plus4/sid.c",
		MAME_DIR .. "src/devices/bus/plus4/sid.h",
		MAME_DIR .. "src/devices/bus/plus4/std.c",
		MAME_DIR .. "src/devices/bus/plus4/std.h",
		MAME_DIR .. "src/devices/bus/plus4/user.c",
		MAME_DIR .. "src/devices/bus/plus4/user.h",
		MAME_DIR .. "src/devices/bus/plus4/diag264_lb_user.c",
		MAME_DIR .. "src/devices/bus/plus4/diag264_lb_user.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/s100/s100.h,BUSES["S100"] = true
---------------------------------------------------

if (BUSES["S100"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/s100/s100.c",
		MAME_DIR .. "src/devices/bus/s100/s100.h",
		MAME_DIR .. "src/devices/bus/s100/dj2db.c",
		MAME_DIR .. "src/devices/bus/s100/dj2db.h",
		MAME_DIR .. "src/devices/bus/s100/djdma.c",
		MAME_DIR .. "src/devices/bus/s100/djdma.h",
		MAME_DIR .. "src/devices/bus/s100/mm65k16s.c",
		MAME_DIR .. "src/devices/bus/s100/mm65k16s.h",
		MAME_DIR .. "src/devices/bus/s100/nsmdsa.c",
		MAME_DIR .. "src/devices/bus/s100/nsmdsa.h",
		MAME_DIR .. "src/devices/bus/s100/nsmdsad.c",
		MAME_DIR .. "src/devices/bus/s100/nsmdsad.h",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.c",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/spc1000/exp.h,BUSES["SPC1000"] = true
---------------------------------------------------

if (BUSES["SPC1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/spc1000/exp.c",
		MAME_DIR .. "src/devices/bus/spc1000/exp.h",
		MAME_DIR .. "src/devices/bus/spc1000/fdd.c",
		MAME_DIR .. "src/devices/bus/spc1000/fdd.h",
		MAME_DIR .. "src/devices/bus/spc1000/vdp.c",
		MAME_DIR .. "src/devices/bus/spc1000/vdp.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/tiki100/exp.h,BUSES["TIKI100"] = true
---------------------------------------------------

if (BUSES["TIKI100"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tiki100/exp.c",
		MAME_DIR .. "src/devices/bus/tiki100/exp.h",
		MAME_DIR .. "src/devices/bus/tiki100/8088.c",
		MAME_DIR .. "src/devices/bus/tiki100/8088.h",
		MAME_DIR .. "src/devices/bus/tiki100/hdc.c",
		MAME_DIR .. "src/devices/bus/tiki100/hdc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/tvc/tvc.h,BUSES["TVC"] = true
---------------------------------------------------

if (BUSES["TVC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tvc/tvc.c",
		MAME_DIR .. "src/devices/bus/tvc/tvc.h",
		MAME_DIR .. "src/devices/bus/tvc/hbf.c",
		MAME_DIR .. "src/devices/bus/tvc/hbf.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vc4000/slot.h,BUSES["VC4000"] = true
---------------------------------------------------

if (BUSES["VC4000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vc4000/slot.c",
		MAME_DIR .. "src/devices/bus/vc4000/slot.h",
		MAME_DIR .. "src/devices/bus/vc4000/rom.c",
		MAME_DIR .. "src/devices/bus/vc4000/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vcs/vcs_slot.h,BUSES["VCS"] = true
---------------------------------------------------

if (BUSES["VCS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vcs/vcs_slot.c",
		MAME_DIR .. "src/devices/bus/vcs/vcs_slot.h",
		MAME_DIR .. "src/devices/bus/vcs/rom.c",
		MAME_DIR .. "src/devices/bus/vcs/rom.h",
		MAME_DIR .. "src/devices/bus/vcs/compumat.c",
		MAME_DIR .. "src/devices/bus/vcs/compumat.h",
		MAME_DIR .. "src/devices/bus/vcs/dpc.c",
		MAME_DIR .. "src/devices/bus/vcs/dpc.h",
		MAME_DIR .. "src/devices/bus/vcs/scharger.c",
		MAME_DIR .. "src/devices/bus/vcs/scharger.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vcs/ctrl.h,BUSES["VCS_CTRL"] = true
---------------------------------------------------

if (BUSES["VCS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/wheel.c",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/wheel.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vectrex/slot.h,BUSES["VECTREX"] = true
---------------------------------------------------

if (BUSES["VECTREX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vectrex/slot.c",
		MAME_DIR .. "src/devices/bus/vectrex/slot.h",
		MAME_DIR .. "src/devices/bus/vectrex/rom.c",
		MAME_DIR .. "src/devices/bus/vectrex/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vic10/exp.h,BUSES["VIC10"] = true
---------------------------------------------------

if (BUSES["VIC10"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vic10/exp.c",
		MAME_DIR .. "src/devices/bus/vic10/exp.h",
		MAME_DIR .. "src/devices/bus/vic10/std.c",
		MAME_DIR .. "src/devices/bus/vic10/std.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vic20/exp.h,BUSES["VIC20"] = true
--@src/devices/bus/vic20/user.h,BUSES["VIC20"] = true
---------------------------------------------------

if (BUSES["VIC20"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vic20/exp.c",
		MAME_DIR .. "src/devices/bus/vic20/exp.h",
		MAME_DIR .. "src/devices/bus/vic20/fe3.c",
		MAME_DIR .. "src/devices/bus/vic20/fe3.h",
		MAME_DIR .. "src/devices/bus/vic20/megacart.c",
		MAME_DIR .. "src/devices/bus/vic20/megacart.h",
		MAME_DIR .. "src/devices/bus/vic20/std.c",
		MAME_DIR .. "src/devices/bus/vic20/std.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1010.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1010.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1110.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1110.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1111.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1111.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1112.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1112.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1210.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1210.h",
		MAME_DIR .. "src/devices/bus/vic20/user.c",
		MAME_DIR .. "src/devices/bus/vic20/user.h",
		MAME_DIR .. "src/devices/bus/vic20/4cga.c",
		MAME_DIR .. "src/devices/bus/vic20/4cga.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1011.c",
		MAME_DIR .. "src/devices/bus/vic20/vic1011.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vidbrain/exp.h,BUSES["VIDBRAIN"] = true
---------------------------------------------------

if (BUSES["VIDBRAIN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vidbrain/exp.c",
		MAME_DIR .. "src/devices/bus/vidbrain/exp.h",
		MAME_DIR .. "src/devices/bus/vidbrain/std.c",
		MAME_DIR .. "src/devices/bus/vidbrain/std.h",
		MAME_DIR .. "src/devices/bus/vidbrain/money_minder.c",
		MAME_DIR .. "src/devices/bus/vidbrain/money_minder.h",
		MAME_DIR .. "src/devices/bus/vidbrain/timeshare.c",
		MAME_DIR .. "src/devices/bus/vidbrain/timeshare.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vip/byteio.h,BUSES["VIP"] = true
--@src/devices/bus/vip/exp.h,BUSES["VIP"] = true
---------------------------------------------------

if (BUSES["VIP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vip/byteio.c",
		MAME_DIR .. "src/devices/bus/vip/byteio.h",
		MAME_DIR .. "src/devices/bus/vip/vp620.c",
		MAME_DIR .. "src/devices/bus/vip/vp620.h",
		MAME_DIR .. "src/devices/bus/vip/exp.c",
		MAME_DIR .. "src/devices/bus/vip/exp.h",
		MAME_DIR .. "src/devices/bus/vip/vp550.c",
		MAME_DIR .. "src/devices/bus/vip/vp550.h",
		MAME_DIR .. "src/devices/bus/vip/vp570.c",
		MAME_DIR .. "src/devices/bus/vip/vp570.h",
		MAME_DIR .. "src/devices/bus/vip/vp575.c",
		MAME_DIR .. "src/devices/bus/vip/vp575.h",
		MAME_DIR .. "src/devices/bus/vip/vp585.c",
		MAME_DIR .. "src/devices/bus/vip/vp585.h",
		MAME_DIR .. "src/devices/bus/vip/vp590.c",
		MAME_DIR .. "src/devices/bus/vip/vp590.h",
		MAME_DIR .. "src/devices/bus/vip/vp595.c",
		MAME_DIR .. "src/devices/bus/vip/vp595.h",
		MAME_DIR .. "src/devices/bus/vip/vp700.c",
		MAME_DIR .. "src/devices/bus/vip/vp700.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/wangpc/wangpc.h,BUSES["WANGPC"] = true
---------------------------------------------------

if (BUSES["WANGPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/wangpc/wangpc.c",
		MAME_DIR .. "src/devices/bus/wangpc/wangpc.h",
		MAME_DIR .. "src/devices/bus/wangpc/emb.c",
		MAME_DIR .. "src/devices/bus/wangpc/emb.h",
		MAME_DIR .. "src/devices/bus/wangpc/lic.c",
		MAME_DIR .. "src/devices/bus/wangpc/lic.h",
		MAME_DIR .. "src/devices/bus/wangpc/lvc.c",
		MAME_DIR .. "src/devices/bus/wangpc/lvc.h",
		MAME_DIR .. "src/devices/bus/wangpc/mcc.c",
		MAME_DIR .. "src/devices/bus/wangpc/mcc.h",
		MAME_DIR .. "src/devices/bus/wangpc/mvc.c",
		MAME_DIR .. "src/devices/bus/wangpc/mvc.h",
		MAME_DIR .. "src/devices/bus/wangpc/rtc.c",
		MAME_DIR .. "src/devices/bus/wangpc/rtc.h",
		MAME_DIR .. "src/devices/bus/wangpc/tig.c",
		MAME_DIR .. "src/devices/bus/wangpc/tig.h",
		MAME_DIR .. "src/devices/bus/wangpc/wdc.c",
		MAME_DIR .. "src/devices/bus/wangpc/wdc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/z88/z88.h,BUSES["Z88"] = true
---------------------------------------------------

if (BUSES["Z88"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/z88/z88.c",
		MAME_DIR .. "src/devices/bus/z88/z88.h",
		MAME_DIR .. "src/devices/bus/z88/flash.c",
		MAME_DIR .. "src/devices/bus/z88/flash.h",
		MAME_DIR .. "src/devices/bus/z88/ram.c",
		MAME_DIR .. "src/devices/bus/z88/ram.h",
		MAME_DIR .. "src/devices/bus/z88/rom.c",
		MAME_DIR .. "src/devices/bus/z88/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/a2bus/a2bus.h,BUSES["A2BUS"] = true
---------------------------------------------------

if (BUSES["A2BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a2bus/a2bus.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2bus.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2lang.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2lang.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskii.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskii.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.h",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.c",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.c",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.h",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.c",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.h",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.c",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.c",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.c",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.c",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.c",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.h",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.c",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nubus/nubus.h,BUSES["NUBUS"] = true
---------------------------------------------------

if (BUSES["NUBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nubus/nubus.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_48gc.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_48gc.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_cb264.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_cb264.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_vikbw.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_vikbw.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_specpdq.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_specpdq.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2hires.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2hires.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_spec8.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_spec8.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_radiustpd.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_radiustpd.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2video.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2video.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_asntmc3b.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_asntmc3b.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_image.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_image.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_wsportrait.c",
		MAME_DIR .. "src/devices/bus/nubus/nubus_wsportrait.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_cb264.c",
		MAME_DIR .. "src/devices/bus/nubus/pds30_cb264.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_procolor816.c",
		MAME_DIR .. "src/devices/bus/nubus/pds30_procolor816.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_sigmalview.c",
		MAME_DIR .. "src/devices/bus/nubus/pds30_sigmalview.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_30hr.c",
		MAME_DIR .. "src/devices/bus/nubus/pds30_30hr.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_mc30.c",
		MAME_DIR .. "src/devices/bus/nubus/pds30_mc30.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/centronics/ctronics.h,BUSES["CENTRONICS"] = true
---------------------------------------------------

if (BUSES["CENTRONICS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/centronics/ctronics.c",
		MAME_DIR .. "src/devices/bus/centronics/ctronics.h",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.c",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.h",
		MAME_DIR .. "src/devices/bus/centronics/covox.c",
		MAME_DIR .. "src/devices/bus/centronics/covox.h",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.c",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.c",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.c",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.c",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.h",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.c",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.h",
		MAME_DIR .. "src/devices/bus/centronics/printer.c",
		MAME_DIR .. "src/devices/bus/centronics/printer.h",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.c",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/centronics/epson_ex800.c",  GEN_DIR .. "emu/layout/ex800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx800.c",  GEN_DIR .. "emu/layout/lx800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.c", GEN_DIR .. "emu/layout/lx800.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "ex800"),
		layoutbuildtask("emu/layout", "lx800"),
	}
end

---------------------------------------------------
--
--@src/devices/bus/rs232/rs232.h,BUSES["RS232"] = true
---------------------------------------------------

if (BUSES["RS232"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/rs232/keyboard.c",
		MAME_DIR .. "src/devices/bus/rs232/keyboard.h",
		MAME_DIR .. "src/devices/bus/rs232/loopback.c",
		MAME_DIR .. "src/devices/bus/rs232/loopback.h",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.c",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.h",
		MAME_DIR .. "src/devices/bus/rs232/printer.c",
		MAME_DIR .. "src/devices/bus/rs232/printer.h",
		MAME_DIR .. "src/devices/bus/rs232/rs232.c",
		MAME_DIR .. "src/devices/bus/rs232/rs232.h",
		MAME_DIR .. "src/devices/bus/rs232/pty.c",
		MAME_DIR .. "src/devices/bus/rs232/pty.h",
		MAME_DIR .. "src/devices/bus/rs232/ser_mouse.c",
		MAME_DIR .. "src/devices/bus/rs232/ser_mouse.h",
		MAME_DIR .. "src/devices/bus/rs232/terminal.c",
		MAME_DIR .. "src/devices/bus/rs232/terminal.h",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.c",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/midi/midi.h,BUSES["MIDI"] = true
---------------------------------------------------

if (BUSES["MIDI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/midi/midi.c",
		MAME_DIR .. "src/devices/bus/midi/midi.h",
		MAME_DIR .. "src/devices/bus/midi/midiinport.c",
		MAME_DIR .. "src/devices/bus/midi/midiinport.h",
		MAME_DIR .. "src/devices/bus/midi/midioutport.c",
		MAME_DIR .. "src/devices/bus/midi/midioutport.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/lpci/pci.h,BUSES["LPCI"] = true
---------------------------------------------------

if (BUSES["LPCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/lpci/pci.c",
		MAME_DIR .. "src/devices/bus/lpci/pci.h",
		MAME_DIR .. "src/devices/bus/lpci/cirrus.c",
		MAME_DIR .. "src/devices/bus/lpci/cirrus.h",
		MAME_DIR .. "src/devices/bus/lpci/i82371ab.c",
		MAME_DIR .. "src/devices/bus/lpci/i82371ab.h",
		MAME_DIR .. "src/devices/bus/lpci/i82371sb.c",
		MAME_DIR .. "src/devices/bus/lpci/i82371sb.h",
		MAME_DIR .. "src/devices/bus/lpci/i82439tx.c",
		MAME_DIR .. "src/devices/bus/lpci/i82439tx.h",
		MAME_DIR .. "src/devices/bus/lpci/northbridge.c",
		MAME_DIR .. "src/devices/bus/lpci/northbridge.h",
		MAME_DIR .. "src/devices/bus/lpci/southbridge.c",
		MAME_DIR .. "src/devices/bus/lpci/southbridge.h",
		MAME_DIR .. "src/devices/bus/lpci/mpc105.c",
		MAME_DIR .. "src/devices/bus/lpci/mpc105.h",
		MAME_DIR .. "src/devices/bus/lpci/vt82c505.c",
		MAME_DIR .. "src/devices/bus/lpci/vt82c505.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nes/nes_slot.h,BUSES["NES"] = true
---------------------------------------------------

if (BUSES["NES"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nes/nes_slot.c",
		MAME_DIR .. "src/devices/bus/nes/nes_slot.h",
		MAME_DIR .. "src/devices/bus/nes/nes_ines.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_pcb.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_unif.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_carts.c",
		MAME_DIR .. "src/devices/bus/nes/nes_carts.h",
		MAME_DIR .. "src/devices/bus/nes/2a03pur.c",
		MAME_DIR .. "src/devices/bus/nes/2a03pur.h",
		MAME_DIR .. "src/devices/bus/nes/act53.c",
		MAME_DIR .. "src/devices/bus/nes/act53.h",
		MAME_DIR .. "src/devices/bus/nes/aladdin.c",
		MAME_DIR .. "src/devices/bus/nes/aladdin.h",
		MAME_DIR .. "src/devices/bus/nes/ave.c",
		MAME_DIR .. "src/devices/bus/nes/ave.h",
		MAME_DIR .. "src/devices/bus/nes/bandai.c",
		MAME_DIR .. "src/devices/bus/nes/bandai.h",
		MAME_DIR .. "src/devices/bus/nes/benshieng.c",
		MAME_DIR .. "src/devices/bus/nes/benshieng.h",
		MAME_DIR .. "src/devices/bus/nes/bootleg.c",
		MAME_DIR .. "src/devices/bus/nes/bootleg.h",
		MAME_DIR .. "src/devices/bus/nes/camerica.c",
		MAME_DIR .. "src/devices/bus/nes/camerica.h",
		MAME_DIR .. "src/devices/bus/nes/cne.c",
		MAME_DIR .. "src/devices/bus/nes/cne.h",
		MAME_DIR .. "src/devices/bus/nes/cony.c",
		MAME_DIR .. "src/devices/bus/nes/cony.h",
		MAME_DIR .. "src/devices/bus/nes/datach.c",
		MAME_DIR .. "src/devices/bus/nes/datach.h",
		MAME_DIR .. "src/devices/bus/nes/discrete.c",
		MAME_DIR .. "src/devices/bus/nes/discrete.h",
		MAME_DIR .. "src/devices/bus/nes/disksys.c",
		MAME_DIR .. "src/devices/bus/nes/disksys.h",
		MAME_DIR .. "src/devices/bus/nes/event.c",
		MAME_DIR .. "src/devices/bus/nes/event.h",
		MAME_DIR .. "src/devices/bus/nes/ggenie.c",
		MAME_DIR .. "src/devices/bus/nes/ggenie.h",
		MAME_DIR .. "src/devices/bus/nes/henggedianzi.c",
		MAME_DIR .. "src/devices/bus/nes/henggedianzi.h",
		MAME_DIR .. "src/devices/bus/nes/hes.c",
		MAME_DIR .. "src/devices/bus/nes/hes.h",
		MAME_DIR .. "src/devices/bus/nes/hosenkan.c",
		MAME_DIR .. "src/devices/bus/nes/hosenkan.h",
		MAME_DIR .. "src/devices/bus/nes/irem.c",
		MAME_DIR .. "src/devices/bus/nes/irem.h",
		MAME_DIR .. "src/devices/bus/nes/jaleco.c",
		MAME_DIR .. "src/devices/bus/nes/jaleco.h",
		MAME_DIR .. "src/devices/bus/nes/jy.c",
		MAME_DIR .. "src/devices/bus/nes/jy.h",
		MAME_DIR .. "src/devices/bus/nes/kaiser.c",
		MAME_DIR .. "src/devices/bus/nes/kaiser.h",
		MAME_DIR .. "src/devices/bus/nes/karastudio.c",
		MAME_DIR .. "src/devices/bus/nes/karastudio.h",
		MAME_DIR .. "src/devices/bus/nes/konami.c",
		MAME_DIR .. "src/devices/bus/nes/konami.h",
		MAME_DIR .. "src/devices/bus/nes/legacy.c",
		MAME_DIR .. "src/devices/bus/nes/legacy.h",
		MAME_DIR .. "src/devices/bus/nes/mmc1.c",
		MAME_DIR .. "src/devices/bus/nes/mmc1.h",
		MAME_DIR .. "src/devices/bus/nes/mmc2.c",
		MAME_DIR .. "src/devices/bus/nes/mmc2.h",
		MAME_DIR .. "src/devices/bus/nes/mmc3.c",
		MAME_DIR .. "src/devices/bus/nes/mmc3.h",
		MAME_DIR .. "src/devices/bus/nes/mmc3_clones.c",
		MAME_DIR .. "src/devices/bus/nes/mmc3_clones.h",
		MAME_DIR .. "src/devices/bus/nes/mmc5.c",
		MAME_DIR .. "src/devices/bus/nes/mmc5.h",
		MAME_DIR .. "src/devices/bus/nes/multigame.c",
		MAME_DIR .. "src/devices/bus/nes/multigame.h",
		MAME_DIR .. "src/devices/bus/nes/namcot.c",
		MAME_DIR .. "src/devices/bus/nes/namcot.h",
		MAME_DIR .. "src/devices/bus/nes/nanjing.c",
		MAME_DIR .. "src/devices/bus/nes/nanjing.h",
		MAME_DIR .. "src/devices/bus/nes/ntdec.c",
		MAME_DIR .. "src/devices/bus/nes/ntdec.h",
		MAME_DIR .. "src/devices/bus/nes/nxrom.c",
		MAME_DIR .. "src/devices/bus/nes/nxrom.h",
		MAME_DIR .. "src/devices/bus/nes/pirate.c",
		MAME_DIR .. "src/devices/bus/nes/pirate.h",
		MAME_DIR .. "src/devices/bus/nes/pt554.c",
		MAME_DIR .. "src/devices/bus/nes/pt554.h",
		MAME_DIR .. "src/devices/bus/nes/racermate.c",
		MAME_DIR .. "src/devices/bus/nes/racermate.h",
		MAME_DIR .. "src/devices/bus/nes/rcm.c",
		MAME_DIR .. "src/devices/bus/nes/rcm.h",
		MAME_DIR .. "src/devices/bus/nes/rexsoft.c",
		MAME_DIR .. "src/devices/bus/nes/rexsoft.h",
		MAME_DIR .. "src/devices/bus/nes/sachen.c",
		MAME_DIR .. "src/devices/bus/nes/sachen.h",
		MAME_DIR .. "src/devices/bus/nes/somari.c",
		MAME_DIR .. "src/devices/bus/nes/somari.h",
		MAME_DIR .. "src/devices/bus/nes/sunsoft.c",
		MAME_DIR .. "src/devices/bus/nes/sunsoft.h",
		MAME_DIR .. "src/devices/bus/nes/sunsoft_dcs.c",
		MAME_DIR .. "src/devices/bus/nes/sunsoft_dcs.h",
		MAME_DIR .. "src/devices/bus/nes/taito.c",
		MAME_DIR .. "src/devices/bus/nes/taito.h",
		MAME_DIR .. "src/devices/bus/nes/tengen.c",
		MAME_DIR .. "src/devices/bus/nes/tengen.h",
		MAME_DIR .. "src/devices/bus/nes/txc.c",
		MAME_DIR .. "src/devices/bus/nes/txc.h",
		MAME_DIR .. "src/devices/bus/nes/waixing.c",
		MAME_DIR .. "src/devices/bus/nes/waixing.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nes_ctrl/ctrl.h,BUSES["NES_CTRL"] = true
---------------------------------------------------

if (BUSES["NES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nes_ctrl/ctrl.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/joypad.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/4score.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/4score.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/arkpaddle.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/arkpaddle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ftrainer.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ftrainer.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamihs.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamihs.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/miracle.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/miracle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/mjpanel.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/mjpanel.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/pachinko.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/pachinko.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/partytap.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/partytap.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/powerpad.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/powerpad.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.c",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/snes/snes_slot.h,BUSES["SNES"] = true
---------------------------------------------------

if (BUSES["SNES"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/snes/snes_slot.c",
		MAME_DIR .. "src/devices/bus/snes/snes_slot.h",
		MAME_DIR .. "src/devices/bus/snes/snes_carts.c",
		MAME_DIR .. "src/devices/bus/snes/snes_carts.h",
		MAME_DIR .. "src/devices/bus/snes/bsx.c",
		MAME_DIR .. "src/devices/bus/snes/bsx.h",
		MAME_DIR .. "src/devices/bus/snes/event.c",
		MAME_DIR .. "src/devices/bus/snes/event.h",
		MAME_DIR .. "src/devices/bus/snes/rom.c",
		MAME_DIR .. "src/devices/bus/snes/rom.h",
		MAME_DIR .. "src/devices/bus/snes/rom21.c",
		MAME_DIR .. "src/devices/bus/snes/rom21.h",
		MAME_DIR .. "src/devices/bus/snes/sa1.c",
		MAME_DIR .. "src/devices/bus/snes/sa1.h",
		MAME_DIR .. "src/devices/bus/snes/sdd1.c",
		MAME_DIR .. "src/devices/bus/snes/sdd1.h",
		MAME_DIR .. "src/devices/bus/snes/sfx.c",
		MAME_DIR .. "src/devices/bus/snes/sfx.h",
		MAME_DIR .. "src/devices/bus/snes/sgb.c",
		MAME_DIR .. "src/devices/bus/snes/sgb.h",
		MAME_DIR .. "src/devices/bus/snes/spc7110.c",
		MAME_DIR .. "src/devices/bus/snes/spc7110.h",
		MAME_DIR .. "src/devices/bus/snes/sufami.c",
		MAME_DIR .. "src/devices/bus/snes/sufami.h",
		MAME_DIR .. "src/devices/bus/snes/upd.c",
		MAME_DIR .. "src/devices/bus/snes/upd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/snes_ctrl/ctrl.h,BUSES["SNES_CTRL"] = true
---------------------------------------------------

if (BUSES["SNES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/snes_ctrl/ctrl.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/bcbattle.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/bcbattle.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/joypad.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/miracle.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/miracle.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/mouse.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/multitap.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/pachinko.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/pachinko.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/sscope.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/sscope.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/twintap.c",
		MAME_DIR .. "src/devices/bus/snes_ctrl/twintap.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vboy/slot.h,BUSES["VBOY"] = true
---------------------------------------------------
if (BUSES["VBOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vboy/slot.c",
		MAME_DIR .. "src/devices/bus/vboy/slot.h",
		MAME_DIR .. "src/devices/bus/vboy/rom.c",
		MAME_DIR .. "src/devices/bus/vboy/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/megadrive/md_slot.h,BUSES["MEGADRIVE"] = true
---------------------------------------------------

if (BUSES["MEGADRIVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/megadrive/md_slot.c",
		MAME_DIR .. "src/devices/bus/megadrive/md_slot.h",
		MAME_DIR .. "src/devices/bus/megadrive/md_carts.c",
		MAME_DIR .. "src/devices/bus/megadrive/md_carts.h",
		MAME_DIR .. "src/devices/bus/megadrive/eeprom.c",
		MAME_DIR .. "src/devices/bus/megadrive/eeprom.h",
		MAME_DIR .. "src/devices/bus/megadrive/ggenie.c",
		MAME_DIR .. "src/devices/bus/megadrive/ggenie.h",
		MAME_DIR .. "src/devices/bus/megadrive/jcart.c",
		MAME_DIR .. "src/devices/bus/megadrive/jcart.h",
		MAME_DIR .. "src/devices/bus/megadrive/rom.c",
		MAME_DIR .. "src/devices/bus/megadrive/rom.h",
		MAME_DIR .. "src/devices/bus/megadrive/sk.c",
		MAME_DIR .. "src/devices/bus/megadrive/sk.h",
		MAME_DIR .. "src/devices/bus/megadrive/stm95.c",
		MAME_DIR .. "src/devices/bus/megadrive/stm95.h",
		MAME_DIR .. "src/devices/bus/megadrive/svp.c",
		MAME_DIR .. "src/devices/bus/megadrive/svp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/neogeo/neogeo_slot.h,BUSES["NEOGEO"] = true
---------------------------------------------------

if (BUSES["NEOGEO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_slot.c",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_slot.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_intf.c",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_intf.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_carts.c",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_carts.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_helper.c",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_helper.h",
		MAME_DIR .. "src/devices/bus/neogeo/banked_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/banked_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/sma_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/sma_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/sma_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/sma_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_hybrid_cart.c",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_hybrid_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/sbp_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/sbp_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kog_prot.c",
		MAME_DIR .. "src/devices/bus/neogeo/kog_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/rom.c",
		MAME_DIR .. "src/devices/bus/neogeo/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/saturn/sat_slot.h,BUSES["SATURN"] = true
---------------------------------------------------

if (BUSES["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/saturn/sat_slot.c",
		MAME_DIR .. "src/devices/bus/saturn/sat_slot.h",
		MAME_DIR .. "src/devices/bus/saturn/bram.c",
		MAME_DIR .. "src/devices/bus/saturn/bram.h",
		MAME_DIR .. "src/devices/bus/saturn/dram.c",
		MAME_DIR .. "src/devices/bus/saturn/dram.h",
		MAME_DIR .. "src/devices/bus/saturn/rom.c",
		MAME_DIR .. "src/devices/bus/saturn/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sega8/sega8_slot.h,BUSES["SEGA8"] = true
---------------------------------------------------

if (BUSES["SEGA8"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sega8/sega8_slot.c",
		MAME_DIR .. "src/devices/bus/sega8/sega8_slot.h",
		MAME_DIR .. "src/devices/bus/sega8/rom.c",
		MAME_DIR .. "src/devices/bus/sega8/rom.h",
		MAME_DIR .. "src/devices/bus/sega8/ccatch.c",
		MAME_DIR .. "src/devices/bus/sega8/ccatch.h",
		MAME_DIR .. "src/devices/bus/sega8/mgear.c",
		MAME_DIR .. "src/devices/bus/sega8/mgear.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sms_ctrl/smsctrl.h,BUSES["SMS_CTRL"] = true
---------------------------------------------------

if (BUSES["SMS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.c",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sms_exp/smsexp.h,BUSES["SMS_EXP"] = true
---------------------------------------------------

if (BUSES["SMS_EXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sms_exp/smsexp.c",
		MAME_DIR .. "src/devices/bus/sms_exp/smsexp.h",
		MAME_DIR .. "src/devices/bus/sms_exp/gender.c",
		MAME_DIR .. "src/devices/bus/sms_exp/gender.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99x/990_dk.h,BUSES["TI99X"] = true
---------------------------------------------------

if (BUSES["TI99X"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.c",
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.c",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.c",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.h",
		MAME_DIR .. "src/devices/bus/ti99x/998board.c",
		MAME_DIR .. "src/devices/bus/ti99x/998board.h",
		MAME_DIR .. "src/devices/bus/ti99x/datamux.c",
		MAME_DIR .. "src/devices/bus/ti99x/datamux.h",
		MAME_DIR .. "src/devices/bus/ti99x/genboard.c",
		MAME_DIR .. "src/devices/bus/ti99x/genboard.h",
		MAME_DIR .. "src/devices/bus/ti99x/grom.c",
		MAME_DIR .. "src/devices/bus/ti99x/grom.h",
		MAME_DIR .. "src/devices/bus/ti99x/gromport.c",
		MAME_DIR .. "src/devices/bus/ti99x/gromport.h",
		MAME_DIR .. "src/devices/bus/ti99x/handset.c",
		MAME_DIR .. "src/devices/bus/ti99x/handset.h",
		MAME_DIR .. "src/devices/bus/ti99x/joyport.c",
		MAME_DIR .. "src/devices/bus/ti99x/joyport.h",
		MAME_DIR .. "src/devices/bus/ti99x/mecmouse.c",
		MAME_DIR .. "src/devices/bus/ti99x/mecmouse.h",
		MAME_DIR .. "src/devices/bus/ti99x/ti99defs.h",
		MAME_DIR .. "src/devices/bus/ti99x/videowrp.c",
		MAME_DIR .. "src/devices/bus/ti99x/videowrp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99_peb/peribox.h,BUSES["TI99PEB"] = true
---------------------------------------------------

if (BUSES["TI99PEB"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99_peb/peribox.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/peribox.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/bwg.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/bwg.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/evpc.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/evpc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/hfdc.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/hfdc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/horizon.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/horizon.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/hsgpl.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/hsgpl.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/memex.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/memex.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/myarcmem.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/myarcmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/pcode.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/pcode.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/samsmem.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/samsmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/spchsyn.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/spchsyn.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_32kmem.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_32kmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_fdc.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_fdc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_rs232.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_rs232.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_ide.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_ide.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_usbsm.c",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_usbsm.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gameboy/gb_slot.h,BUSES["GAMEBOY"] = true
---------------------------------------------------

if (BUSES["GAMEBOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gameboy/gb_slot.c",
		MAME_DIR .. "src/devices/bus/gameboy/gb_slot.h",
		MAME_DIR .. "src/devices/bus/gameboy/rom.c",
		MAME_DIR .. "src/devices/bus/gameboy/rom.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.c",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gamegear/ggext.h,BUSES["GAMEGEAR"] = true
---------------------------------------------------

if (BUSES["GAMEGEAR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gamegear/ggext.c",
		MAME_DIR .. "src/devices/bus/gamegear/ggext.h",
		MAME_DIR .. "src/devices/bus/gamegear/smsctrladp.c",
		MAME_DIR .. "src/devices/bus/gamegear/smsctrladp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gba/gba_slot.h,BUSES["GBA"] = true
---------------------------------------------------

if (BUSES["GBA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gba/gba_slot.c",
		MAME_DIR .. "src/devices/bus/gba/gba_slot.h",
		MAME_DIR .. "src/devices/bus/gba/rom.c",
		MAME_DIR .. "src/devices/bus/gba/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/bml3/bml3bus.h,BUSES["BML3"] = true
---------------------------------------------------
if (BUSES["BML3"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bml3/bml3bus.c",
		MAME_DIR .. "src/devices/bus/bml3/bml3bus.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1802.c",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1802.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1805.c",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1805.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3kanji.c",
		MAME_DIR .. "src/devices/bus/bml3/bml3kanji.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/coco/cococart.h,BUSES["COCO"] = true
---------------------------------------------------
if (BUSES["COCO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coco/cococart.c",
		MAME_DIR .. "src/devices/bus/coco/cococart.h",
		MAME_DIR .. "src/devices/bus/coco/coco_232.c",
		MAME_DIR .. "src/devices/bus/coco/coco_232.h",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.c",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.h",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.c",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.h",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.c",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.h",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.c",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.h",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.c",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cpc/cpcexp.h,BUSES["CPC"] = true
---------------------------------------------------
if (BUSES["CPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cpc/cpcexp.c",
		MAME_DIR .. "src/devices/bus/cpc/cpcexp.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_ssa1.c",
		MAME_DIR .. "src/devices/bus/cpc/cpc_ssa1.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rom.c",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rom.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_pds.c",
		MAME_DIR .. "src/devices/bus/cpc/cpc_pds.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rs232.c",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rs232.h",
		MAME_DIR .. "src/devices/bus/cpc/mface2.c",
		MAME_DIR .. "src/devices/bus/cpc/mface2.h",
		MAME_DIR .. "src/devices/bus/cpc/symbfac2.c",
		MAME_DIR .. "src/devices/bus/cpc/symbfac2.h",
		MAME_DIR .. "src/devices/bus/cpc/amdrum.c",
		MAME_DIR .. "src/devices/bus/cpc/amdrum.h",
		MAME_DIR .. "src/devices/bus/cpc/playcity.c",
		MAME_DIR .. "src/devices/bus/cpc/playcity.h",
		MAME_DIR .. "src/devices/bus/cpc/smartwatch.c",
		MAME_DIR .. "src/devices/bus/cpc/smartwatch.h",
		MAME_DIR .. "src/devices/bus/cpc/brunword4.c",
		MAME_DIR .. "src/devices/bus/cpc/brunword4.h",
		MAME_DIR .. "src/devices/bus/cpc/hd20.c",
		MAME_DIR .. "src/devices/bus/cpc/hd20.h",
		MAME_DIR .. "src/devices/bus/cpc/ddi1.c",
		MAME_DIR .. "src/devices/bus/cpc/ddi1.h",
		MAME_DIR .. "src/devices/bus/cpc/magicsound.c",
		MAME_DIR .. "src/devices/bus/cpc/magicsound.h",
		MAME_DIR .. "src/devices/bus/cpc/doubler.c",
		MAME_DIR .. "src/devices/bus/cpc/doubler.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/epson_sio/epson_sio.h,BUSES["EPSON_SIO"] = true
---------------------------------------------------
if (BUSES["EPSON_SIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/epson_sio/epson_sio.c",
		MAME_DIR .. "src/devices/bus/epson_sio/epson_sio.h",
		MAME_DIR .. "src/devices/bus/epson_sio/pf10.c",
		MAME_DIR .. "src/devices/bus/epson_sio/pf10.h",
		MAME_DIR .. "src/devices/bus/epson_sio/tf20.c",
		MAME_DIR .. "src/devices/bus/epson_sio/tf20.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pce/pce_slot.h,BUSES["PCE"] = true
---------------------------------------------------
if (BUSES["PCE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pce/pce_slot.c",
		MAME_DIR .. "src/devices/bus/pce/pce_slot.h",
		MAME_DIR .. "src/devices/bus/pce/pce_rom.c",
		MAME_DIR .. "src/devices/bus/pce/pce_rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/scv/slot.h,BUSES["SCV"] = true
---------------------------------------------------
if (BUSES["SCV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/scv/slot.c",
		MAME_DIR .. "src/devices/bus/scv/slot.h",
		MAME_DIR .. "src/devices/bus/scv/rom.c",
		MAME_DIR .. "src/devices/bus/scv/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/x68k/x68kexp.h,BUSES["X68K"] = true
---------------------------------------------------
if (BUSES["X68K"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/x68k/x68kexp.c",
		MAME_DIR .. "src/devices/bus/x68k/x68kexp.h",
		MAME_DIR .. "src/devices/bus/x68k/x68k_neptunex.c",
		MAME_DIR .. "src/devices/bus/x68k/x68k_neptunex.h",
		MAME_DIR .. "src/devices/bus/x68k/x68k_scsiext.c",
		MAME_DIR .. "src/devices/bus/x68k/x68k_scsiext.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/abckb/abckb.h,BUSES["ABCKB"] = true
---------------------------------------------------
if (BUSES["ABCKB"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/abckb/abckb.c",
		MAME_DIR .. "src/devices/bus/abckb/abckb.h",
		MAME_DIR .. "src/devices/bus/abckb/abc77.c",
		MAME_DIR .. "src/devices/bus/abckb/abc77.h",
		MAME_DIR .. "src/devices/bus/abckb/abc99.c",
		MAME_DIR .. "src/devices/bus/abckb/abc99.h",
		MAME_DIR .. "src/devices/bus/abckb/abc800kb.c",
		MAME_DIR .. "src/devices/bus/abckb/abc800kb.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/compucolor/compclr_flp.h,BUSES["COMPUCOLOR"] = true
---------------------------------------------------
if (BUSES["COMPUCOLOR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/compucolor/floppy.c",
		MAME_DIR .. "src/devices/bus/compucolor/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/scsi/scsi.h,BUSES["SCSI"] = true
---------------------------------------------------
if (BUSES["SCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/scsi/scsi.c",
		MAME_DIR .. "src/devices/bus/scsi/scsi.h",
		MAME_DIR .. "src/devices/bus/scsi/scsicd.c",
		MAME_DIR .. "src/devices/bus/scsi/scsicd.h",
		MAME_DIR .. "src/devices/bus/scsi/scsihd.c",
		MAME_DIR .. "src/devices/bus/scsi/scsihd.h",
		MAME_DIR .. "src/devices/bus/scsi/scsihle.c",
		MAME_DIR .. "src/devices/bus/scsi/scsihle.h",
		MAME_DIR .. "src/devices/bus/scsi/cdu76s.c",
		MAME_DIR .. "src/devices/bus/scsi/cdu76s.h",
		MAME_DIR .. "src/devices/bus/scsi/acb4070.c",
		MAME_DIR .. "src/devices/bus/scsi/acb4070.h",
		MAME_DIR .. "src/devices/bus/scsi/d9060hd.c",
		MAME_DIR .. "src/devices/bus/scsi/d9060hd.h",
		MAME_DIR .. "src/devices/bus/scsi/sa1403d.c",
		MAME_DIR .. "src/devices/bus/scsi/sa1403d.h",
		MAME_DIR .. "src/devices/bus/scsi/s1410.c",
		MAME_DIR .. "src/devices/bus/scsi/s1410.h",
		MAME_DIR .. "src/devices/bus/scsi/pc9801_sasi.c",
		MAME_DIR .. "src/devices/bus/scsi/pc9801_sasi.h",
		MAME_DIR .. "src/devices/bus/scsi/omti5100.c",
		MAME_DIR .. "src/devices/bus/scsi/omti5100.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/macpds/macpds.h,BUSES["MACPDS"] = true
---------------------------------------------------
if (BUSES["MACPDS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/macpds/macpds.c",
		MAME_DIR .. "src/devices/bus/macpds/macpds.h",
		MAME_DIR .. "src/devices/bus/macpds/pds_tpdfpd.c",
		MAME_DIR .. "src/devices/bus/macpds/pds_tpdfpd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/oricext/oricext.h,BUSES["ORICEXT"] = true
---------------------------------------------------
if (BUSES["ORICEXT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/oricext/oricext.c",
		MAME_DIR .. "src/devices/bus/oricext/oricext.h",
		MAME_DIR .. "src/devices/bus/oricext/jasmin.c",
		MAME_DIR .. "src/devices/bus/oricext/jasmin.h",
		MAME_DIR .. "src/devices/bus/oricext/microdisc.c",
		MAME_DIR .. "src/devices/bus/oricext/microdisc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/a1bus/a1bus.h,BUSES["A1BUS"] = true
---------------------------------------------------

if (BUSES["A1BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a1bus/a1bus.c",
		MAME_DIR .. "src/devices/bus/a1bus/a1bus.h",
		MAME_DIR .. "src/devices/bus/a1bus/a1cassette.c",
		MAME_DIR .. "src/devices/bus/a1bus/a1cassette.h",
		MAME_DIR .. "src/devices/bus/a1bus/a1cffa.c",
		MAME_DIR .. "src/devices/bus/a1bus/a1cffa.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/amiga/zorro/zorro.h,BUSES["ZORRO"] = true
---------------------------------------------------

if (BUSES["ZORRO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/amiga/zorro/zorro.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/zorro.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/cards.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/cards.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2052.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2052.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a590.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a590.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/action_replay.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/action_replay.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.c",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ql/exp.h,BUSES["QL"] = true
---------------------------------------------------

if (BUSES["QL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ql/exp.c",
		MAME_DIR .. "src/devices/bus/ql/exp.h",
		MAME_DIR .. "src/devices/bus/ql/cst_qdisc.c",
		MAME_DIR .. "src/devices/bus/ql/cst_qdisc.h",
		MAME_DIR .. "src/devices/bus/ql/cst_q_plus4.c",
		MAME_DIR .. "src/devices/bus/ql/cst_q_plus4.h",
		MAME_DIR .. "src/devices/bus/ql/cumana_fdi.c",
		MAME_DIR .. "src/devices/bus/ql/cumana_fdi.h",
		MAME_DIR .. "src/devices/bus/ql/kempston_di.c",
		MAME_DIR .. "src/devices/bus/ql/kempston_di.h",
		MAME_DIR .. "src/devices/bus/ql/miracle_gold_card.c",
		MAME_DIR .. "src/devices/bus/ql/miracle_gold_card.h",
		MAME_DIR .. "src/devices/bus/ql/mp_fdi.c",
		MAME_DIR .. "src/devices/bus/ql/mp_fdi.h",
		MAME_DIR .. "src/devices/bus/ql/opd_basic_master.c",
		MAME_DIR .. "src/devices/bus/ql/opd_basic_master.h",
		MAME_DIR .. "src/devices/bus/ql/pcml_qdisk.c",
		MAME_DIR .. "src/devices/bus/ql/pcml_qdisk.h",
		MAME_DIR .. "src/devices/bus/ql/qubide.c",
		MAME_DIR .. "src/devices/bus/ql/qubide.h",
		MAME_DIR .. "src/devices/bus/ql/sandy_superdisk.c",
		MAME_DIR .. "src/devices/bus/ql/sandy_superdisk.h",
		MAME_DIR .. "src/devices/bus/ql/sandy_superqboard.c",
		MAME_DIR .. "src/devices/bus/ql/sandy_superqboard.h",
		MAME_DIR .. "src/devices/bus/ql/trumpcard.c",
		MAME_DIR .. "src/devices/bus/ql/trumpcard.h",
		MAME_DIR .. "src/devices/bus/ql/rom.c",
		MAME_DIR .. "src/devices/bus/ql/rom.h",
		MAME_DIR .. "src/devices/bus/ql/miracle_hd.c",
		MAME_DIR .. "src/devices/bus/ql/miracle_hd.h",
		MAME_DIR .. "src/devices/bus/ql/std.c",
		MAME_DIR .. "src/devices/bus/ql/std.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vtech/memexp/memexp.h,BUSES["VTECH_MEMEXP"] = true
---------------------------------------------------

if (BUSES["VTECH_MEMEXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vtech/memexp/memexp.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memexp.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/carts.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/carts.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/floppy.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/floppy.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memory.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memory.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/rs232.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/rs232.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/wordpro.c",
		MAME_DIR .. "src/devices/bus/vtech/memexp/wordpro.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vtech/ioexp/ioexp.h,BUSES["VTECH_IOEXP"] = true
---------------------------------------------------

if (BUSES["VTECH_IOEXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vtech/ioexp/ioexp.c",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/ioexp.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/carts.c",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/carts.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/joystick.c",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/joystick.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/printer.c",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/printer.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/wswan/slot.h,BUSES["WSWAN"] = true
---------------------------------------------------

if (BUSES["WSWAN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/wswan/slot.c",
		MAME_DIR .. "src/devices/bus/wswan/slot.h",
		MAME_DIR .. "src/devices/bus/wswan/rom.c",
		MAME_DIR .. "src/devices/bus/wswan/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/psx/ctlrport.h,BUSES["PSX_CONTROLLER"] = true
---------------------------------------------------

if (BUSES["PSX_CONTROLLER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psx/ctlrport.c",
		MAME_DIR .. "src/devices/bus/psx/ctlrport.h",
		MAME_DIR .. "src/devices/bus/psx/analogue.c",
		MAME_DIR .. "src/devices/bus/psx/analogue.h",
		MAME_DIR .. "src/devices/bus/psx/multitap.c",
		MAME_DIR .. "src/devices/bus/psx/multitap.h",
		MAME_DIR .. "src/devices/bus/psx/memcard.c",
		MAME_DIR .. "src/devices/bus/psx/memcard.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nasbus/nasbus.h,BUSES["NASBUS"] = true
---------------------------------------------------

if (BUSES["NASBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nasbus/nasbus.c",
		MAME_DIR .. "src/devices/bus/nasbus/nasbus.h",
		MAME_DIR .. "src/devices/bus/nasbus/cards.c",
		MAME_DIR .. "src/devices/bus/nasbus/cards.h",
		MAME_DIR .. "src/devices/bus/nasbus/avc.c",
		MAME_DIR .. "src/devices/bus/nasbus/avc.h",
		MAME_DIR .. "src/devices/bus/nasbus/floppy.c",
		MAME_DIR .. "src/devices/bus/nasbus/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cgenie/expansion.h,BUSES["CGENIE_EXPANSION"] = true
---------------------------------------------------

if (BUSES["CGENIE_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cgenie/expansion/expansion.c",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/expansion.h",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/carts.c",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/carts.h",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/floppy.c",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cgenie/parallel.h,BUSES["CGENIE_PARALLEL"] = true
---------------------------------------------------

if (BUSES["CGENIE_PARALLEL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cgenie/parallel/parallel.c",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/parallel.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/carts.c",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/carts.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/joystick.c",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/joystick.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/printer.c",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/printer.h",
	}
end
