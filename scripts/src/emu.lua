project ("emu")
uuid ("e6fa15e4-a354-4526-acef-13c8e80fcacf")
kind "StaticLib"
options {
	"ForceCPP",
}

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/emu",
	MAME_DIR .. "src/lib",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
	MAME_DIR .. "3rdparty/expat/lib",
	MAME_DIR .. "3rdparty/lua/src",
	MAME_DIR .. "3rdparty/zlib",
	GEN_DIR  .. "emu",
	GEN_DIR  .. "emu/layout",
}

files {
	MAME_DIR .. "src/emu/hashfile.c",
	MAME_DIR .. "src/emu/addrmap.c",
	MAME_DIR .. "src/emu/attotime.c",
	MAME_DIR .. "src/emu/audit.c",
	MAME_DIR .. "src/emu/cheat.c",
	MAME_DIR .. "src/emu/clifront.c",
	MAME_DIR .. "src/emu/cliopts.c",
	MAME_DIR .. "src/emu/config.c",
	MAME_DIR .. "src/emu/crsshair.c",
	MAME_DIR .. "src/emu/debugger.c",
	MAME_DIR .. "src/emu/devdelegate.c",
	MAME_DIR .. "src/emu/devcb.c",
	MAME_DIR .. "src/emu/devcpu.c",
	MAME_DIR .. "src/emu/devfind.c",
	MAME_DIR .. "src/emu/device.c",
	MAME_DIR .. "src/emu/didisasm.c",
	MAME_DIR .. "src/emu/diexec.c",
	MAME_DIR .. "src/emu/digfx.c",
	MAME_DIR .. "src/emu/diimage.c",
	MAME_DIR .. "src/emu/dimemory.c",
	MAME_DIR .. "src/emu/dinetwork.c",
	MAME_DIR .. "src/emu/dinvram.c",
	MAME_DIR .. "src/emu/dioutput.c",
	MAME_DIR .. "src/emu/dirtc.c",
	MAME_DIR .. "src/emu/diserial.c",
	MAME_DIR .. "src/emu/dislot.c",
	MAME_DIR .. "src/emu/disound.c",
	MAME_DIR .. "src/emu/dispatch.c",
	MAME_DIR .. "src/emu/distate.c",
	MAME_DIR .. "src/emu/divideo.c",
	MAME_DIR .. "src/emu/drawgfx.c",
	MAME_DIR .. "src/emu/driver.c",
	MAME_DIR .. "src/emu/drivenum.c",
	MAME_DIR .. "src/emu/emualloc.c",
	MAME_DIR .. "src/emu/emucore.c",
	MAME_DIR .. "src/emu/emuopts.c",
	MAME_DIR .. "src/emu/emupal.c",
	MAME_DIR .. "src/emu/fileio.c",
	MAME_DIR .. "src/emu/hash.c",
	MAME_DIR .. "src/emu/image.c",
	MAME_DIR .. "src/emu/info.c",
	MAME_DIR .. "src/emu/input.c",
	MAME_DIR .. "src/emu/ioport.c",
	MAME_DIR .. "src/emu/luaengine.c",
	MAME_DIR .. "src/emu/mame.c",
	MAME_DIR .. "src/emu/machine.c",
	MAME_DIR .. "src/emu/mconfig.c",
	MAME_DIR .. "src/emu/memarray.c",
	MAME_DIR .. "src/emu/memory.c",
	MAME_DIR .. "src/emu/network.c",
	MAME_DIR .. "src/emu/parameters.c",
	MAME_DIR .. "src/emu/output.c",
	MAME_DIR .. "src/emu/render.c",
	MAME_DIR .. "src/emu/rendfont.c",
	MAME_DIR .. "src/emu/rendlay.c",
	MAME_DIR .. "src/emu/rendutil.c",
	MAME_DIR .. "src/emu/romload.c",
	MAME_DIR .. "src/emu/save.c",
	MAME_DIR .. "src/emu/schedule.c",
	MAME_DIR .. "src/emu/screen.c",
	MAME_DIR .. "src/emu/softlist.c",
	MAME_DIR .. "src/emu/sound.c",
	MAME_DIR .. "src/emu/speaker.c",
	MAME_DIR .. "src/emu/sprite.c",
	MAME_DIR .. "src/emu/tilemap.c",
	MAME_DIR .. "src/emu/timer.c",
	MAME_DIR .. "src/emu/uiinput.c",
	MAME_DIR .. "src/emu/ui/ui.c",
	MAME_DIR .. "src/emu/ui/menu.c",
	MAME_DIR .. "src/emu/ui/mainmenu.c",
	MAME_DIR .. "src/emu/ui/miscmenu.c",
	MAME_DIR .. "src/emu/ui/barcode.c",
	MAME_DIR .. "src/emu/ui/cheatopt.c",
	MAME_DIR .. "src/emu/ui/devopt.c",
	MAME_DIR .. "src/emu/ui/filemngr.c",
	MAME_DIR .. "src/emu/ui/filesel.c",
	MAME_DIR .. "src/emu/ui/imgcntrl.c",
	MAME_DIR .. "src/emu/ui/info.c",
	MAME_DIR .. "src/emu/ui/inputmap.c",
	MAME_DIR .. "src/emu/ui/selgame.c",
	MAME_DIR .. "src/emu/ui/sliders.c",
	MAME_DIR .. "src/emu/ui/slotopt.c",
	MAME_DIR .. "src/emu/ui/swlist.c",
	MAME_DIR .. "src/emu/ui/tapectrl.c",
	MAME_DIR .. "src/emu/ui/videoopt.c",
	MAME_DIR .. "src/emu/ui/viewgfx.c",
	MAME_DIR .. "src/emu/validity.c",
	MAME_DIR .. "src/emu/video.c",
	MAME_DIR .. "src/emu/debug/debugcmd.c",
	MAME_DIR .. "src/emu/debug/debugcon.c",
	MAME_DIR .. "src/emu/debug/debugcpu.c",
	MAME_DIR .. "src/emu/debug/debughlp.c",
	MAME_DIR .. "src/emu/debug/debugvw.c",
	MAME_DIR .. "src/emu/debug/dvdisasm.c",
	MAME_DIR .. "src/emu/debug/dvmemory.c",
	MAME_DIR .. "src/emu/debug/dvbpoints.c",
	MAME_DIR .. "src/emu/debug/dvwpoints.c",
	MAME_DIR .. "src/emu/debug/dvstate.c",
	MAME_DIR .. "src/emu/debug/dvtext.c",
	MAME_DIR .. "src/emu/debug/express.c",
	MAME_DIR .. "src/emu/debug/textbuf.c",
	MAME_DIR .. "src/emu/profiler.c",
	MAME_DIR .. "src/emu/webengine.c",
	MAME_DIR .. "src/emu/sound/filter.c",
	MAME_DIR .. "src/emu/sound/flt_vol.c",
	MAME_DIR .. "src/emu/sound/flt_rc.c",
	MAME_DIR .. "src/emu/sound/wavwrite.c",
	MAME_DIR .. "src/emu/sound/samples.c",
	MAME_DIR .. "src/emu/drivers/empty.c",
	MAME_DIR .. "src/emu/drivers/testcpu.c",
	MAME_DIR .. "src/emu/machine/bcreader.c",
	MAME_DIR .. "src/emu/machine/buffer.c",
	MAME_DIR .. "src/emu/machine/clock.c",
	MAME_DIR .. "src/emu/machine/generic.c",
	MAME_DIR .. "src/emu/machine/keyboard.c",
	MAME_DIR .. "src/emu/machine/laserdsc.c",
	MAME_DIR .. "src/emu/machine/latch.c",
	MAME_DIR .. "src/emu/machine/netlist.c",
	MAME_DIR .. "src/emu/machine/nvram.c",
	MAME_DIR .. "src/emu/machine/ram.c",
	MAME_DIR .. "src/emu/machine/legscsi.c",
	MAME_DIR .. "src/emu/machine/terminal.c",
	MAME_DIR .. "src/emu/imagedev/bitbngr.c",
	MAME_DIR .. "src/emu/imagedev/cassette.c",
	MAME_DIR .. "src/emu/imagedev/chd_cd.c",
	MAME_DIR .. "src/emu/imagedev/diablo.c",
	MAME_DIR .. "src/emu/imagedev/flopdrv.c",
	MAME_DIR .. "src/emu/imagedev/floppy.c",
	MAME_DIR .. "src/emu/imagedev/harddriv.c",
	MAME_DIR .. "src/emu/imagedev/midiin.c",
	MAME_DIR .. "src/emu/imagedev/midiout.c",
	MAME_DIR .. "src/emu/imagedev/printer.c",
	MAME_DIR .. "src/emu/imagedev/snapquik.c",
	MAME_DIR .. "src/emu/video/generic.c",
	MAME_DIR .. "src/emu/video/resnet.c",
	MAME_DIR .. "src/emu/video/rgbutil.c",
	MAME_DIR .. "src/emu/video/vector.c",
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
	kind "StaticLib"
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
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
		MAME_DIR .. "src/emu/cpu/m68000",
	}
	
	dofile(path.join("src", "cpu.lua"))

	dofile(path.join("src", "sound.lua"))
	
	dofile(path.join("src", "netlist.lua"))
	
	dofile(path.join("src", "video.lua"))

	dofile(path.join("src", "machine.lua"))

	
	project ("bus")
	uuid ("5d782c89-cf7e-4cfe-8f9f-0d4bfc16c91d")
	kind "StaticLib"
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
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		MAME_DIR .. "src/mess", -- some mess bus devices need this
		MAME_DIR .. "src/mame", -- used for nes bus devices
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
	}

	dofile(path.join("src", "bus.lua"))
	
	
	project ("dasm")
	uuid ("f2d28b0a-6da5-4f78-b629-d834aa00429d")
	kind "StaticLib"
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
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "emu",
	}
	
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