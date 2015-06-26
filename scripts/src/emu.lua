-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

project ("emu")
targetsubdir(_OPTIONS["target"] .."_" .. _OPTIONS["subtarget"])
uuid ("e6fa15e4-a354-4526-acef-13c8e80fcacf")
kind (LIBTYPE)
options {
	"ForceCPP",
}

-- MEWUI
dofile("mewui.lua")
includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/emu",
	MAME_DIR .. "src/lib",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
	GEN_DIR  .. "emu",
	GEN_DIR  .. "emu/layout",
}
if _OPTIONS["with-bundled-expat"] then
	includedirs {
		MAME_DIR .. "3rdparty/expat/lib",
	}
end
if _OPTIONS["with-bundled-zlib"] then
	includedirs {
		MAME_DIR .. "3rdparty/zlib",
	}
end
if _OPTIONS["with-bundled-lua"] then
	includedirs {
		MAME_DIR .. "3rdparty/lua/src",
	}
end

files {
	MAME_DIR .. "src/emu/emu.h",
	MAME_DIR .. "src/emu/gamedrv.h",
	MAME_DIR .. "src/emu/hashfile.c",
	MAME_DIR .. "src/emu/hashfile.h",
	MAME_DIR .. "src/emu/addrmap.c",
	MAME_DIR .. "src/emu/addrmap.h",
	MAME_DIR .. "src/emu/attotime.c",
	MAME_DIR .. "src/emu/attotime.h",
	MAME_DIR .. "src/emu/audit.c",
	MAME_DIR .. "src/emu/audit.h",
	MAME_DIR .. "src/emu/cheat.c",
	MAME_DIR .. "src/emu/cheat.h",
	MAME_DIR .. "src/emu/clifront.c",
	MAME_DIR .. "src/emu/clifront.h",
	MAME_DIR .. "src/emu/cliopts.c",
	MAME_DIR .. "src/emu/cliopts.h",
	MAME_DIR .. "src/emu/config.c",
	MAME_DIR .. "src/emu/config.h",
	MAME_DIR .. "src/emu/crsshair.c",
	MAME_DIR .. "src/emu/crsshair.h",
	MAME_DIR .. "src/emu/debugger.c",
	MAME_DIR .. "src/emu/debugger.h",
	MAME_DIR .. "src/emu/devdelegate.c",
	MAME_DIR .. "src/emu/devdelegate.h",
	MAME_DIR .. "src/emu/devcb.c",
	MAME_DIR .. "src/emu/devcb.h",
	MAME_DIR .. "src/emu/devcpu.c",
	MAME_DIR .. "src/emu/devcpu.h",
	MAME_DIR .. "src/emu/devfind.c",
	MAME_DIR .. "src/emu/devfind.h",
	MAME_DIR .. "src/emu/device.c",
	MAME_DIR .. "src/emu/device.h",
	MAME_DIR .. "src/emu/didisasm.c",
	MAME_DIR .. "src/emu/didisasm.h",
	MAME_DIR .. "src/emu/diexec.c",
	MAME_DIR .. "src/emu/diexec.h",
	MAME_DIR .. "src/emu/digfx.c",
	MAME_DIR .. "src/emu/digfx.h",
	MAME_DIR .. "src/emu/diimage.c",
	MAME_DIR .. "src/emu/diimage.h",
	MAME_DIR .. "src/emu/dimemory.c",
	MAME_DIR .. "src/emu/dimemory.h",
	MAME_DIR .. "src/emu/dinetwork.c",
	MAME_DIR .. "src/emu/dinetwork.h",
	MAME_DIR .. "src/emu/dinvram.c",
	MAME_DIR .. "src/emu/dinvram.h",
	MAME_DIR .. "src/emu/dioutput.c",
	MAME_DIR .. "src/emu/dioutput.h",
	MAME_DIR .. "src/emu/dirtc.c",
	MAME_DIR .. "src/emu/dirtc.h",
	MAME_DIR .. "src/emu/diserial.c",
	MAME_DIR .. "src/emu/diserial.h",
	MAME_DIR .. "src/emu/dislot.c",
	MAME_DIR .. "src/emu/dislot.h",
	MAME_DIR .. "src/emu/disound.c",
	MAME_DIR .. "src/emu/disound.h",
	MAME_DIR .. "src/emu/dispatch.c",
	MAME_DIR .. "src/emu/dispatch.h",
	MAME_DIR .. "src/emu/distate.c",
	MAME_DIR .. "src/emu/distate.h",
	MAME_DIR .. "src/emu/divideo.c",
	MAME_DIR .. "src/emu/divideo.h",
	MAME_DIR .. "src/emu/drawgfx.c",
	MAME_DIR .. "src/emu/drawgfx.h",
	MAME_DIR .. "src/emu/drawgfxm.h",
	MAME_DIR .. "src/emu/driver.c",
	MAME_DIR .. "src/emu/driver.h",
	MAME_DIR .. "src/emu/drivenum.c",
	MAME_DIR .. "src/emu/drivenum.h",
	MAME_DIR .. "src/emu/emualloc.c",
	MAME_DIR .. "src/emu/emualloc.h",
	MAME_DIR .. "src/emu/emucore.c",
	MAME_DIR .. "src/emu/emucore.h",
	MAME_DIR .. "src/emu/emuopts.c",
	MAME_DIR .. "src/emu/emuopts.h",
	MAME_DIR .. "src/emu/emupal.c",
	MAME_DIR .. "src/emu/emupal.h",
	MAME_DIR .. "src/emu/fileio.c",
	MAME_DIR .. "src/emu/fileio.h",
	MAME_DIR .. "src/emu/hash.c",
	MAME_DIR .. "src/emu/hash.h",
	MAME_DIR .. "src/emu/image.c",
	MAME_DIR .. "src/emu/image.h",
	MAME_DIR .. "src/emu/info.c",
	MAME_DIR .. "src/emu/info.h",
	MAME_DIR .. "src/emu/input.c",
	MAME_DIR .. "src/emu/input.h",
	MAME_DIR .. "src/emu/ioport.c",
	MAME_DIR .. "src/emu/ioport.h",
	MAME_DIR .. "src/emu/inpttype.h",
	MAME_DIR .. "src/emu/luaengine.c",
	MAME_DIR .. "src/emu/luaengine.h",
	MAME_DIR .. "src/emu/mame.c",
	MAME_DIR .. "src/emu/mame.h",
	MAME_DIR .. "src/emu/machine.c",
	MAME_DIR .. "src/emu/machine.h",
	MAME_DIR .. "src/emu/mconfig.c",
	MAME_DIR .. "src/emu/mconfig.h",
	MAME_DIR .. "src/emu/memarray.c",
	MAME_DIR .. "src/emu/memarray.h",
	MAME_DIR .. "src/emu/memory.c",
	MAME_DIR .. "src/emu/memory.h",
	MAME_DIR .. "src/emu/network.c",
	MAME_DIR .. "src/emu/network.h",
	MAME_DIR .. "src/emu/parameters.c",
	MAME_DIR .. "src/emu/parameters.h",
	MAME_DIR .. "src/emu/output.c",
	MAME_DIR .. "src/emu/output.h",
	MAME_DIR .. "src/emu/render.c",
	MAME_DIR .. "src/emu/render.h",
	MAME_DIR .. "src/emu/rendfont.c",
	MAME_DIR .. "src/emu/rendfont.h",
	MAME_DIR .. "src/emu/rendlay.c",
	MAME_DIR .. "src/emu/rendlay.h",
	MAME_DIR .. "src/emu/rendutil.c",
	MAME_DIR .. "src/emu/rendutil.h",
	MAME_DIR .. "src/emu/romload.c",
	MAME_DIR .. "src/emu/romload.h",
	MAME_DIR .. "src/emu/save.c",
	MAME_DIR .. "src/emu/save.h",
	MAME_DIR .. "src/emu/schedule.c",
	MAME_DIR .. "src/emu/schedule.h",
	MAME_DIR .. "src/emu/screen.c",
	MAME_DIR .. "src/emu/screen.h",
	MAME_DIR .. "src/emu/softlist.c",
	MAME_DIR .. "src/emu/softlist.h",
	MAME_DIR .. "src/emu/sound.c",
	MAME_DIR .. "src/emu/sound.h",
	MAME_DIR .. "src/emu/speaker.c",
	MAME_DIR .. "src/emu/speaker.h",
	MAME_DIR .. "src/emu/sprite.c",
	MAME_DIR .. "src/emu/sprite.h",
	MAME_DIR .. "src/emu/tilemap.c",
	MAME_DIR .. "src/emu/tilemap.h",
	MAME_DIR .. "src/emu/timer.c",
	MAME_DIR .. "src/emu/timer.h",
	MAME_DIR .. "src/emu/uiinput.c",
	MAME_DIR .. "src/emu/uiinput.h",
	MAME_DIR .. "src/emu/ui/ui.c",
	MAME_DIR .. "src/emu/ui/ui.h",
	MAME_DIR .. "src/emu/ui/devctrl.h",
	MAME_DIR .. "src/emu/ui/menu.c",
	MAME_DIR .. "src/emu/ui/menu.h",
	MAME_DIR .. "src/emu/ui/mainmenu.c",
	MAME_DIR .. "src/emu/ui/mainmenu.h",
	MAME_DIR .. "src/emu/ui/miscmenu.c",
	MAME_DIR .. "src/emu/ui/miscmenu.h",
	MAME_DIR .. "src/emu/ui/barcode.c",
	MAME_DIR .. "src/emu/ui/barcode.h",
	MAME_DIR .. "src/emu/ui/cheatopt.c",
	MAME_DIR .. "src/emu/ui/cheatopt.h",
	MAME_DIR .. "src/emu/ui/devopt.c",
	MAME_DIR .. "src/emu/ui/devopt.h",
	MAME_DIR .. "src/emu/ui/filemngr.c",
	MAME_DIR .. "src/emu/ui/filemngr.h",
	MAME_DIR .. "src/emu/ui/filesel.c",
	MAME_DIR .. "src/emu/ui/filesel.h",
	MAME_DIR .. "src/emu/ui/imgcntrl.c",
	MAME_DIR .. "src/emu/ui/imgcntrl.h",
	MAME_DIR .. "src/emu/ui/info.c",
	MAME_DIR .. "src/emu/ui/info.h",
	MAME_DIR .. "src/emu/ui/inputmap.c",
	MAME_DIR .. "src/emu/ui/inputmap.h",
	MAME_DIR .. "src/emu/ui/selgame.c",
	MAME_DIR .. "src/emu/ui/selgame.h",
	MAME_DIR .. "src/emu/ui/sliders.c",
	MAME_DIR .. "src/emu/ui/sliders.h",
	MAME_DIR .. "src/emu/ui/slotopt.c",
	MAME_DIR .. "src/emu/ui/slotopt.h",
	MAME_DIR .. "src/emu/ui/swlist.c",
	MAME_DIR .. "src/emu/ui/swlist.h",
	MAME_DIR .. "src/emu/ui/tapectrl.c",
	MAME_DIR .. "src/emu/ui/tapectrl.h",
	MAME_DIR .. "src/emu/ui/videoopt.c",
	MAME_DIR .. "src/emu/ui/videoopt.h",
	MAME_DIR .. "src/emu/ui/viewgfx.c",
	MAME_DIR .. "src/emu/ui/viewgfx.h",
	MAME_DIR .. "src/emu/validity.c",
	MAME_DIR .. "src/emu/validity.h",
	MAME_DIR .. "src/emu/video.c",
	MAME_DIR .. "src/emu/video.h",
	MAME_DIR .. "src/emu/rendersw.inc",
	MAME_DIR .. "src/emu/debug/debugcmd.c",
	MAME_DIR .. "src/emu/debug/debugcmd.h",
	MAME_DIR .. "src/emu/debug/debugcon.c",
	MAME_DIR .. "src/emu/debug/debugcon.h",
	MAME_DIR .. "src/emu/debug/debugcpu.c",
	MAME_DIR .. "src/emu/debug/debugcpu.h",
	MAME_DIR .. "src/emu/debug/debughlp.c",
	MAME_DIR .. "src/emu/debug/debughlp.h",
	MAME_DIR .. "src/emu/debug/debugvw.c",
	MAME_DIR .. "src/emu/debug/debugvw.h",
	MAME_DIR .. "src/emu/debug/dvdisasm.c",
	MAME_DIR .. "src/emu/debug/dvdisasm.h",
	MAME_DIR .. "src/emu/debug/dvmemory.c",
	MAME_DIR .. "src/emu/debug/dvmemory.h",
	MAME_DIR .. "src/emu/debug/dvbpoints.c",
	MAME_DIR .. "src/emu/debug/dvbpoints.h",
	MAME_DIR .. "src/emu/debug/dvwpoints.c",
	MAME_DIR .. "src/emu/debug/dvwpoints.h",
	MAME_DIR .. "src/emu/debug/dvstate.c",
	MAME_DIR .. "src/emu/debug/dvstate.h",
	MAME_DIR .. "src/emu/debug/dvtext.c",
	MAME_DIR .. "src/emu/debug/dvtext.h",
	MAME_DIR .. "src/emu/debug/express.c",
	MAME_DIR .. "src/emu/debug/express.h",
	MAME_DIR .. "src/emu/debug/textbuf.c",
	MAME_DIR .. "src/emu/debug/textbuf.h",
	MAME_DIR .. "src/emu/profiler.c",
	MAME_DIR .. "src/emu/profiler.h",
	MAME_DIR .. "src/emu/webengine.c",
	MAME_DIR .. "src/emu/webengine.h",
	MAME_DIR .. "src/emu/sound/filter.c",
	MAME_DIR .. "src/emu/sound/filter.h",
	MAME_DIR .. "src/emu/sound/flt_vol.c",
	MAME_DIR .. "src/emu/sound/flt_vol.h",
	MAME_DIR .. "src/emu/sound/flt_rc.c",
	MAME_DIR .. "src/emu/sound/flt_rc.h",
	MAME_DIR .. "src/emu/sound/wavwrite.c",
	MAME_DIR .. "src/emu/sound/wavwrite.h",
	MAME_DIR .. "src/emu/sound/samples.c",
	MAME_DIR .. "src/emu/sound/samples.h",
	MAME_DIR .. "src/emu/drivers/empty.c",
	MAME_DIR .. "src/emu/drivers/testcpu.c",
	MAME_DIR .. "src/emu/drivers/xtal.h",
	MAME_DIR .. "src/emu/machine/bcreader.c",
	MAME_DIR .. "src/emu/machine/bcreader.h",
	MAME_DIR .. "src/emu/machine/buffer.c",
	MAME_DIR .. "src/emu/machine/buffer.h",
	MAME_DIR .. "src/emu/machine/clock.c",
	MAME_DIR .. "src/emu/machine/clock.h",
	MAME_DIR .. "src/emu/machine/generic.c",
	MAME_DIR .. "src/emu/machine/generic.h",
	MAME_DIR .. "src/emu/machine/keyboard.c",
	MAME_DIR .. "src/emu/machine/keyboard.h",
	MAME_DIR .. "src/emu/machine/laserdsc.c",
	MAME_DIR .. "src/emu/machine/laserdsc.h",
	MAME_DIR .. "src/emu/machine/latch.c",
	MAME_DIR .. "src/emu/machine/latch.h",
	MAME_DIR .. "src/emu/machine/nvram.c",
	MAME_DIR .. "src/emu/machine/nvram.h",
	MAME_DIR .. "src/emu/machine/ram.c",
	MAME_DIR .. "src/emu/machine/ram.h",
	MAME_DIR .. "src/emu/machine/legscsi.c",
	MAME_DIR .. "src/emu/machine/legscsi.h",
	MAME_DIR .. "src/emu/machine/terminal.c",
	MAME_DIR .. "src/emu/machine/terminal.h",
	MAME_DIR .. "src/emu/imagedev/bitbngr.c",
	MAME_DIR .. "src/emu/imagedev/bitbngr.h",
	MAME_DIR .. "src/emu/imagedev/cassette.c",
	MAME_DIR .. "src/emu/imagedev/cassette.h",
	MAME_DIR .. "src/emu/imagedev/chd_cd.c",
	MAME_DIR .. "src/emu/imagedev/chd_cd.h",
	MAME_DIR .. "src/emu/imagedev/diablo.c",
	MAME_DIR .. "src/emu/imagedev/diablo.h",
	MAME_DIR .. "src/emu/imagedev/flopdrv.c",
	MAME_DIR .. "src/emu/imagedev/flopdrv.h",
	MAME_DIR .. "src/emu/imagedev/floppy.c",
	MAME_DIR .. "src/emu/imagedev/floppy.h",
	MAME_DIR .. "src/emu/imagedev/harddriv.c",
	MAME_DIR .. "src/emu/imagedev/harddriv.h",
	MAME_DIR .. "src/emu/imagedev/midiin.c",
	MAME_DIR .. "src/emu/imagedev/midiin.h",
	MAME_DIR .. "src/emu/imagedev/midiout.c",
	MAME_DIR .. "src/emu/imagedev/midiout.h",
	MAME_DIR .. "src/emu/imagedev/printer.c",
	MAME_DIR .. "src/emu/imagedev/printer.h",
	MAME_DIR .. "src/emu/imagedev/snapquik.c",
	MAME_DIR .. "src/emu/imagedev/snapquik.h",
	MAME_DIR .. "src/emu/video/generic.c",
	MAME_DIR .. "src/emu/video/generic.h",
	MAME_DIR .. "src/emu/video/resnet.c",
	MAME_DIR .. "src/emu/video/resnet.h",
	MAME_DIR .. "src/emu/video/rgbutil.h",
	MAME_DIR .. "src/emu/video/rgbgen.c",
	MAME_DIR .. "src/emu/video/rgbgen.h",
	MAME_DIR .. "src/emu/video/rgbsse.c",
	MAME_DIR .. "src/emu/video/rgbsse.h",
	MAME_DIR .. "src/emu/video/rgbvmx.c",
	MAME_DIR .. "src/emu/video/rgbvmx.h",
	MAME_DIR .. "src/emu/video/vector.c",
	MAME_DIR .. "src/emu/video/vector.h",
	MAME_DIR .. "src/emu/video/poly.h",
}

