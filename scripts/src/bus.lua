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
		MAME_DIR .. "src/devices/bus/a800/atari810.cpp",
		MAME_DIR .. "src/devices/bus/a800/atari810.h",
		MAME_DIR .. "src/devices/bus/a800/atari1050.cpp",
		MAME_DIR .. "src/devices/bus/a800/atari1050.h",
		MAME_DIR .. "src/devices/bus/a800/cassette.cpp",
		MAME_DIR .. "src/devices/bus/a800/cassette.h",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.cpp",
		MAME_DIR .. "src/devices/bus/a800/a800_slot.h",
		MAME_DIR .. "src/devices/bus/a800/a800_carts.cpp",
		MAME_DIR .. "src/devices/bus/a800/a800_carts.h",
		MAME_DIR .. "src/devices/bus/a800/rom.cpp",
		MAME_DIR .. "src/devices/bus/a800/rom.h",
		MAME_DIR .. "src/devices/bus/a800/a5200_supercart.cpp",
		MAME_DIR .. "src/devices/bus/a800/a5200_supercart.h",
		MAME_DIR .. "src/devices/bus/a800/atrax.cpp",
		MAME_DIR .. "src/devices/bus/a800/atrax.h",
		MAME_DIR .. "src/devices/bus/a800/bbsb.cpp",
		MAME_DIR .. "src/devices/bus/a800/bbsb.h",
		MAME_DIR .. "src/devices/bus/a800/corina.cpp",
		MAME_DIR .. "src/devices/bus/a800/corina.h",
		MAME_DIR .. "src/devices/bus/a800/maxflash.cpp",
		MAME_DIR .. "src/devices/bus/a800/maxflash.h",
		MAME_DIR .. "src/devices/bus/a800/oss.cpp",
		MAME_DIR .. "src/devices/bus/a800/oss.h",
		MAME_DIR .. "src/devices/bus/a800/phoenix.cpp",
		MAME_DIR .. "src/devices/bus/a800/phoenix.h",
		MAME_DIR .. "src/devices/bus/a800/rtime8.cpp",
		MAME_DIR .. "src/devices/bus/a800/rtime8.h",
		MAME_DIR .. "src/devices/bus/a800/sic.cpp",
		MAME_DIR .. "src/devices/bus/a800/sic.h",
		MAME_DIR .. "src/devices/bus/a800/sparta.cpp",
		MAME_DIR .. "src/devices/bus/a800/sparta.h",
		MAME_DIR .. "src/devices/bus/a800/supercharger.cpp",
		MAME_DIR .. "src/devices/bus/a800/supercharger.h",
		MAME_DIR .. "src/devices/bus/a800/telelink2.cpp",
		MAME_DIR .. "src/devices/bus/a800/telelink2.h",
		MAME_DIR .. "src/devices/bus/a800/ultracart.cpp",
		MAME_DIR .. "src/devices/bus/a800/ultracart.h",
		MAME_DIR .. "src/devices/bus/a800/williams.cpp",
		MAME_DIR .. "src/devices/bus/a800/williams.h",
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
		MAME_DIR .. "src/devices/bus/abcbus/cadmouse.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/cadmouse.h",
		MAME_DIR .. "src/devices/bus/abcbus/db4106.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/db4106.h",
		MAME_DIR .. "src/devices/bus/abcbus/db4107.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/db4107.h",
		MAME_DIR .. "src/devices/bus/abcbus/db4112.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/db4112.h",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/fd2.h",
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
		MAME_DIR .. "src/devices/bus/abcbus/ssa.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/ssa.h",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/uni800.h",
		MAME_DIR .. "src/devices/bus/abcbus/unidisk.cpp",
		MAME_DIR .. "src/devices/bus/abcbus/unidisk.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/acorn/bus.h,BUSES["ACORN"] = true
---------------------------------------------------

