-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   vector.lua
--
--   Small driver-specific example makefile
--   Use make SUBTARGET=vector to build
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in vector.lst.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["M6502"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["M6800"] = true
CPUS["M6809"] = true
CPUS["M680X0"] = true
CPUS["TMS9900"] = true
CPUS["TMS32010"] = true
CPUS["TMS32025"] = true
CPUS["TMS32031"] = true
CPUS["TMS32051"] = true
CPUS["TMS32082"] = true
CPUS["TMS57002"] = true
CPUS["COP400"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in vector.lst.
--------------------------------------------------

SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["YM2151"] = true
SOUNDS["ASTROCADE"] = true
SOUNDS["TMS5220"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["HC55516"] = true
SOUNDS["YM3812"] = true
SOUNDS["CEM3394"] = true
SOUNDS["VOTRAX"] = true
SOUNDS["POKEY"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["6821PIA"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL7474"] = true
MACHINES["TIMEKPR"] = true
MACHINES["RIOT6532"] = true
MACHINES["PIT8253"] = true
MACHINES["Z80CTC"] = true
MACHINES["68681"] = true
MACHINES["BANKDEV"] = true
MACHINES["X2212"] = true


--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["CENTRONICS"] = true
BUSES["GENERIC"] = true

--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in vector.lst
--------------------------------------------------

function createProjects_mame_vector(_target, _subtarget)
	project ("mame_vector")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-vector"))
	
	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files {
	MAME_DIR .. "src/mame/video/avgdvg.cpp",
	MAME_DIR .. "src/mame/video/avgdvg.h",

	MAME_DIR .. "src/mame/drivers/arcadecl.cpp",
	MAME_DIR .. "src/mame/includes/arcadecl.h",
	MAME_DIR .. "src/mame/video/arcadecl.cpp",
	MAME_DIR .. "src/mame/drivers/asteroid.cpp",
	MAME_DIR .. "src/mame/includes/asteroid.h",
	MAME_DIR .. "src/mame/machine/asteroid.cpp",
	MAME_DIR .. "src/mame/audio/asteroid.cpp",
	MAME_DIR .. "src/mame/audio/llander.cpp",
	MAME_DIR .. "src/mame/drivers/atarifb.cpp",
	MAME_DIR .. "src/mame/includes/atarifb.h",
	MAME_DIR .. "src/mame/machine/atarifb.cpp",
	MAME_DIR .. "src/mame/audio/atarifb.cpp",
	MAME_DIR .. "src/mame/video/atarifb.cpp",
	MAME_DIR .. "src/mame/drivers/atarig1.cpp",
	MAME_DIR .. "src/mame/includes/atarig1.h",
	MAME_DIR .. "src/mame/video/atarig1.cpp",
	MAME_DIR .. "src/mame/includes/slapstic.h",
	MAME_DIR .. "src/mame/drivers/atarig42.cpp",
	MAME_DIR .. "src/mame/includes/atarig42.h",
	MAME_DIR .. "src/mame/video/atarig42.cpp",
	MAME_DIR .. "src/mame/drivers/atarigt.cpp",
	MAME_DIR .. "src/mame/includes/atarigt.h",
	MAME_DIR .. "src/mame/video/atarigt.cpp",
	MAME_DIR .. "src/mame/drivers/atarigx2.cpp",
	MAME_DIR .. "src/mame/includes/atarigx2.h",
	MAME_DIR .. "src/mame/video/atarigx2.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy1.cpp",
	MAME_DIR .. "src/mame/includes/atarisy1.h",
	MAME_DIR .. "src/mame/video/atarisy1.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy2.cpp",
	MAME_DIR .. "src/mame/includes/atarisy2.h",
	MAME_DIR .. "src/mame/video/atarisy2.cpp",
	MAME_DIR .. "src/mame/drivers/atarisy4.cpp",
	MAME_DIR .. "src/mame/drivers/atarittl.cpp",
	--MAME_DIR .. "src/mame/drivers/atetris.cpp",
	--MAME_DIR .. "src/mame/includes/atetris.h",
	--MAME_DIR .. "src/mame/video/atetris.cpp",
	MAME_DIR .. "src/mame/drivers/avalnche.cpp",
	MAME_DIR .. "src/mame/includes/avalnche.h",
	MAME_DIR .. "src/mame/audio/avalnche.cpp",
	--MAME_DIR .. "src/mame/drivers/badlands.cpp",
	--MAME_DIR .. "src/mame/includes/badlands.h",
	--MAME_DIR .. "src/mame/video/badlands.cpp",
	--MAME_DIR .. "src/mame/drivers/bartop52.cpp",
	--MAME_DIR .. "src/mame/drivers/batman.cpp",
	--MAME_DIR .. "src/mame/includes/batman.h",
	--MAME_DIR .. "src/mame/video/batman.cpp",
	--MAME_DIR .. "src/mame/drivers/beathead.cpp",
	--MAME_DIR .. "src/mame/includes/beathead.h",
	--MAME_DIR .. "src/mame/video/beathead.cpp",
	--MAME_DIR .. "src/mame/drivers/blstroid.cpp",
	--MAME_DIR .. "src/mame/includes/blstroid.h",
	--MAME_DIR .. "src/mame/video/blstroid.cpp",
	--MAME_DIR .. "src/mame/drivers/boxer.cpp",
	--MAME_DIR .. "src/mame/drivers/bsktball.cpp",
	--MAME_DIR .. "src/mame/includes/bsktball.h",
	--MAME_DIR .. "src/mame/machine/bsktball.cpp",
	--MAME_DIR .. "src/mame/audio/bsktball.cpp",
	--MAME_DIR .. "src/mame/video/bsktball.cpp",
	MAME_DIR .. "src/mame/drivers/bwidow.cpp",
	MAME_DIR .. "src/mame/includes/bwidow.h",
	MAME_DIR .. "src/mame/audio/bwidow.cpp",
	MAME_DIR .. "src/mame/drivers/bzone.cpp",
	MAME_DIR .. "src/mame/includes/bzone.h",
	MAME_DIR .. "src/mame/audio/bzone.cpp",
	--MAME_DIR .. "src/mame/drivers/canyon.cpp",
	--MAME_DIR .. "src/mame/includes/canyon.h",
	--MAME_DIR .. "src/mame/audio/canyon.cpp",
	--MAME_DIR .. "src/mame/video/canyon.cpp",
	--MAME_DIR .. "src/mame/drivers/cball.cpp",
	--MAME_DIR .. "src/mame/drivers/ccastles.cpp",
	--MAME_DIR .. "src/mame/includes/ccastles.h",
	--MAME_DIR .. "src/mame/video/ccastles.cpp",
	--MAME_DIR .. "src/mame/drivers/centiped.cpp",
	--MAME_DIR .. "src/mame/includes/centiped.h",
	--MAME_DIR .. "src/mame/video/centiped.cpp",
	--MAME_DIR .. "src/mame/drivers/cloak.cpp",
	--MAME_DIR .. "src/mame/includes/cloak.h",
	--MAME_DIR .. "src/mame/video/cloak.cpp",
	--MAME_DIR .. "src/mame/drivers/cloud9.cpp",
	--MAME_DIR .. "src/mame/includes/cloud9.h",
	--MAME_DIR .. "src/mame/video/cloud9.cpp",
	--MAME_DIR .. "src/mame/drivers/cmmb.cpp",
	--MAME_DIR .. "src/mame/drivers/cops.cpp",
	--MAME_DIR .. "src/mame/drivers/copsnrob.cpp",
	--MAME_DIR .. "src/mame/includes/copsnrob.h",
	--MAME_DIR .. "src/mame/audio/copsnrob.cpp",
	--MAME_DIR .. "src/mame/video/copsnrob.cpp",
	--MAME_DIR .. "src/mame/drivers/cyberbal.cpp",
	--MAME_DIR .. "src/mame/includes/cyberbal.h",
	--MAME_DIR .. "src/mame/audio/cyberbal.cpp",
	--MAME_DIR .. "src/mame/video/cyberbal.cpp",
	--MAME_DIR .. "src/mame/drivers/destroyr.cpp",
	--MAME_DIR .. "src/mame/drivers/dragrace.cpp",
	--MAME_DIR .. "src/mame/includes/dragrace.h",
	--MAME_DIR .. "src/mame/audio/dragrace.cpp",
	--MAME_DIR .. "src/mame/video/dragrace.cpp",
	MAME_DIR .. "src/mame/drivers/eprom.cpp",
	MAME_DIR .. "src/mame/includes/eprom.h",
	MAME_DIR .. "src/mame/video/eprom.cpp",
	--MAME_DIR .. "src/mame/drivers/firefox.cpp",
	--MAME_DIR .. "src/mame/drivers/firetrk.cpp",
	--MAME_DIR .. "src/mame/includes/firetrk.h",
	--MAME_DIR .. "src/mame/audio/firetrk.cpp",
	--MAME_DIR .. "src/mame/video/firetrk.cpp",
	--MAME_DIR .. "src/mame/drivers/flyball.cpp",
	--MAME_DIR .. "src/mame/drivers/foodf.cpp",
	--MAME_DIR .. "src/mame/includes/foodf.h",
	--MAME_DIR .. "src/mame/video/foodf.cpp",
	--MAME_DIR .. "src/mame/drivers/gauntlet.cpp",
	--MAME_DIR .. "src/mame/includes/gauntlet.h",
	--MAME_DIR .. "src/mame/video/gauntlet.cpp",
	--MAME_DIR .. "src/mame/drivers/harddriv.cpp",
	--MAME_DIR .. "src/mame/includes/harddriv.h",
	--MAME_DIR .. "src/mame/machine/harddriv.cpp",
	--MAME_DIR .. "src/mame/audio/harddriv.cpp",
	--MAME_DIR .. "src/mame/video/harddriv.cpp",
	--MAME_DIR .. "src/mame/drivers/irobot.cpp",
	--MAME_DIR .. "src/mame/includes/irobot.h",
	--MAME_DIR .. "src/mame/machine/irobot.cpp",
	--MAME_DIR .. "src/mame/video/irobot.cpp",
	--MAME_DIR .. "src/mame/drivers/jaguar.cpp",
	--MAME_DIR .. "src/mame/includes/jaguar.h",
	--MAME_DIR .. "src/mame/audio/jaguar.cpp",
	--MAME_DIR .. "src/mame/video/jaguar.cpp",
	--MAME_DIR .. "src/mame/video/jagblit.h",
	--MAME_DIR .. "src/mame/video/jagblit.inc",
	--MAME_DIR .. "src/mame/video/jagobj.inc",
	MAME_DIR .. "src/mame/drivers/jedi.cpp",
	MAME_DIR .. "src/mame/includes/jedi.h",
	MAME_DIR .. "src/mame/audio/jedi.cpp",
	MAME_DIR .. "src/mame/video/jedi.cpp",
	--MAME_DIR .. "src/mame/drivers/klax.cpp",
	--MAME_DIR .. "src/mame/includes/klax.h",
	--MAME_DIR .. "src/mame/video/klax.cpp",
	--MAME_DIR .. "src/mame/drivers/liberatr.cpp",
	--MAME_DIR .. "src/mame/includes/liberatr.h",
	--MAME_DIR .. "src/mame/video/liberatr.cpp",
	--MAME_DIR .. "src/mame/drivers/mediagx.cpp",
	--MAME_DIR .. "src/mame/drivers/metalmx.cpp",
	--MAME_DIR .. "src/mame/includes/metalmx.h",
	--MAME_DIR .. "src/mame/drivers/mgolf.cpp",
	MAME_DIR .. "src/mame/drivers/mhavoc.cpp",
	MAME_DIR .. "src/mame/includes/mhavoc.h",
	MAME_DIR .. "src/mame/machine/mhavoc.cpp",
	--MAME_DIR .. "src/mame/drivers/missile.cpp",
	--MAME_DIR .. "src/mame/drivers/nitedrvr.cpp",
	--MAME_DIR .. "src/mame/includes/nitedrvr.h",
	--MAME_DIR .. "src/mame/machine/nitedrvr.cpp",
	--MAME_DIR .. "src/mame/audio/nitedrvr.cpp",
	--MAME_DIR .. "src/mame/video/nitedrvr.cpp",
	--MAME_DIR .. "src/mame/drivers/offtwall.cpp",
	--MAME_DIR .. "src/mame/includes/offtwall.h",
	--MAME_DIR .. "src/mame/video/offtwall.cpp",
	--MAME_DIR .. "src/mame/drivers/orbit.cpp",
	--MAME_DIR .. "src/mame/includes/orbit.h",
	--MAME_DIR .. "src/mame/audio/orbit.cpp",
	--MAME_DIR .. "src/mame/video/orbit.cpp",
	--MAME_DIR .. "src/mame/drivers/pong.cpp",
	--MAME_DIR .. "src/mame/drivers/nl_pong.cpp",
	--MAME_DIR .. "src/mame/drivers/nl_pongd.cpp",
	--MAME_DIR .. "src/mame/drivers/nl_breakout.cpp",
	--MAME_DIR .. "src/mame/drivers/poolshrk.cpp",
	--MAME_DIR .. "src/mame/includes/poolshrk.h",
	--MAME_DIR .. "src/mame/audio/poolshrk.cpp",
	--MAME_DIR .. "src/mame/video/poolshrk.cpp",
	MAME_DIR .. "src/mame/drivers/quantum.cpp",
	--MAME_DIR .. "src/mame/drivers/quizshow.cpp",
	--MAME_DIR .. "src/mame/drivers/rampart.cpp",
	--MAME_DIR .. "src/mame/includes/rampart.h",
	--MAME_DIR .. "src/mame/video/rampart.cpp",
	--MAME_DIR .. "src/mame/drivers/relief.cpp",
	--MAME_DIR .. "src/mame/includes/relief.h",
	--MAME_DIR .. "src/mame/video/relief.cpp",
	--MAME_DIR .. "src/mame/drivers/runaway.cpp",
	--MAME_DIR .. "src/mame/includes/runaway.h",
	--MAME_DIR .. "src/mame/video/runaway.cpp",
	--MAME_DIR .. "src/mame/drivers/sbrkout.cpp",
	--MAME_DIR .. "src/mame/drivers/shuuz.cpp",
	--MAME_DIR .. "src/mame/includes/shuuz.h",
	--MAME_DIR .. "src/mame/video/shuuz.cpp",
	--MAME_DIR .. "src/mame/drivers/skullxbo.cpp",
	--MAME_DIR .. "src/mame/includes/skullxbo.h",
	--MAME_DIR .. "src/mame/video/skullxbo.cpp",
	--MAME_DIR .. "src/mame/drivers/skydiver.cpp",
	--MAME_DIR .. "src/mame/includes/skydiver.h",
	--MAME_DIR .. "src/mame/audio/skydiver.cpp",
	--MAME_DIR .. "src/mame/video/skydiver.cpp",
	--MAME_DIR .. "src/mame/drivers/skyraid.cpp",
	--MAME_DIR .. "src/mame/includes/skyraid.h",
	--MAME_DIR .. "src/mame/audio/skyraid.cpp",
	--MAME_DIR .. "src/mame/video/skyraid.cpp",
	--MAME_DIR .. "src/mame/drivers/sprint2.cpp",
	--MAME_DIR .. "src/mame/includes/sprint2.h",
	--MAME_DIR .. "src/mame/audio/sprint2.cpp",
	--MAME_DIR .. "src/mame/video/sprint2.cpp",
	--MAME_DIR .. "src/mame/drivers/sprint4.cpp",
	--MAME_DIR .. "src/mame/includes/sprint4.h",
	--MAME_DIR .. "src/mame/video/sprint4.cpp",
	--MAME_DIR .. "src/mame/audio/sprint4.cpp",
	--MAME_DIR .. "src/mame/audio/sprint4.h",
	--MAME_DIR .. "src/mame/drivers/sprint8.cpp",
	--MAME_DIR .. "src/mame/includes/sprint8.h",
	--MAME_DIR .. "src/mame/audio/sprint8.cpp",
	--MAME_DIR .. "src/mame/video/sprint8.cpp",
	--MAME_DIR .. "src/mame/drivers/starshp1.cpp",
	--MAME_DIR .. "src/mame/includes/starshp1.h",
	--MAME_DIR .. "src/mame/audio/starshp1.cpp",
	--MAME_DIR .. "src/mame/video/starshp1.cpp",
	MAME_DIR .. "src/mame/drivers/starwars.cpp",
	MAME_DIR .. "src/mame/includes/starwars.h",
	MAME_DIR .. "src/mame/machine/starwars.cpp",
	MAME_DIR .. "src/mame/audio/starwars.cpp",
	--MAME_DIR .. "src/mame/drivers/subs.cpp",
	--MAME_DIR .. "src/mame/includes/subs.h",
	--MAME_DIR .. "src/mame/machine/subs.cpp",
	--MAME_DIR .. "src/mame/audio/subs.cpp",
	--MAME_DIR .. "src/mame/video/subs.cpp",
	--MAME_DIR .. "src/mame/drivers/tank8.cpp",
	--MAME_DIR .. "src/mame/includes/tank8.h",
	--MAME_DIR .. "src/mame/audio/tank8.cpp",
	--MAME_DIR .. "src/mame/video/tank8.cpp",
	MAME_DIR .. "src/mame/drivers/tempest.cpp",
	--MAME_DIR .. "src/mame/drivers/thunderj.cpp",
	--MAME_DIR .. "src/mame/includes/thunderj.h",
	--MAME_DIR .. "src/mame/video/thunderj.cpp",
	MAME_DIR .. "src/mame/drivers/tomcat.cpp",
	--MAME_DIR .. "src/mame/drivers/toobin.cpp",
	--MAME_DIR .. "src/mame/includes/toobin.h",
	--MAME_DIR .. "src/mame/video/toobin.cpp",
	--MAME_DIR .. "src/mame/drivers/tourtabl.cpp",
	--MAME_DIR .. "src/mame/video/tia.cpp",
	--MAME_DIR .. "src/mame/video/tia.h",
	--MAME_DIR .. "src/mame/drivers/triplhnt.cpp",
	--MAME_DIR .. "src/mame/includes/triplhnt.h",
	--MAME_DIR .. "src/mame/audio/triplhnt.cpp",
	--MAME_DIR .. "src/mame/video/triplhnt.cpp",
	--MAME_DIR .. "src/mame/drivers/tunhunt.cpp",
	--MAME_DIR .. "src/mame/includes/tunhunt.h",
	--MAME_DIR .. "src/mame/video/tunhunt.cpp",
	--MAME_DIR .. "src/mame/drivers/ultratnk.cpp",
	--MAME_DIR .. "src/mame/includes/ultratnk.h",
	--MAME_DIR .. "src/mame/video/ultratnk.cpp",
	--MAME_DIR .. "src/mame/drivers/videopin.cpp",
	--MAME_DIR .. "src/mame/includes/videopin.h",
	--MAME_DIR .. "src/mame/audio/videopin.cpp",
	--MAME_DIR .. "src/mame/video/videopin.cpp",
	--MAME_DIR .. "src/mame/drivers/vindictr.cpp",
	--MAME_DIR .. "src/mame/includes/vindictr.h",
	--MAME_DIR .. "src/mame/video/vindictr.cpp",
	--MAME_DIR .. "src/mame/drivers/wolfpack.cpp",
	--MAME_DIR .. "src/mame/includes/wolfpack.h",
	--MAME_DIR .. "src/mame/video/wolfpack.cpp",
	--MAME_DIR .. "src/mame/drivers/xybots.cpp",
	--MAME_DIR .. "src/mame/includes/xybots.h",
	--MAME_DIR .. "src/mame/video/xybots.cpp",
	MAME_DIR .. "src/mame/machine/asic65.cpp",
	MAME_DIR .. "src/mame/machine/asic65.h",
	MAME_DIR .. "src/mame/machine/atari_vg.cpp",
	MAME_DIR .. "src/mame/machine/atari_vg.h",
	MAME_DIR .. "src/mame/machine/atarigen.cpp",
	MAME_DIR .. "src/mame/machine/atarigen.h",
	MAME_DIR .. "src/mame/machine/mathbox.cpp",
	MAME_DIR .. "src/mame/machine/mathbox.h",
	MAME_DIR .. "src/mame/machine/slapstic.cpp",
	MAME_DIR .. "src/mame/audio/atarijsa.cpp",
	MAME_DIR .. "src/mame/audio/atarijsa.h",
	MAME_DIR .. "src/mame/audio/cage.cpp",
	MAME_DIR .. "src/mame/audio/cage.h",
	MAME_DIR .. "src/mame/audio/redbaron.cpp",
	MAME_DIR .. "src/mame/audio/redbaron.h",
	MAME_DIR .. "src/mame/video/atarimo.cpp",
	MAME_DIR .. "src/mame/video/atarimo.h",
	MAME_DIR .. "src/mame/video/atarirle.cpp",
	MAME_DIR .. "src/mame/video/atarirle.h",
}
end

function linkProjects_mame_vector(_target, _subtarget)
	links {
		"mame_vector",
	}
end