dependency {
	--------------------------------------------------
	-- additional dependencies
	--------------------------------------------------
	{ MAME_DIR .. "src/emu/rendfont.c", GEN_DIR .. "emu/uismall.fh" },
	-------------------------------------------------
	-- core layouts
	--------------------------------------------------
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/dualhovu.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/dualhsxs.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/dualhuov.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/horizont.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/triphsxs.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/quadhsxs.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/vertical.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/lcd.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/lcd_rot.lh" },
	{ MAME_DIR .. "src/emu/rendlay.c", GEN_DIR .. "emu/layout/noscreens.lh" },

	{ MAME_DIR .. "src/emu/video.c",   GEN_DIR .. "emu/layout/snap.lh" },

}

custombuildtask {
	{ MAME_DIR .. "src/emu/uismall.png"         , GEN_DIR .. "emu/uismall.fh",  {  MAME_DIR.. "src/build/png2bdc.py",  MAME_DIR .. "src/build/file2str.py" }, {"@echo Converting uismall.png...", PYTHON .. " $(1) $(<) temp.bdc", PYTHON .. " $(2) temp.bdc $(@) font_uismall UINT8" }},

	layoutbuildtask("emu/layout", "dualhovu"),
	layoutbuildtask("emu/layout", "dualhsxs"),
	layoutbuildtask("emu/layout", "dualhuov"),
	layoutbuildtask("emu/layout", "horizont"),
	layoutbuildtask("emu/layout", "triphsxs"),
	layoutbuildtask("emu/layout", "quadhsxs"),
	layoutbuildtask("emu/layout", "vertical"),
	layoutbuildtask("emu/layout", "lcd"),
	layoutbuildtask("emu/layout", "lcd_rot"),
	layoutbuildtask("emu/layout", "noscreens"),
	layoutbuildtask("emu/layout", "snap"),
}

