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
		MAME_DIR .. "src/devices/bus/a7800/a78_slot.cpp",
		MAME_DIR .. "src/devices/bus/a7800/a78_slot.h",
		MAME_DIR .. "src/devices/bus/a7800/a78_carts.h",
		MAME_DIR .. "src/devices/bus/a7800/rom.cpp",
		MAME_DIR .. "src/devices/bus/a7800/rom.h",
		MAME_DIR .. "src/devices/bus/a7800/hiscore.cpp",
		MAME_DIR .. "src/devices/bus/a7800/hiscore.h",
		MAME_DIR .. "src/devices/bus/a7800/xboard.cpp",
		MAME_DIR .. "src/devices/bus/a7800/xboard.h",
		MAME_DIR .. "src/devices/bus/a7800/cpuwiz.cpp",
		MAME_DIR .. "src/devices/bus/a7800/cpuwiz.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/a800/a800_slot.h,BUSES["A800"] = true
---------------------------------------------------

if (BUSES["A800"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a800/a8sio.cpp",
		MAME_DIR .. "src/devices/bus/a800/a8sio.h",
		MAME_DIR .. "src/devices/bus/a800/cassette.cpp",
		MAME_DIR .. "src/devices/bus/a800/cassette.h",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.cpp",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.h",
		MAME_DIR .. "src/devices/bus/a800/a800_carts.h",
		MAME_DIR .. "src/devices/bus/a800/rom.cpp",
		MAME_DIR .. "src/devices/bus/a800/rom.h",
		MAME_DIR .. "src/devices/bus/a800/oss.cpp",
		MAME_DIR .. "src/devices/bus/a800/oss.h",
		MAME_DIR .. "src/devices/bus/a800/sparta.cpp",
		MAME_DIR .. "src/devices/bus/a800/sparta.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/abcbus/abcbus.h,BUSES["ABCBUS"] = true
---------------------------------------------------

if (BUSES["ABCBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/abcbus/abcbus.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/abcbus.h",
		MAME_DIR .. "src/devices/bus/abcbus/abc890.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/abc890.h",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.h",
		MAME_DIR .. "src/devices/bus/abcbus/hdc.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/hdc.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux10828.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/lux10828.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux21046.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/lux21046.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux21056.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/lux21056.h",
		MAME_DIR .. "src/devices/bus/abcbus/lux4105.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/lux4105.h",
		MAME_DIR .. "src/devices/bus/abcbus/memcard.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/memcard.h",
		MAME_DIR .. "src/devices/bus/abcbus/ram.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/ram.h",
		MAME_DIR .. "src/devices/bus/abcbus/sio.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/sio.h",
		MAME_DIR .. "src/devices/bus/abcbus/slutprov.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/slutprov.h",
		MAME_DIR .. "src/devices/bus/abcbus/turbo.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/turbo.h",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/adam/exp.h,BUSES["ADAM"] = true
---------------------------------------------------

if (BUSES["ADAM"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/adam/exp.cpp",
		MAME_DIR .. "src/devices/bus/adam/exp.h",
		MAME_DIR .. "src/devices/bus/adam/adamlink.cpp",
		MAME_DIR .. "src/devices/bus/adam/adamlink.h",
		MAME_DIR .. "src/devices/bus/adam/ide.cpp",
		MAME_DIR .. "src/devices/bus/adam/ide.h",
		MAME_DIR .. "src/devices/bus/adam/ram.cpp",
		MAME_DIR .. "src/devices/bus/adam/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/adamnet/adamnet.h,BUSES["ADAMNET"] = true
---------------------------------------------------

if (BUSES["ADAMNET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/adamnet/adamnet.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/adamnet.h",
		MAME_DIR .. "src/devices/bus/adamnet/ddp.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/ddp.h",
		MAME_DIR .. "src/devices/bus/adamnet/fdc.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/fdc.h",
		MAME_DIR .. "src/devices/bus/adamnet/kb.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/kb.h",
		MAME_DIR .. "src/devices/bus/adamnet/printer.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/printer.h",
		MAME_DIR .. "src/devices/bus/adamnet/spi.cpp",
		MAME_DIR .. "src/devices/bus/adamnet/spi.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apf/slot.h,BUSES["APF"] = true
---------------------------------------------------

if (BUSES["APF"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apf/slot.cpp",
		MAME_DIR .. "src/devices/bus/apf/slot.h",
		MAME_DIR .. "src/devices/bus/apf/rom.cpp",
		MAME_DIR .. "src/devices/bus/apf/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apricot/expansion.h,BUSES["APRICOT_EXPANSION"] = true
---------------------------------------------------

if (BUSES["APRICOT_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apricot/expansion.cpp",
		MAME_DIR .. "src/devices/bus/apricot/expansion.h",
		MAME_DIR .. "src/devices/bus/apricot/cards.cpp",
		MAME_DIR .. "src/devices/bus/apricot/cards.h",
		MAME_DIR .. "src/devices/bus/apricot/ram.cpp",
		MAME_DIR .. "src/devices/bus/apricot/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/arcadia/slot.h,BUSES["ARCADIA"] = true
---------------------------------------------------

if (BUSES["ARCADIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/arcadia/slot.cpp",
		MAME_DIR .. "src/devices/bus/arcadia/slot.h",
		MAME_DIR .. "src/devices/bus/arcadia/rom.cpp",
		MAME_DIR .. "src/devices/bus/arcadia/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/astrocde/slot.h,BUSES["ASTROCADE"] = true
---------------------------------------------------

if (BUSES["ASTROCADE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/astrocde/slot.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/slot.h",
		MAME_DIR .. "src/devices/bus/astrocde/rom.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/rom.h",
		MAME_DIR .. "src/devices/bus/astrocde/exp.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/exp.h",
		MAME_DIR .. "src/devices/bus/astrocde/ram.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bw2/exp.h,BUSES["BW2"] = true
---------------------------------------------------

if (BUSES["BW2"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bw2/exp.cpp",
		MAME_DIR .. "src/devices/bus/bw2/exp.h",
		MAME_DIR .. "src/devices/bus/bw2/ramcard.cpp",
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
		MAME_DIR .. "src/devices/bus/c64/exp.cpp",
		MAME_DIR .. "src/devices/bus/c64/exp.h",
		MAME_DIR .. "src/devices/bus/c64/c128_comal80.cpp",
		MAME_DIR .. "src/devices/bus/c64/c128_comal80.h",
		MAME_DIR .. "src/devices/bus/c64/c128_partner.cpp",
		MAME_DIR .. "src/devices/bus/c64/c128_partner.h",
		MAME_DIR .. "src/devices/bus/c64/comal80.cpp",
		MAME_DIR .. "src/devices/bus/c64/comal80.h",
		MAME_DIR .. "src/devices/bus/c64/cpm.cpp",
		MAME_DIR .. "src/devices/bus/c64/cpm.h",
		MAME_DIR .. "src/devices/bus/c64/currah_speech.cpp",
		MAME_DIR .. "src/devices/bus/c64/currah_speech.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep256.cpp",
		MAME_DIR .. "src/devices/bus/c64/dela_ep256.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep64.cpp",
		MAME_DIR .. "src/devices/bus/c64/dela_ep64.h",
		MAME_DIR .. "src/devices/bus/c64/dela_ep7x8.cpp",
		MAME_DIR .. "src/devices/bus/c64/dela_ep7x8.h",
		MAME_DIR .. "src/devices/bus/c64/dinamic.cpp",
		MAME_DIR .. "src/devices/bus/c64/dinamic.h",
		MAME_DIR .. "src/devices/bus/c64/dqbb.cpp",
		MAME_DIR .. "src/devices/bus/c64/dqbb.h",
		MAME_DIR .. "src/devices/bus/c64/easy_calc_result.cpp",
		MAME_DIR .. "src/devices/bus/c64/easy_calc_result.h",
		MAME_DIR .. "src/devices/bus/c64/easyflash.cpp",
		MAME_DIR .. "src/devices/bus/c64/easyflash.h",
		MAME_DIR .. "src/devices/bus/c64/epyx_fast_load.cpp",
		MAME_DIR .. "src/devices/bus/c64/epyx_fast_load.h",
		MAME_DIR .. "src/devices/bus/c64/exos.cpp",
		MAME_DIR .. "src/devices/bus/c64/exos.h",
		MAME_DIR .. "src/devices/bus/c64/fcc.cpp",
		MAME_DIR .. "src/devices/bus/c64/fcc.h",
		MAME_DIR .. "src/devices/bus/c64/final.cpp",
		MAME_DIR .. "src/devices/bus/c64/final.h",
		MAME_DIR .. "src/devices/bus/c64/final3.cpp",
		MAME_DIR .. "src/devices/bus/c64/final3.h",
		MAME_DIR .. "src/devices/bus/c64/fun_play.cpp",
		MAME_DIR .. "src/devices/bus/c64/fun_play.h",
		MAME_DIR .. "src/devices/bus/c64/georam.cpp",
		MAME_DIR .. "src/devices/bus/c64/georam.h",
		MAME_DIR .. "src/devices/bus/c64/ide64.cpp",
		MAME_DIR .. "src/devices/bus/c64/ide64.h",
		MAME_DIR .. "src/devices/bus/c64/ieee488.cpp",
		MAME_DIR .. "src/devices/bus/c64/ieee488.h",
		MAME_DIR .. "src/devices/bus/c64/kingsoft.cpp",
		MAME_DIR .. "src/devices/bus/c64/kingsoft.h",
		MAME_DIR .. "src/devices/bus/c64/mach5.cpp",
		MAME_DIR .. "src/devices/bus/c64/mach5.h",
		MAME_DIR .. "src/devices/bus/c64/magic_desk.cpp",
		MAME_DIR .. "src/devices/bus/c64/magic_desk.h",
		MAME_DIR .. "src/devices/bus/c64/magic_formel.cpp",
		MAME_DIR .. "src/devices/bus/c64/magic_formel.h",
		MAME_DIR .. "src/devices/bus/c64/magic_voice.cpp",
		MAME_DIR .. "src/devices/bus/c64/magic_voice.h",
		MAME_DIR .. "src/devices/bus/c64/midi_maplin.cpp",
		MAME_DIR .. "src/devices/bus/c64/midi_maplin.h",
		MAME_DIR .. "src/devices/bus/c64/midi_namesoft.cpp",
		MAME_DIR .. "src/devices/bus/c64/midi_namesoft.h",
		MAME_DIR .. "src/devices/bus/c64/midi_passport.cpp",
		MAME_DIR .. "src/devices/bus/c64/midi_passport.h",
		MAME_DIR .. "src/devices/bus/c64/midi_sci.cpp",
		MAME_DIR .. "src/devices/bus/c64/midi_sci.h",
		MAME_DIR .. "src/devices/bus/c64/midi_siel.cpp",
		MAME_DIR .. "src/devices/bus/c64/midi_siel.h",
		MAME_DIR .. "src/devices/bus/c64/mikro_assembler.cpp",
		MAME_DIR .. "src/devices/bus/c64/mikro_assembler.h",
		MAME_DIR .. "src/devices/bus/c64/multiscreen.cpp",
		MAME_DIR .. "src/devices/bus/c64/multiscreen.h",
		MAME_DIR .. "src/devices/bus/c64/music64.cpp",
		MAME_DIR .. "src/devices/bus/c64/music64.h",
		MAME_DIR .. "src/devices/bus/c64/neoram.cpp",
		MAME_DIR .. "src/devices/bus/c64/neoram.h",
		MAME_DIR .. "src/devices/bus/c64/ocean.cpp",
		MAME_DIR .. "src/devices/bus/c64/ocean.h",
		MAME_DIR .. "src/devices/bus/c64/pagefox.cpp",
		MAME_DIR .. "src/devices/bus/c64/pagefox.h",
		MAME_DIR .. "src/devices/bus/c64/partner.cpp",
		MAME_DIR .. "src/devices/bus/c64/partner.h",
		MAME_DIR .. "src/devices/bus/c64/prophet64.cpp",
		MAME_DIR .. "src/devices/bus/c64/prophet64.h",
		MAME_DIR .. "src/devices/bus/c64/ps64.cpp",
		MAME_DIR .. "src/devices/bus/c64/ps64.h",
		MAME_DIR .. "src/devices/bus/c64/reu.cpp",
		MAME_DIR .. "src/devices/bus/c64/reu.h",
		MAME_DIR .. "src/devices/bus/c64/rex.cpp",
		MAME_DIR .. "src/devices/bus/c64/rex.h",
		MAME_DIR .. "src/devices/bus/c64/rex_ep256.cpp",
		MAME_DIR .. "src/devices/bus/c64/rex_ep256.h",
		MAME_DIR .. "src/devices/bus/c64/ross.cpp",
		MAME_DIR .. "src/devices/bus/c64/ross.h",
		MAME_DIR .. "src/devices/bus/c64/sfx_sound_expander.cpp",
		MAME_DIR .. "src/devices/bus/c64/sfx_sound_expander.h",
		MAME_DIR .. "src/devices/bus/c64/silverrock.cpp",
		MAME_DIR .. "src/devices/bus/c64/silverrock.h",
		MAME_DIR .. "src/devices/bus/c64/simons_basic.cpp",
		MAME_DIR .. "src/devices/bus/c64/simons_basic.h",
		MAME_DIR .. "src/devices/bus/c64/stardos.cpp",
		MAME_DIR .. "src/devices/bus/c64/stardos.h",
		MAME_DIR .. "src/devices/bus/c64/std.cpp",
		MAME_DIR .. "src/devices/bus/c64/std.h",
		MAME_DIR .. "src/devices/bus/c64/structured_basic.cpp",
		MAME_DIR .. "src/devices/bus/c64/structured_basic.h",
		MAME_DIR .. "src/devices/bus/c64/super_explode.cpp",
		MAME_DIR .. "src/devices/bus/c64/super_explode.h",
		MAME_DIR .. "src/devices/bus/c64/super_games.cpp",
		MAME_DIR .. "src/devices/bus/c64/super_games.h",
		MAME_DIR .. "src/devices/bus/c64/supercpu.cpp",
		MAME_DIR .. "src/devices/bus/c64/supercpu.h",
		MAME_DIR .. "src/devices/bus/c64/sw8k.cpp",
		MAME_DIR .. "src/devices/bus/c64/sw8k.h",
		MAME_DIR .. "src/devices/bus/c64/swiftlink.cpp",
		MAME_DIR .. "src/devices/bus/c64/swiftlink.h",
		MAME_DIR .. "src/devices/bus/c64/system3.cpp",
		MAME_DIR .. "src/devices/bus/c64/system3.h",
		MAME_DIR .. "src/devices/bus/c64/tdos.cpp",
		MAME_DIR .. "src/devices/bus/c64/tdos.h",
		MAME_DIR .. "src/devices/bus/c64/turbo232.cpp",
		MAME_DIR .. "src/devices/bus/c64/turbo232.h",
		MAME_DIR .. "src/devices/bus/c64/vizastar.cpp",
		MAME_DIR .. "src/devices/bus/c64/vizastar.h",
		MAME_DIR .. "src/devices/bus/c64/vw64.cpp",
		MAME_DIR .. "src/devices/bus/c64/vw64.h",
		MAME_DIR .. "src/devices/bus/c64/warp_speed.cpp",
		MAME_DIR .. "src/devices/bus/c64/warp_speed.h",
		MAME_DIR .. "src/devices/bus/c64/westermann.cpp",
		MAME_DIR .. "src/devices/bus/c64/westermann.h",
		MAME_DIR .. "src/devices/bus/c64/xl80.cpp",
		MAME_DIR .. "src/devices/bus/c64/xl80.h",
		MAME_DIR .. "src/devices/bus/c64/zaxxon.cpp",
		MAME_DIR .. "src/devices/bus/c64/zaxxon.h",
		MAME_DIR .. "src/devices/bus/c64/user.cpp",
		MAME_DIR .. "src/devices/bus/c64/user.h",
		MAME_DIR .. "src/devices/bus/c64/4dxh.cpp",
		MAME_DIR .. "src/devices/bus/c64/4dxh.h",
		MAME_DIR .. "src/devices/bus/c64/4ksa.cpp",
		MAME_DIR .. "src/devices/bus/c64/4ksa.h",
		MAME_DIR .. "src/devices/bus/c64/4tba.cpp",
		MAME_DIR .. "src/devices/bus/c64/4tba.h",
		MAME_DIR .. "src/devices/bus/c64/16kb.cpp",
		MAME_DIR .. "src/devices/bus/c64/16kb.h",
		MAME_DIR .. "src/devices/bus/c64/bn1541.cpp",
		MAME_DIR .. "src/devices/bus/c64/bn1541.h",
		MAME_DIR .. "src/devices/bus/c64/geocable.cpp",
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
		MAME_DIR .. "src/devices/bus/cbm2/exp.cpp",
		MAME_DIR .. "src/devices/bus/cbm2/exp.h",
		MAME_DIR .. "src/devices/bus/cbm2/24k.cpp",
		MAME_DIR .. "src/devices/bus/cbm2/24k.h",
		MAME_DIR .. "src/devices/bus/cbm2/hrg.cpp",
		MAME_DIR .. "src/devices/bus/cbm2/hrg.h",
		MAME_DIR .. "src/devices/bus/cbm2/std.cpp",
		MAME_DIR .. "src/devices/bus/cbm2/std.h",
		MAME_DIR .. "src/devices/bus/cbm2/user.cpp",
		MAME_DIR .. "src/devices/bus/cbm2/user.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/cbmiec/cbmiec.h,BUSES["CBMIEC"] = true
---------------------------------------------------

if (BUSES["CBMIEC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cbmiec/cbmiec.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/cbmiec.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1541.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/c1541.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1571.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/c1571.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1581.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/c1581.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c64_nl10.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/c64_nl10.h",
		MAME_DIR .. "src/devices/bus/cbmiec/cmdhd.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/cmdhd.h",
		MAME_DIR .. "src/devices/bus/cbmiec/diag264_lb_iec.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/diag264_lb_iec.h",
		MAME_DIR .. "src/devices/bus/cbmiec/fd2000.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/fd2000.h",
		MAME_DIR .. "src/devices/bus/cbmiec/interpod.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/interpod.h",
		MAME_DIR .. "src/devices/bus/cbmiec/serialbox.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/serialbox.h",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1515.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1515.h",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1520.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/vic1520.h",
		MAME_DIR .. "src/devices/bus/cbmiec/c1526.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/c1526.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/chanf/slot.h,BUSES["CHANNELF"] = true
---------------------------------------------------

if (BUSES["CHANNELF"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/chanf/slot.cpp",
		MAME_DIR .. "src/devices/bus/chanf/slot.h",
		MAME_DIR .. "src/devices/bus/chanf/rom.cpp",
		MAME_DIR .. "src/devices/bus/chanf/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/comx35/exp.h,BUSES["COMX35"] = true
---------------------------------------------------

if (BUSES["COMX35"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/comx35/exp.cpp",
		MAME_DIR .. "src/devices/bus/comx35/exp.h",
		MAME_DIR .. "src/devices/bus/comx35/clm.cpp",
		MAME_DIR .. "src/devices/bus/comx35/clm.h",
		MAME_DIR .. "src/devices/bus/comx35/expbox.cpp",
		MAME_DIR .. "src/devices/bus/comx35/expbox.h",
		MAME_DIR .. "src/devices/bus/comx35/eprom.cpp",
		MAME_DIR .. "src/devices/bus/comx35/eprom.h",
		MAME_DIR .. "src/devices/bus/comx35/fdc.cpp",
		MAME_DIR .. "src/devices/bus/comx35/fdc.h",
		MAME_DIR .. "src/devices/bus/comx35/joycard.cpp",
		MAME_DIR .. "src/devices/bus/comx35/joycard.h",
		MAME_DIR .. "src/devices/bus/comx35/printer.cpp",
		MAME_DIR .. "src/devices/bus/comx35/printer.h",
		MAME_DIR .. "src/devices/bus/comx35/ram.cpp",
		MAME_DIR .. "src/devices/bus/comx35/ram.h",
		MAME_DIR .. "src/devices/bus/comx35/thermal.cpp",
		MAME_DIR .. "src/devices/bus/comx35/thermal.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/coleco/ctrl.h,BUSES["COLECO"] = true
---------------------------------------------------

if (BUSES["COLECO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coleco/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/coleco/ctrl.h",
		MAME_DIR .. "src/devices/bus/coleco/hand.cpp",
		MAME_DIR .. "src/devices/bus/coleco/hand.h",
		MAME_DIR .. "src/devices/bus/coleco/sac.cpp",
		MAME_DIR .. "src/devices/bus/coleco/sac.h",
		MAME_DIR .. "src/devices/bus/coleco/exp.cpp",
		MAME_DIR .. "src/devices/bus/coleco/exp.h",
		MAME_DIR .. "src/devices/bus/coleco/std.cpp",
		MAME_DIR .. "src/devices/bus/coleco/std.h",
		MAME_DIR .. "src/devices/bus/coleco/xin1.h",
		MAME_DIR .. "src/devices/bus/coleco/xin1.cpp",
	}
end


---------------------------------------------------
--
--@src/devices/bus/crvision/slot.h,BUSES["CRVISION"] = true
---------------------------------------------------

if (BUSES["CRVISION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/crvision/slot.cpp",
		MAME_DIR .. "src/devices/bus/crvision/slot.h",
		MAME_DIR .. "src/devices/bus/crvision/rom.cpp",
		MAME_DIR .. "src/devices/bus/crvision/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/dmv/dmv.h,BUSES["DMV"] = true
---------------------------------------------------

if (BUSES["DMV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.cpp",
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.h",
		MAME_DIR .. "src/devices/bus/dmv/k210.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k210.h",
		MAME_DIR .. "src/devices/bus/dmv/k220.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k220.h",
		MAME_DIR .. "src/devices/bus/dmv/k230.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k230.h",
		MAME_DIR .. "src/devices/bus/dmv/k233.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k233.h",
		MAME_DIR .. "src/devices/bus/dmv/k801.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k801.h",
		MAME_DIR .. "src/devices/bus/dmv/k803.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k803.h",
		MAME_DIR .. "src/devices/bus/dmv/k806.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k806.h",
		MAME_DIR .. "src/devices/bus/dmv/ram.cpp",
		MAME_DIR .. "src/devices/bus/dmv/ram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ecbbus/ecbbus.h,BUSES["ECBBUS"] = true
---------------------------------------------------

if (BUSES["ECBBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ecbbus/ecbbus.cpp",
		MAME_DIR .. "src/devices/bus/ecbbus/ecbbus.h",
		MAME_DIR .. "src/devices/bus/ecbbus/grip.cpp",
		MAME_DIR .. "src/devices/bus/ecbbus/grip.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/econet/econet.h,BUSES["ECONET"] = true
---------------------------------------------------

if (BUSES["ECONET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/econet/econet.cpp",
		MAME_DIR .. "src/devices/bus/econet/econet.h",
		MAME_DIR .. "src/devices/bus/econet/e01.cpp",
		MAME_DIR .. "src/devices/bus/econet/e01.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ep64/exp.h,BUSES["EP64"] = true
---------------------------------------------------

if (BUSES["EP64"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ep64/exp.cpp",
		MAME_DIR .. "src/devices/bus/ep64/exp.h",
		MAME_DIR .. "src/devices/bus/ep64/exdos.cpp",
		MAME_DIR .. "src/devices/bus/ep64/exdos.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/generic/slot.h,BUSES["GENERIC"] = true
---------------------------------------------------

if (BUSES["GENERIC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/generic/slot.cpp",
		MAME_DIR .. "src/devices/bus/generic/slot.h",
		MAME_DIR .. "src/devices/bus/generic/carts.cpp",
		MAME_DIR .. "src/devices/bus/generic/carts.h",
		MAME_DIR .. "src/devices/bus/generic/ram.cpp",
		MAME_DIR .. "src/devices/bus/generic/ram.h",
		MAME_DIR .. "src/devices/bus/generic/rom.cpp",
		MAME_DIR .. "src/devices/bus/generic/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ieee488/ieee488.h,BUSES["IEEE488"] = true
---------------------------------------------------

if (BUSES["IEEE488"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ieee488/ieee488.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/ieee488.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2031.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c2031.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2040.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c2040.h",
		MAME_DIR .. "src/devices/bus/ieee488/c2040fdc.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c2040fdc.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8050.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c8050.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8050fdc.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c8050fdc.h",
		MAME_DIR .. "src/devices/bus/ieee488/c8280.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/c8280.h",
		MAME_DIR .. "src/devices/bus/ieee488/d9060.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/d9060.h",
		MAME_DIR .. "src/devices/bus/ieee488/softbox.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/softbox.h",
		MAME_DIR .. "src/devices/bus/ieee488/hardbox.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/hardbox.h",
		MAME_DIR .. "src/devices/bus/ieee488/shark.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/shark.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/iq151/iq151.h,BUSES["IQ151"] = true
---------------------------------------------------

if (BUSES["IQ151"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/iq151/iq151.cpp",
		MAME_DIR .. "src/devices/bus/iq151/iq151.h",
		MAME_DIR .. "src/devices/bus/iq151/disc2.cpp",
		MAME_DIR .. "src/devices/bus/iq151/disc2.h",
		MAME_DIR .. "src/devices/bus/iq151/grafik.cpp",
		MAME_DIR .. "src/devices/bus/iq151/grafik.h",
		MAME_DIR .. "src/devices/bus/iq151/minigraf.cpp",
		MAME_DIR .. "src/devices/bus/iq151/minigraf.h",
		MAME_DIR .. "src/devices/bus/iq151/ms151a.cpp",
		MAME_DIR .. "src/devices/bus/iq151/ms151a.h",
		MAME_DIR .. "src/devices/bus/iq151/rom.cpp",
		MAME_DIR .. "src/devices/bus/iq151/rom.h",
		MAME_DIR .. "src/devices/bus/iq151/staper.cpp",
		MAME_DIR .. "src/devices/bus/iq151/staper.h",
		MAME_DIR .. "src/devices/bus/iq151/video32.cpp",
		MAME_DIR .. "src/devices/bus/iq151/video32.h",
		MAME_DIR .. "src/devices/bus/iq151/video64.cpp",
		MAME_DIR .. "src/devices/bus/iq151/video64.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/isbx/isbx.h,BUSES["IMI7000"] = true
---------------------------------------------------

if (BUSES["IMI7000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/imi7000/imi7000.cpp",
		MAME_DIR .. "src/devices/bus/imi7000/imi7000.h",
		MAME_DIR .. "src/devices/bus/imi7000/imi5000h.cpp",
		MAME_DIR .. "src/devices/bus/imi7000/imi5000h.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/intv/slot.h,BUSES["INTV"] = true
---------------------------------------------------

if (BUSES["INTV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/intv/slot.cpp",
		MAME_DIR .. "src/devices/bus/intv/slot.h",
		MAME_DIR .. "src/devices/bus/intv/rom.cpp",
		MAME_DIR .. "src/devices/bus/intv/rom.h",
		MAME_DIR .. "src/devices/bus/intv/voice.cpp",
		MAME_DIR .. "src/devices/bus/intv/voice.h",
		MAME_DIR .. "src/devices/bus/intv/ecs.cpp",
		MAME_DIR .. "src/devices/bus/intv/ecs.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/isa/isa.h,BUSES["ISA"] = true
---------------------------------------------------

if (BUSES["ISA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/isa/isa.cpp",
		MAME_DIR .. "src/devices/bus/isa/isa.h",
		MAME_DIR .. "src/devices/bus/isa/isa_cards.cpp",
		MAME_DIR .. "src/devices/bus/isa/isa_cards.h",
		MAME_DIR .. "src/devices/bus/isa/mda.cpp",
		MAME_DIR .. "src/devices/bus/isa/mda.h",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.cpp",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.h",
		MAME_DIR .. "src/devices/bus/isa/adlib.cpp",
		MAME_DIR .. "src/devices/bus/isa/adlib.h",
		MAME_DIR .. "src/devices/bus/isa/com.cpp",
		MAME_DIR .. "src/devices/bus/isa/com.h",
		MAME_DIR .. "src/devices/bus/isa/fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/fdc.h",
		MAME_DIR .. "src/devices/bus/isa/mufdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/mufdc.h",
		MAME_DIR .. "src/devices/bus/isa/finalchs.cpp",
		MAME_DIR .. "src/devices/bus/isa/finalchs.h",
		MAME_DIR .. "src/devices/bus/isa/gblaster.cpp",
		MAME_DIR .. "src/devices/bus/isa/gblaster.h",
		MAME_DIR .. "src/devices/bus/isa/gus.cpp",
		MAME_DIR .. "src/devices/bus/isa/gus.h",
		MAME_DIR .. "src/devices/bus/isa/sb16.cpp",
		MAME_DIR .. "src/devices/bus/isa/sb16.h",
		MAME_DIR .. "src/devices/bus/isa/hdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/hdc.h",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.cpp",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.h",
		MAME_DIR .. "src/devices/bus/isa/mpu401.cpp",
		MAME_DIR .. "src/devices/bus/isa/mpu401.h",
		MAME_DIR .. "src/devices/bus/isa/sblaster.cpp",
		MAME_DIR .. "src/devices/bus/isa/sblaster.h",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.cpp",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.h",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.cpp",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.h",
		MAME_DIR .. "src/devices/bus/isa/ide.cpp",
		MAME_DIR .. "src/devices/bus/isa/ide.h",
		MAME_DIR .. "src/devices/bus/isa/xtide.cpp",
		MAME_DIR .. "src/devices/bus/isa/xtide.h",
		MAME_DIR .. "src/devices/bus/isa/side116.cpp",
		MAME_DIR .. "src/devices/bus/isa/side116.h",
		MAME_DIR .. "src/devices/bus/isa/aha1542.cpp",
		MAME_DIR .. "src/devices/bus/isa/aha1542.h",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.cpp",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.h",
		MAME_DIR .. "src/devices/bus/isa/dectalk.cpp",
		MAME_DIR .. "src/devices/bus/isa/dectalk.h",
		MAME_DIR .. "src/devices/bus/isa/pds.cpp",
		MAME_DIR .. "src/devices/bus/isa/pds.h",
		MAME_DIR .. "src/devices/bus/isa/omti8621.cpp",
		MAME_DIR .. "src/devices/bus/isa/omti8621.h",
		MAME_DIR .. "src/devices/bus/isa/cga.cpp",
		MAME_DIR .. "src/devices/bus/isa/cga.h",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.h",
		MAME_DIR .. "src/devices/bus/isa/ega.cpp",
		MAME_DIR .. "src/devices/bus/isa/ega.h",
		MAME_DIR .. "src/devices/bus/isa/pgc.cpp",
		MAME_DIR .. "src/devices/bus/isa/pgc.h",
		MAME_DIR .. "src/devices/bus/isa/vga.cpp",
		MAME_DIR .. "src/devices/bus/isa/vga.h",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.cpp",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.h",
		MAME_DIR .. "src/devices/bus/isa/mach32.cpp",
		MAME_DIR .. "src/devices/bus/isa/mach32.h",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.h",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.h",
		MAME_DIR .. "src/devices/bus/isa/s3virge.cpp",
		MAME_DIR .. "src/devices/bus/isa/s3virge.h",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.cpp",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.h",
		MAME_DIR .. "src/devices/bus/isa/3c503.cpp",
		MAME_DIR .. "src/devices/bus/isa/3c503.h",
		MAME_DIR .. "src/devices/bus/isa/ne1000.cpp",
		MAME_DIR .. "src/devices/bus/isa/ne1000.h",
		MAME_DIR .. "src/devices/bus/isa/ne2000.cpp",
		MAME_DIR .. "src/devices/bus/isa/ne2000.h",
		MAME_DIR .. "src/devices/bus/isa/3c505.cpp",
		MAME_DIR .. "src/devices/bus/isa/3c505.h",
		MAME_DIR .. "src/devices/bus/isa/lpt.cpp",
		MAME_DIR .. "src/devices/bus/isa/lpt.h",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.cpp",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.h",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.cpp",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.h",
		MAME_DIR .. "src/devices/bus/isa/sc499.cpp",
		MAME_DIR .. "src/devices/bus/isa/sc499.h",
		MAME_DIR .. "src/devices/bus/isa/aga.cpp",
		MAME_DIR .. "src/devices/bus/isa/aga.h",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.h",
		MAME_DIR .. "src/devices/bus/isa/num9rev.cpp",
		MAME_DIR .. "src/devices/bus/isa/num9rev.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/isbx/isbx.h,BUSES["ISBX"] = true
---------------------------------------------------

if (BUSES["ISBX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/isbx/isbx.cpp",
		MAME_DIR .. "src/devices/bus/isbx/isbx.h",
		MAME_DIR .. "src/devices/bus/isbx/compis_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isbx/compis_fdc.h",
		MAME_DIR .. "src/devices/bus/isbx/isbc_218a.cpp",
		MAME_DIR .. "src/devices/bus/isbx/isbc_218a.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/msx_slot/slot.h,BUSES["MSX_SLOT"] = true
---------------------------------------------------

if (BUSES["MSX_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/msx_slot/bunsetsu.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/bunsetsu.h",
		MAME_DIR .. "src/devices/bus/msx_slot/cartridge.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx_slot/disk.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/disk.h",
		MAME_DIR .. "src/devices/bus/msx_slot/fs4600.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/fs4600.h",
		MAME_DIR .. "src/devices/bus/msx_slot/music.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/music.h",
		MAME_DIR .. "src/devices/bus/msx_slot/panasonic08.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/panasonic08.h",
		MAME_DIR .. "src/devices/bus/msx_slot/rom.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/rom.h",
		MAME_DIR .. "src/devices/bus/msx_slot/ram.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/ram.h",
		MAME_DIR .. "src/devices/bus/msx_slot/ram_mm.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/ram_mm.h",
		MAME_DIR .. "src/devices/bus/msx_slot/slot.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/slot.h",
		MAME_DIR .. "src/devices/bus/msx_slot/sony08.cpp",
		MAME_DIR .. "src/devices/bus/msx_slot/sony08.h",
		MAME_DIR .. "src/devices/bus/msx_cart/arc.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/arc.h",
		MAME_DIR .. "src/devices/bus/msx_cart/ascii.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/ascii.h",
		MAME_DIR .. "src/devices/bus/msx_cart/bm_012.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/bm_012.h",
		MAME_DIR .. "src/devices/bus/msx_cart/cartridge.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx_cart/crossblaim.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/crossblaim.h",
		MAME_DIR .. "src/devices/bus/msx_cart/disk.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/disk.h",
		MAME_DIR .. "src/devices/bus/msx_cart/dooly.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/dooly.h",
		MAME_DIR .. "src/devices/bus/msx_cart/fmpac.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/fmpac.h",
		MAME_DIR .. "src/devices/bus/msx_cart/fs_sr022.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/fs_sr022.h",
		MAME_DIR .. "src/devices/bus/msx_cart/halnote.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/halnote.h",
		MAME_DIR .. "src/devices/bus/msx_cart/hfox.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/hfox.h",
		MAME_DIR .. "src/devices/bus/msx_cart/holy_quran.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/holy_quran.h",
		MAME_DIR .. "src/devices/bus/msx_cart/konami.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/konami.h",
		MAME_DIR .. "src/devices/bus/msx_cart/korean.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/korean.h",
		MAME_DIR .. "src/devices/bus/msx_cart/majutsushi.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/majutsushi.h",
		MAME_DIR .. "src/devices/bus/msx_cart/moonsound.h",
		MAME_DIR .. "src/devices/bus/msx_cart/moonsound.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio.h",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio_kb.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/msx_audio_kb.h",
		MAME_DIR .. "src/devices/bus/msx_cart/msxdos2.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/msxdos2.h",
		MAME_DIR .. "src/devices/bus/msx_cart/nomapper.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/nomapper.h",
		MAME_DIR .. "src/devices/bus/msx_cart/rtype.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/rtype.h",
		MAME_DIR .. "src/devices/bus/msx_cart/superloderunner.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/superloderunner.h",
		MAME_DIR .. "src/devices/bus/msx_cart/super_swangi.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/super_swangi.h",
		MAME_DIR .. "src/devices/bus/msx_cart/yamaha.cpp",
		MAME_DIR .. "src/devices/bus/msx_cart/yamaha.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/kc/kc.h,BUSES["KC"] = true
---------------------------------------------------

if (BUSES["KC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/kc/kc.cpp",
		MAME_DIR .. "src/devices/bus/kc/kc.h",
		MAME_DIR .. "src/devices/bus/kc/d002.cpp",
		MAME_DIR .. "src/devices/bus/kc/d002.h",
		MAME_DIR .. "src/devices/bus/kc/d004.cpp",
		MAME_DIR .. "src/devices/bus/kc/d004.h",
		MAME_DIR .. "src/devices/bus/kc/ram.cpp",
		MAME_DIR .. "src/devices/bus/kc/ram.h",
		MAME_DIR .. "src/devices/bus/kc/rom.cpp",
		MAME_DIR .. "src/devices/bus/kc/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/odyssey2/slot.h,BUSES["O2"] = true
---------------------------------------------------

if (BUSES["O2"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/odyssey2/slot.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/slot.h",
		MAME_DIR .. "src/devices/bus/odyssey2/rom.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/rom.h",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.h",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pc_joy/pc_joy.h,BUSES["PC_JOY"] = true
---------------------------------------------------

if (BUSES["PC_JOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy.cpp",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy.h",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.cpp",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pc_kbd/pc_kbdc.h,BUSES["PC_KBD"] = true
---------------------------------------------------

if (BUSES["PC_KBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc_kbd/pc_kbdc.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc_kbdc.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/keyboards.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/keyboards.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/ec1841.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/ec1841.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/iskr1030.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/iskr1030.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/keytro.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/keytro.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/msnat.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/msnat.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc83.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/pc83.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat84.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat84.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcxt83.cpp",
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
		MAME_DIR .. "src/devices/bus/pet/cass.cpp",
		MAME_DIR .. "src/devices/bus/pet/cass.h",
		MAME_DIR .. "src/devices/bus/pet/c2n.cpp",
		MAME_DIR .. "src/devices/bus/pet/c2n.h",
		MAME_DIR .. "src/devices/bus/pet/diag264_lb_tape.cpp",
		MAME_DIR .. "src/devices/bus/pet/diag264_lb_tape.h",
		MAME_DIR .. "src/devices/bus/pet/exp.cpp",
		MAME_DIR .. "src/devices/bus/pet/exp.h",
		MAME_DIR .. "src/devices/bus/pet/64k.cpp",
		MAME_DIR .. "src/devices/bus/pet/64k.h",
		MAME_DIR .. "src/devices/bus/pet/hsg.cpp",
		MAME_DIR .. "src/devices/bus/pet/hsg.h",
		MAME_DIR .. "src/devices/bus/pet/superpet.cpp",
		MAME_DIR .. "src/devices/bus/pet/superpet.h",
		MAME_DIR .. "src/devices/bus/pet/user.cpp",
		MAME_DIR .. "src/devices/bus/pet/user.h",
		MAME_DIR .. "src/devices/bus/pet/diag.cpp",
		MAME_DIR .. "src/devices/bus/pet/diag.h",
		MAME_DIR .. "src/devices/bus/pet/petuja.cpp",
		MAME_DIR .. "src/devices/bus/pet/petuja.h",
		MAME_DIR .. "src/devices/bus/pet/cb2snd.cpp",
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
		MAME_DIR .. "src/devices/bus/plus4/exp.cpp",
		MAME_DIR .. "src/devices/bus/plus4/exp.h",
		MAME_DIR .. "src/devices/bus/plus4/c1551.cpp",
		MAME_DIR .. "src/devices/bus/plus4/c1551.h",
		MAME_DIR .. "src/devices/bus/plus4/sid.cpp",
		MAME_DIR .. "src/devices/bus/plus4/sid.h",
		MAME_DIR .. "src/devices/bus/plus4/std.cpp",
		MAME_DIR .. "src/devices/bus/plus4/std.h",
		MAME_DIR .. "src/devices/bus/plus4/user.cpp",
		MAME_DIR .. "src/devices/bus/plus4/user.h",
		MAME_DIR .. "src/devices/bus/plus4/diag264_lb_user.cpp",
		MAME_DIR .. "src/devices/bus/plus4/diag264_lb_user.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/s100/s100.h,BUSES["S100"] = true
---------------------------------------------------

if (BUSES["S100"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/s100/s100.cpp",
		MAME_DIR .. "src/devices/bus/s100/s100.h",
		MAME_DIR .. "src/devices/bus/s100/dj2db.cpp",
		MAME_DIR .. "src/devices/bus/s100/dj2db.h",
		MAME_DIR .. "src/devices/bus/s100/djdma.cpp",
		MAME_DIR .. "src/devices/bus/s100/djdma.h",
		MAME_DIR .. "src/devices/bus/s100/mm65k16s.cpp",
		MAME_DIR .. "src/devices/bus/s100/mm65k16s.h",
		MAME_DIR .. "src/devices/bus/s100/nsmdsa.cpp",
		MAME_DIR .. "src/devices/bus/s100/nsmdsa.h",
		MAME_DIR .. "src/devices/bus/s100/nsmdsad.cpp",
		MAME_DIR .. "src/devices/bus/s100/nsmdsad.h",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.cpp",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/spc1000/exp.h,BUSES["SPC1000"] = true
---------------------------------------------------

if (BUSES["SPC1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/spc1000/exp.cpp",
		MAME_DIR .. "src/devices/bus/spc1000/exp.h",
		MAME_DIR .. "src/devices/bus/spc1000/fdd.cpp",
		MAME_DIR .. "src/devices/bus/spc1000/fdd.h",
		MAME_DIR .. "src/devices/bus/spc1000/vdp.cpp",
		MAME_DIR .. "src/devices/bus/spc1000/vdp.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/tiki100/exp.h,BUSES["TIKI100"] = true
---------------------------------------------------

if (BUSES["TIKI100"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tiki100/exp.cpp",
		MAME_DIR .. "src/devices/bus/tiki100/exp.h",
		MAME_DIR .. "src/devices/bus/tiki100/8088.cpp",
		MAME_DIR .. "src/devices/bus/tiki100/8088.h",
		MAME_DIR .. "src/devices/bus/tiki100/hdc.cpp",
		MAME_DIR .. "src/devices/bus/tiki100/hdc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/tvc/tvc.h,BUSES["TVC"] = true
---------------------------------------------------

if (BUSES["TVC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tvc/tvc.cpp",
		MAME_DIR .. "src/devices/bus/tvc/tvc.h",
		MAME_DIR .. "src/devices/bus/tvc/hbf.cpp",
		MAME_DIR .. "src/devices/bus/tvc/hbf.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vc4000/slot.h,BUSES["VC4000"] = true
---------------------------------------------------

if (BUSES["VC4000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vc4000/slot.cpp",
		MAME_DIR .. "src/devices/bus/vc4000/slot.h",
		MAME_DIR .. "src/devices/bus/vc4000/rom.cpp",
		MAME_DIR .. "src/devices/bus/vc4000/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vcs/vcs_slot.h,BUSES["VCS"] = true
---------------------------------------------------

if (BUSES["VCS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vcs/vcs_slot.cpp",
		MAME_DIR .. "src/devices/bus/vcs/vcs_slot.h",
		MAME_DIR .. "src/devices/bus/vcs/rom.cpp",
		MAME_DIR .. "src/devices/bus/vcs/rom.h",
		MAME_DIR .. "src/devices/bus/vcs/compumat.cpp",
		MAME_DIR .. "src/devices/bus/vcs/compumat.h",
		MAME_DIR .. "src/devices/bus/vcs/dpc.cpp",
		MAME_DIR .. "src/devices/bus/vcs/dpc.h",
		MAME_DIR .. "src/devices/bus/vcs/harmony_melody.cpp",
		MAME_DIR .. "src/devices/bus/vcs/harmony_melody.h",
		MAME_DIR .. "src/devices/bus/vcs/scharger.cpp",
		MAME_DIR .. "src/devices/bus/vcs/scharger.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vcs/ctrl.h,BUSES["VCS_CTRL"] = true
---------------------------------------------------

if (BUSES["VCS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/wheel.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/wheel.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vectrex/slot.h,BUSES["VECTREX"] = true
---------------------------------------------------

if (BUSES["VECTREX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vectrex/slot.cpp",
		MAME_DIR .. "src/devices/bus/vectrex/slot.h",
		MAME_DIR .. "src/devices/bus/vectrex/rom.cpp",
		MAME_DIR .. "src/devices/bus/vectrex/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vic10/exp.h,BUSES["VIC10"] = true
---------------------------------------------------

if (BUSES["VIC10"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vic10/exp.cpp",
		MAME_DIR .. "src/devices/bus/vic10/exp.h",
		MAME_DIR .. "src/devices/bus/vic10/std.cpp",
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
		MAME_DIR .. "src/devices/bus/vic20/exp.cpp",
		MAME_DIR .. "src/devices/bus/vic20/exp.h",
		MAME_DIR .. "src/devices/bus/vic20/fe3.cpp",
		MAME_DIR .. "src/devices/bus/vic20/fe3.h",
		MAME_DIR .. "src/devices/bus/vic20/megacart.cpp",
		MAME_DIR .. "src/devices/bus/vic20/megacart.h",
		MAME_DIR .. "src/devices/bus/vic20/std.cpp",
		MAME_DIR .. "src/devices/bus/vic20/std.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1010.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1010.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1110.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1110.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1111.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1111.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1112.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1112.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1210.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1210.h",
		MAME_DIR .. "src/devices/bus/vic20/user.cpp",
		MAME_DIR .. "src/devices/bus/vic20/user.h",
		MAME_DIR .. "src/devices/bus/vic20/4cga.cpp",
		MAME_DIR .. "src/devices/bus/vic20/4cga.h",
		MAME_DIR .. "src/devices/bus/vic20/vic1011.cpp",
		MAME_DIR .. "src/devices/bus/vic20/vic1011.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/vidbrain/exp.h,BUSES["VIDBRAIN"] = true
---------------------------------------------------

if (BUSES["VIDBRAIN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vidbrain/exp.cpp",
		MAME_DIR .. "src/devices/bus/vidbrain/exp.h",
		MAME_DIR .. "src/devices/bus/vidbrain/std.cpp",
		MAME_DIR .. "src/devices/bus/vidbrain/std.h",
		MAME_DIR .. "src/devices/bus/vidbrain/money_minder.cpp",
		MAME_DIR .. "src/devices/bus/vidbrain/money_minder.h",
		MAME_DIR .. "src/devices/bus/vidbrain/timeshare.cpp",
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
		MAME_DIR .. "src/devices/bus/vip/byteio.cpp",
		MAME_DIR .. "src/devices/bus/vip/byteio.h",
		MAME_DIR .. "src/devices/bus/vip/vp620.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp620.h",
		MAME_DIR .. "src/devices/bus/vip/exp.cpp",
		MAME_DIR .. "src/devices/bus/vip/exp.h",
		MAME_DIR .. "src/devices/bus/vip/vp550.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp550.h",
		MAME_DIR .. "src/devices/bus/vip/vp570.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp570.h",
		MAME_DIR .. "src/devices/bus/vip/vp575.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp575.h",
		MAME_DIR .. "src/devices/bus/vip/vp585.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp585.h",
		MAME_DIR .. "src/devices/bus/vip/vp590.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp590.h",
		MAME_DIR .. "src/devices/bus/vip/vp595.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp595.h",
		MAME_DIR .. "src/devices/bus/vip/vp700.cpp",
		MAME_DIR .. "src/devices/bus/vip/vp700.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/wangpc/wangpc.h,BUSES["WANGPC"] = true
---------------------------------------------------

if (BUSES["WANGPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/wangpc/wangpc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/wangpc.h",
		MAME_DIR .. "src/devices/bus/wangpc/emb.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/emb.h",
		MAME_DIR .. "src/devices/bus/wangpc/lic.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/lic.h",
		MAME_DIR .. "src/devices/bus/wangpc/lvc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/lvc.h",
		MAME_DIR .. "src/devices/bus/wangpc/mcc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/mcc.h",
		MAME_DIR .. "src/devices/bus/wangpc/mvc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/mvc.h",
		MAME_DIR .. "src/devices/bus/wangpc/rtc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/rtc.h",
		MAME_DIR .. "src/devices/bus/wangpc/tig.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/tig.h",
		MAME_DIR .. "src/devices/bus/wangpc/wdc.cpp",
		MAME_DIR .. "src/devices/bus/wangpc/wdc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/z88/z88.h,BUSES["Z88"] = true
---------------------------------------------------

if (BUSES["Z88"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/z88/z88.cpp",
		MAME_DIR .. "src/devices/bus/z88/z88.h",
		MAME_DIR .. "src/devices/bus/z88/flash.cpp",
		MAME_DIR .. "src/devices/bus/z88/flash.h",
		MAME_DIR .. "src/devices/bus/z88/ram.cpp",
		MAME_DIR .. "src/devices/bus/z88/ram.h",
		MAME_DIR .. "src/devices/bus/z88/rom.cpp",
		MAME_DIR .. "src/devices/bus/z88/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/a2bus/a2bus.h,BUSES["A2BUS"] = true
---------------------------------------------------

if (BUSES["A2BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a2bus/a2bus.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2bus.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2lang.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2lang.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskii.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskii.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.h",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.h",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.h",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.h",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nubus/nubus.h,BUSES["NUBUS"] = true
---------------------------------------------------

if (BUSES["NUBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nubus/nubus.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_48gc.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_48gc.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_cb264.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_cb264.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_vikbw.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_vikbw.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_specpdq.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_specpdq.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2hires.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2hires.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_spec8.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_spec8.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_radiustpd.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_radiustpd.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2video.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_m2video.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_asntmc3b.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_asntmc3b.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_image.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_image.h",
		MAME_DIR .. "src/devices/bus/nubus/nubus_wsportrait.cpp",
		MAME_DIR .. "src/devices/bus/nubus/nubus_wsportrait.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_cb264.cpp",
		MAME_DIR .. "src/devices/bus/nubus/pds30_cb264.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_procolor816.cpp",
		MAME_DIR .. "src/devices/bus/nubus/pds30_procolor816.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_sigmalview.cpp",
		MAME_DIR .. "src/devices/bus/nubus/pds30_sigmalview.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_30hr.cpp",
		MAME_DIR .. "src/devices/bus/nubus/pds30_30hr.h",
		MAME_DIR .. "src/devices/bus/nubus/pds30_mc30.cpp",
		MAME_DIR .. "src/devices/bus/nubus/pds30_mc30.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/centronics/ctronics.h,BUSES["CENTRONICS"] = true
---------------------------------------------------

if (BUSES["CENTRONICS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/centronics/ctronics.cpp",
		MAME_DIR .. "src/devices/bus/centronics/ctronics.h",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.cpp",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.h",
		MAME_DIR .. "src/devices/bus/centronics/covox.cpp",
		MAME_DIR .. "src/devices/bus/centronics/covox.h",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.cpp",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.h",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.cpp",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.h",
		MAME_DIR .. "src/devices/bus/centronics/printer.cpp",
		MAME_DIR .. "src/devices/bus/centronics/printer.h",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.cpp",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/centronics/epson_ex800.cpp",  GEN_DIR .. "emu/layout/ex800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx800.cpp",  GEN_DIR .. "emu/layout/lx800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.cpp", GEN_DIR .. "emu/layout/lx800.lh" },
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
		MAME_DIR .. "src/devices/bus/rs232/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/rs232/keyboard.h",
		MAME_DIR .. "src/devices/bus/rs232/loopback.cpp",
		MAME_DIR .. "src/devices/bus/rs232/loopback.h",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.cpp",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.h",
		MAME_DIR .. "src/devices/bus/rs232/printer.cpp",
		MAME_DIR .. "src/devices/bus/rs232/printer.h",
		MAME_DIR .. "src/devices/bus/rs232/rs232.cpp",
		MAME_DIR .. "src/devices/bus/rs232/rs232.h",
		MAME_DIR .. "src/devices/bus/rs232/pty.cpp",
		MAME_DIR .. "src/devices/bus/rs232/pty.h",
		MAME_DIR .. "src/devices/bus/rs232/ser_mouse.cpp",
		MAME_DIR .. "src/devices/bus/rs232/ser_mouse.h",
		MAME_DIR .. "src/devices/bus/rs232/terminal.cpp",
		MAME_DIR .. "src/devices/bus/rs232/terminal.h",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.cpp",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/midi/midi.h,BUSES["MIDI"] = true
---------------------------------------------------

if (BUSES["MIDI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/midi/midi.cpp",
		MAME_DIR .. "src/devices/bus/midi/midi.h",
		MAME_DIR .. "src/devices/bus/midi/midiinport.cpp",
		MAME_DIR .. "src/devices/bus/midi/midiinport.h",
		MAME_DIR .. "src/devices/bus/midi/midioutport.cpp",
		MAME_DIR .. "src/devices/bus/midi/midioutport.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/lpci/pci.h,BUSES["LPCI"] = true
---------------------------------------------------

if (BUSES["LPCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/lpci/pci.cpp",
		MAME_DIR .. "src/devices/bus/lpci/pci.h",
		MAME_DIR .. "src/devices/bus/lpci/cirrus.cpp",
		MAME_DIR .. "src/devices/bus/lpci/cirrus.h",
		MAME_DIR .. "src/devices/bus/lpci/i82371ab.cpp",
		MAME_DIR .. "src/devices/bus/lpci/i82371ab.h",
		MAME_DIR .. "src/devices/bus/lpci/i82371sb.cpp",
		MAME_DIR .. "src/devices/bus/lpci/i82371sb.h",
		MAME_DIR .. "src/devices/bus/lpci/i82439tx.cpp",
		MAME_DIR .. "src/devices/bus/lpci/i82439tx.h",
		MAME_DIR .. "src/devices/bus/lpci/northbridge.cpp",
		MAME_DIR .. "src/devices/bus/lpci/northbridge.h",
		MAME_DIR .. "src/devices/bus/lpci/southbridge.cpp",
		MAME_DIR .. "src/devices/bus/lpci/southbridge.h",
		MAME_DIR .. "src/devices/bus/lpci/mpc105.cpp",
		MAME_DIR .. "src/devices/bus/lpci/mpc105.h",
		MAME_DIR .. "src/devices/bus/lpci/vt82c505.cpp",
		MAME_DIR .. "src/devices/bus/lpci/vt82c505.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nes/nes_slot.h,BUSES["NES"] = true
---------------------------------------------------

if (BUSES["NES"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nes/nes_slot.cpp",
		MAME_DIR .. "src/devices/bus/nes/nes_slot.h",
		MAME_DIR .. "src/devices/bus/nes/nes_ines.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_pcb.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_unif.inc",
		MAME_DIR .. "src/devices/bus/nes/nes_carts.cpp",
		MAME_DIR .. "src/devices/bus/nes/nes_carts.h",
		MAME_DIR .. "src/devices/bus/nes/2a03pur.cpp",
		MAME_DIR .. "src/devices/bus/nes/2a03pur.h",
		MAME_DIR .. "src/devices/bus/nes/act53.cpp",
		MAME_DIR .. "src/devices/bus/nes/act53.h",
		MAME_DIR .. "src/devices/bus/nes/aladdin.cpp",
		MAME_DIR .. "src/devices/bus/nes/aladdin.h",
		MAME_DIR .. "src/devices/bus/nes/ave.cpp",
		MAME_DIR .. "src/devices/bus/nes/ave.h",
		MAME_DIR .. "src/devices/bus/nes/bandai.cpp",
		MAME_DIR .. "src/devices/bus/nes/bandai.h",
		MAME_DIR .. "src/devices/bus/nes/benshieng.cpp",
		MAME_DIR .. "src/devices/bus/nes/benshieng.h",
		MAME_DIR .. "src/devices/bus/nes/bootleg.cpp",
		MAME_DIR .. "src/devices/bus/nes/bootleg.h",
		MAME_DIR .. "src/devices/bus/nes/camerica.cpp",
		MAME_DIR .. "src/devices/bus/nes/camerica.h",
		MAME_DIR .. "src/devices/bus/nes/cne.cpp",
		MAME_DIR .. "src/devices/bus/nes/cne.h",
		MAME_DIR .. "src/devices/bus/nes/cony.cpp",
		MAME_DIR .. "src/devices/bus/nes/cony.h",
		MAME_DIR .. "src/devices/bus/nes/datach.cpp",
		MAME_DIR .. "src/devices/bus/nes/datach.h",
		MAME_DIR .. "src/devices/bus/nes/discrete.cpp",
		MAME_DIR .. "src/devices/bus/nes/discrete.h",
		MAME_DIR .. "src/devices/bus/nes/disksys.cpp",
		MAME_DIR .. "src/devices/bus/nes/disksys.h",
		MAME_DIR .. "src/devices/bus/nes/event.cpp",
		MAME_DIR .. "src/devices/bus/nes/event.h",
		MAME_DIR .. "src/devices/bus/nes/ggenie.cpp",
		MAME_DIR .. "src/devices/bus/nes/ggenie.h",
		MAME_DIR .. "src/devices/bus/nes/henggedianzi.cpp",
		MAME_DIR .. "src/devices/bus/nes/henggedianzi.h",
		MAME_DIR .. "src/devices/bus/nes/hes.cpp",
		MAME_DIR .. "src/devices/bus/nes/hes.h",
		MAME_DIR .. "src/devices/bus/nes/hosenkan.cpp",
		MAME_DIR .. "src/devices/bus/nes/hosenkan.h",
		MAME_DIR .. "src/devices/bus/nes/irem.cpp",
		MAME_DIR .. "src/devices/bus/nes/irem.h",
		MAME_DIR .. "src/devices/bus/nes/jaleco.cpp",
		MAME_DIR .. "src/devices/bus/nes/jaleco.h",
		MAME_DIR .. "src/devices/bus/nes/jy.cpp",
		MAME_DIR .. "src/devices/bus/nes/jy.h",
		MAME_DIR .. "src/devices/bus/nes/kaiser.cpp",
		MAME_DIR .. "src/devices/bus/nes/kaiser.h",
		MAME_DIR .. "src/devices/bus/nes/karastudio.cpp",
		MAME_DIR .. "src/devices/bus/nes/karastudio.h",
		MAME_DIR .. "src/devices/bus/nes/konami.cpp",
		MAME_DIR .. "src/devices/bus/nes/konami.h",
		MAME_DIR .. "src/devices/bus/nes/legacy.cpp",
		MAME_DIR .. "src/devices/bus/nes/legacy.h",
		MAME_DIR .. "src/devices/bus/nes/mmc1.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc1.h",
		MAME_DIR .. "src/devices/bus/nes/mmc2.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc2.h",
		MAME_DIR .. "src/devices/bus/nes/mmc3.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc3.h",
		MAME_DIR .. "src/devices/bus/nes/mmc3_clones.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc3_clones.h",
		MAME_DIR .. "src/devices/bus/nes/mmc5.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc5.h",
		MAME_DIR .. "src/devices/bus/nes/multigame.cpp",
		MAME_DIR .. "src/devices/bus/nes/multigame.h",
		MAME_DIR .. "src/devices/bus/nes/namcot.cpp",
		MAME_DIR .. "src/devices/bus/nes/namcot.h",
		MAME_DIR .. "src/devices/bus/nes/nanjing.cpp",
		MAME_DIR .. "src/devices/bus/nes/nanjing.h",
		MAME_DIR .. "src/devices/bus/nes/ntdec.cpp",
		MAME_DIR .. "src/devices/bus/nes/ntdec.h",
		MAME_DIR .. "src/devices/bus/nes/nxrom.cpp",
		MAME_DIR .. "src/devices/bus/nes/nxrom.h",
		MAME_DIR .. "src/devices/bus/nes/pirate.cpp",
		MAME_DIR .. "src/devices/bus/nes/pirate.h",
		MAME_DIR .. "src/devices/bus/nes/pt554.cpp",
		MAME_DIR .. "src/devices/bus/nes/pt554.h",
		MAME_DIR .. "src/devices/bus/nes/racermate.cpp",
		MAME_DIR .. "src/devices/bus/nes/racermate.h",
		MAME_DIR .. "src/devices/bus/nes/rcm.cpp",
		MAME_DIR .. "src/devices/bus/nes/rcm.h",
		MAME_DIR .. "src/devices/bus/nes/rexsoft.cpp",
		MAME_DIR .. "src/devices/bus/nes/rexsoft.h",
		MAME_DIR .. "src/devices/bus/nes/sachen.cpp",
		MAME_DIR .. "src/devices/bus/nes/sachen.h",
		MAME_DIR .. "src/devices/bus/nes/somari.cpp",
		MAME_DIR .. "src/devices/bus/nes/somari.h",
		MAME_DIR .. "src/devices/bus/nes/sunsoft.cpp",
		MAME_DIR .. "src/devices/bus/nes/sunsoft.h",
		MAME_DIR .. "src/devices/bus/nes/sunsoft_dcs.cpp",
		MAME_DIR .. "src/devices/bus/nes/sunsoft_dcs.h",
		MAME_DIR .. "src/devices/bus/nes/taito.cpp",
		MAME_DIR .. "src/devices/bus/nes/taito.h",
		MAME_DIR .. "src/devices/bus/nes/tengen.cpp",
		MAME_DIR .. "src/devices/bus/nes/tengen.h",
		MAME_DIR .. "src/devices/bus/nes/txc.cpp",
		MAME_DIR .. "src/devices/bus/nes/txc.h",
		MAME_DIR .. "src/devices/bus/nes/waixing.cpp",
		MAME_DIR .. "src/devices/bus/nes/waixing.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nes_ctrl/ctrl.h,BUSES["NES_CTRL"] = true
---------------------------------------------------

if (BUSES["NES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nes_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/joypad.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/4score.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/4score.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/arkpaddle.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/arkpaddle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ftrainer.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/ftrainer.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamihs.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamihs.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/miracle.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/miracle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/mjpanel.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/mjpanel.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/pachinko.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/pachinko.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/partytap.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/partytap.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/powerpad.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/powerpad.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/snes/snes_slot.h,BUSES["SNES"] = true
---------------------------------------------------

if (BUSES["SNES"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/snes/snes_slot.cpp",
		MAME_DIR .. "src/devices/bus/snes/snes_slot.h",
		MAME_DIR .. "src/devices/bus/snes/snes_carts.cpp",
		MAME_DIR .. "src/devices/bus/snes/snes_carts.h",
		MAME_DIR .. "src/devices/bus/snes/bsx.cpp",
		MAME_DIR .. "src/devices/bus/snes/bsx.h",
		MAME_DIR .. "src/devices/bus/snes/event.cpp",
		MAME_DIR .. "src/devices/bus/snes/event.h",
		MAME_DIR .. "src/devices/bus/snes/rom.cpp",
		MAME_DIR .. "src/devices/bus/snes/rom.h",
		MAME_DIR .. "src/devices/bus/snes/rom21.cpp",
		MAME_DIR .. "src/devices/bus/snes/rom21.h",
		MAME_DIR .. "src/devices/bus/snes/sa1.cpp",
		MAME_DIR .. "src/devices/bus/snes/sa1.h",
		MAME_DIR .. "src/devices/bus/snes/sdd1.cpp",
		MAME_DIR .. "src/devices/bus/snes/sdd1.h",
		MAME_DIR .. "src/devices/bus/snes/sfx.cpp",
		MAME_DIR .. "src/devices/bus/snes/sfx.h",
		MAME_DIR .. "src/devices/bus/snes/sgb.cpp",
		MAME_DIR .. "src/devices/bus/snes/sgb.h",
		MAME_DIR .. "src/devices/bus/snes/spc7110.cpp",
		MAME_DIR .. "src/devices/bus/snes/spc7110.h",
		MAME_DIR .. "src/devices/bus/snes/sufami.cpp",
		MAME_DIR .. "src/devices/bus/snes/sufami.h",
		MAME_DIR .. "src/devices/bus/snes/upd.cpp",
		MAME_DIR .. "src/devices/bus/snes/upd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/snes_ctrl/ctrl.h,BUSES["SNES_CTRL"] = true
---------------------------------------------------

if (BUSES["SNES_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/snes_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/bcbattle.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/bcbattle.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/joypad.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/miracle.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/miracle.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/multitap.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/pachinko.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/pachinko.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/sscope.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/sscope.h",
		MAME_DIR .. "src/devices/bus/snes_ctrl/twintap.cpp",
		MAME_DIR .. "src/devices/bus/snes_ctrl/twintap.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vboy/slot.h,BUSES["VBOY"] = true
---------------------------------------------------
if (BUSES["VBOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vboy/slot.cpp",
		MAME_DIR .. "src/devices/bus/vboy/slot.h",
		MAME_DIR .. "src/devices/bus/vboy/rom.cpp",
		MAME_DIR .. "src/devices/bus/vboy/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/megadrive/md_slot.h,BUSES["MEGADRIVE"] = true
---------------------------------------------------

if (BUSES["MEGADRIVE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/megadrive/md_slot.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/md_slot.h",
		MAME_DIR .. "src/devices/bus/megadrive/md_carts.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/md_carts.h",
		MAME_DIR .. "src/devices/bus/megadrive/eeprom.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/eeprom.h",
		MAME_DIR .. "src/devices/bus/megadrive/ggenie.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/ggenie.h",
		MAME_DIR .. "src/devices/bus/megadrive/jcart.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/jcart.h",
		MAME_DIR .. "src/devices/bus/megadrive/rom.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/rom.h",
		MAME_DIR .. "src/devices/bus/megadrive/sk.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/sk.h",
		MAME_DIR .. "src/devices/bus/megadrive/stm95.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/stm95.h",
		MAME_DIR .. "src/devices/bus/megadrive/svp.cpp",
		MAME_DIR .. "src/devices/bus/megadrive/svp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/neogeo/neogeo_slot.h,BUSES["NEOGEO"] = true
---------------------------------------------------

if (BUSES["NEOGEO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_slot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_slot.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_intf.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_intf.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_carts.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_carts.h",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_helper.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/neogeo_helper.h",
		MAME_DIR .. "src/devices/bus/neogeo/banked_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/banked_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/sma_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/sma_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/sma_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/sma_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/cmc_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof2002_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pvc_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof98_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_hybrid_cart.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/bootleg_hybrid_cart.h",
		MAME_DIR .. "src/devices/bus/neogeo/sbp_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/sbp_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/kog_prot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kog_prot.h",
		MAME_DIR .. "src/devices/bus/neogeo/rom.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/saturn/sat_slot.h,BUSES["SATURN"] = true
---------------------------------------------------

if (BUSES["SATURN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/saturn/sat_slot.cpp",
		MAME_DIR .. "src/devices/bus/saturn/sat_slot.h",
		MAME_DIR .. "src/devices/bus/saturn/bram.cpp",
		MAME_DIR .. "src/devices/bus/saturn/bram.h",
		MAME_DIR .. "src/devices/bus/saturn/dram.cpp",
		MAME_DIR .. "src/devices/bus/saturn/dram.h",
		MAME_DIR .. "src/devices/bus/saturn/rom.cpp",
		MAME_DIR .. "src/devices/bus/saturn/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sega8/sega8_slot.h,BUSES["SEGA8"] = true
---------------------------------------------------

if (BUSES["SEGA8"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sega8/sega8_slot.cpp",
		MAME_DIR .. "src/devices/bus/sega8/sega8_slot.h",
		MAME_DIR .. "src/devices/bus/sega8/rom.cpp",
		MAME_DIR .. "src/devices/bus/sega8/rom.h",
		MAME_DIR .. "src/devices/bus/sega8/ccatch.cpp",
		MAME_DIR .. "src/devices/bus/sega8/ccatch.h",
		MAME_DIR .. "src/devices/bus/sega8/mgear.cpp",
		MAME_DIR .. "src/devices/bus/sega8/mgear.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sms_ctrl/smsctrl.h,BUSES["SMS_CTRL"] = true
---------------------------------------------------

if (BUSES["SMS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sms_exp/smsexp.h,BUSES["SMS_EXP"] = true
---------------------------------------------------

if (BUSES["SMS_EXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sms_exp/smsexp.cpp",
		MAME_DIR .. "src/devices/bus/sms_exp/smsexp.h",
		MAME_DIR .. "src/devices/bus/sms_exp/gender.cpp",
		MAME_DIR .. "src/devices/bus/sms_exp/gender.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99x/990_dk.h,BUSES["TI99X"] = true
---------------------------------------------------

if (BUSES["TI99X"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.h",
		MAME_DIR .. "src/devices/bus/ti99x/998board.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/998board.h",
		MAME_DIR .. "src/devices/bus/ti99x/datamux.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/datamux.h",
		MAME_DIR .. "src/devices/bus/ti99x/genboard.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/genboard.h",
		MAME_DIR .. "src/devices/bus/ti99x/grom.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/grom.h",
		MAME_DIR .. "src/devices/bus/ti99x/gromport.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/gromport.h",
		MAME_DIR .. "src/devices/bus/ti99x/handset.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/handset.h",
		MAME_DIR .. "src/devices/bus/ti99x/joyport.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/joyport.h",
		MAME_DIR .. "src/devices/bus/ti99x/mecmouse.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/mecmouse.h",
		MAME_DIR .. "src/devices/bus/ti99x/ti99defs.h",
		MAME_DIR .. "src/devices/bus/ti99x/videowrp.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/videowrp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99_peb/peribox.h,BUSES["TI99PEB"] = true
---------------------------------------------------

if (BUSES["TI99PEB"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99_peb/peribox.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/peribox.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/bwg.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/bwg.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/evpc.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/evpc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/hfdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/hfdc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/horizon.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/horizon.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/hsgpl.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/hsgpl.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/memex.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/memex.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/myarcmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/myarcmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/pcode.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/pcode.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/samsmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/samsmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/spchsyn.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/spchsyn.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_32kmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_32kmem.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_fdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_fdc.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_rs232.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/ti_rs232.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_ide.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_ide.h",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_usbsm.cpp",
		MAME_DIR .. "src/devices/bus/ti99_peb/tn_usbsm.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gameboy/gb_slot.h,BUSES["GAMEBOY"] = true
---------------------------------------------------

if (BUSES["GAMEBOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gameboy/gb_slot.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/gb_slot.h",
		MAME_DIR .. "src/devices/bus/gameboy/rom.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/rom.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gamegear/ggext.h,BUSES["GAMEGEAR"] = true
---------------------------------------------------

if (BUSES["GAMEGEAR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gamegear/ggext.cpp",
		MAME_DIR .. "src/devices/bus/gamegear/ggext.h",
		MAME_DIR .. "src/devices/bus/gamegear/smsctrladp.cpp",
		MAME_DIR .. "src/devices/bus/gamegear/smsctrladp.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gba/gba_slot.h,BUSES["GBA"] = true
---------------------------------------------------

if (BUSES["GBA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gba/gba_slot.cpp",
		MAME_DIR .. "src/devices/bus/gba/gba_slot.h",
		MAME_DIR .. "src/devices/bus/gba/rom.cpp",
		MAME_DIR .. "src/devices/bus/gba/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/bml3/bml3bus.h,BUSES["BML3"] = true
---------------------------------------------------
if (BUSES["BML3"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bml3/bml3bus.cpp",
		MAME_DIR .. "src/devices/bus/bml3/bml3bus.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1802.cpp",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1802.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1805.cpp",
		MAME_DIR .. "src/devices/bus/bml3/bml3mp1805.h",
		MAME_DIR .. "src/devices/bus/bml3/bml3kanji.cpp",
		MAME_DIR .. "src/devices/bus/bml3/bml3kanji.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/coco/cococart.h,BUSES["COCO"] = true
---------------------------------------------------
if (BUSES["COCO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coco/cococart.cpp",
		MAME_DIR .. "src/devices/bus/coco/cococart.h",
		MAME_DIR .. "src/devices/bus/coco/coco_232.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_232.h",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.h",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.h",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.h",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.h",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cpc/cpcexp.h,BUSES["CPC"] = true
---------------------------------------------------
if (BUSES["CPC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cpc/cpcexp.cpp",
		MAME_DIR .. "src/devices/bus/cpc/cpcexp.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_ssa1.cpp",
		MAME_DIR .. "src/devices/bus/cpc/cpc_ssa1.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rom.cpp",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rom.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_pds.cpp",
		MAME_DIR .. "src/devices/bus/cpc/cpc_pds.h",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rs232.cpp",
		MAME_DIR .. "src/devices/bus/cpc/cpc_rs232.h",
		MAME_DIR .. "src/devices/bus/cpc/mface2.cpp",
		MAME_DIR .. "src/devices/bus/cpc/mface2.h",
		MAME_DIR .. "src/devices/bus/cpc/symbfac2.cpp",
		MAME_DIR .. "src/devices/bus/cpc/symbfac2.h",
		MAME_DIR .. "src/devices/bus/cpc/amdrum.cpp",
		MAME_DIR .. "src/devices/bus/cpc/amdrum.h",
		MAME_DIR .. "src/devices/bus/cpc/playcity.cpp",
		MAME_DIR .. "src/devices/bus/cpc/playcity.h",
		MAME_DIR .. "src/devices/bus/cpc/smartwatch.cpp",
		MAME_DIR .. "src/devices/bus/cpc/smartwatch.h",
		MAME_DIR .. "src/devices/bus/cpc/brunword4.cpp",
		MAME_DIR .. "src/devices/bus/cpc/brunword4.h",
		MAME_DIR .. "src/devices/bus/cpc/hd20.cpp",
		MAME_DIR .. "src/devices/bus/cpc/hd20.h",
		MAME_DIR .. "src/devices/bus/cpc/ddi1.cpp",
		MAME_DIR .. "src/devices/bus/cpc/ddi1.h",
		MAME_DIR .. "src/devices/bus/cpc/magicsound.cpp",
		MAME_DIR .. "src/devices/bus/cpc/magicsound.h",
		MAME_DIR .. "src/devices/bus/cpc/doubler.cpp",
		MAME_DIR .. "src/devices/bus/cpc/doubler.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/epson_sio/epson_sio.h,BUSES["EPSON_SIO"] = true
---------------------------------------------------
if (BUSES["EPSON_SIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/epson_sio/epson_sio.cpp",
		MAME_DIR .. "src/devices/bus/epson_sio/epson_sio.h",
		MAME_DIR .. "src/devices/bus/epson_sio/pf10.cpp",
		MAME_DIR .. "src/devices/bus/epson_sio/pf10.h",
		MAME_DIR .. "src/devices/bus/epson_sio/tf20.cpp",
		MAME_DIR .. "src/devices/bus/epson_sio/tf20.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pce/pce_slot.h,BUSES["PCE"] = true
---------------------------------------------------
if (BUSES["PCE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pce/pce_slot.cpp",
		MAME_DIR .. "src/devices/bus/pce/pce_slot.h",
		MAME_DIR .. "src/devices/bus/pce/pce_rom.cpp",
		MAME_DIR .. "src/devices/bus/pce/pce_rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/scv/slot.h,BUSES["SCV"] = true
---------------------------------------------------
if (BUSES["SCV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/scv/slot.cpp",
		MAME_DIR .. "src/devices/bus/scv/slot.h",
		MAME_DIR .. "src/devices/bus/scv/rom.cpp",
		MAME_DIR .. "src/devices/bus/scv/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/x68k/x68kexp.h,BUSES["X68K"] = true
---------------------------------------------------
if (BUSES["X68K"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/x68k/x68kexp.cpp",
		MAME_DIR .. "src/devices/bus/x68k/x68kexp.h",
		MAME_DIR .. "src/devices/bus/x68k/x68k_neptunex.cpp",
		MAME_DIR .. "src/devices/bus/x68k/x68k_neptunex.h",
		MAME_DIR .. "src/devices/bus/x68k/x68k_scsiext.cpp",
		MAME_DIR .. "src/devices/bus/x68k/x68k_scsiext.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/abckb/abckb.h,BUSES["ABCKB"] = true
---------------------------------------------------
if (BUSES["ABCKB"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/abckb/abckb.cpp",
		MAME_DIR .. "src/devices/bus/abckb/abckb.h",
		MAME_DIR .. "src/devices/bus/abckb/abc77.cpp",
		MAME_DIR .. "src/devices/bus/abckb/abc77.h",
		MAME_DIR .. "src/devices/bus/abckb/abc99.cpp",
		MAME_DIR .. "src/devices/bus/abckb/abc99.h",
		MAME_DIR .. "src/devices/bus/abckb/abc800kb.cpp",
		MAME_DIR .. "src/devices/bus/abckb/abc800kb.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/compucolor/compclr_flp.h,BUSES["COMPUCOLOR"] = true
---------------------------------------------------
if (BUSES["COMPUCOLOR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/compucolor/floppy.cpp",
		MAME_DIR .. "src/devices/bus/compucolor/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/scsi/scsi.h,BUSES["SCSI"] = true
---------------------------------------------------
if (BUSES["SCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/scsi/scsi.cpp",
		MAME_DIR .. "src/devices/bus/scsi/scsi.h",
		MAME_DIR .. "src/devices/bus/scsi/scsicd.cpp",
		MAME_DIR .. "src/devices/bus/scsi/scsicd.h",
		MAME_DIR .. "src/devices/bus/scsi/scsihd.cpp",
		MAME_DIR .. "src/devices/bus/scsi/scsihd.h",
		MAME_DIR .. "src/devices/bus/scsi/scsihle.cpp",
		MAME_DIR .. "src/devices/bus/scsi/scsihle.h",
		MAME_DIR .. "src/devices/bus/scsi/cdu76s.cpp",
		MAME_DIR .. "src/devices/bus/scsi/cdu76s.h",
		MAME_DIR .. "src/devices/bus/scsi/acb4070.cpp",
		MAME_DIR .. "src/devices/bus/scsi/acb4070.h",
		MAME_DIR .. "src/devices/bus/scsi/d9060hd.cpp",
		MAME_DIR .. "src/devices/bus/scsi/d9060hd.h",
		MAME_DIR .. "src/devices/bus/scsi/sa1403d.cpp",
		MAME_DIR .. "src/devices/bus/scsi/sa1403d.h",
		MAME_DIR .. "src/devices/bus/scsi/s1410.cpp",
		MAME_DIR .. "src/devices/bus/scsi/s1410.h",
		MAME_DIR .. "src/devices/bus/scsi/pc9801_sasi.cpp",
		MAME_DIR .. "src/devices/bus/scsi/pc9801_sasi.h",
		MAME_DIR .. "src/devices/bus/scsi/omti5100.cpp",
		MAME_DIR .. "src/devices/bus/scsi/omti5100.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/macpds/macpds.h,BUSES["MACPDS"] = true
---------------------------------------------------
if (BUSES["MACPDS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/macpds/macpds.cpp",
		MAME_DIR .. "src/devices/bus/macpds/macpds.h",
		MAME_DIR .. "src/devices/bus/macpds/pds_tpdfpd.cpp",
		MAME_DIR .. "src/devices/bus/macpds/pds_tpdfpd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/oricext/oricext.h,BUSES["ORICEXT"] = true
---------------------------------------------------
if (BUSES["ORICEXT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/oricext/oricext.cpp",
		MAME_DIR .. "src/devices/bus/oricext/oricext.h",
		MAME_DIR .. "src/devices/bus/oricext/jasmin.cpp",
		MAME_DIR .. "src/devices/bus/oricext/jasmin.h",
		MAME_DIR .. "src/devices/bus/oricext/microdisc.cpp",
		MAME_DIR .. "src/devices/bus/oricext/microdisc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/a1bus/a1bus.h,BUSES["A1BUS"] = true
---------------------------------------------------

if (BUSES["A1BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a1bus/a1bus.cpp",
		MAME_DIR .. "src/devices/bus/a1bus/a1bus.h",
		MAME_DIR .. "src/devices/bus/a1bus/a1cassette.cpp",
		MAME_DIR .. "src/devices/bus/a1bus/a1cassette.h",
		MAME_DIR .. "src/devices/bus/a1bus/a1cffa.cpp",
		MAME_DIR .. "src/devices/bus/a1bus/a1cffa.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/amiga/zorro/zorro.h,BUSES["ZORRO"] = true
---------------------------------------------------

if (BUSES["ZORRO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/amiga/zorro/zorro.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/zorro.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/cards.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/cards.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2052.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2052.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a590.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a590.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/action_replay.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/action_replay.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ql/exp.h,BUSES["QL"] = true
---------------------------------------------------

if (BUSES["QL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ql/exp.cpp",
		MAME_DIR .. "src/devices/bus/ql/exp.h",
		MAME_DIR .. "src/devices/bus/ql/cst_qdisc.cpp",
		MAME_DIR .. "src/devices/bus/ql/cst_qdisc.h",
		MAME_DIR .. "src/devices/bus/ql/cst_q_plus4.cpp",
		MAME_DIR .. "src/devices/bus/ql/cst_q_plus4.h",
		MAME_DIR .. "src/devices/bus/ql/cumana_fdi.cpp",
		MAME_DIR .. "src/devices/bus/ql/cumana_fdi.h",
		MAME_DIR .. "src/devices/bus/ql/kempston_di.cpp",
		MAME_DIR .. "src/devices/bus/ql/kempston_di.h",
		MAME_DIR .. "src/devices/bus/ql/miracle_gold_card.cpp",
		MAME_DIR .. "src/devices/bus/ql/miracle_gold_card.h",
		MAME_DIR .. "src/devices/bus/ql/mp_fdi.cpp",
		MAME_DIR .. "src/devices/bus/ql/mp_fdi.h",
		MAME_DIR .. "src/devices/bus/ql/opd_basic_master.cpp",
		MAME_DIR .. "src/devices/bus/ql/opd_basic_master.h",
		MAME_DIR .. "src/devices/bus/ql/pcml_qdisk.cpp",
		MAME_DIR .. "src/devices/bus/ql/pcml_qdisk.h",
		MAME_DIR .. "src/devices/bus/ql/qubide.cpp",
		MAME_DIR .. "src/devices/bus/ql/qubide.h",
		MAME_DIR .. "src/devices/bus/ql/sandy_superdisk.cpp",
		MAME_DIR .. "src/devices/bus/ql/sandy_superdisk.h",
		MAME_DIR .. "src/devices/bus/ql/sandy_superqboard.cpp",
		MAME_DIR .. "src/devices/bus/ql/sandy_superqboard.h",
		MAME_DIR .. "src/devices/bus/ql/trumpcard.cpp",
		MAME_DIR .. "src/devices/bus/ql/trumpcard.h",
		MAME_DIR .. "src/devices/bus/ql/rom.cpp",
		MAME_DIR .. "src/devices/bus/ql/rom.h",
		MAME_DIR .. "src/devices/bus/ql/miracle_hd.cpp",
		MAME_DIR .. "src/devices/bus/ql/miracle_hd.h",
		MAME_DIR .. "src/devices/bus/ql/std.cpp",
		MAME_DIR .. "src/devices/bus/ql/std.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vtech/memexp/memexp.h,BUSES["VTECH_MEMEXP"] = true
---------------------------------------------------

if (BUSES["VTECH_MEMEXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vtech/memexp/memexp.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memexp.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/carts.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/carts.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/floppy.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/floppy.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memory.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/memory.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/rs232.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/rs232.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/wordpro.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/wordpro.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vtech/ioexp/ioexp.h,BUSES["VTECH_IOEXP"] = true
---------------------------------------------------

if (BUSES["VTECH_IOEXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vtech/ioexp/ioexp.cpp",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/ioexp.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/carts.cpp",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/carts.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/joystick.cpp",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/joystick.h",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/printer.cpp",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/printer.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/wswan/slot.h,BUSES["WSWAN"] = true
---------------------------------------------------

if (BUSES["WSWAN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/wswan/slot.cpp",
		MAME_DIR .. "src/devices/bus/wswan/slot.h",
		MAME_DIR .. "src/devices/bus/wswan/rom.cpp",
		MAME_DIR .. "src/devices/bus/wswan/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/psx/ctlrport.h,BUSES["PSX_CONTROLLER"] = true
---------------------------------------------------

if (BUSES["PSX_CONTROLLER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psx/ctlrport.cpp",
		MAME_DIR .. "src/devices/bus/psx/ctlrport.h",
		MAME_DIR .. "src/devices/bus/psx/analogue.cpp",
		MAME_DIR .. "src/devices/bus/psx/analogue.h",
		MAME_DIR .. "src/devices/bus/psx/multitap.cpp",
		MAME_DIR .. "src/devices/bus/psx/multitap.h",
		MAME_DIR .. "src/devices/bus/psx/memcard.cpp",
		MAME_DIR .. "src/devices/bus/psx/memcard.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nasbus/nasbus.h,BUSES["NASBUS"] = true
---------------------------------------------------

if (BUSES["NASBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nasbus/nasbus.cpp",
		MAME_DIR .. "src/devices/bus/nasbus/nasbus.h",
		MAME_DIR .. "src/devices/bus/nasbus/cards.cpp",
		MAME_DIR .. "src/devices/bus/nasbus/cards.h",
		MAME_DIR .. "src/devices/bus/nasbus/avc.cpp",
		MAME_DIR .. "src/devices/bus/nasbus/avc.h",
		MAME_DIR .. "src/devices/bus/nasbus/floppy.cpp",
		MAME_DIR .. "src/devices/bus/nasbus/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cgenie/expansion.h,BUSES["CGENIE_EXPANSION"] = true
---------------------------------------------------

if (BUSES["CGENIE_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cgenie/expansion/expansion.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/expansion.h",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/carts.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/carts.h",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/floppy.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/expansion/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cgenie/parallel.h,BUSES["CGENIE_PARALLEL"] = true
---------------------------------------------------

if (BUSES["CGENIE_PARALLEL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cgenie/parallel/parallel.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/parallel.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/carts.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/carts.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/joystick.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/joystick.h",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/printer.cpp",
		MAME_DIR .. "src/devices/bus/cgenie/parallel/printer.h",
	}
end
