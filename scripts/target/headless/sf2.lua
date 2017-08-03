---------------------------------------------------------------------------
--
--   Street Fighter 2 HeadLess makefile
--
---------------------------------------------------------------------------

MACHINES["EEPROMDEV"] = true
MACHINES["TIMEKPR"] = true
MACHINES["GEN_LATCH"] = true
MACHINES["UPD4701"] = true
CPUS["Z80"] = true
CPUS["M680X0"] = true
CPUS["DSP16A"] = true
SOUNDS["QSOUND"] = true
SOUNDS["YM2151"] = true
SOUNDS["MSM5205"] = true
SOUNDS["OKIM6295"] = true

function createProjects_headless_sf2(_target, _subtarget)
    project ("headless_sf2")
    targetsubdir(_target .."_" .. _subtarget)
    kind (LIBTYPE)
    uuid (os.uuid("drv-headless_sf2"))
    addprojectflags()
    precompiledheaders()

    includedirs {
        MAME_DIR .. "src/osd",
        MAME_DIR .. "src/emu",
        MAME_DIR .. "src/devices",
        MAME_DIR .. "src/mame",
        MAME_DIR .. "src/lib",
        MAME_DIR .. "src/lib/util",
        MAME_DIR .. "3rdparty",
    }

    files{
        MAME_DIR .. "src/mame/drivers/cps1.cpp",
        MAME_DIR .. "src/mame/includes/cps1.h",
        MAME_DIR .. "src/mame/video/cps1.cpp",
        MAME_DIR .. "src/mame/machine/kabuki.cpp",
        MAME_DIR .. "src/mame/machine/kabuki.h",
    }
end

function linkProjects_headless_sf2(_target, _subtarget)
    links {
        "headless_sf2",
    }
end