if (BUSES["ACORN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/acorn/bus.cpp",
		MAME_DIR .. "src/devices/bus/acorn/bus.h",
		MAME_DIR .. "src/devices/bus/acorn/atom/discpack.cpp",
		MAME_DIR .. "src/devices/bus/acorn/atom/discpack.h",
		MAME_DIR .. "src/devices/bus/acorn/atom/econet.cpp",
		MAME_DIR .. "src/devices/bus/acorn/atom/econet.h",
		MAME_DIR .. "src/devices/bus/acorn/atom/sid.cpp",
		MAME_DIR .. "src/devices/bus/acorn/atom/sid.h",
		MAME_DIR .. "src/devices/bus/acorn/cms/4080term.cpp",
		MAME_DIR .. "src/devices/bus/acorn/cms/4080term.h",
		MAME_DIR .. "src/devices/bus/acorn/cms/fdc.cpp",
		MAME_DIR .. "src/devices/bus/acorn/cms/fdc.h",
		MAME_DIR .. "src/devices/bus/acorn/cms/hires.cpp",
		MAME_DIR .. "src/devices/bus/acorn/cms/hires.h",
		MAME_DIR .. "src/devices/bus/acorn/cms/ieee.cpp",
		MAME_DIR .. "src/devices/bus/acorn/cms/ieee.h",
		MAME_DIR .. "src/devices/bus/acorn/system/32k.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/32k.h",
		MAME_DIR .. "src/devices/bus/acorn/system/8k.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/8k.h",
		MAME_DIR .. "src/devices/bus/acorn/system/cass.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/cass.h",
		MAME_DIR .. "src/devices/bus/acorn/system/econet.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/econet.h",
		MAME_DIR .. "src/devices/bus/acorn/system/fdc.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/fdc.h",
		MAME_DIR .. "src/devices/bus/acorn/system/vdu40.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/vdu40.h",
		MAME_DIR .. "src/devices/bus/acorn/system/vdu80.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/vdu80.h",
		MAME_DIR .. "src/devices/bus/acorn/system/vib.cpp",
		MAME_DIR .. "src/devices/bus/acorn/system/vib.h",
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
--@src/devices/bus/adb/adb.h,BUSES["ADB"] = true
---------------------------------------------------

if (BUSES["ADB"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/adb/adb.cpp",
		MAME_DIR .. "src/devices/bus/adb/adb.h",
		MAME_DIR .. "src/devices/bus/adb/adbhle.cpp",
		MAME_DIR .. "src/devices/bus/adb/adbhle.h",
		MAME_DIR .. "src/devices/bus/adb/a9m0330.cpp",
		MAME_DIR .. "src/devices/bus/adb/a9m0330.h",
		MAME_DIR .. "src/devices/bus/adb/a9m0331.cpp",
		MAME_DIR .. "src/devices/bus/adb/a9m0331.h",
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
--@src/devices/bus/apricot/expansion/expansion.h,BUSES["APRICOT_EXPANSION"] = true
---------------------------------------------------

if (BUSES["APRICOT_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apricot/expansion/expansion.cpp",
		MAME_DIR .. "src/devices/bus/apricot/expansion/expansion.h",
		MAME_DIR .. "src/devices/bus/apricot/expansion/cards.cpp",
		MAME_DIR .. "src/devices/bus/apricot/expansion/cards.h",
		MAME_DIR .. "src/devices/bus/apricot/expansion/ram.cpp",
		MAME_DIR .. "src/devices/bus/apricot/expansion/ram.h",
		MAME_DIR .. "src/devices/bus/apricot/expansion/winchester.cpp",
		MAME_DIR .. "src/devices/bus/apricot/expansion/winchester.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apricot/keyboard/keyboard.h,BUSES["APRICOT_KEYBOARD"] = true
---------------------------------------------------

if (BUSES["APRICOT_KEYBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apricot/keyboard/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/apricot/keyboard/keyboard.h",
		MAME_DIR .. "src/devices/bus/apricot/keyboard/hle.cpp",
		MAME_DIR .. "src/devices/bus/apricot/keyboard/hle.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/apricot/video/video.h,BUSES["APRICOT_VIDEO"] = true
---------------------------------------------------

if (BUSES["APRICOT_VIDEO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/apricot/video/video.cpp",
		MAME_DIR .. "src/devices/bus/apricot/video/video.h",
		MAME_DIR .. "src/devices/bus/apricot/video/cards.cpp",
		MAME_DIR .. "src/devices/bus/apricot/video/cards.h",
		MAME_DIR .. "src/devices/bus/apricot/video/mono.cpp",
		MAME_DIR .. "src/devices/bus/apricot/video/mono.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/aquarius/slot.h,BUSES["AQUARIUS"] = true
---------------------------------------------------

if (BUSES["AQUARIUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/aquarius/slot.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/slot.h",
		MAME_DIR .. "src/devices/bus/aquarius/c1541.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/c1541.h",
		MAME_DIR .. "src/devices/bus/aquarius/mini.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/mini.h",
		MAME_DIR .. "src/devices/bus/aquarius/qdisk.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/qdisk.h",
		MAME_DIR .. "src/devices/bus/aquarius/ram.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/ram.h",
		MAME_DIR .. "src/devices/bus/aquarius/rom.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/rom.h",
		MAME_DIR .. "src/devices/bus/aquarius/supercart.cpp",
		MAME_DIR .. "src/devices/bus/aquarius/supercart.h",
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
--@src/devices/bus/archimedes/econet/slot.h,BUSES["ARCHIMEDES_ECONET"] = true
---------------------------------------------------

if (BUSES["ARCHIMEDES_ECONET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/archimedes/econet/slot.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/econet/slot.h",
		MAME_DIR .. "src/devices/bus/archimedes/econet/econet.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/econet/econet.h",
		MAME_DIR .. "src/devices/bus/archimedes/econet/midi.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/econet/midi.h",
		MAME_DIR .. "src/devices/bus/archimedes/econet/rtfmjoy.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/econet/rtfmjoy.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/archimedes/podule/slot.h,BUSES["ARCHIMEDES_PODULE"] = true
---------------------------------------------------

if (BUSES["ARCHIMEDES_PODULE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/archimedes/podule/slot.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/slot.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/a448.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/a448.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/acejoy.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/acejoy.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/armadeus.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/armadeus.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/eaglem2.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/eaglem2.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether1.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether1.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether2.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether2.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether3.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ether3.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ethera.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ethera.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/etherd.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/etherd.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/etherr.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/etherr.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/faxpack.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/faxpack.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/greyhawk.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/greyhawk.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_cw.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_cw.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_morley.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_morley.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_we.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/hdisc_we.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ide_be.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ide_be.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ide_rdev.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/ide_rdev.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_hccs.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_hccs.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_morley.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_morley.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_we.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/io_we.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/lark.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/lark.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/laserd.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/laserd.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/midi_emr.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/midi_emr.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/midimax.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/midimax.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/nexus.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/nexus.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/rom.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/rom.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/rs423.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/rs423.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scan256.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scan256.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scanlight.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scanlight.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_a500.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_a500.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_acorn.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_acorn.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_cumana.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_cumana.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_ling.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_ling.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_morley.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_morley.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_oak.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_oak.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_vti.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/scsi_vti.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/serial.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/serial.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/spectra.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/spectra.h",
		MAME_DIR .. "src/devices/bus/archimedes/podule/tube.cpp",
		MAME_DIR .. "src/devices/bus/archimedes/podule/tube.h",
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
		MAME_DIR .. "src/devices/bus/astrocde/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/ctrl.h",
		MAME_DIR .. "src/devices/bus/astrocde/joy.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/joy.h",
		MAME_DIR .. "src/devices/bus/astrocde/cassette.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/cassette.h",
		MAME_DIR .. "src/devices/bus/astrocde/accessory.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/accessory.h",
		MAME_DIR .. "src/devices/bus/astrocde/lightpen.cpp",
		MAME_DIR .. "src/devices/bus/astrocde/lightpen.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/ata/atadev.h,BUSES["ATA"] = true
--@src/devices/bus/ata/ataintf.h,BUSES["ATA"] = true
---------------------------------------------------

if (BUSES["ATA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ata/atadev.cpp",
		MAME_DIR .. "src/devices/bus/ata/atadev.h",
		MAME_DIR .. "src/devices/bus/ata/ataintf.cpp",
		MAME_DIR .. "src/devices/bus/ata/ataintf.h",
		MAME_DIR .. "src/devices/bus/ata/atapicdr.cpp",
		MAME_DIR .. "src/devices/bus/ata/atapicdr.h",
		MAME_DIR .. "src/devices/bus/ata/atapihle.cpp",
		MAME_DIR .. "src/devices/bus/ata/atapihle.h",
		MAME_DIR .. "src/devices/bus/ata/cp2024.cpp",
		MAME_DIR .. "src/devices/bus/ata/cp2024.h",
		MAME_DIR .. "src/devices/bus/ata/cr589.cpp",
		MAME_DIR .. "src/devices/bus/ata/cr589.h",
		MAME_DIR .. "src/devices/bus/ata/gdrom.cpp",
		MAME_DIR .. "src/devices/bus/ata/gdrom.h",
		MAME_DIR .. "src/devices/bus/ata/hdd.cpp",
		MAME_DIR .. "src/devices/bus/ata/hdd.h",
		MAME_DIR .. "src/devices/bus/ata/px320a.cpp",
		MAME_DIR .. "src/devices/bus/ata/px320a.h",
		MAME_DIR .. "src/devices/bus/ata/xm3301.cpp",
		MAME_DIR .. "src/devices/bus/ata/xm3301.h",
		MAME_DIR .. "src/devices/bus/ata/zip100.cpp",
		MAME_DIR .. "src/devices/bus/ata/zip100.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/fdc/fdc.h,BUSES["BBC_FDC"] = true
---------------------------------------------------

if (BUSES["BBC_FDC"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/fdc/fdc.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/fdc.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/acorn.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/acorn.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/ams.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/ams.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/cumana.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/cumana.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/cv1797.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/cv1797.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/kenda.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/kenda.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/opus.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/opus.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/solidisk.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/solidisk.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/udm.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/udm.h",
		MAME_DIR .. "src/devices/bus/bbc/fdc/watford.cpp",
		MAME_DIR .. "src/devices/bus/bbc/fdc/watford.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/analogue/analogue.h,BUSES["BBC_ANALOGUE"] = true
---------------------------------------------------

if (BUSES["BBC_ANALOGUE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/analogue/analogue.cpp",
		MAME_DIR .. "src/devices/bus/bbc/analogue/analogue.h",
		MAME_DIR .. "src/devices/bus/bbc/analogue/bitstik.cpp",
		MAME_DIR .. "src/devices/bus/bbc/analogue/bitstik.h",
		MAME_DIR .. "src/devices/bus/bbc/analogue/joystick.cpp",
		MAME_DIR .. "src/devices/bus/bbc/analogue/joystick.h",
		MAME_DIR .. "src/devices/bus/bbc/analogue/cfa3000a.cpp",
		MAME_DIR .. "src/devices/bus/bbc/analogue/cfa3000a.h",
		MAME_DIR .. "src/devices/bus/bbc/analogue/quinkey.cpp",
		MAME_DIR .. "src/devices/bus/bbc/analogue/quinkey.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/cart/slot.h,BUSES["BBC_CART"] = true
---------------------------------------------------

if (BUSES["BBC_CART"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/cart/slot.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/slot.h",
		MAME_DIR .. "src/devices/bus/bbc/cart/click.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/click.h",
		MAME_DIR .. "src/devices/bus/bbc/cart/mastersd.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/mastersd.h",
		MAME_DIR .. "src/devices/bus/bbc/cart/mega256.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/mega256.h",
		MAME_DIR .. "src/devices/bus/bbc/cart/mr8000.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/mr8000.h",
		MAME_DIR .. "src/devices/bus/bbc/cart/msc.cpp",
		MAME_DIR .. "src/devices/bus/bbc/cart/msc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/internal/internal.h,BUSES["BBC_INTERNAL"] = true
---------------------------------------------------

if (BUSES["BBC_INTERNAL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/internal/internal.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/internal.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/aries.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/aries.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/atpl.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/atpl.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/cumana68k.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/cumana68k.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/integrab.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/integrab.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/memex.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/memex.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/morleyaa.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/morleyaa.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/overlay.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/overlay.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/peartree.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/peartree.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/ramamp.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/ramamp.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/raven20.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/raven20.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/romex.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/romex.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/stlswr.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/stlswr.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/stl2m128.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/stl2m128.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/stl4m32.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/stl4m32.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/werom.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/werom.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/weromram.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/weromram.h",
		MAME_DIR .. "src/devices/bus/bbc/internal/we32kram.cpp",
		MAME_DIR .. "src/devices/bus/bbc/internal/we32kram.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/exp/exp.h,BUSES["BBC_EXP"] = true
---------------------------------------------------

if (BUSES["BBC_EXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/exp/exp.cpp",
		MAME_DIR .. "src/devices/bus/bbc/exp/exp.h",
		MAME_DIR .. "src/devices/bus/bbc/exp/autocue.cpp",
		MAME_DIR .. "src/devices/bus/bbc/exp/autocue.h",
		MAME_DIR .. "src/devices/bus/bbc/exp/mertec.cpp",
		MAME_DIR .. "src/devices/bus/bbc/exp/mertec.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/joyport/joyport.h,BUSES["BBC_JOYPORT"] = true
---------------------------------------------------

if (BUSES["BBC_JOYPORT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/joyport/joyport.cpp",
		MAME_DIR .. "src/devices/bus/bbc/joyport/joyport.h",
		MAME_DIR .. "src/devices/bus/bbc/joyport/joystick.cpp",
		MAME_DIR .. "src/devices/bus/bbc/joyport/joystick.h",
		MAME_DIR .. "src/devices/bus/bbc/joyport/mouse.cpp",
		MAME_DIR .. "src/devices/bus/bbc/joyport/mouse.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/modem/modem.h,BUSES["BBC_MODEM"] = true
---------------------------------------------------

if (BUSES["BBC_MODEM"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/modem/modem.cpp",
		MAME_DIR .. "src/devices/bus/bbc/modem/modem.h",
		MAME_DIR .. "src/devices/bus/bbc/modem/meup.cpp",
		MAME_DIR .. "src/devices/bus/bbc/modem/meup.h",
		MAME_DIR .. "src/devices/bus/bbc/modem/scsiaiv.cpp",
		MAME_DIR .. "src/devices/bus/bbc/modem/scsiaiv.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/1mhzbus/1mhzbus.h,BUSES["BBC_1MHZBUS"] = true
---------------------------------------------------

if (BUSES["BBC_1MHZBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/1mhzbus.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/1mhzbus.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/24bbc.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/24bbc.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/2ndserial.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/2ndserial.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/autoprom.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/autoprom.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/barrybox.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/barrybox.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/beebopl.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/beebopl.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/beebsid.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/beebsid.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cc500.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cc500.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cisco.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cisco.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/datacentre.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/datacentre.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/emrmidi.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/emrmidi.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ide.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ide.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ieee488.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ieee488.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/m2000.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/m2000.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/m5000.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/m5000.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/multiform.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/multiform.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/opus3.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/opus3.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/pdram.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/pdram.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/pms64k.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/pms64k.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ramdisc.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/ramdisc.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/sasi.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/sasi.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/scsi.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/scsi.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/sprite.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/sprite.h",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cfa3000opt.cpp",
		MAME_DIR .. "src/devices/bus/bbc/1mhzbus/cfa3000opt.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/rom/slot.h,BUSES["BBC_ROM"] = true
---------------------------------------------------

if (BUSES["BBC_ROM"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/rom/slot.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/slot.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/rom.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/rom.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/ram.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/ram.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/nvram.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/nvram.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/datagem.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/datagem.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/dfs.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/dfs.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/genie.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/genie.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/pal.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/pal.h",
		MAME_DIR .. "src/devices/bus/bbc/rom/rtc.cpp",
		MAME_DIR .. "src/devices/bus/bbc/rom/rtc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/tube/tube.h,BUSES["BBC_TUBE"] = true
---------------------------------------------------

if (BUSES["BBC_TUBE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/tube/tube.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_32016.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_32016.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_6502.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_6502.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_80186.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_80186.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_80286.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_80286.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_a500.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_a500.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_arm.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_arm.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_arm7.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_arm7.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_casper.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_casper.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_cms6809.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_cms6809.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_matchbox.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_matchbox.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_rc6502.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_rc6502.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_x25.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_x25.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_z80.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_z80.h",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_zep100.cpp",
		MAME_DIR .. "src/devices/bus/bbc/tube/tube_zep100.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/bbc/userport/userport.h,BUSES["BBC_USERPORT"] = true
---------------------------------------------------

if (BUSES["BBC_USERPORT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/bbc/userport/userport.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/userport.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/beebspch.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/beebspch.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/lcd.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/lcd.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/lvlecho.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/lvlecho.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/m4000.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/m4000.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/palext.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/palext.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/pointer.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/pointer.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/sdcard.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/sdcard.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/usersplit.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/usersplit.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/voicebox.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/voicebox.h",
		MAME_DIR .. "src/devices/bus/bbc/userport/cfa3000kbd.cpp",
		MAME_DIR .. "src/devices/bus/bbc/userport/cfa3000kbd.h",
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
		MAME_DIR .. "src/devices/bus/c64/buscard.cpp",
		MAME_DIR .. "src/devices/bus/c64/buscard.h",
		MAME_DIR .. "src/devices/bus/c64/buscard2.cpp",
		MAME_DIR .. "src/devices/bus/c64/buscard2.h",
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
		MAME_DIR .. "src/devices/bus/c64/speakeasy.cpp",
		MAME_DIR .. "src/devices/bus/c64/speakeasy.h",
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
		MAME_DIR .. "src/devices/bus/c64/tibdd001.cpp",
		MAME_DIR .. "src/devices/bus/c64/tibdd001.h",
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
		MAME_DIR .. "src/devices/bus/c64/z80videopak.cpp",
		MAME_DIR .. "src/devices/bus/c64/z80videopak.h",
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
--@src/devices/bus/casloopy/slot.h,BUSES["CASLOOPY"] = true
---------------------------------------------------

if (BUSES["CASLOOPY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/casloopy/slot.cpp",
		MAME_DIR .. "src/devices/bus/casloopy/slot.h",
		MAME_DIR .. "src/devices/bus/casloopy/rom.cpp",
		MAME_DIR .. "src/devices/bus/casloopy/rom.h",
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
		MAME_DIR .. "src/devices/bus/cbmiec/mps1200.cpp",
		MAME_DIR .. "src/devices/bus/cbmiec/mps1200.h",
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
--@src/devices/bus/coleco/controller/ctrl.h,BUSES["COLECO_CONTROLLER"] = true
---------------------------------------------------

if (BUSES["COLECO_CONTROLLER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coleco/controller/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/coleco/controller/ctrl.h",
		MAME_DIR .. "src/devices/bus/coleco/controller/hand.cpp",
		MAME_DIR .. "src/devices/bus/coleco/controller/hand.h",
		MAME_DIR .. "src/devices/bus/coleco/controller/sac.cpp",
		MAME_DIR .. "src/devices/bus/coleco/controller/sac.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/coleco/cartridge/exp.h,BUSES["COLECO_CART"] = true
---------------------------------------------------

if (BUSES["COLECO_CART"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coleco/cartridge/exp.cpp",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/exp.h",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/activision.cpp",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/activision.h",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/megacart.cpp",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/megacart.h",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/std.cpp",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/std.h",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/xin1.h",
		MAME_DIR .. "src/devices/bus/coleco/cartridge/xin1.cpp",
	}
end


---------------------------------------------------
--
--@src/devices/bus/coleco/expansion/expansion.h,BUSES["COLECO_EXPANSION"] = true
---------------------------------------------------

if (BUSES["COLECO_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coleco/expansion/expansion.cpp",
		MAME_DIR .. "src/devices/bus/coleco/expansion/expansion.h",
		MAME_DIR .. "src/devices/bus/coleco/expansion/cards.cpp",
		MAME_DIR .. "src/devices/bus/coleco/expansion/cards.h",
		MAME_DIR .. "src/devices/bus/coleco/expansion/sgm.cpp",
		MAME_DIR .. "src/devices/bus/coleco/expansion/sgm.h",
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
--@src/devices/bus/dmv/dmvbus.h,BUSES["DMV"] = true
---------------------------------------------------

if (BUSES["DMV"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.cpp",
		MAME_DIR .. "src/devices/bus/dmv/dmvbus.h",
		MAME_DIR .. "src/devices/bus/dmv/k012.cpp",
		MAME_DIR .. "src/devices/bus/dmv/k012.h",
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
--@src/devices/bus/ekara/slot.h,BUSES["EKARA"] = true
---------------------------------------------------

if (BUSES["EKARA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ekara/slot.cpp",
		MAME_DIR .. "src/devices/bus/ekara/slot.h",
		MAME_DIR .. "src/devices/bus/ekara/rom.cpp",
		MAME_DIR .. "src/devices/bus/ekara/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/electron/exp.h,BUSES["ELECTRON"] = true
---------------------------------------------------

if (BUSES["ELECTRON"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/electron/exp.cpp",
		MAME_DIR .. "src/devices/bus/electron/exp.h",
		MAME_DIR .. "src/devices/bus/electron/elksd64.cpp",
		MAME_DIR .. "src/devices/bus/electron/elksd64.h",
		MAME_DIR .. "src/devices/bus/electron/elksd128.cpp",
		MAME_DIR .. "src/devices/bus/electron/elksd128.h",
		MAME_DIR .. "src/devices/bus/electron/fbjoy.cpp",
		MAME_DIR .. "src/devices/bus/electron/fbjoy.h",
		MAME_DIR .. "src/devices/bus/electron/fbprint.cpp",
		MAME_DIR .. "src/devices/bus/electron/fbprint.h",
		MAME_DIR .. "src/devices/bus/electron/mc68k.cpp",
		MAME_DIR .. "src/devices/bus/electron/mc68k.h",
		MAME_DIR .. "src/devices/bus/electron/mode7.cpp",
		MAME_DIR .. "src/devices/bus/electron/mode7.h",
		MAME_DIR .. "src/devices/bus/electron/plus1.cpp",
		MAME_DIR .. "src/devices/bus/electron/plus1.h",
		MAME_DIR .. "src/devices/bus/electron/plus2.cpp",
		MAME_DIR .. "src/devices/bus/electron/plus2.h",
		MAME_DIR .. "src/devices/bus/electron/plus3.cpp",
		MAME_DIR .. "src/devices/bus/electron/plus3.h",
		MAME_DIR .. "src/devices/bus/electron/pwrjoy.cpp",
		MAME_DIR .. "src/devices/bus/electron/pwrjoy.h",
		MAME_DIR .. "src/devices/bus/electron/rombox.cpp",
		MAME_DIR .. "src/devices/bus/electron/rombox.h",
		MAME_DIR .. "src/devices/bus/electron/romboxp.cpp",
		MAME_DIR .. "src/devices/bus/electron/romboxp.h",
		MAME_DIR .. "src/devices/bus/electron/sidewndr.cpp",
		MAME_DIR .. "src/devices/bus/electron/sidewndr.h",
		MAME_DIR .. "src/devices/bus/electron/voxbox.cpp",
		MAME_DIR .. "src/devices/bus/electron/voxbox.h",
		MAME_DIR .. "src/devices/bus/electron/m2105.cpp",
		MAME_DIR .. "src/devices/bus/electron/m2105.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/electron/cart/slot.h,BUSES["ELECTRON_CART"] = true
---------------------------------------------------

if (BUSES["ELECTRON_CART"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/electron/cart/slot.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/slot.h",
		MAME_DIR .. "src/devices/bus/electron/cart/abr.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/abr.h",
		MAME_DIR .. "src/devices/bus/electron/cart/ap34.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/ap34.h",
		MAME_DIR .. "src/devices/bus/electron/cart/ap5.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/ap5.h",
		MAME_DIR .. "src/devices/bus/electron/cart/aqr.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/aqr.h",
		MAME_DIR .. "src/devices/bus/electron/cart/click.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/click.h",
		MAME_DIR .. "src/devices/bus/electron/cart/cumana.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/cumana.h",
		MAME_DIR .. "src/devices/bus/electron/cart/elksdp1.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/elksdp1.h",
		MAME_DIR .. "src/devices/bus/electron/cart/mgc.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/mgc.h",
		MAME_DIR .. "src/devices/bus/electron/cart/peg400.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/peg400.h",
		MAME_DIR .. "src/devices/bus/electron/cart/romp144.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/romp144.h",
		MAME_DIR .. "src/devices/bus/electron/cart/rs423.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/rs423.h",
		MAME_DIR .. "src/devices/bus/electron/cart/sndexp.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/sndexp.h",
		MAME_DIR .. "src/devices/bus/electron/cart/sndexp3.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/sndexp3.h",
		MAME_DIR .. "src/devices/bus/electron/cart/sp64.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/sp64.h",
		MAME_DIR .. "src/devices/bus/electron/cart/std.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/std.h",
		MAME_DIR .. "src/devices/bus/electron/cart/stlefs.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/stlefs.h",
		MAME_DIR .. "src/devices/bus/electron/cart/tube.cpp",
		MAME_DIR .. "src/devices/bus/electron/cart/tube.h",
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
--@src/devices/bus/fmt_scsi/fmt_scsi.h,BUSES["FMT_SCSI"] = true
---------------------------------------------------

if (BUSES["FMT_SCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/fmt_scsi/fmt_scsi.cpp",
		MAME_DIR .. "src/devices/bus/fmt_scsi/fmt_scsi.h",
		MAME_DIR .. "src/devices/bus/fmt_scsi/fmt121.cpp",
		MAME_DIR .. "src/devices/bus/fmt_scsi/fmt121.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/fp1000/fp1000_exp.h,BUSES["FP1000"] = true
---------------------------------------------------

if (BUSES["FP1000"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/fp1000/fp1000_exp.cpp",
		MAME_DIR .. "src/devices/bus/fp1000/fp1000_exp.h",
		MAME_DIR .. "src/devices/bus/fp1000/fp1020fd.cpp",
		MAME_DIR .. "src/devices/bus/fp1000/fp1020fd.h",
		MAME_DIR .. "src/devices/bus/fp1000/fp1030_rampack.cpp",
		MAME_DIR .. "src/devices/bus/fp1000/fp1030_rampack.h",
		MAME_DIR .. "src/devices/bus/fp1000/fp1060io.cpp",
		MAME_DIR .. "src/devices/bus/fp1000/fp1060io.h",
		MAME_DIR .. "src/devices/bus/fp1000/fp1060io_exp.cpp",
		MAME_DIR .. "src/devices/bus/fp1000/fp1060io_exp.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/gamate/slot.h,BUSES["GAMATE"] = true
---------------------------------------------------

if (BUSES["GAMATE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gamate/slot.cpp",
		MAME_DIR .. "src/devices/bus/gamate/slot.h",
		MAME_DIR .. "src/devices/bus/gamate/rom.cpp",
		MAME_DIR .. "src/devices/bus/gamate/rom.h",
		MAME_DIR .. "src/devices/bus/gamate/gamate_protection.cpp",
		MAME_DIR .. "src/devices/bus/gamate/gamate_protection.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/gio64/gio64.h,BUSES["GIO64"] = true
---------------------------------------------------

if (BUSES["GIO64"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gio64/gio64.cpp",
		MAME_DIR .. "src/devices/bus/gio64/gio64.h",
		MAME_DIR .. "src/devices/bus/gio64/newport.cpp",
		MAME_DIR .. "src/devices/bus/gio64/newport.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/hp_hil/hp_hil.h,BUSES["HPHIL"] = true
---------------------------------------------------

if (BUSES["HPHIL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hp_hil/hp_hil.cpp",
		MAME_DIR .. "src/devices/bus/hp_hil/hp_hil.h",
		MAME_DIR .. "src/devices/bus/hp_hil/hil_devices.cpp",
		MAME_DIR .. "src/devices/bus/hp_hil/hil_devices.h",
		MAME_DIR .. "src/devices/bus/hp_hil/hlebase.cpp",
		MAME_DIR .. "src/devices/bus/hp_hil/hlebase.h",
		MAME_DIR .. "src/devices/bus/hp_hil/hlemouse.cpp",
		MAME_DIR .. "src/devices/bus/hp_hil/hlemouse.h",
		MAME_DIR .. "src/devices/bus/hp_hil/hlekbd.cpp",
		MAME_DIR .. "src/devices/bus/hp_hil/hlekbd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/hp_dio/hp_dio.h,BUSES["HPDIO"] = true
---------------------------------------------------

if (BUSES["HPDIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hp_dio/hp_dio.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp_dio.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98265a.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98265a.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98543.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98543.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98544.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98544.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98550.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98550.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98603a.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98603a.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98603b.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98603b.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98620.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98620.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98624.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98624.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98628_9.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98628_9.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98643.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98643.h",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98644.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/hp98644.h",
		MAME_DIR .. "src/devices/bus/hp_dio/human_interface.cpp",
		MAME_DIR .. "src/devices/bus/hp_dio/human_interface.h",
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
--@src/devices/bus/hexbus/hexbus.h,BUSES["HEXBUS"] = true
---------------------------------------------------

if (BUSES["HEXBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hexbus/hexbus.cpp",
		MAME_DIR .. "src/devices/bus/hexbus/hexbus.h",
		MAME_DIR .. "src/devices/bus/hexbus/hx5102.cpp",
		MAME_DIR .. "src/devices/bus/hexbus/hx5102.h",
		MAME_DIR .. "src/devices/bus/hexbus/tp0370.cpp",
		MAME_DIR .. "src/devices/bus/hexbus/tp0370.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/idpartner/bus.h,BUSES["IDPARTNER"] = true
---------------------------------------------------

if (BUSES["IDPARTNER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/idpartner/bus.cpp",
		MAME_DIR .. "src/devices/bus/idpartner/bus.h",
		MAME_DIR .. "src/devices/bus/idpartner/gdp.cpp",
		MAME_DIR .. "src/devices/bus/idpartner/gdp.h",
		MAME_DIR .. "src/devices/bus/idpartner/sasi.cpp",
		MAME_DIR .. "src/devices/bus/idpartner/sasi.h",
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
		MAME_DIR .. "src/devices/bus/ieee488/hp9122c.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/hp9122c.h",
		MAME_DIR .. "src/devices/bus/ieee488/hp9133.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/hp9133.h",
		MAME_DIR .. "src/devices/bus/ieee488/hp9895.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/hp9895.h",
		MAME_DIR .. "src/devices/bus/ieee488/remote488.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/remote488.h",
		MAME_DIR .. "src/devices/bus/ieee488/grid2102.cpp",
		MAME_DIR .. "src/devices/bus/ieee488/grid2102.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/ieee488/hp9122c.cpp", GEN_DIR .. "emu/layout/hp9122c.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "hp9122c"),
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
--@src/devices/bus/imi7000/imi7000.h,BUSES["IMI7000"] = true
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
--@src/devices/bus/intellec4/intellec4.h,BUSES["INTELLEC4"] = true
---------------------------------------------------

if (BUSES["INTELLEC4"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/intellec4/insdatastor.cpp",
		MAME_DIR .. "src/devices/bus/intellec4/insdatastor.h",
		MAME_DIR .. "src/devices/bus/intellec4/intellec4.cpp",
		MAME_DIR .. "src/devices/bus/intellec4/intellec4.h",
		MAME_DIR .. "src/devices/bus/intellec4/prommemory.cpp",
		MAME_DIR .. "src/devices/bus/intellec4/prommemory.h",
		MAME_DIR .. "src/devices/bus/intellec4/tapereader.cpp",
		MAME_DIR .. "src/devices/bus/intellec4/tapereader.h",
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
--@src/devices/bus/intv_ctrl/ctrl.h,BUSES["INTV_CTRL"] = true
---------------------------------------------------

if (BUSES["INTV_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/intv_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/intv_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/intv_ctrl/handctrl.cpp",
		MAME_DIR .. "src/devices/bus/intv_ctrl/handctrl.h",
		MAME_DIR .. "src/devices/bus/intv_ctrl/ecs_ctrl.cpp",
		MAME_DIR .. "src/devices/bus/intv_ctrl/ecs_ctrl.h",
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
		MAME_DIR .. "src/devices/bus/isa/3c503.cpp",
		MAME_DIR .. "src/devices/bus/isa/3c503.h",
		MAME_DIR .. "src/devices/bus/isa/3c505.cpp",
		MAME_DIR .. "src/devices/bus/isa/3c505.h",
		MAME_DIR .. "src/devices/bus/isa/3xtwin.cpp",
		MAME_DIR .. "src/devices/bus/isa/3xtwin.h",
		MAME_DIR .. "src/devices/bus/isa/acb2072.cpp",
		MAME_DIR .. "src/devices/bus/isa/acb2072.h",
		MAME_DIR .. "src/devices/bus/isa/adlib.cpp",
		MAME_DIR .. "src/devices/bus/isa/adlib.h",
		MAME_DIR .. "src/devices/bus/isa/aga.cpp",
		MAME_DIR .. "src/devices/bus/isa/aga.h",
		MAME_DIR .. "src/devices/bus/isa/aha1542b.cpp",
		MAME_DIR .. "src/devices/bus/isa/aha1542b.h",
		MAME_DIR .. "src/devices/bus/isa/aha1542c.cpp",
		MAME_DIR .. "src/devices/bus/isa/aha1542c.h",
		MAME_DIR .. "src/devices/bus/isa/aha174x.cpp",
		MAME_DIR .. "src/devices/bus/isa/aha174x.h",
		MAME_DIR .. "src/devices/bus/isa/asc88.cpp",
		MAME_DIR .. "src/devices/bus/isa/asc88.h",
		MAME_DIR .. "src/devices/bus/isa/bblue2.cpp",
		MAME_DIR .. "src/devices/bus/isa/bblue2.h",
		MAME_DIR .. "src/devices/bus/isa/bt54x.cpp",
		MAME_DIR .. "src/devices/bus/isa/bt54x.h",
		MAME_DIR .. "src/devices/bus/isa/cga.cpp",
		MAME_DIR .. "src/devices/bus/isa/cga.h",
		MAME_DIR .. "src/devices/bus/isa/chessmdr.cpp",
		MAME_DIR .. "src/devices/bus/isa/chessmdr.h",
		MAME_DIR .. "src/devices/bus/isa/chessmsr.cpp",
		MAME_DIR .. "src/devices/bus/isa/chessmsr.h",
		MAME_DIR .. "src/devices/bus/isa/cl_sh260.cpp",
		MAME_DIR .. "src/devices/bus/isa/cl_sh260.h",
		MAME_DIR .. "src/devices/bus/isa/com.cpp",
		MAME_DIR .. "src/devices/bus/isa/com.h",
		MAME_DIR .. "src/devices/bus/isa/dcb.cpp",
		MAME_DIR .. "src/devices/bus/isa/dcb.h",
		MAME_DIR .. "src/devices/bus/isa/dectalk.cpp",
		MAME_DIR .. "src/devices/bus/isa/dectalk.h",
		MAME_DIR .. "src/devices/bus/isa/ega.cpp",
		MAME_DIR .. "src/devices/bus/isa/ega.h",
		MAME_DIR .. "src/devices/bus/isa/eis_hgb107x.cpp",
		MAME_DIR .. "src/devices/bus/isa/eis_hgb107x.h",
		MAME_DIR .. "src/devices/bus/isa/eis_sad8852.cpp",
		MAME_DIR .. "src/devices/bus/isa/eis_sad8852.h",
		MAME_DIR .. "src/devices/bus/isa/eis_twib.cpp",
		MAME_DIR .. "src/devices/bus/isa/eis_twib.h",
		MAME_DIR .. "src/devices/bus/isa/ex1280.cpp",
		MAME_DIR .. "src/devices/bus/isa/ex1280.h",
		MAME_DIR .. "src/devices/bus/isa/fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/fdc.h",
		MAME_DIR .. "src/devices/bus/isa/finalchs.cpp",
		MAME_DIR .. "src/devices/bus/isa/finalchs.h",
		MAME_DIR .. "src/devices/bus/isa/gblaster.cpp",
		MAME_DIR .. "src/devices/bus/isa/gblaster.h",
		MAME_DIR .. "src/devices/bus/isa/gus.cpp",
		MAME_DIR .. "src/devices/bus/isa/gus.h",
		MAME_DIR .. "src/devices/bus/isa/hdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/hdc.h",
		MAME_DIR .. "src/devices/bus/isa/hpblp.cpp",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.cpp",
		MAME_DIR .. "src/devices/bus/isa/ibm_mfc.h",
		MAME_DIR .. "src/devices/bus/isa/ibm_speech.cpp",
		MAME_DIR .. "src/devices/bus/isa/ibm_speech.h",
		MAME_DIR .. "src/devices/bus/isa/ide.cpp",
		MAME_DIR .. "src/devices/bus/isa/ide.h",
		MAME_DIR .. "src/devices/bus/isa/lbaenhancer.cpp",
		MAME_DIR .. "src/devices/bus/isa/lbaenhancer.h",
		MAME_DIR .. "src/devices/bus/isa/lpt.cpp",
		MAME_DIR .. "src/devices/bus/isa/lpt.h",
		MAME_DIR .. "src/devices/bus/isa/lrk330.cpp",
		MAME_DIR .. "src/devices/bus/isa/lrk330.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/mc1502_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.cpp",
		MAME_DIR .. "src/devices/bus/isa/mc1502_rom.h",
		MAME_DIR .. "src/devices/bus/isa/mcd.cpp",
		MAME_DIR .. "src/devices/bus/isa/mcd.h",
		MAME_DIR .. "src/devices/bus/isa/mda.cpp",
		MAME_DIR .. "src/devices/bus/isa/mda.h",
		MAME_DIR .. "src/devices/bus/isa/mpu401.cpp",
		MAME_DIR .. "src/devices/bus/isa/mpu401.h",
		MAME_DIR .. "src/devices/bus/isa/mufdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/mufdc.h",
		MAME_DIR .. "src/devices/bus/isa/myb3k_com.cpp",
		MAME_DIR .. "src/devices/bus/isa/myb3k_com.h",
		MAME_DIR .. "src/devices/bus/isa/myb3k_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/myb3k_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/ne1000.cpp",
		MAME_DIR .. "src/devices/bus/isa/ne1000.h",
		MAME_DIR .. "src/devices/bus/isa/ne2000.cpp",
		MAME_DIR .. "src/devices/bus/isa/ne2000.h",
		MAME_DIR .. "src/devices/bus/isa/np600.cpp",
		MAME_DIR .. "src/devices/bus/isa/np600.h",
		MAME_DIR .. "src/devices/bus/isa/num9rev.cpp",
		MAME_DIR .. "src/devices/bus/isa/num9rev.h",
		MAME_DIR .. "src/devices/bus/isa/omti8621.cpp",
		MAME_DIR .. "src/devices/bus/isa/omti8621.h",
		MAME_DIR .. "src/devices/bus/isa/opus100pm.cpp",
		MAME_DIR .. "src/devices/bus/isa/opus100pm.h",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_fdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_hdc.h",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_rom.h",
		MAME_DIR .. "src/devices/bus/isa/p1_sound.cpp",
		MAME_DIR .. "src/devices/bus/isa/p1_sound.h",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.cpp",
		MAME_DIR .. "src/devices/bus/isa/pc1640_iga.h",
		MAME_DIR .. "src/devices/bus/isa/pcmidi.cpp",
		MAME_DIR .. "src/devices/bus/isa/pcmidi.h",
		MAME_DIR .. "src/devices/bus/isa/pds.cpp",
		MAME_DIR .. "src/devices/bus/isa/pds.h",
		MAME_DIR .. "src/devices/bus/isa/pgc.cpp",
		MAME_DIR .. "src/devices/bus/isa/pgc.h",
		MAME_DIR .. "src/devices/bus/isa/prose4k1.cpp",
		MAME_DIR .. "src/devices/bus/isa/prose4k1.h",
		MAME_DIR .. "src/devices/bus/isa/sb16.cpp",
		MAME_DIR .. "src/devices/bus/isa/sb16.h",
		MAME_DIR .. "src/devices/bus/isa/sblaster.cpp",
		MAME_DIR .. "src/devices/bus/isa/sblaster.h",
		MAME_DIR .. "src/devices/bus/isa/sc499.cpp",
		MAME_DIR .. "src/devices/bus/isa/sc499.h",
		MAME_DIR .. "src/devices/bus/isa/side116.cpp",
		MAME_DIR .. "src/devices/bus/isa/side116.h",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.cpp",
		MAME_DIR .. "src/devices/bus/isa/ssi2001.h",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.cpp",
		MAME_DIR .. "src/devices/bus/isa/stereo_fx.h",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_cirrus.h",
		MAME_DIR .. "src/devices/bus/isa/svga_paradise.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_paradise.h",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_s3.h",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_trident.h",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.cpp",
		MAME_DIR .. "src/devices/bus/isa/svga_tseng.h",
		MAME_DIR .. "src/devices/bus/isa/tekram_dc820.cpp",
		MAME_DIR .. "src/devices/bus/isa/tekram_dc820.h",
		MAME_DIR .. "src/devices/bus/isa/ultra12f.cpp",
		MAME_DIR .. "src/devices/bus/isa/ultra12f.h",
		MAME_DIR .. "src/devices/bus/isa/ultra14f.cpp",
		MAME_DIR .. "src/devices/bus/isa/ultra14f.h",
		MAME_DIR .. "src/devices/bus/isa/ultra24f.cpp",
		MAME_DIR .. "src/devices/bus/isa/ultra24f.h",
		MAME_DIR .. "src/devices/bus/isa/vga.cpp",
		MAME_DIR .. "src/devices/bus/isa/vga.h",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.cpp",
		MAME_DIR .. "src/devices/bus/isa/vga_ati.h",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.cpp",
		MAME_DIR .. "src/devices/bus/isa/wd1002a_wx1.h",
		MAME_DIR .. "src/devices/bus/isa/wd1007a.cpp",
		MAME_DIR .. "src/devices/bus/isa/wd1007a.h",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.cpp",
		MAME_DIR .. "src/devices/bus/isa/wdxt_gen.h",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.cpp",
		MAME_DIR .. "src/devices/bus/isa/xsu_cards.h",
		MAME_DIR .. "src/devices/bus/isa/xtide.cpp",
		MAME_DIR .. "src/devices/bus/isa/xtide.h",
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
--@src/devices/bus/jakks_gamekey/slot.h,BUSES["JAKKS_GAMEKEY"] = true
---------------------------------------------------

if (BUSES["JAKKS_GAMEKEY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/jakks_gamekey/slot.cpp",
		MAME_DIR .. "src/devices/bus/jakks_gamekey/slot.h",
		MAME_DIR .. "src/devices/bus/jakks_gamekey/rom.cpp",
		MAME_DIR .. "src/devices/bus/jakks_gamekey/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/mpf1/slot.h,BUSES["MPF1"] = true
---------------------------------------------------

if (BUSES["MPF1"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mpf1/slot.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/slot.h",
		MAME_DIR .. "src/devices/bus/mpf1/epb.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/epb.h",
		MAME_DIR .. "src/devices/bus/mpf1/iom.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/iom.h",
		MAME_DIR .. "src/devices/bus/mpf1/prt.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/prt.h",
		MAME_DIR .. "src/devices/bus/mpf1/sgb.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/sgb.h",
		MAME_DIR .. "src/devices/bus/mpf1/ssb.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/ssb.h",
		MAME_DIR .. "src/devices/bus/mpf1/tva.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/tva.h",
		MAME_DIR .. "src/devices/bus/mpf1/vid.cpp",
		MAME_DIR .. "src/devices/bus/mpf1/vid.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/msx/ctrl/ctrl.h,BUSES["MSX_CTRL"] = true
---------------------------------------------------

if (BUSES["MSX_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/msx/ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/hypershot.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/hypershot.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/joystick.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/joystick.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/libbler.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/libbler.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/magickey.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/magickey.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/sgadapt.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/sgadapt.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/towns6b.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/towns6b.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/townspad.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/townspad.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/vaus.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/vaus.h",
		MAME_DIR .. "src/devices/bus/msx/ctrl/xe1ap.cpp",
		MAME_DIR .. "src/devices/bus/msx/ctrl/xe1ap.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/msx/slot/slot.h,BUSES["MSX_SLOT"] = true
---------------------------------------------------

if (BUSES["MSX_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/msx/slot/ax230.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/ax230.h",
		MAME_DIR .. "src/devices/bus/msx/slot/bruc100.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/bruc100.h",
		MAME_DIR .. "src/devices/bus/msx/slot/bunsetsu.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/bunsetsu.h",
		MAME_DIR .. "src/devices/bus/msx/slot/cartridge.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx/slot/disk.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/disk.h",
		MAME_DIR .. "src/devices/bus/msx/slot/fs4600.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/fs4600.h",
		MAME_DIR .. "src/devices/bus/msx/slot/fsa1fm.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/fsa1fm.h",
		MAME_DIR .. "src/devices/bus/msx/slot/msx_write.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/msx_write.h",
		MAME_DIR .. "src/devices/bus/msx/slot/music.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/music.h",
		MAME_DIR .. "src/devices/bus/msx/slot/panasonic08.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/panasonic08.h",
		MAME_DIR .. "src/devices/bus/msx/slot/panasonic08r.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/panasonic08r.h",
		MAME_DIR .. "src/devices/bus/msx/slot/rom.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/rom.h",
		MAME_DIR .. "src/devices/bus/msx/slot/ram.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/ram.h",
		MAME_DIR .. "src/devices/bus/msx/slot/ram_mm.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/ram_mm.h",
		MAME_DIR .. "src/devices/bus/msx/slot/msx_rs232.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/msx_rs232.h",
		MAME_DIR .. "src/devices/bus/msx/slot/slot.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/slot.h",
		MAME_DIR .. "src/devices/bus/msx/slot/sony08.cpp",
		MAME_DIR .. "src/devices/bus/msx/slot/sony08.h",
		MAME_DIR .. "src/devices/bus/msx/beecard/beecard.cpp",
		MAME_DIR .. "src/devices/bus/msx/beecard/beecard.h",
		MAME_DIR .. "src/devices/bus/msx/cart/arc.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/arc.h",
		MAME_DIR .. "src/devices/bus/msx/cart/ascii.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/ascii.h",
		MAME_DIR .. "src/devices/bus/msx/cart/beepack.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/beepack.h",
		MAME_DIR .. "src/devices/bus/msx/cart/bm_012.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/bm_012.h",
		MAME_DIR .. "src/devices/bus/msx/cart/cartridge.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/cartridge.h",
		MAME_DIR .. "src/devices/bus/msx/cart/crossblaim.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/crossblaim.h",
		MAME_DIR .. "src/devices/bus/msx/cart/disk.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/disk.h",
		MAME_DIR .. "src/devices/bus/msx/cart/dooly.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/dooly.h",
		MAME_DIR .. "src/devices/bus/msx/cart/easi_speech.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/easi_speech.h",
		MAME_DIR .. "src/devices/bus/msx/cart/fmpac.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/fmpac.h",
		MAME_DIR .. "src/devices/bus/msx/cart/franky.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/franky.h",
		MAME_DIR .. "src/devices/bus/msx/cart/fs_sr021.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/fs_sr021.h",
		MAME_DIR .. "src/devices/bus/msx/cart/fs_sr022.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/fs_sr022.h",
		MAME_DIR .. "src/devices/bus/msx/cart/halnote.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/halnote.h",
		MAME_DIR .. "src/devices/bus/msx/cart/hbi55.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/hbi55.h",
		MAME_DIR .. "src/devices/bus/msx/cart/hfox.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/hfox.h",
		MAME_DIR .. "src/devices/bus/msx/cart/holy_quran.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/holy_quran.h",
		MAME_DIR .. "src/devices/bus/msx/cart/ide.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/ide.h",
		MAME_DIR .. "src/devices/bus/msx/cart/ink.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/ink.h",
		MAME_DIR .. "src/devices/bus/msx/cart/kanji.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/kanji.h",
		MAME_DIR .. "src/devices/bus/msx/cart/konami.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/konami.h",
		MAME_DIR .. "src/devices/bus/msx/cart/korean.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/korean.h",
		MAME_DIR .. "src/devices/bus/msx/cart/loveplus.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/loveplus.h",
		MAME_DIR .. "src/devices/bus/msx/cart/majutsushi.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/majutsushi.h",
		MAME_DIR .. "src/devices/bus/msx/cart/matra.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/matra.h",
		MAME_DIR .. "src/devices/bus/msx/cart/moonsound.h",
		MAME_DIR .. "src/devices/bus/msx/cart/moonsound.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/msx_audio.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/msx_audio.h",
		MAME_DIR .. "src/devices/bus/msx/cart/msx_audio_kb.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/msx_audio_kb.h",
		MAME_DIR .. "src/devices/bus/msx/cart/msxdos2.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/msxdos2.h",
		MAME_DIR .. "src/devices/bus/msx/cart/nomapper.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/nomapper.h",
		MAME_DIR .. "src/devices/bus/msx/cart/quickdisk.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/quickdisk.h",
		MAME_DIR .. "src/devices/bus/msx/cart/ram.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/ram.h",
		MAME_DIR .. "src/devices/bus/msx/cart/rtype.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/rtype.h",
		MAME_DIR .. "src/devices/bus/msx/cart/scsi.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/scsi.h",
		MAME_DIR .. "src/devices/bus/msx/cart/slotexpander.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/slotexpander.h",
		MAME_DIR .. "src/devices/bus/msx/cart/slotoptions.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/slotoptions.h",
		MAME_DIR .. "src/devices/bus/msx/cart/softcard.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/softcard.h",
		MAME_DIR .. "src/devices/bus/msx/cart/superloderunner.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/superloderunner.h",
		MAME_DIR .. "src/devices/bus/msx/cart/super_swangi.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/super_swangi.h",
		MAME_DIR .. "src/devices/bus/msx/cart/yamaha_ucn01.cpp",
		MAME_DIR .. "src/devices/bus/msx/cart/yamaha_ucn01.h",
		MAME_DIR .. "src/devices/bus/msx/minicart/minicart.cpp",
		MAME_DIR .. "src/devices/bus/msx/minicart/minicart.h",
		MAME_DIR .. "src/devices/bus/msx/module/module.cpp",
		MAME_DIR .. "src/devices/bus/msx/module/module.h",
		MAME_DIR .. "src/devices/bus/msx/module/sfg.cpp",
		MAME_DIR .. "src/devices/bus/msx/module/sfg.h",
		MAME_DIR .. "src/devices/bus/msx/module/skw01.cpp",
		MAME_DIR .. "src/devices/bus/msx/module/skw01.h",
		MAME_DIR .. "src/devices/bus/msx/softcard/softcard.cpp",
		MAME_DIR .. "src/devices/bus/msx/softcard/softcard.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/mtx/exp.h,BUSES["MTX"] = true
---------------------------------------------------

if (BUSES["MTX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mtx/exp.cpp",
		MAME_DIR .. "src/devices/bus/mtx/exp.h",
		MAME_DIR .. "src/devices/bus/mtx/cfx.cpp",
		MAME_DIR .. "src/devices/bus/mtx/cfx.h",
		MAME_DIR .. "src/devices/bus/mtx/magrom.cpp",
		MAME_DIR .. "src/devices/bus/mtx/magrom.h",
		MAME_DIR .. "src/devices/bus/mtx/rompak.cpp",
		MAME_DIR .. "src/devices/bus/mtx/rompak.h",
		MAME_DIR .. "src/devices/bus/mtx/sdx.cpp",
		MAME_DIR .. "src/devices/bus/mtx/sdx.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/mc10/mc10_cart.h,BUSES["MC10"] = true
---------------------------------------------------
if (BUSES["MC10"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mc10/mc10_cart.cpp",
		MAME_DIR .. "src/devices/bus/mc10/mc10_cart.h",
		MAME_DIR .. "src/devices/bus/mc10/mcx128.cpp",
		MAME_DIR .. "src/devices/bus/mc10/mcx128.h",
		MAME_DIR .. "src/devices/bus/mc10/multiports_ext.cpp",
		MAME_DIR .. "src/devices/bus/mc10/multiports_ext.h",
		MAME_DIR .. "src/devices/bus/mc10/pak.cpp",
		MAME_DIR .. "src/devices/bus/mc10/pak.h",
		MAME_DIR .. "src/devices/bus/mc10/ram.cpp",
		MAME_DIR .. "src/devices/bus/mc10/ram.h",
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
		MAME_DIR .. "src/devices/bus/odyssey2/4in1.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/4in1.h",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/chess.h",
		MAME_DIR .. "src/devices/bus/odyssey2/homecomp.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/homecomp.h",
		MAME_DIR .. "src/devices/bus/odyssey2/ktaa.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/ktaa.h",
		MAME_DIR .. "src/devices/bus/odyssey2/rally.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/rally.h",
		MAME_DIR .. "src/devices/bus/odyssey2/test.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/test.h",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.cpp",
		MAME_DIR .. "src/devices/bus/odyssey2/voice.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pasopia/pac2.h,BUSES["PASOPIA"] = true
---------------------------------------------------

if (BUSES["PASOPIA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pasopia/pac2.cpp",
		MAME_DIR .. "src/devices/bus/pasopia/pac2.h",
		MAME_DIR .. "src/devices/bus/pasopia/pac2exp.cpp",
		MAME_DIR .. "src/devices/bus/pasopia/pac2exp.h",
		MAME_DIR .. "src/devices/bus/pasopia/rampac2.cpp",
		MAME_DIR .. "src/devices/bus/pasopia/rampac2.h",
		MAME_DIR .. "src/devices/bus/pasopia/rompac2.cpp",
		MAME_DIR .. "src/devices/bus/pasopia/rompac2.h",
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
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_magnum6.cpp",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_magnum6.h",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.cpp",
		MAME_DIR .. "src/devices/bus/pc_joy/pc_joy_sw.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/pccard/pccard.h,BUSES["PCCARD"] = true
---------------------------------------------------

if (BUSES["PCCARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pccard/pccard.cpp",
		MAME_DIR .. "src/devices/bus/pccard/pccard.h",
		MAME_DIR .. "src/devices/bus/pccard/ataflash.cpp",
		MAME_DIR .. "src/devices/bus/pccard/ataflash.h",
		MAME_DIR .. "src/devices/bus/pccard/k573npu.cpp",
		MAME_DIR .. "src/devices/bus/pccard/k573npu.h",
		MAME_DIR .. "src/devices/bus/pccard/konami_dual.cpp",
		MAME_DIR .. "src/devices/bus/pccard/konami_dual.h",
		MAME_DIR .. "src/devices/bus/pccard/linflash.cpp",
		MAME_DIR .. "src/devices/bus/pccard/linflash.h",
		MAME_DIR .. "src/devices/bus/pccard/sram.cpp",
		MAME_DIR .. "src/devices/bus/pccard/sram.h",
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
		MAME_DIR .. "src/devices/bus/pc_kbd/cherry_mx1500.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/cherry_mx1500.h",
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
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat101.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/pcat101.h",
		MAME_DIR .. "src/devices/bus/pc_kbd/hle_mouse.cpp",
		MAME_DIR .. "src/devices/bus/pc_kbd/hle_mouse.h",
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
		MAME_DIR .. "src/devices/bus/pet/2joysnd.h",
		MAME_DIR .. "src/devices/bus/pet/2joysnd.cpp",
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
--@src/devices/bus/pofo/exp.h,BUSES["POFO"] = true
---------------------------------------------------

if (BUSES["POFO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pofo/exp.cpp",
		MAME_DIR .. "src/devices/bus/pofo/exp.h",
		MAME_DIR .. "src/devices/bus/pofo/hpc101.cpp",
		MAME_DIR .. "src/devices/bus/pofo/hpc101.h",
		MAME_DIR .. "src/devices/bus/pofo/hpc102.cpp",
		MAME_DIR .. "src/devices/bus/pofo/hpc102.h",
		MAME_DIR .. "src/devices/bus/pofo/hpc104.cpp",
		MAME_DIR .. "src/devices/bus/pofo/hpc104.h",
		MAME_DIR .. "src/devices/bus/pofo/ccm.cpp",
		MAME_DIR .. "src/devices/bus/pofo/ccm.h",
		MAME_DIR .. "src/devices/bus/pofo/ram.cpp",
		MAME_DIR .. "src/devices/bus/pofo/ram.h",
		MAME_DIR .. "src/devices/bus/pofo/rom.cpp",
		MAME_DIR .. "src/devices/bus/pofo/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/psion/honda/slot.h,BUSES["PSION_HONDA"] = true
---------------------------------------------------

if (BUSES["PSION_HONDA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psion/honda/slot.cpp",
		MAME_DIR .. "src/devices/bus/psion/honda/slot.h",
		MAME_DIR .. "src/devices/bus/psion/honda/parallel.cpp",
		MAME_DIR .. "src/devices/bus/psion/honda/parallel.h",
		MAME_DIR .. "src/devices/bus/psion/honda/pclink.cpp",
		MAME_DIR .. "src/devices/bus/psion/honda/pclink.h",
		MAME_DIR .. "src/devices/bus/psion/honda/ssd.cpp",
		MAME_DIR .. "src/devices/bus/psion/honda/ssd.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/psion/module/slot.h,BUSES["PSION_MODULE"] = true
---------------------------------------------------

if (BUSES["PSION_MODULE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psion/module/slot.cpp",
		MAME_DIR .. "src/devices/bus/psion/module/slot.h",
		MAME_DIR .. "src/devices/bus/psion/module/serpar.cpp",
		MAME_DIR .. "src/devices/bus/psion/module/serpar.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/psion/sibo/slot.h,BUSES["PSION_SIBO"] = true
---------------------------------------------------

if (BUSES["PSION_SIBO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psion/sibo/slot.cpp",
		MAME_DIR .. "src/devices/bus/psion/sibo/slot.h",
		MAME_DIR .. "src/devices/bus/psion/sibo/3fax.cpp",
		MAME_DIR .. "src/devices/bus/psion/sibo/3fax.h",
		MAME_DIR .. "src/devices/bus/psion/sibo/3link.cpp",
		MAME_DIR .. "src/devices/bus/psion/sibo/3link.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/rc2014/rc2014.h,BUSES["RC2014"] = true
---------------------------------------------------

if (BUSES["RC2014"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/rc2014/rc2014.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/rc2014.h",
		MAME_DIR .. "src/devices/bus/rc2014/cf.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/cf.h",
		MAME_DIR .. "src/devices/bus/rc2014/clock.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/clock.h",
		MAME_DIR .. "src/devices/bus/rc2014/edge.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/edge.h",
		MAME_DIR .. "src/devices/bus/rc2014/fdc.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/fdc.h",
		MAME_DIR .. "src/devices/bus/rc2014/ide.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/ide.h",
		MAME_DIR .. "src/devices/bus/rc2014/micro.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/micro.h",
		MAME_DIR .. "src/devices/bus/rc2014/modules.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/modules.h",
		MAME_DIR .. "src/devices/bus/rc2014/ram.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/ram.h",
		MAME_DIR .. "src/devices/bus/rc2014/rom.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/rom.h",
		MAME_DIR .. "src/devices/bus/rc2014/romram.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/romram.h",
		MAME_DIR .. "src/devices/bus/rc2014/rtc.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/rtc.h",
		MAME_DIR .. "src/devices/bus/rc2014/serial.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/serial.h",
		MAME_DIR .. "src/devices/bus/rc2014/sound.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/sound.h",
		MAME_DIR .. "src/devices/bus/rc2014/z180cpu.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/z180cpu.h",
		MAME_DIR .. "src/devices/bus/rc2014/z80cpu.cpp",
		MAME_DIR .. "src/devices/bus/rc2014/z80cpu.h",
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
		MAME_DIR .. "src/devices/bus/s100/am310.cpp",
		MAME_DIR .. "src/devices/bus/s100/am310.h",
		MAME_DIR .. "src/devices/bus/s100/ascsasi.cpp",
		MAME_DIR .. "src/devices/bus/s100/ascsasi.h",
		MAME_DIR .. "src/devices/bus/s100/dg640.cpp",
		MAME_DIR .. "src/devices/bus/s100/dg640.h",
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
		MAME_DIR .. "src/devices/bus/s100/poly16k.cpp",
		MAME_DIR .. "src/devices/bus/s100/poly16k.h",
		MAME_DIR .. "src/devices/bus/s100/polyfdc.cpp",
		MAME_DIR .. "src/devices/bus/s100/polyfdc.h",
		MAME_DIR .. "src/devices/bus/s100/polyvti.cpp",
		MAME_DIR .. "src/devices/bus/s100/polyvti.h",
		MAME_DIR .. "src/devices/bus/s100/seals8k.cpp",
		MAME_DIR .. "src/devices/bus/s100/seals8k.h",
		MAME_DIR .. "src/devices/bus/s100/vectordualmode.cpp",
		MAME_DIR .. "src/devices/bus/s100/vectordualmode.h",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.cpp",
		MAME_DIR .. "src/devices/bus/s100/wunderbus.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/segaai/segaai_exp.h,BUSES["SEGAAI"] = true
---------------------------------------------------

if (BUSES["SEGAAI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/segaai/rom.cpp",
		MAME_DIR .. "src/devices/bus/segaai/rom.h",
		MAME_DIR .. "src/devices/bus/segaai/segaai_exp.cpp",
		MAME_DIR .. "src/devices/bus/segaai/segaai_exp.h",
		MAME_DIR .. "src/devices/bus/segaai/segaai_slot.cpp",
		MAME_DIR .. "src/devices/bus/segaai/segaai_slot.h",
		MAME_DIR .. "src/devices/bus/segaai/soundbox.cpp",
		MAME_DIR .. "src/devices/bus/segaai/soundbox.h",
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
--@src/devices/bus/ss50/interface.h,BUSES["SS50"] = true
---------------------------------------------------

if (BUSES["SS50"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ss50/interface.cpp",
		MAME_DIR .. "src/devices/bus/ss50/interface.h",
		MAME_DIR .. "src/devices/bus/ss50/dc5.cpp",
		MAME_DIR .. "src/devices/bus/ss50/dc5.h",
		MAME_DIR .. "src/devices/bus/ss50/mpc.cpp",
		MAME_DIR .. "src/devices/bus/ss50/mpc.h",
		MAME_DIR .. "src/devices/bus/ss50/mps.cpp",
		MAME_DIR .. "src/devices/bus/ss50/mps.h",
		MAME_DIR .. "src/devices/bus/ss50/mps2.cpp",
		MAME_DIR .. "src/devices/bus/ss50/mps2.h",
		MAME_DIR .. "src/devices/bus/ss50/mpt.cpp",
		MAME_DIR .. "src/devices/bus/ss50/mpt.h",
		MAME_DIR .. "src/devices/bus/ss50/piaide.cpp",
		MAME_DIR .. "src/devices/bus/ss50/piaide.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/supracan/slot.h,BUSES["SUPRACAN"] = true
---------------------------------------------------

if (BUSES["SUPRACAN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/supracan/slot.cpp",
		MAME_DIR .. "src/devices/bus/supracan/slot.h",
		MAME_DIR .. "src/devices/bus/supracan/rom.cpp",
		MAME_DIR .. "src/devices/bus/supracan/rom.h",
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
--@src/devices/bus/tim011/exp.h,BUSES["TIM011"] = true
---------------------------------------------------

if (BUSES["TIM011"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tim011/exp.cpp",
		MAME_DIR .. "src/devices/bus/tim011/exp.h",
		MAME_DIR .. "src/devices/bus/tim011/aycard.cpp",
		MAME_DIR .. "src/devices/bus/tim011/aycard.h",
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
--@src/devices/bus/vcs_ctrl/ctrl.h,BUSES["VCS_CTRL"] = true
---------------------------------------------------

if (BUSES["VCS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/cx85.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/cx85.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joystick.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/joybooster.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/keypad.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/lightpen.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/paddles.h",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/trakball.cpp",
		MAME_DIR .. "src/devices/bus/vcs_ctrl/trakball.h",
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
		MAME_DIR .. "src/devices/bus/vic10/multimax.cpp",
		MAME_DIR .. "src/devices/bus/vic10/multimax.h",
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
		MAME_DIR .. "src/devices/bus/vic20/videopak.cpp",
		MAME_DIR .. "src/devices/bus/vic20/videopak.h",
		MAME_DIR .. "src/devices/bus/vic20/speakeasy.cpp",
		MAME_DIR .. "src/devices/bus/vic20/speakeasy.h",
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
		MAME_DIR .. "src/devices/bus/vidbrain/comp_language.cpp",
		MAME_DIR .. "src/devices/bus/vidbrain/comp_language.h",
		MAME_DIR .. "src/devices/bus/vidbrain/info_manager.cpp",
		MAME_DIR .. "src/devices/bus/vidbrain/info_manager.h",
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
--@src/devices/bus/vme/vme.h,BUSES["VME"] = true
---------------------------------------------------

if (BUSES["VME"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vme/vme.cpp",
		MAME_DIR .. "src/devices/bus/vme/vme.h",
		MAME_DIR .. "src/devices/bus/vme/vme_cards.cpp",
		MAME_DIR .. "src/devices/bus/vme/vme_cards.h",
		MAME_DIR .. "src/devices/bus/vme/cp31.cpp",
		MAME_DIR .. "src/devices/bus/vme/cp31.h",
		MAME_DIR .. "src/devices/bus/vme/enp10.cpp",
		MAME_DIR .. "src/devices/bus/vme/enp10.h",
		MAME_DIR .. "src/devices/bus/vme/hcpu30.cpp",
		MAME_DIR .. "src/devices/bus/vme/hcpu30.h",
		MAME_DIR .. "src/devices/bus/vme/hk68v10.cpp",
		MAME_DIR .. "src/devices/bus/vme/hk68v10.h",
		MAME_DIR .. "src/devices/bus/vme/mvme120.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme120.h",
		MAME_DIR .. "src/devices/bus/vme/mvme147.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme147.h",
		MAME_DIR .. "src/devices/bus/vme/mvme180.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme180.h",
		MAME_DIR .. "src/devices/bus/vme/mvme181.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme181.h",
		MAME_DIR .. "src/devices/bus/vme/mvme187.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme187.h",
		MAME_DIR .. "src/devices/bus/vme/mvme327a.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme327a.h",
		MAME_DIR .. "src/devices/bus/vme/mvme328.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme328.h",
		MAME_DIR .. "src/devices/bus/vme/mvme350.cpp",
		MAME_DIR .. "src/devices/bus/vme/mvme350.h",
		MAME_DIR .. "src/devices/bus/vme/mzr8105.cpp",
		MAME_DIR .. "src/devices/bus/vme/mzr8105.h",
		MAME_DIR .. "src/devices/bus/vme/mzr8300.cpp",
		MAME_DIR .. "src/devices/bus/vme/mzr8300.h",
		MAME_DIR .. "src/devices/bus/vme/smvme2000.cpp",
		MAME_DIR .. "src/devices/bus/vme/smvme2000.h",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu1.cpp",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu1.h",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu20.cpp",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu20.h",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu30.cpp",
		MAME_DIR .. "src/devices/bus/vme/sys68k_cpu30.h",
		MAME_DIR .. "src/devices/bus/vme/sys68k_iscsi.cpp",
		MAME_DIR .. "src/devices/bus/vme/sys68k_iscsi.h",
		MAME_DIR .. "src/devices/bus/vme/sys68k_isio.cpp",
		MAME_DIR .. "src/devices/bus/vme/sys68k_isio.h",
		MAME_DIR .. "src/devices/bus/vme/tp880v.cpp",
		MAME_DIR .. "src/devices/bus/vme/tp880v.h",
		MAME_DIR .. "src/devices/bus/vme/tp881v.cpp",
		MAME_DIR .. "src/devices/bus/vme/tp881v.h",
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
--@src/devices/bus/wysekbd/wysekbd.h,BUSES["WYSEKBD"] = true
---------------------------------------------------

if (BUSES["WYSEKBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/wysekbd/wysegakb.cpp",
		MAME_DIR .. "src/devices/bus/wysekbd/wysegakb.h",
		MAME_DIR .. "src/devices/bus/wysekbd/wysekbd.cpp",
		MAME_DIR .. "src/devices/bus/wysekbd/wysekbd.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/z29_kbd/keyboard.h,BUSES["Z29_KBD"] = true
---------------------------------------------------

if (BUSES["Z29_KBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/z29_kbd/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/z29_kbd/keyboard.h",
		MAME_DIR .. "src/devices/bus/z29_kbd/he191_3425.cpp",
		MAME_DIR .. "src/devices/bus/z29_kbd/he191_3425.h",
		MAME_DIR .. "src/devices/bus/z29_kbd/md_kbd.cpp",
		MAME_DIR .. "src/devices/bus/z29_kbd/md_kbd.h",
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
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eauxslot.h",
		MAME_DIR .. "src/devices/bus/a2bus/4play.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/4play.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2alfam2.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2applicard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2arcadebd.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2cffa.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2corvus.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2diskiing.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2dx1.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2echoii.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eext80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2eramworks3.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2estd80col.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2hsscsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2iwm.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2iwm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2mcms.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2memexp.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2midi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2mockingboard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2parprn.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2parprn.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2pic.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2sam.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2scsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2sd.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2sd.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2softcard.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2ssc.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2superdrive.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2superdrive.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2swyft.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2themill.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2thunderclock.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2ultraterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2videoterm.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2vulcan.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2wico_trackball.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2wico_trackball.h",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/a2zipdrive.h",
		MAME_DIR .. "src/devices/bus/a2bus/ace2x00.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ace2x00.h",
		MAME_DIR .. "src/devices/bus/a2bus/agat_fdc.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/agat_fdc.h",
		MAME_DIR .. "src/devices/bus/a2bus/agat7langcard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/agat7langcard.h",
		MAME_DIR .. "src/devices/bus/a2bus/agat7ports.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/agat7ports.h",
		MAME_DIR .. "src/devices/bus/a2bus/agat7ram.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/agat7ram.h",
		MAME_DIR .. "src/devices/bus/a2bus/agat840k_hle.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/agat840k_hle.h",
		MAME_DIR .. "src/devices/bus/a2bus/booti.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/booti.h",
		MAME_DIR .. "src/devices/bus/a2bus/byte8251.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/byte8251.h",
		MAME_DIR .. "src/devices/bus/a2bus/cards.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/cards.h",
		MAME_DIR .. "src/devices/bus/a2bus/ccs7710.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ccs7710.h",
		MAME_DIR .. "src/devices/bus/a2bus/cmsscsi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/cmsscsi.h",
		MAME_DIR .. "src/devices/bus/a2bus/computereyes2.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/computereyes2.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc01.h",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/corvfdc02.h",
		MAME_DIR .. "src/devices/bus/a2bus/excel9.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/excel9.h",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ezcgi.h",
		MAME_DIR .. "src/devices/bus/a2bus/grafex.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/grafex.h",
		MAME_DIR .. "src/devices/bus/a2bus/grappler.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/grappler.h",
		MAME_DIR .. "src/devices/bus/a2bus/lancegs.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/lancegs.h",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/laser128.h",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/mouse.h",
		MAME_DIR .. "src/devices/bus/a2bus/nippelclock.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/nippelclock.h",
		MAME_DIR .. "src/devices/bus/a2bus/noisemaker.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/noisemaker.h",
		MAME_DIR .. "src/devices/bus/a2bus/pc_xporter.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/pc_xporter.h",
		MAME_DIR .. "src/devices/bus/a2bus/prodosromdrive.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/prodosromdrive.h",
		MAME_DIR .. "src/devices/bus/a2bus/q68.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/q68.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard128k.h",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ramcard16k.h",
		MAME_DIR .. "src/devices/bus/a2bus/romcard.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/romcard.h",
		MAME_DIR .. "src/devices/bus/a2bus/sider.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/sider.h",
		MAME_DIR .. "src/devices/bus/a2bus/snesmax.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/snesmax.h",
		MAME_DIR .. "src/devices/bus/a2bus/softcard3.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/softcard3.h",
		MAME_DIR .. "src/devices/bus/a2bus/ssbapple.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ssbapple.h",
		MAME_DIR .. "src/devices/bus/a2bus/ssprite.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/ssprite.h",
		MAME_DIR .. "src/devices/bus/a2bus/suprterminal.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/suprterminal.h",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/timemasterho.h",
		MAME_DIR .. "src/devices/bus/a2bus/titan3plus2.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/titan3plus2.h",
		MAME_DIR .. "src/devices/bus/a2bus/transwarp.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/transwarp.h",
		MAME_DIR .. "src/devices/bus/a2bus/uniprint.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/uniprint.h",
		MAME_DIR .. "src/devices/bus/a2bus/uthernet.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/uthernet.h",
		MAME_DIR .. "src/devices/bus/a2bus/vistaa800.cpp",
		MAME_DIR .. "src/devices/bus/a2bus/vistaa800.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/a2gameio/gameio.h,BUSES["A2GAMEIO"] = true
---------------------------------------------------

if (BUSES["A2GAMEIO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/a2gameio/brightpen.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/brightpen.h",
		MAME_DIR .. "src/devices/bus/a2gameio/computereyes.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/computereyes.h",
		MAME_DIR .. "src/devices/bus/a2gameio/gameio.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/gameio.h",
		MAME_DIR .. "src/devices/bus/a2gameio/joystick.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/joystick.h",
		MAME_DIR .. "src/devices/bus/a2gameio/joyport.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/joyport.h",
		MAME_DIR .. "src/devices/bus/a2gameio/joyport_paddles.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/joyport_paddles.h",
		MAME_DIR .. "src/devices/bus/a2gameio/paddles.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/paddles.h",
		MAME_DIR .. "src/devices/bus/a2gameio/gizmo.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/gizmo.h",
		MAME_DIR .. "src/devices/bus/a2gameio/wico_joystick.cpp",
		MAME_DIR .. "src/devices/bus/a2gameio/wico_joystick.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nscsi/cd.h,BUSES["NSCSI"] = true
--@src/devices/bus/nscsi/devices.h,BUSES["NSCSI"] = true
--@src/devices/bus/nscsi/hd.h,BUSES["NSCSI"] = true
--@src/devices/bus/nscsi/s1410.h,BUSES["NSCSI"] = true
---------------------------------------------------

if (BUSES["NSCSI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nscsi/applecd.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/applecd.h",
		MAME_DIR .. "src/devices/bus/nscsi/cd.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cd.h",
		MAME_DIR .. "src/devices/bus/nscsi/cdd2000.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cdd2000.h",
		MAME_DIR .. "src/devices/bus/nscsi/cdrn820s.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cdrn820s.h",
		MAME_DIR .. "src/devices/bus/nscsi/cdu75s.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cdu75s.h",
		MAME_DIR .. "src/devices/bus/nscsi/cdu415.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cdu415.h",
		MAME_DIR .. "src/devices/bus/nscsi/cdu561.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cdu561.h",
		MAME_DIR .. "src/devices/bus/nscsi/cfp1080s.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cfp1080s.h",
		MAME_DIR .. "src/devices/bus/nscsi/crd254sh.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/crd254sh.h",
		MAME_DIR .. "src/devices/bus/nscsi/cw7501.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/cw7501.h",
		MAME_DIR .. "src/devices/bus/nscsi/devices.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/devices.h",
		MAME_DIR .. "src/devices/bus/nscsi/hd.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/hd.h",
		MAME_DIR .. "src/devices/bus/nscsi/s1410.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/s1410.h",
		MAME_DIR .. "src/devices/bus/nscsi/smoc501.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/smoc501.h",
		MAME_DIR .. "src/devices/bus/nscsi/tape.cpp",
		MAME_DIR .. "src/devices/bus/nscsi/tape.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nubus/nubus.h,BUSES["NUBUS"] = true
---------------------------------------------------

if (BUSES["NUBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nubus/cards.cpp",
		MAME_DIR .. "src/devices/bus/nubus/cards.h",
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
		MAME_DIR .. "src/devices/bus/nubus/8lc.cpp",
		MAME_DIR .. "src/devices/bus/nubus/8lc.h",
		MAME_DIR .. "src/devices/bus/nubus/bootbug.cpp",
		MAME_DIR .. "src/devices/bus/nubus/bootbug.h",
		MAME_DIR .. "src/devices/bus/nubus/quadralink.cpp",
		MAME_DIR .. "src/devices/bus/nubus/quadralink.h",
		MAME_DIR .. "src/devices/bus/nubus/laserview.cpp",
		MAME_DIR .. "src/devices/bus/nubus/laserview.h",
		MAME_DIR .. "src/devices/bus/nubus/supermac.cpp",
		MAME_DIR .. "src/devices/bus/nubus/supermac.h",
		MAME_DIR .. "src/devices/bus/nubus/thunder4gx.cpp",
		MAME_DIR .. "src/devices/bus/nubus/thunder4gx.h",
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
		MAME_DIR .. "src/devices/bus/centronics/adaptator.cpp",
		MAME_DIR .. "src/devices/bus/centronics/adaptator.h",
		MAME_DIR .. "src/devices/bus/centronics/ctronics.cpp",
		MAME_DIR .. "src/devices/bus/centronics/ctronics.h",
		MAME_DIR .. "src/devices/bus/centronics/chessmec.cpp",
		MAME_DIR .. "src/devices/bus/centronics/chessmec.h",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.cpp",
		MAME_DIR .. "src/devices/bus/centronics/comxpl80.h",
		MAME_DIR .. "src/devices/bus/centronics/covox.cpp",
		MAME_DIR .. "src/devices/bus/centronics/covox.h",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.cpp",
		MAME_DIR .. "src/devices/bus/centronics/digiblst.h",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.cpp",
		MAME_DIR .. "src/devices/bus/centronics/dsjoy.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_ex800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx800.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.h",
		MAME_DIR .. "src/devices/bus/centronics/epson_rx80.cpp",
		MAME_DIR .. "src/devices/bus/centronics/epson_rx80.h",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.cpp",
		MAME_DIR .. "src/devices/bus/centronics/nec_p72.h",
		MAME_DIR .. "src/devices/bus/centronics/nlq401.cpp",
		MAME_DIR .. "src/devices/bus/centronics/nlq401.h",
		MAME_DIR .. "src/devices/bus/centronics/printer.cpp",
		MAME_DIR .. "src/devices/bus/centronics/printer.h",
		MAME_DIR .. "src/devices/bus/centronics/samdac.cpp",
		MAME_DIR .. "src/devices/bus/centronics/samdac.h",
		MAME_DIR .. "src/devices/bus/centronics/smartboard.cpp",
		MAME_DIR .. "src/devices/bus/centronics/smartboard.h",
		MAME_DIR .. "src/devices/bus/centronics/spjoy.cpp",
		MAME_DIR .. "src/devices/bus/centronics/spjoy.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/centronics/epson_ex800.cpp",  GEN_DIR .. "emu/layout/ex800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx800.cpp",  GEN_DIR .. "emu/layout/lx800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/epson_lx810l.cpp", GEN_DIR .. "emu/layout/lx800.lh" },
		{ MAME_DIR .. "src/devices/bus/centronics/smartboard.cpp",   GEN_DIR .. "emu/layout/smartboard.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "ex800"),
		layoutbuildtask("emu/layout", "lx800"),
		layoutbuildtask("emu/layout", "smartboard"),
	}
end

---------------------------------------------------
--
--@src/devices/bus/rs232/rs232.h,BUSES["RS232"] = true
---------------------------------------------------

if (BUSES["RS232"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/rs232/exorterm.cpp",
		MAME_DIR .. "src/devices/bus/rs232/exorterm.h",
		MAME_DIR .. "src/devices/bus/rs232/hlemouse.cpp",
		MAME_DIR .. "src/devices/bus/rs232/hlemouse.h",
		MAME_DIR .. "src/devices/bus/rs232/ie15.cpp",
		MAME_DIR .. "src/devices/bus/rs232/ie15.h",
		MAME_DIR .. "src/devices/bus/rs232/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/rs232/keyboard.h",
		MAME_DIR .. "src/devices/bus/rs232/loopback.cpp",
		MAME_DIR .. "src/devices/bus/rs232/loopback.h",
		MAME_DIR .. "src/devices/bus/rs232/mboardd.cpp",
		MAME_DIR .. "src/devices/bus/rs232/mboardd.h",
		MAME_DIR .. "src/devices/bus/rs232/nss_tvinterface.cpp",
		MAME_DIR .. "src/devices/bus/rs232/nss_tvinterface.h",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.cpp",
		MAME_DIR .. "src/devices/bus/rs232/null_modem.h",
		MAME_DIR .. "src/devices/bus/rs232/patchbox.cpp",
		MAME_DIR .. "src/devices/bus/rs232/patchbox.h",
		MAME_DIR .. "src/devices/bus/rs232/printer.cpp",
		MAME_DIR .. "src/devices/bus/rs232/printer.h",
		MAME_DIR .. "src/devices/bus/rs232/pty.cpp",
		MAME_DIR .. "src/devices/bus/rs232/pty.h",
		MAME_DIR .. "src/devices/bus/rs232/rs232.cpp",
		MAME_DIR .. "src/devices/bus/rs232/rs232.h",
		MAME_DIR .. "src/devices/bus/rs232/rs232_sync_io.cpp",
		MAME_DIR .. "src/devices/bus/rs232/rs232_sync_io.h",
		MAME_DIR .. "src/devices/bus/rs232/scorpion.cpp",
		MAME_DIR .. "src/devices/bus/rs232/scorpion.h",
		MAME_DIR .. "src/devices/bus/rs232/sun_kbd.cpp",
		MAME_DIR .. "src/devices/bus/rs232/sun_kbd.h",
		MAME_DIR .. "src/devices/bus/rs232/swtpc8212.cpp",
		MAME_DIR .. "src/devices/bus/rs232/swtpc8212.h",
		MAME_DIR .. "src/devices/bus/rs232/teletex800.cpp",
		MAME_DIR .. "src/devices/bus/rs232/teletex800.h",
		MAME_DIR .. "src/devices/bus/rs232/terminal.cpp",
		MAME_DIR .. "src/devices/bus/rs232/terminal.h",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.cpp",
		MAME_DIR .. "src/devices/bus/rs232/xvd701.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/rs232/teletex800.cpp",   GEN_DIR .. "emu/layout/teletex800.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "teletex800"),
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
		MAME_DIR .. "src/devices/bus/nes/nes_ines.hxx",
		MAME_DIR .. "src/devices/bus/nes/nes_pcb.hxx",
		MAME_DIR .. "src/devices/bus/nes/nes_unif.hxx",
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
		MAME_DIR .. "src/devices/bus/nes/batlab.cpp",
		MAME_DIR .. "src/devices/bus/nes/batlab.h",
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
		MAME_DIR .. "src/devices/bus/nes/irem.cpp",
		MAME_DIR .. "src/devices/bus/nes/irem.h",
		MAME_DIR .. "src/devices/bus/nes/jaleco.cpp",
		MAME_DIR .. "src/devices/bus/nes/jaleco.h",
		MAME_DIR .. "src/devices/bus/nes/jncota.cpp",
		MAME_DIR .. "src/devices/bus/nes/jncota.h",
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
		MAME_DIR .. "src/devices/bus/nes/mmc1_clones.cpp",
		MAME_DIR .. "src/devices/bus/nes/mmc1_clones.h",
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
		MAME_DIR .. "src/devices/bus/nes/sealie.cpp",
		MAME_DIR .. "src/devices/bus/nes/sealie.h",
		MAME_DIR .. "src/devices/bus/nes/somari.cpp",
		MAME_DIR .. "src/devices/bus/nes/somari.h",
		MAME_DIR .. "src/devices/bus/nes/subor.cpp",
		MAME_DIR .. "src/devices/bus/nes/subor.h",
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
		MAME_DIR .. "src/devices/bus/nes/vrc_clones.cpp",
		MAME_DIR .. "src/devices/bus/nes/vrc_clones.h",
		MAME_DIR .. "src/devices/bus/nes/waixing.cpp",
		MAME_DIR .. "src/devices/bus/nes/waixing.h",
		MAME_DIR .. "src/devices/bus/nes/zemina.cpp",
		MAME_DIR .. "src/devices/bus/nes/zemina.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/nes_ctrl/ctrl.h,BUSES["NES_CTRL"] = true
--@src/devices/bus/nes_ctrl/zapper_sensor.h,BUSES["NES_CTRL"] = true
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
		MAME_DIR .. "src/devices/bus/nes_ctrl/bandaihs.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bandaihs.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/bcbattle.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/dorepiano.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/dorepiano.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fckeybrd.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fcmat.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/fcmat.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/hori.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamibag.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/konamibag.h",
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
		MAME_DIR .. "src/devices/bus/nes_ctrl/rob.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/rob.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/sharpcass.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/sharpcass.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/snesadapter.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/snesadapter.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/suborkey.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/turbofile.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/turbofile.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper.h",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper_sensor.cpp",
		MAME_DIR .. "src/devices/bus/nes_ctrl/zapper_sensor.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/nes_ctrl/rob.cpp",   GEN_DIR .. "emu/layout/nes_rob.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "nes_rob"),
	}
end

---------------------------------------------------
--
--@src/devices/bus/sdk85/memexp.h,BUSES["SDK85"] = true
---------------------------------------------------

if (BUSES["SDK85"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sdk85/memexp.cpp",
		MAME_DIR .. "src/devices/bus/sdk85/memexp.h",
		MAME_DIR .. "src/devices/bus/sdk85/i8755.cpp",
		MAME_DIR .. "src/devices/bus/sdk85/i8755.h",
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
		MAME_DIR .. "src/devices/bus/snes/profighter.cpp",
		MAME_DIR .. "src/devices/bus/snes/profighter.h",
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
--@src/devices/bus/mononcol/slot.h,BUSES["MONONCOL"] = true
---------------------------------------------------

if (BUSES["MONONCOL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mononcol/slot.cpp",
		MAME_DIR .. "src/devices/bus/mononcol/slot.h",
		MAME_DIR .. "src/devices/bus/mononcol/carts.cpp",
		MAME_DIR .. "src/devices/bus/mononcol/carts.h",
		MAME_DIR .. "src/devices/bus/mononcol/rom.cpp",
		MAME_DIR .. "src/devices/bus/mononcol/rom.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/neogeo/slot.h,BUSES["NEOGEO"] = true
---------------------------------------------------

if (BUSES["NEOGEO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/neogeo/slot.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/slot.h",
		MAME_DIR .. "src/devices/bus/neogeo/carts.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/carts.h",
		MAME_DIR .. "src/devices/bus/neogeo/rom.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/rom.h",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/fatfury2.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof98.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof98.h",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/mslugx.h",
		MAME_DIR .. "src/devices/bus/neogeo/cmc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/cmc.h",
		MAME_DIR .. "src/devices/bus/neogeo/sma.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/sma.h",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pcm2.h",
		MAME_DIR .. "src/devices/bus/neogeo/kof2k2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/kof2k2.h",
		MAME_DIR .. "src/devices/bus/neogeo/pvc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/pvc.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_cthd.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_cthd.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_misc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_misc.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_svc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_svc.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof2k2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof2k2.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof2k3.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof2k3.h",
		MAME_DIR .. "src/devices/bus/neogeo/sbp.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/sbp.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_fatfury2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_fatfury2.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof98.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof98.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_mslugx.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_mslugx.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_cmc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_cmc.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_sma.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_sma.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_pcm2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_pcm2.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof2k2.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof2k2.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_pvc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_pvc.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_cthd.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_cthd.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof2k3bl.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_kof2k3bl.h",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof10th.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/boot_kof10th.h",
		MAME_DIR .. "src/devices/bus/neogeo/prot_misc.cpp",
		MAME_DIR .. "src/devices/bus/neogeo/prot_misc.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/neogeo_ctrl/ctrl.h,BUSES["NEOGEO_CTRL"] = true
---------------------------------------------------

if (BUSES["NEOGEO_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/joystick.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/joystick.h",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/mahjong.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/mahjong.h",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/dial.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/dial.h",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/irrmaze.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/irrmaze.h",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/kizuna4p.cpp",
		MAME_DIR .. "src/devices/bus/neogeo_ctrl/kizuna4p.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sat_ctrl/ctrl.h,BUSES["SAT_CTRL"] = true
---------------------------------------------------

if (BUSES["SAT_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sat_ctrl/ctrl.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/ctrl.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/analog.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/analog.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/joy.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/joy.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/joy_md.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/joy_md.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/keybd.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/keybd.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/multitap.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/pointer.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/pointer.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/racing.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/racing.h",
		MAME_DIR .. "src/devices/bus/sat_ctrl/segatap.cpp",
		MAME_DIR .. "src/devices/bus/sat_ctrl/segatap.h",
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
--@src/devices/bus/sg1000_exp/sg1000exp.h,BUSES["SG1000_EXP"] = true
---------------------------------------------------

if (BUSES["SG1000_EXP"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sg1000_exp/fm_unit.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/fm_unit.h",
		MAME_DIR .. "src/devices/bus/sg1000_exp/kblink.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/kblink.h",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sg1000exp.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sg1000exp.h",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sk1100.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sk1100.h",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sk1100prn.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sk1100prn.h",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sp400.cpp",
		MAME_DIR .. "src/devices/bus/sg1000_exp/sp400.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sms_ctrl/smsctrl.h,BUSES["SMS_CTRL"] = true
---------------------------------------------------

if (BUSES["SMS_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sms_ctrl/controllers.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/controllers.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/diypaddle.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/diypaddle.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/hypershot.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/hypershot.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/graphic.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/joypad.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/lphaser.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/md6bt.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/md6bt.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/mdpad.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/mdpad.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/paddle.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rfu.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rs232adapt.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/rs232adapt.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/smsctrl.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sports.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/sportsjp.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/teamplayer.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/teamplayer.h",
		MAME_DIR .. "src/devices/bus/sms_ctrl/xe1ap.cpp",
		MAME_DIR .. "src/devices/bus/sms_ctrl/xe1ap.h",
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
--@src/devices/bus/tanbus/tanbus.h,BUSES["TANBUS"] = true
---------------------------------------------------

if (BUSES["TANBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/keyboard.h",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/mt006.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/mt006.h",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/mt009.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/mt009.h",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/spinveti.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/keyboard/spinveti.h",
		MAME_DIR .. "src/devices/bus/tanbus/bullsnd.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/bullsnd.h",
		MAME_DIR .. "src/devices/bus/tanbus/etirtc.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/etirtc.h",
		MAME_DIR .. "src/devices/bus/tanbus/etisnd.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/etisnd.h",
		MAME_DIR .. "src/devices/bus/tanbus/tanbus.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tanbus.h",
		MAME_DIR .. "src/devices/bus/tanbus/tanex.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tanex.h",
		MAME_DIR .. "src/devices/bus/tanbus/tandos.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tandos.h",
		MAME_DIR .. "src/devices/bus/tanbus/tanhrg.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tanhrg.h",
		MAME_DIR .. "src/devices/bus/tanbus/tanram.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tanram.h",
		MAME_DIR .. "src/devices/bus/tanbus/tanrtc.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tanrtc.h",
		MAME_DIR .. "src/devices/bus/tanbus/mpvdu.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/mpvdu.h",
		MAME_DIR .. "src/devices/bus/tanbus/ra32k.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/ra32k.h",
		MAME_DIR .. "src/devices/bus/tanbus/radisc.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/radisc.h",
		MAME_DIR .. "src/devices/bus/tanbus/ravdu.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/ravdu.h",
		MAME_DIR .. "src/devices/bus/tanbus/tug64k.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tug64k.h",
		MAME_DIR .. "src/devices/bus/tanbus/tug8082.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tug8082.h",
		MAME_DIR .. "src/devices/bus/tanbus/tugcombo.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tugcombo.h",
		MAME_DIR .. "src/devices/bus/tanbus/tugpgm.cpp",
		MAME_DIR .. "src/devices/bus/tanbus/tugpgm.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti8x/ti8x.h,BUSES["TI8X"] = true
---------------------------------------------------

if (BUSES["TI8X"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti8x/bitsocket.cpp",
		MAME_DIR .. "src/devices/bus/ti8x/bitsocket.h",
		MAME_DIR .. "src/devices/bus/ti8x/graphlinkhle.cpp",
		MAME_DIR .. "src/devices/bus/ti8x/graphlinkhle.h",
		MAME_DIR .. "src/devices/bus/ti8x/teeconn.cpp",
		MAME_DIR .. "src/devices/bus/ti8x/teeconn.h",
		MAME_DIR .. "src/devices/bus/ti8x/ti8x.cpp",
		MAME_DIR .. "src/devices/bus/ti8x/ti8x.h",
		MAME_DIR .. "src/devices/bus/ti8x/tispeaker.cpp",
		MAME_DIR .. "src/devices/bus/ti8x/tispeaker.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99x/990_dk.h,BUSES["TI99X"] = true
--@src/devices/bus/ti99x/990_tap.h,BUSES["TI99X"] = true
--@src/devices/bus/ti99x/990_hd.h,BUSES["TI99X"] = true
---------------------------------------------------

if (BUSES["TI99X"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_dk.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_hd.h",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.cpp",
		MAME_DIR .. "src/devices/bus/ti99x/990_tap.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/ti99/colorbus/colorbus.h,BUSES["TI99"] = true
--@src/devices/bus/ti99/gromport/cartridges.h,BUSES["TI99"] = true
--@src/devices/bus/ti99/joyport/joyport.h,BUSES["TI99"] = true
--@src/devices/bus/ti99/peb/peribox.h,BUSES["TI99"] = true
--@src/devices/bus/ti99/internal/genboard.h,BUSES["TI99"] = true
---------------------------------------------------

if (BUSES["TI99"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/ti99/internal/992board.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/992board.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/998board.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/998board.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/buffram.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/buffram.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/datamux.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/datamux.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/evpcconn.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/evpcconn.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/genboard.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/genboard.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/genkbd.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/genkbd.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/ioport.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/ioport.h",
		MAME_DIR .. "src/devices/bus/ti99/internal/splitter.cpp",
		MAME_DIR .. "src/devices/bus/ti99/internal/splitter.h",
		MAME_DIR .. "src/devices/bus/ti99/colorbus/busmouse.cpp",
		MAME_DIR .. "src/devices/bus/ti99/colorbus/busmouse.h",
		MAME_DIR .. "src/devices/bus/ti99/colorbus/colorbus.cpp",
		MAME_DIR .. "src/devices/bus/ti99/colorbus/colorbus.h",
		MAME_DIR .. "src/devices/bus/ti99/gromport/gromport.cpp",
		MAME_DIR .. "src/devices/bus/ti99/gromport/gromport.h",
		MAME_DIR .. "src/devices/bus/ti99/gromport/cartridges.cpp",
		MAME_DIR .. "src/devices/bus/ti99/gromport/cartridges.h",
		MAME_DIR .. "src/devices/bus/ti99/gromport/gkracker.cpp",
		MAME_DIR .. "src/devices/bus/ti99/gromport/gkracker.h",
		MAME_DIR .. "src/devices/bus/ti99/gromport/multiconn.cpp",
		MAME_DIR .. "src/devices/bus/ti99/gromport/multiconn.h",
		MAME_DIR .. "src/devices/bus/ti99/gromport/singleconn.cpp",
		MAME_DIR .. "src/devices/bus/ti99/gromport/singleconn.h",
		MAME_DIR .. "src/devices/bus/ti99/joyport/handset.cpp",
		MAME_DIR .. "src/devices/bus/ti99/joyport/handset.h",
		MAME_DIR .. "src/devices/bus/ti99/joyport/joyport.cpp",
		MAME_DIR .. "src/devices/bus/ti99/joyport/joyport.h",
		MAME_DIR .. "src/devices/bus/ti99/joyport/mecmouse.cpp",
		MAME_DIR .. "src/devices/bus/ti99/joyport/mecmouse.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/peribox.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/peribox.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/bwg.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/bwg.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/cc_fdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/cc_fdc.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/myarcfdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/myarcfdc.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/evpc.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/evpc.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/forti.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/forti.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/hfdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/hfdc.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/horizon.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/horizon.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/hsgpl.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/hsgpl.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/memex.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/memex.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/myarcmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/myarcmem.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/pcode.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/pcode.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/pgram.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/pgram.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/samsmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/samsmem.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/scsicard.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/scsicard.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/sidmaster.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/sidmaster.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/spchsyn.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/spchsyn.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_32kmem.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_32kmem.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_fdc.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_fdc.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_rs232.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/ti_rs232.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/tipi.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/tipi.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/tn_ide.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/tn_ide.h",
		MAME_DIR .. "src/devices/bus/ti99/peb/tn_usbsm.cpp",
		MAME_DIR .. "src/devices/bus/ti99/peb/tn_usbsm.h",
		MAME_DIR .. "src/devices/bus/ti99/sidecar/arcturus.cpp",
		MAME_DIR .. "src/devices/bus/ti99/sidecar/arcturus.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/gameboy/slot.h,BUSES["GAMEBOY"] = true
---------------------------------------------------

if (BUSES["GAMEBOY"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/gameboy/camera.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/camera.h",
		MAME_DIR .. "src/devices/bus/gameboy/cartbase.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/cartbase.h",
		MAME_DIR .. "src/devices/bus/gameboy/cartbase.ipp",
		MAME_DIR .. "src/devices/bus/gameboy/cartheader.h",
		MAME_DIR .. "src/devices/bus/gameboy/carts.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/carts.h",
		MAME_DIR .. "src/devices/bus/gameboy/gbck003.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/gbck003.h",
		MAME_DIR .. "src/devices/bus/gameboy/gbslot.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/gbslot.h",
		MAME_DIR .. "src/devices/bus/gameboy/gbxfile.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/gbxfile.h",
		MAME_DIR .. "src/devices/bus/gameboy/huc1.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/huc1.h",
		MAME_DIR .. "src/devices/bus/gameboy/huc3.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/huc3.h",
		MAME_DIR .. "src/devices/bus/gameboy/liebao.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/liebao.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc2.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc2.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc3.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc3.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc6.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc6.h",
		MAME_DIR .. "src/devices/bus/gameboy/mbc7.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mbc7.h",
		MAME_DIR .. "src/devices/bus/gameboy/mdslot.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mdslot.h",
		MAME_DIR .. "src/devices/bus/gameboy/mmm01.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/mmm01.h",
		MAME_DIR .. "src/devices/bus/gameboy/ntnew.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/ntnew.h",
		MAME_DIR .. "src/devices/bus/gameboy/rom.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/rom.h",
		MAME_DIR .. "src/devices/bus/gameboy/slmulti.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/slmulti.h",
		MAME_DIR .. "src/devices/bus/gameboy/slot.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/slot.h",
		MAME_DIR .. "src/devices/bus/gameboy/tama5.cpp",
		MAME_DIR .. "src/devices/bus/gameboy/tama5.h",
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
		MAME_DIR .. "src/devices/bus/bml3/mp1802.cpp",
		MAME_DIR .. "src/devices/bus/bml3/mp1802.h",
		MAME_DIR .. "src/devices/bus/bml3/mp1805.cpp",
		MAME_DIR .. "src/devices/bus/bml3/mp1805.h",
		MAME_DIR .. "src/devices/bus/bml3/kanji.cpp",
		MAME_DIR .. "src/devices/bus/bml3/kanji.h",
		MAME_DIR .. "src/devices/bus/bml3/rtc.cpp",
		MAME_DIR .. "src/devices/bus/bml3/rtc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/coco/cococart.h,BUSES["COCO"] = true
---------------------------------------------------
if (BUSES["COCO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/coco/coco_dcmodem.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_dcmodem.h",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_dwsock.h",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_fdc.h",
		MAME_DIR .. "src/devices/bus/coco/coco_gmc.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_gmc.h",
		MAME_DIR .. "src/devices/bus/coco/coco_ide.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_ide.h",
		MAME_DIR .. "src/devices/bus/coco/coco_max.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_max.h",
		MAME_DIR .. "src/devices/bus/coco/coco_midi.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_midi.h",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_multi.h",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_orch90.h",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_pak.h",
		MAME_DIR .. "src/devices/bus/coco/coco_psg.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_psg.h",
		MAME_DIR .. "src/devices/bus/coco/coco_ram.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_ram.h",
		MAME_DIR .. "src/devices/bus/coco/coco_rs232.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_rs232.h",
		MAME_DIR .. "src/devices/bus/coco/coco_ssc.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_ssc.h",
		MAME_DIR .. "src/devices/bus/coco/coco_stecomp.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_stecomp.h",
		MAME_DIR .. "src/devices/bus/coco/coco_sym12.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_sym12.h",
		MAME_DIR .. "src/devices/bus/coco/coco_t4426.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_t4426.h",
		MAME_DIR .. "src/devices/bus/coco/coco_wpk.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_wpk.h",
		MAME_DIR .. "src/devices/bus/coco/coco_wpk2p.cpp",
		MAME_DIR .. "src/devices/bus/coco/coco_wpk2p.h",
		MAME_DIR .. "src/devices/bus/coco/cococart.cpp",
		MAME_DIR .. "src/devices/bus/coco/cococart.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_amtor.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_amtor.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_claw.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_claw.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_fdc.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_fdc.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_jcbsnd.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_jcbsnd.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_jcbspch.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_jcbspch.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_msx2.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_msx2.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_serial.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_serial.h",
		MAME_DIR .. "src/devices/bus/coco/dragon_sprites.cpp",
		MAME_DIR .. "src/devices/bus/coco/dragon_sprites.h",
		MAME_DIR .. "src/devices/bus/coco/meb_intrf.cpp",
		MAME_DIR .. "src/devices/bus/coco/meb_intrf.h",
		MAME_DIR .. "src/devices/bus/coco/meb_rtime.cpp",
		MAME_DIR .. "src/devices/bus/coco/meb_rtime.h",
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
		MAME_DIR .. "src/devices/bus/cpc/transtape.cpp",
		MAME_DIR .. "src/devices/bus/cpc/transtape.h",
		MAME_DIR .. "src/devices/bus/cpc/musicmachine.cpp",
		MAME_DIR .. "src/devices/bus/cpc/musicmachine.h",
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
--@src/devices/bus/epson_qx/option.h,BUSES["EPSON_QX"] = true
---------------------------------------------------
if (BUSES["EPSON_QX"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/epson_qx/cqgmem.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/cqgmem.h",
		MAME_DIR .. "src/devices/bus/epson_qx/cr1510.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/cr1510.h",
		MAME_DIR .. "src/devices/bus/epson_qx/ide.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/ide.h",
		MAME_DIR .. "src/devices/bus/epson_qx/multifont.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/multifont.h",
		MAME_DIR .. "src/devices/bus/epson_qx/option.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/option.h",
		MAME_DIR .. "src/devices/bus/epson_qx/sound_card.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/sound_card.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/epson_qx/keyboard/keyboard.h,BUSES["EPSON_QX_KEYBOARD"] = true
---------------------------------------------------

if (BUSES["EPSON_QX_KEYBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/epson_qx/keyboard/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/keyboard/keyboard.h",
		MAME_DIR .. "src/devices/bus/epson_qx/keyboard/matrix.cpp",
		MAME_DIR .. "src/devices/bus/epson_qx/keyboard/matrix.h",
	}
	dependency {
		{ MAME_DIR .. "src/devices/bus/epson_qx/keyboard/keyboard.cpp", GEN_DIR .. "emu/layout/qx10ascii.lh" },
		{ MAME_DIR .. "src/devices/bus/epson_qx/keyboard/keyboard.cpp", GEN_DIR .. "emu/layout/qx10hasci.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "qx10ascii"),
		layoutbuildtask("emu/layout", "qx10hasci"),
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
		MAME_DIR .. "src/devices/bus/pce/pce_scdsys.cpp",
		MAME_DIR .. "src/devices/bus/pce/pce_scdsys.h",
		MAME_DIR .. "src/devices/bus/pce/pce_acard.cpp",
		MAME_DIR .. "src/devices/bus/pce/pce_acard.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pce_ctrl/pcectrl.h,BUSES["PCE_CTRL"] = true
---------------------------------------------------

if (BUSES["PCE_CTRL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pce_ctrl/pcectrl.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/pcectrl.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/joypad2.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/joypad2.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/joypad6.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/joypad6.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/mouse.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/mouse.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/multitap.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/multitap.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/pachinko.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/pachinko.h",
		MAME_DIR .. "src/devices/bus/pce_ctrl/xhe3.cpp",
		MAME_DIR .. "src/devices/bus/pce_ctrl/xhe3.h",
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
		MAME_DIR .. "src/devices/bus/x68k/x68k_midi.cpp",
		MAME_DIR .. "src/devices/bus/x68k/x68k_midi.h",
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
		MAME_DIR .. "src/devices/bus/abckb/r8.cpp",
		MAME_DIR .. "src/devices/bus/abckb/r8.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/compucolor/floppy.h,BUSES["COMPUCOLOR"] = true
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
		MAME_DIR .. "src/devices/bus/scsi/scsicd512.cpp",
		MAME_DIR .. "src/devices/bus/scsi/scsicd512.h",
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
--@src/devices/bus/mackbd/mackbd.h,BUSES["MACKBD"] = true
---------------------------------------------------

if (BUSES["MACKBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mackbd/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/mackbd/keyboard.h",
		MAME_DIR .. "src/devices/bus/mackbd/mackbd.cpp",
		MAME_DIR .. "src/devices/bus/mackbd/mackbd.h",
		MAME_DIR .. "src/devices/bus/mackbd/pluskbd.cpp",
		MAME_DIR .. "src/devices/bus/mackbd/pluskbd.h",
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
		MAME_DIR .. "src/devices/bus/macpds/hyperdrive.cpp",
		MAME_DIR .. "src/devices/bus/macpds/hyperdrive.h",
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
--@src/devices/bus/kim1/kim1bus.h,BUSES["KIM1BUS"] = true
---------------------------------------------------

if (BUSES["KIM1BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/kim1/cards.cpp",
		MAME_DIR .. "src/devices/bus/kim1/cards.h",
		MAME_DIR .. "src/devices/bus/kim1/kim1bus.cpp",
		MAME_DIR .. "src/devices/bus/kim1/kim1bus.h",
		MAME_DIR .. "src/devices/bus/kim1/k1008_vismem.cpp",
		MAME_DIR .. "src/devices/bus/kim1/k1008_vismem.h",
		MAME_DIR .. "src/devices/bus/kim1/k1016_16k.cpp",
		MAME_DIR .. "src/devices/bus/kim1/k1016_16k.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/amiga/keyboard/keyboard.h,BUSES["AMIGA_KEYBOARD"] = true
---------------------------------------------------

if (BUSES["AMIGA_KEYBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/amiga/keyboard/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/keyboard.h",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/matrix.cpp",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/matrix.h",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/a1200.cpp",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/a1200.h",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/a2000.cpp",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/a2000.h",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/mitsumi.cpp",
		MAME_DIR .. "src/devices/bus/amiga/keyboard/mitsumi.h",
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
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2058.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2058.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2065.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2065.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2091.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2091.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/a2232.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/buddha.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/merlin.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/merlin.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/oktagon2008.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/oktagon2008.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/picasso2.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/picasso2.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/rainbow2.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/rainbow2.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/ripple.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/ripple.h",
		MAME_DIR .. "src/devices/bus/amiga/zorro/toccata.cpp",
		MAME_DIR .. "src/devices/bus/amiga/zorro/toccata.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sunkbd/sunkbd.h,BUSES["SUNKBD"] = true
---------------------------------------------------

if (BUSES["SUNKBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sunkbd/hlekbd.cpp",
		MAME_DIR .. "src/devices/bus/sunkbd/hlekbd.h",
		MAME_DIR .. "src/devices/bus/sunkbd/sunkbd.cpp",
		MAME_DIR .. "src/devices/bus/sunkbd/sunkbd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sunmouse/sunmouse.h,BUSES["SUNMOUSE"] = true
---------------------------------------------------

if (BUSES["SUNMOUSE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sunmouse/hlemouse.cpp",
		MAME_DIR .. "src/devices/bus/sunmouse/hlemouse.h",
		MAME_DIR .. "src/devices/bus/sunmouse/sunmouse.cpp",
		MAME_DIR .. "src/devices/bus/sunmouse/sunmouse.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/sbus/sbus.h,BUSES["SBUS"] = true
---------------------------------------------------

if (BUSES["SBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/sbus/artecon.cpp",
		MAME_DIR .. "src/devices/bus/sbus/artecon.h",
		MAME_DIR .. "src/devices/bus/sbus/bwtwo.cpp",
		MAME_DIR .. "src/devices/bus/sbus/bwtwo.h",
		MAME_DIR .. "src/devices/bus/sbus/cgsix.cpp",
		MAME_DIR .. "src/devices/bus/sbus/cgsix.h",
		MAME_DIR .. "src/devices/bus/sbus/cgthree.cpp",
		MAME_DIR .. "src/devices/bus/sbus/cgthree.h",
		MAME_DIR .. "src/devices/bus/sbus/hme.cpp",
		MAME_DIR .. "src/devices/bus/sbus/hme.h",
		MAME_DIR .. "src/devices/bus/sbus/sunpc.cpp",
		MAME_DIR .. "src/devices/bus/sbus/sunpc.h",
		MAME_DIR .. "src/devices/bus/sbus/sbus.cpp",
		MAME_DIR .. "src/devices/bus/sbus/sbus.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/qbus/qbus.h,BUSES["QBUS"] = true
---------------------------------------------------

if (BUSES["QBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/qbus/bk_kmd.cpp",
		MAME_DIR .. "src/devices/bus/qbus/bk_kmd.h",
		MAME_DIR .. "src/devices/bus/qbus/dsd4432.cpp",
		MAME_DIR .. "src/devices/bus/qbus/dsd4432.h",
		MAME_DIR .. "src/devices/bus/qbus/dvk_kgd.cpp",
		MAME_DIR .. "src/devices/bus/qbus/dvk_kgd.h",
		MAME_DIR .. "src/devices/bus/qbus/dvk_mx.cpp",
		MAME_DIR .. "src/devices/bus/qbus/dvk_mx.h",
		MAME_DIR .. "src/devices/bus/qbus/pc11.cpp",
		MAME_DIR .. "src/devices/bus/qbus/pc11.h",
		MAME_DIR .. "src/devices/bus/qbus/qbus.cpp",
		MAME_DIR .. "src/devices/bus/qbus/qbus.h",
		MAME_DIR .. "src/devices/bus/qbus/qg640.cpp",
		MAME_DIR .. "src/devices/bus/qbus/qg640.h",
		MAME_DIR .. "src/devices/bus/qbus/qtx.cpp",
		MAME_DIR .. "src/devices/bus/qbus/qtx.h",
		MAME_DIR .. "src/devices/bus/qbus/tdl12.cpp",
		MAME_DIR .. "src/devices/bus/qbus/tdl12.h",
		MAME_DIR .. "src/devices/bus/qbus/uknc_kmd.cpp",
		MAME_DIR .. "src/devices/bus/qbus/uknc_kmd.h",
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
--@src/devices/bus/spectrum/exp.h,BUSES["SPECTRUM"] = true
---------------------------------------------------

if (BUSES["SPECTRUM"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/spectrum/exp.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/exp.h",
		MAME_DIR .. "src/devices/bus/spectrum/beta.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/beta.h",
		MAME_DIR .. "src/devices/bus/spectrum/beta128.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/beta128.h",
		MAME_DIR .. "src/devices/bus/spectrum/d40.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/d40.h",
		MAME_DIR .. "src/devices/bus/spectrum/intf1.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/intf1.h",
		MAME_DIR .. "src/devices/bus/spectrum/intf2.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/intf2.h",
		MAME_DIR .. "src/devices/bus/spectrum/floppyone.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/floppyone.h",
		MAME_DIR .. "src/devices/bus/spectrum/fuller.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/fuller.h",
		MAME_DIR .. "src/devices/bus/spectrum/kempjoy.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/kempjoy.h",
		MAME_DIR .. "src/devices/bus/spectrum/kempdisc.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/kempdisc.h",
		MAME_DIR .. "src/devices/bus/spectrum/logitek.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/logitek.h",
		MAME_DIR .. "src/devices/bus/spectrum/lprint.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/lprint.h",
		MAME_DIR .. "src/devices/bus/spectrum/melodik.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/melodik.h",
		MAME_DIR .. "src/devices/bus/spectrum/mface.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/mface.h",
		MAME_DIR .. "src/devices/bus/spectrum/mgt.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/mgt.h",
		MAME_DIR .. "src/devices/bus/spectrum/mikroplus.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/mikroplus.h",
		MAME_DIR .. "src/devices/bus/spectrum/mpoker.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/mpoker.h",
		MAME_DIR .. "src/devices/bus/spectrum/musicmachine.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/musicmachine.h",
		MAME_DIR .. "src/devices/bus/spectrum/opus.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/opus.h",
		MAME_DIR .. "src/devices/bus/spectrum/plus2test.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/plus2test.h",
		MAME_DIR .. "src/devices/bus/spectrum/protek.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/protek.h",
		MAME_DIR .. "src/devices/bus/spectrum/sdi.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/sdi.h",
		MAME_DIR .. "src/devices/bus/spectrum/sixword.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/sixword.h",
		MAME_DIR .. "src/devices/bus/spectrum/speccydos.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/speccydos.h",
		MAME_DIR .. "src/devices/bus/spectrum/specdrum.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/specdrum.h",
		MAME_DIR .. "src/devices/bus/spectrum/specmate.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/specmate.h",
		MAME_DIR .. "src/devices/bus/spectrum/uslot.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/uslot.h",
		MAME_DIR .. "src/devices/bus/spectrum/usource.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/usource.h",
		MAME_DIR .. "src/devices/bus/spectrum/uspeech.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/uspeech.h",
		MAME_DIR .. "src/devices/bus/spectrum/vtx5000.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/vtx5000.h",
		MAME_DIR .. "src/devices/bus/spectrum/wafa.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/wafa.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/spectrum/zxbus.h,BUSES["ZXBUS"] = true
---------------------------------------------------

if (BUSES["ZXBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/spectrum/zxbus.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/zxbus.h",
		MAME_DIR .. "src/devices/bus/spectrum/nemoide.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/nemoide.h",
		MAME_DIR .. "src/devices/bus/spectrum/neogs.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/neogs.h",
		MAME_DIR .. "src/devices/bus/spectrum/smuc.cpp",
		MAME_DIR .. "src/devices/bus/spectrum/smuc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/uts_kbd/uts_kbd.h,BUSES["UTS_KBD"] = true
---------------------------------------------------

if (BUSES["UTS_KBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/uts_kbd/400kbd.cpp",
		MAME_DIR .. "src/devices/bus/uts_kbd/400kbd.h",
		MAME_DIR .. "src/devices/bus/uts_kbd/extw.cpp",
		MAME_DIR .. "src/devices/bus/uts_kbd/extw.h",
		MAME_DIR .. "src/devices/bus/uts_kbd/uts_kbd.cpp",
		MAME_DIR .. "src/devices/bus/uts_kbd/uts_kbd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/vsmile/vsmile_ctrl.h,BUSES["VSMILE"] = true
--@src/devices/bus/vsmile/vsmile_slot.h,BUSES["VSMILE"] = true
---------------------------------------------------

if (BUSES["VSMILE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/vsmile/vsmile_ctrl.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/vsmile_ctrl.h",
		MAME_DIR .. "src/devices/bus/vsmile/pad.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/pad.h",
		MAME_DIR .. "src/devices/bus/vsmile/mat.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/mat.h",
		MAME_DIR .. "src/devices/bus/vsmile/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/keyboard.h",
		MAME_DIR .. "src/devices/bus/vsmile/vsmile_slot.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/vsmile_slot.h",
		MAME_DIR .. "src/devices/bus/vsmile/rom.cpp",
		MAME_DIR .. "src/devices/bus/vsmile/rom.h",
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
		MAME_DIR .. "src/devices/bus/vtech/memexp/rtty.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/rtty.h",
		MAME_DIR .. "src/devices/bus/vtech/memexp/sdloader.cpp",
		MAME_DIR .. "src/devices/bus/vtech/memexp/sdloader.h",
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
		MAME_DIR .. "src/devices/bus/vtech/ioexp/lpen.cpp",
		MAME_DIR .. "src/devices/bus/vtech/ioexp/lpen.h",
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
--@src/devices/bus/psx/parallel.h,BUSES["PSX_PARALLEL"] = true
---------------------------------------------------

if (BUSES["PSX_PARALLEL"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psx/parallel.cpp",
		MAME_DIR .. "src/devices/bus/psx/parallel.h",
		MAME_DIR .. "src/devices/bus/psx/gamebooster.cpp",
		MAME_DIR .. "src/devices/bus/psx/gamebooster.h",
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
--@src/devices/bus/cgenie/expansion/expansion.h,BUSES["CGENIE_EXPANSION"] = true
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
--@src/devices/bus/cgenie/parallel/parallel.h,BUSES["CGENIE_PARALLEL"] = true
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

---------------------------------------------------
--
--@src/devices/bus/m5/slot.h,BUSES["M5"] = true
---------------------------------------------------
if (BUSES["M5"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/m5/slot.cpp",
		MAME_DIR .. "src/devices/bus/m5/slot.h",
		MAME_DIR .. "src/devices/bus/m5/rom.cpp",
		MAME_DIR .. "src/devices/bus/m5/rom.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/newbrain/exp.h,BUSES["NEWBRAIN"] = true
---------------------------------------------------

if (BUSES["NEWBRAIN"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/newbrain/exp.cpp",
		MAME_DIR .. "src/devices/bus/newbrain/exp.h",
		MAME_DIR .. "src/devices/bus/newbrain/eim.cpp",
		MAME_DIR .. "src/devices/bus/newbrain/eim.h",
		MAME_DIR .. "src/devices/bus/newbrain/fdc.cpp",
		MAME_DIR .. "src/devices/bus/newbrain/fdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/svi3x8/expander/expander.h,BUSES["SVI_EXPANDER"] = true
---------------------------------------------------

if (BUSES["SVI_EXPANDER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/svi3x8/expander/expander.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/expander.h",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/modules.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/modules.h",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv601.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv601.h",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv602.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv602.h",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv603.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/expander/sv603.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/svi3x8/slot/slot.h,BUSES["SVI_SLOT"] = true
---------------------------------------------------

if (BUSES["SVI_SLOT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/svi3x8/slot/slot.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/slot.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/cards.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/cards.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv801.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv801.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv802.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv802.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv803.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv803.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv805.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv805.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv806.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv806.h",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv807.cpp",
		MAME_DIR .. "src/devices/bus/svi3x8/slot/sv807.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/hp80_io/hp80_io.h,BUSES["HP80_IO"] = true
---------------------------------------------------

if (BUSES["HP80_IO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hp80_io/hp80_io.cpp",
		MAME_DIR .. "src/devices/bus/hp80_io/hp80_io.h",
		MAME_DIR .. "src/devices/bus/hp80_io/82900.cpp",
		MAME_DIR .. "src/devices/bus/hp80_io/82900.h",
		MAME_DIR .. "src/devices/bus/hp80_io/82937.cpp",
		MAME_DIR .. "src/devices/bus/hp80_io/82937.h",
		MAME_DIR .. "src/devices/bus/hp80_io/82939.cpp",
		MAME_DIR .. "src/devices/bus/hp80_io/82939.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/hp9845_io/hp9845_io.h,BUSES["HP9845_IO"] = true
---------------------------------------------------

if (BUSES["HP9845_IO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9845_io.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9845_io.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/98032.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/98032.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/98034.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/98034.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/98035.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/98035.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/98036.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/98036.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/98046.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/98046.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9871.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9871.h",
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9885.cpp",
		MAME_DIR .. "src/devices/bus/hp9845_io/hp9885.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/hp_ipc_io/hp_ipc_io.h,BUSES["HP_IPC_IO"] = true
---------------------------------------------------

if (BUSES["HP_IPC_IO"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/hp_ipc_io/hp_ipc_io.cpp",
		MAME_DIR .. "src/devices/bus/hp_ipc_io/hp_ipc_io.h",
		MAME_DIR .. "src/devices/bus/hp_ipc_io/82919.cpp",
		MAME_DIR .. "src/devices/bus/hp_ipc_io/82919.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/compis/graphics.h,BUSES["COMPIS_GRAPHICS"] = true
---------------------------------------------------

if (BUSES["COMPIS_GRAPHICS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/compis/graphics.cpp",
		MAME_DIR .. "src/devices/bus/compis/graphics.h",
		MAME_DIR .. "src/devices/bus/compis/hrg.cpp",
		MAME_DIR .. "src/devices/bus/compis/hrg.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pc1512/mouse.h,BUSES["PC1512"] = true
---------------------------------------------------

if (BUSES["PC1512"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc1512/mouse.cpp",
		MAME_DIR .. "src/devices/bus/pc1512/mouse.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/cbus/pc9801_cbus.h,BUSES["CBUS"] = true
---------------------------------------------------

if (BUSES["CBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/cbus/amd98.cpp",
		MAME_DIR .. "src/devices/bus/cbus/amd98.h",
		MAME_DIR .. "src/devices/bus/cbus/mpu_pc98.cpp",
		MAME_DIR .. "src/devices/bus/cbus/mpu_pc98.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_26.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_26.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_55.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_55.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_86.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_86.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_118.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_118.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_cbus.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_cbus.h",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_snd.cpp",
		MAME_DIR .. "src/devices/bus/cbus/pc9801_snd.h",
		MAME_DIR .. "src/devices/bus/cbus/sb16_ct2720.cpp",
		MAME_DIR .. "src/devices/bus/cbus/sb16_ct2720.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/psi_kbd/psi_kbd.h,BUSES["PSI_KEYBOARD"] = true
---------------------------------------------------

if (BUSES["PSI_KEYBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/psi_kbd/psi_kbd.cpp",
		MAME_DIR .. "src/devices/bus/psi_kbd/psi_kbd.h",
		MAME_DIR .. "src/devices/bus/psi_kbd/ergoline.cpp",
		MAME_DIR .. "src/devices/bus/psi_kbd/ergoline.h",
		MAME_DIR .. "src/devices/bus/psi_kbd/hle.cpp",
		MAME_DIR .. "src/devices/bus/psi_kbd/hle.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/interpro/sr/sr.h,BUSES["INTERPRO_SR"] = true
---------------------------------------------------

if (BUSES["INTERPRO_SR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/interpro/sr/sr.cpp",
		MAME_DIR .. "src/devices/bus/interpro/sr/sr.h",
		MAME_DIR .. "src/devices/bus/interpro/sr/sr_cards.cpp",
		MAME_DIR .. "src/devices/bus/interpro/sr/sr_cards.h",
		MAME_DIR .. "src/devices/bus/interpro/sr/gt.cpp",
		MAME_DIR .. "src/devices/bus/interpro/sr/gt.h",
		MAME_DIR .. "src/devices/bus/interpro/sr/edge.cpp",
		MAME_DIR .. "src/devices/bus/interpro/sr/edge.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/interpro/keyboard/keyboard.h,BUSES["INTERPRO_KEYBOARD"] = true
---------------------------------------------------

if (BUSES["INTERPRO_KEYBOARD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/interpro/keyboard/keyboard.cpp",
		MAME_DIR .. "src/devices/bus/interpro/keyboard/keyboard.h",
		MAME_DIR .. "src/devices/bus/interpro/keyboard/hle.cpp",
		MAME_DIR .. "src/devices/bus/interpro/keyboard/hle.h",
		MAME_DIR .. "src/devices/bus/interpro/keyboard/lle.cpp",
		MAME_DIR .. "src/devices/bus/interpro/keyboard/lle.h"
	}
end

---------------------------------------------------
--
--@src/devices/bus/interpro/mouse/mouse.h,BUSES["INTERPRO_MOUSE"] = true
---------------------------------------------------

if (BUSES["INTERPRO_MOUSE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/interpro/mouse/mouse.cpp",
		MAME_DIR .. "src/devices/bus/interpro/mouse/mouse.h"
	}
end

---------------------------------------------------
--
--@src/devices/bus/einstein/pipe/pipe.h,BUSES["TATUNG_PIPE"] = true
---------------------------------------------------

if (BUSES["TATUNG_PIPE"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/einstein/pipe/pipe.cpp",
		MAME_DIR .. "src/devices/bus/einstein/pipe/pipe.h",
		MAME_DIR .. "src/devices/bus/einstein/pipe/silicon_disc.cpp",
		MAME_DIR .. "src/devices/bus/einstein/pipe/silicon_disc.h",
		MAME_DIR .. "src/devices/bus/einstein/pipe/speculator.cpp",
		MAME_DIR .. "src/devices/bus/einstein/pipe/speculator.h",
		MAME_DIR .. "src/devices/bus/einstein/pipe/tk02.cpp",
		MAME_DIR .. "src/devices/bus/einstein/pipe/tk02.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/einstein/userport/userport.h,BUSES["EINSTEIN_USERPORT"] = true
---------------------------------------------------

if (BUSES["EINSTEIN_USERPORT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/einstein/userport/userport.cpp",
		MAME_DIR .. "src/devices/bus/einstein/userport/userport.h",
		MAME_DIR .. "src/devices/bus/einstein/userport/mouse.cpp",
		MAME_DIR .. "src/devices/bus/einstein/userport/mouse.h",
		MAME_DIR .. "src/devices/bus/einstein/userport/speech.cpp",
		MAME_DIR .. "src/devices/bus/einstein/userport/speech.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/tmc600/euro.h,BUSES["TMC600"] = true
---------------------------------------------------

if (BUSES["TMC600"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/tmc600/euro.cpp",
		MAME_DIR .. "src/devices/bus/tmc600/euro.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/multibus/multibus.h,BUSES["MULTIBUS"] = true
---------------------------------------------------

if (BUSES["MULTIBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/multibus/multibus.cpp",
		MAME_DIR .. "src/devices/bus/multibus/multibus.h",
		MAME_DIR .. "src/devices/bus/multibus/isbc202.cpp",
		MAME_DIR .. "src/devices/bus/multibus/isbc202.h",
		MAME_DIR .. "src/devices/bus/multibus/isbc8024.cpp",
		MAME_DIR .. "src/devices/bus/multibus/isbc8024.h",
		MAME_DIR .. "src/devices/bus/multibus/cpuap.cpp",
		MAME_DIR .. "src/devices/bus/multibus/cpuap.h",
		MAME_DIR .. "src/devices/bus/multibus/serad.cpp",
		MAME_DIR .. "src/devices/bus/multibus/serad.h",
		MAME_DIR .. "src/devices/bus/multibus/labtam_3232.cpp",
		MAME_DIR .. "src/devices/bus/multibus/labtam_3232.h",
		MAME_DIR .. "src/devices/bus/multibus/labtam_vducom.cpp",
		MAME_DIR .. "src/devices/bus/multibus/labtam_vducom.h",
		MAME_DIR .. "src/devices/bus/multibus/labtam_z80sbc.cpp",
		MAME_DIR .. "src/devices/bus/multibus/labtam_z80sbc.h",
		MAME_DIR .. "src/devices/bus/multibus/robotron_k7070.cpp",
		MAME_DIR .. "src/devices/bus/multibus/robotron_k7070.h",
		MAME_DIR .. "src/devices/bus/multibus/robotron_k7071.cpp",
		MAME_DIR .. "src/devices/bus/multibus/robotron_k7071.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/rtpc/kbd_con.h,BUSES["RTPC_KBD"] = true
---------------------------------------------------

if (BUSES["RTPC_KBD"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/rtpc/kbd_con.cpp",
		MAME_DIR .. "src/devices/bus/rtpc/kbd_con.h",
		MAME_DIR .. "src/devices/bus/rtpc/kbd.cpp",
		MAME_DIR .. "src/devices/bus/rtpc/kbd.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/saitek_osa/expansion.h,BUSES["SAITEK_OSA"] = true
---------------------------------------------------

if (BUSES["SAITEK_OSA"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/saitek_osa/expansion.cpp",
		MAME_DIR .. "src/devices/bus/saitek_osa/expansion.h",
		MAME_DIR .. "src/devices/bus/saitek_osa/maestro.cpp",
		MAME_DIR .. "src/devices/bus/saitek_osa/maestro.h",
		MAME_DIR .. "src/devices/bus/saitek_osa/maestroa.cpp",
		MAME_DIR .. "src/devices/bus/saitek_osa/maestroa.h",
		MAME_DIR .. "src/devices/bus/saitek_osa/sparc.cpp",
		MAME_DIR .. "src/devices/bus/saitek_osa/sparc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/samcoupe/drive/drive.h,BUSES["SAMCOUPE_DRIVE_PORT"] = true
---------------------------------------------------

if (BUSES["SAMCOUPE_DRIVE_PORT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/samcoupe/drive/drive.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/drive.h",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/modules.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/modules.h",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/atom.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/atom.h",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/floppy.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/drive/floppy.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/samcoupe/expansion/expansion.h,BUSES["SAMCOUPE_EXPANSION"] = true
---------------------------------------------------

if (BUSES["SAMCOUPE_EXPANSION"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/expansion.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/expansion.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/modules.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/modules.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/blue_sampler.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/blue_sampler.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/dallas.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/dallas.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/onemeg.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/onemeg.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sambus.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sambus.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sdide.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sdide.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sid.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/sid.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/spi.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/spi.h",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/voicebox.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/expansion/voicebox.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/samcoupe/mouse/mouse.h,BUSES["SAMCOUPE_MOUSE_PORT"] = true
---------------------------------------------------

if (BUSES["SAMCOUPE_MOUSE_PORT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/mouseport.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/mouseport.h",
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/modules.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/modules.h",
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/mouse.cpp",
		MAME_DIR .. "src/devices/bus/samcoupe/mouse/mouse.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/thomson/extension.h,BUSES["THOMSON"] = true
---------------------------------------------------

if (BUSES["THOMSON"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/thomson/extension.cpp",
		MAME_DIR .. "src/devices/bus/thomson/extension.h",
		MAME_DIR .. "src/devices/bus/thomson/cc90_232.cpp",
		MAME_DIR .. "src/devices/bus/thomson/cc90_232.h",
		MAME_DIR .. "src/devices/bus/thomson/cd90_015.cpp",
		MAME_DIR .. "src/devices/bus/thomson/cd90_015.h",
		MAME_DIR .. "src/devices/bus/thomson/cq90_028.cpp",
		MAME_DIR .. "src/devices/bus/thomson/cq90_028.h",
		MAME_DIR .. "src/devices/bus/thomson/cd90_351.cpp",
		MAME_DIR .. "src/devices/bus/thomson/cd90_351.h",
		MAME_DIR .. "src/devices/bus/thomson/cd90_640.cpp",
		MAME_DIR .. "src/devices/bus/thomson/cd90_640.h",
		MAME_DIR .. "src/devices/bus/thomson/md90_120.cpp",
		MAME_DIR .. "src/devices/bus/thomson/md90_120.h",
		MAME_DIR .. "src/devices/bus/thomson/midipak.cpp",
		MAME_DIR .. "src/devices/bus/thomson/midipak.h",
		MAME_DIR .. "src/devices/bus/thomson/nanoreseau.cpp",
		MAME_DIR .. "src/devices/bus/thomson/nanoreseau.h",
		MAME_DIR .. "src/devices/bus/thomson/rf57_932.cpp",
		MAME_DIR .. "src/devices/bus/thomson/rf57_932.h",
		MAME_DIR .. "src/devices/bus/thomson/speech.cpp",
		MAME_DIR .. "src/devices/bus/thomson/speech.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pc8801/pc8801_exp.h,BUSES["PC8801"] = true
---------------------------------------------------

if (BUSES["PC8801"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pc8801/gsx8800.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/gsx8800.h",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_23.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_23.h",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_31.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_31.h",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_exp.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/pc8801_exp.h",
		MAME_DIR .. "src/devices/bus/pc8801/pcg8100.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/pcg8100.h",
		MAME_DIR .. "src/devices/bus/pc8801/jmbx1.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/jmbx1.h",
		MAME_DIR .. "src/devices/bus/pc8801/hmb20.cpp",
		MAME_DIR .. "src/devices/bus/pc8801/hmb20.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/jvs/jvs.h,BUSES["JVS"] = true
---------------------------------------------------

if (BUSES["JVS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/jvs/cyberlead.cpp",
		MAME_DIR .. "src/devices/bus/jvs/cyberlead.h",
		MAME_DIR .. "src/devices/bus/jvs/jvs.cpp",
		MAME_DIR .. "src/devices/bus/jvs/jvs.h",
		MAME_DIR .. "src/devices/bus/jvs/jvshle.cpp",
		MAME_DIR .. "src/devices/bus/jvs/jvshle.h",
		MAME_DIR .. "src/devices/bus/jvs/namcoio.cpp",
		MAME_DIR .. "src/devices/bus/jvs/namcoio.h",
	}

	dependency {
		{ MAME_DIR .. "src/devices/bus/jvs/cyberlead.cpp", GEN_DIR .. "emu/layout/cyberlead.lh" },
	}

	custombuildtask {
		layoutbuildtask("emu/layout", "cyberlead"),
	}
end


---------------------------------------------------
--
--@src/devices/bus/mtu130/board.h,BUSES["MTU130"] = true
---------------------------------------------------

if (BUSES["MTU130"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mtu130/board.cpp",
		MAME_DIR .. "src/devices/bus/mtu130/board.h",
		MAME_DIR .. "src/devices/bus/mtu130/datamover.cpp",
		MAME_DIR .. "src/devices/bus/mtu130/datamover.h",
	}
 end

---------------------------------------------------
--
--@src/devices/bus/nabupc/option.h,BUSES["NABU"] = true
---------------------------------------------------
if (BUSES["NABU"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/nabupc/adapter.cpp",
		MAME_DIR .. "src/devices/bus/nabupc/adapter.h",
		MAME_DIR .. "src/devices/bus/nabupc/fdc.cpp",
		MAME_DIR .. "src/devices/bus/nabupc/fdc.h",
		MAME_DIR .. "src/devices/bus/nabupc/hdd.cpp",
		MAME_DIR .. "src/devices/bus/nabupc/hdd.h",
		MAME_DIR .. "src/devices/bus/nabupc/option.cpp",
		MAME_DIR .. "src/devices/bus/nabupc/option.h",
		MAME_DIR .. "src/devices/bus/nabupc/rs232.cpp",
		MAME_DIR .. "src/devices/bus/nabupc/rs232.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/mc68000/sysbus.h,BUSES["MC68000_SYSBUS"] = true
---------------------------------------------------

if (BUSES["MC68000_SYSBUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/mc68000/sysbus.cpp",
		MAME_DIR .. "src/devices/bus/mc68000/sysbus.h",
		MAME_DIR .. "src/devices/bus/mc68000/cards.cpp",
		MAME_DIR .. "src/devices/bus/mc68000/cards.h",
		MAME_DIR .. "src/devices/bus/mc68000/floppy.cpp",
		MAME_DIR .. "src/devices/bus/mc68000/floppy.h",
		MAME_DIR .. "src/devices/bus/mc68000/ram.cpp",
		MAME_DIR .. "src/devices/bus/mc68000/ram.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/waveblaster/waveblaster.h,BUSES["WAVEBLASTER"] = true
---------------------------------------------------

if (BUSES["WAVEBLASTER"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/waveblaster/waveblaster.cpp",
		MAME_DIR .. "src/devices/bus/waveblaster/waveblaster.h",
		MAME_DIR .. "src/devices/bus/waveblaster/omniwave.cpp",
		MAME_DIR .. "src/devices/bus/waveblaster/omniwave.h",
		MAME_DIR .. "src/devices/bus/waveblaster/db50xg.cpp",
		MAME_DIR .. "src/devices/bus/waveblaster/db50xg.h",
		MAME_DIR .. "src/devices/bus/waveblaster/db60xg.cpp",
		MAME_DIR .. "src/devices/bus/waveblaster/db60xg.h",
		MAME_DIR .. "src/devices/bus/waveblaster/wg130.cpp",
		MAME_DIR .. "src/devices/bus/waveblaster/wg130.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/pci/pci_slot.h,BUSES["PCI"] = true
---------------------------------------------------

if (BUSES["PCI"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/pci/pci_slot.cpp",
		MAME_DIR .. "src/devices/bus/pci/pci_slot.h",
		MAME_DIR .. "src/devices/bus/pci/aha2940au.cpp",
		MAME_DIR .. "src/devices/bus/pci/aha2940au.h",
		MAME_DIR .. "src/devices/bus/pci/audiowerk2.cpp",
		MAME_DIR .. "src/devices/bus/pci/audiowerk2.h",
		MAME_DIR .. "src/devices/bus/pci/clgd5446.cpp",
		MAME_DIR .. "src/devices/bus/pci/clgd5446.h",
		MAME_DIR .. "src/devices/bus/pci/clgd546x_laguna.cpp",
		MAME_DIR .. "src/devices/bus/pci/clgd546x_laguna.h",
		MAME_DIR .. "src/devices/bus/pci/ds2416.cpp",
		MAME_DIR .. "src/devices/bus/pci/ds2416.h",
		MAME_DIR .. "src/devices/bus/pci/ess_maestro.cpp",
		MAME_DIR .. "src/devices/bus/pci/ess_maestro.h",
		MAME_DIR .. "src/devices/bus/pci/geforce.cpp",
		MAME_DIR .. "src/devices/bus/pci/geforce.h",
		MAME_DIR .. "src/devices/bus/pci/mga2064w.cpp",
		MAME_DIR .. "src/devices/bus/pci/mga2064w.h",
		MAME_DIR .. "src/devices/bus/pci/ncr53c825.cpp",
		MAME_DIR .. "src/devices/bus/pci/ncr53c825.h",
		MAME_DIR .. "src/devices/bus/pci/neon250.cpp",
		MAME_DIR .. "src/devices/bus/pci/neon250.h",
		MAME_DIR .. "src/devices/bus/pci/oti_spitfire.cpp",
		MAME_DIR .. "src/devices/bus/pci/oti_spitfire.h",
		MAME_DIR .. "src/devices/bus/pci/opti82c861.cpp",
		MAME_DIR .. "src/devices/bus/pci/opti82c861.h",
		MAME_DIR .. "src/devices/bus/pci/pdc20262.cpp",
		MAME_DIR .. "src/devices/bus/pci/pdc20262.h",
		MAME_DIR .. "src/devices/bus/pci/promotion.cpp",
		MAME_DIR .. "src/devices/bus/pci/promotion.h",
		MAME_DIR .. "src/devices/bus/pci/riva128.cpp",
		MAME_DIR .. "src/devices/bus/pci/riva128.h",
		MAME_DIR .. "src/devices/bus/pci/rivatnt.cpp",
		MAME_DIR .. "src/devices/bus/pci/rivatnt.h",
		MAME_DIR .. "src/devices/bus/pci/rtl8029as_pci.cpp",
		MAME_DIR .. "src/devices/bus/pci/rtl8029as_pci.h",
		MAME_DIR .. "src/devices/bus/pci/rtl8139_pci.cpp",
		MAME_DIR .. "src/devices/bus/pci/rtl8139_pci.h",
		MAME_DIR .. "src/devices/bus/pci/sis6326.cpp",
		MAME_DIR .. "src/devices/bus/pci/sis6326.h",
		MAME_DIR .. "src/devices/bus/pci/sonicvibes.cpp",
		MAME_DIR .. "src/devices/bus/pci/sonicvibes.h",
		MAME_DIR .. "src/devices/bus/pci/sw1000xg.cpp",
		MAME_DIR .. "src/devices/bus/pci/sw1000xg.h",
		MAME_DIR .. "src/devices/bus/pci/virge_pci.cpp",
		MAME_DIR .. "src/devices/bus/pci/virge_pci.h",
		MAME_DIR .. "src/devices/bus/pci/vision.cpp",
		MAME_DIR .. "src/devices/bus/pci/vision.h",
		MAME_DIR .. "src/devices/bus/pci/vt6306.cpp",
		MAME_DIR .. "src/devices/bus/pci/vt6306.h",
		MAME_DIR .. "src/devices/bus/pci/wd9710_pci.cpp",
		MAME_DIR .. "src/devices/bus/pci/wd9710_pci.h",
		MAME_DIR .. "src/devices/bus/pci/ymp21.cpp",
		MAME_DIR .. "src/devices/bus/pci/ymp21.h",
		MAME_DIR .. "src/devices/bus/pci/zr36057.cpp",
		MAME_DIR .. "src/devices/bus/pci/zr36057.h",
	}
end


---------------------------------------------------
--
--@src/devices/bus/plg1x0/plg1x0.h,BUSES["PLG1X0"] = true
---------------------------------------------------

if (BUSES["PLG1X0"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/plg1x0/plg1x0.cpp",
		MAME_DIR .. "src/devices/bus/plg1x0/plg1x0.h",
		MAME_DIR .. "src/devices/bus/plg1x0/plg100-vl.cpp",
		MAME_DIR .. "src/devices/bus/plg1x0/plg100-vl.h",
		MAME_DIR .. "src/devices/bus/plg1x0/plg150-ap.cpp",
		MAME_DIR .. "src/devices/bus/plg1x0/plg150-ap.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/heathzenith/h89/h89bus.h,BUSES["H89BUS"] = true
---------------------------------------------------

if (BUSES["H89BUS"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/heathzenith/h89/cdr_fdc_880h.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/cdr_fdc_880h.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h89bus.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h89bus.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h17_fdc.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h17_fdc.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h_88_3.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h_88_3.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h_88_5.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/h_88_5.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/mms77316_fdc.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/mms77316_fdc.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/sigmasoft_sound.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/sigmasoft_sound.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/sigmasoft_parallel_port.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/sigmasoft_parallel_port.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/we_pullup.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/we_pullup.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/z_89_11.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/z_89_11.h",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/z37_fdc.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/z37_fdc.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/heathzenith/h89/intr_cntrl.h,BUSES["HEATH_INTR_SOCKET"] = true
---------------------------------------------------

if (BUSES["HEATH_INTR_SOCKET"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/heathzenith/h89/intr_cntrl.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h89/intr_cntrl.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/heathzenith/h19/tlb.h,BUSES["HEATH_TLB_CONNECTOR"] = true
---------------------------------------------------

if (BUSES["HEATH_TLB_CONNECTOR"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/heathzenith/h19/tlb.cpp",
		MAME_DIR .. "src/devices/bus/heathzenith/h19/tlb.h",
	}
end

---------------------------------------------------
--
--@src/devices/bus/amiga/cpuslot/cpuslot.h,BUSES["AMIGA_CPUSLOT"] = true
---------------------------------------------------

if (BUSES["AMIGA_CPUSLOT"]~=null) then
	files {
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/cpuslot.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/cpuslot.h",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/cards.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/cards.h",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/a570.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/a570.h",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/a590.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/a590.h",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/action_replay.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/action_replay.h",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/megamix500.cpp",
		MAME_DIR .. "src/devices/bus/amiga/cpuslot/megamix500.h",
	}
end