function emuProject(_target, _subtarget)

	disasm_files = { }
	disasm_dependency = { }
	disasm_custombuildtask = { }

	project ("optional")
	uuid (os.uuid("optional-" .. _target .."_" .. _subtarget))
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mame", -- used for sound amiga
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
		MAME_DIR .. "src/emu/cpu/m68000",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end

	if _OPTIONS["with-bundled-zlib"] then
		includedirs {
			MAME_DIR .. "3rdparty/zlib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	dofile(path.join("src", "cpu.lua"))

	dofile(path.join("src", "sound.lua"))

	dofile(path.join("src", "video.lua"))

	dofile(path.join("src", "machine.lua"))

if (_OPTIONS["DRIVERS"] == nil) then 
	project ("bus")
	uuid ("5d782c89-cf7e-4cfe-8f9f-0d4bfc16c91d")
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "src/mess", -- some mess bus devices need this
		MAME_DIR .. "src/mame", -- used for nes bus devices
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end
	if _OPTIONS["with-bundled-zlib"] then
		includedirs {
			MAME_DIR .. "3rdparty/zlib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	dofile(path.join("src", "bus.lua"))
else
	dofile(path.join("src", "bus.lua"))
end
	
	--	netlist now defines a project
	dofile(path.join("src", "netlist.lua"))


	project ("dasm")
	uuid ("f2d28b0a-6da5-4f78-b629-d834aa00429d")
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "emu",
	}
	if _OPTIONS["with-bundled-expat"] then
		includedirs {
			MAME_DIR .. "3rdparty/expat/lib",
		}
	end

	if _OPTIONS["with-bundled-zlib"] then
		includedirs {
			MAME_DIR .. "3rdparty/zlib",
		}
	end
	if _OPTIONS["with-bundled-lua"] then
		includedirs {
			MAME_DIR .. "3rdparty/lua/src",
		}
	end

	files {
		disasm_files
	}

	if #disasm_dependency > 0 then
		dependency {
			disasm_dependency[1]
		}
	end

	if #disasm_custombuildtask > 0 then
		custombuildtask {
			disasm_custombuildtask[1]
		}
	end
end
