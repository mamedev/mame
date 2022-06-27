-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   nl.lua
--
--   Compiles all drivers using netlist code
--   Use make SUBTARGET=nl to build
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in nl.lst.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["M6502"] = true
CPUS["M6800"] = true
CPUS["M6803"] = true
CPUS["M6809"] = true
CPUS["MCS48"] = true
CPUS["I8085"] = true
CPUS["MCS51"] = true
--CPUS["M6800"] = true
--CPUS["M6809"] = true
--CPUS["M680X0"] = true
CPUS["M680X0"] = true
--CPUS["TMS9900"] = true
--CPUS["COP400"] = true
CPUS["F8"] = true
CPUS["CCPU"] = true
CPUS["MCS40"] = true
CPUS["TMS9900"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in nl.lst.
--------------------------------------------------

--SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["MSM5205"] = true
--SOUNDS["ASTROCADE"] = true
SOUNDS["TMS5220"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["UPD7759"] = true
--SOUNDS["HC55516"] = true
--SOUNDS["YM3812"] = true
--SOUNDS["CEM3394"] = true
--SOUNDS["VOTRAX"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2413"] = true
SOUNDS["BEEP"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["DIGITALKER"] = true
SOUNDS["SN76477"] = true
SOUNDS["SN76496"] = true
SOUNDS["SP0250"] = true
SOUNDS["TMS36XX"] = true
SOUNDS["TMS5110"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["FIXFREQ"] = true
VIDEOS["PWM_DISPLAY"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["INPUT_MERGER"] = true
MACHINES["NETLIST"] = true
MACHINES["Z80DMA"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DAISY"] = true
MACHINES["GEN_LATCH"] = true
MACHINES["AY31015"] = true
MACHINES["KB3600"] = true
MACHINES["COM8116"] = true

MACHINES["TTL7474"] = true
MACHINES["TTL74145"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74259"] = true
MACHINES["6522VIA"] = true

MACHINES["6821PIA"] = true
MACHINES["I8255"] = true
MACHINES["I8243"] = true
MACHINES["WATCHDOG"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["UPD4701"] = true
MACHINES["CXD1095"] = true
--MACHINES["TTL74148"] = true
--MACHINES["TTL74153"] = true
--MACHINES["TTL7474"] = true
--MACHINES["RIOT6532"] = true
MACHINES["PIT8253"] = true
--MACHINES["Z80CTC"] = true
--MACHINES["68681"] = true
--MACHINES["BANKDEV"] = true
MACHINES["F3853"] = true
MACHINES["MB14241"] = true
MACHINES["STEPPERS"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

-- not needed by nl.lua but build system wants at least one bus
BUSES["CENTRONICS"] = true

--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in nl.lst
--------------------------------------------------

function createProjects_mame_nl(_target, _subtarget)
	project ("mame_netlist")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-nl"))
	addprojectflags()
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame/shared",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
		ext_includedir("rapidjson"),
	}

files{
	MAME_DIR .. "src/mame/atari/pong.cpp",
	MAME_DIR .. "src/mame/atari/nl_pong.cpp",
	MAME_DIR .. "src/mame/atari/nl_pong.h",
	MAME_DIR .. "src/mame/atari/nl_pongdoubles.cpp",
	MAME_DIR .. "src/mame/atari/nl_pongdoubles.h",
	MAME_DIR .. "src/mame/atari/nl_breakout.cpp",
	MAME_DIR .. "src/mame/atari/nl_breakout.h",
	MAME_DIR .. "src/mame/atari/nl_rebound.cpp",
	MAME_DIR .. "src/mame/atari/nl_rebound.h",
	MAME_DIR .. "src/mame/skeleton/nl_hazelvid.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_hazelvid.h",

	MAME_DIR .. "src/mame/atari/atarittl.cpp",
	MAME_DIR .. "src/mame/atari/nl_stuntcyc.cpp",
	MAME_DIR .. "src/mame/atari/nl_stuntcyc.h",
	MAME_DIR .. "src/mame/atari/nl_gtrak10.cpp",
	MAME_DIR .. "src/mame/atari/nl_gtrak10.h",
	MAME_DIR .. "src/mame/atari/nl_tank.cpp",
	MAME_DIR .. "src/mame/atari/nl_tank.h",

	MAME_DIR .. "src/mame/atari/nl_flyball.cpp",
	MAME_DIR .. "src/mame/atari/nl_flyball.h",
	MAME_DIR .. "src/mame/atari/flyball.cpp",

	MAME_DIR .. "src/mame/atari/nl_destroyr.cpp",
	MAME_DIR .. "src/mame/atari/nl_destroyr.h",
	MAME_DIR .. "src/mame/atari/destroyr.cpp",

	MAME_DIR .. "src/mame/skeleton/hazeltin.cpp",

	MAME_DIR .. "src/mame/capcom/1942.cpp",
	MAME_DIR .. "src/mame/capcom/1942.h",
	MAME_DIR .. "src/mame/capcom/1942_v.cpp",
	MAME_DIR .. "src/mame/capcom/nl_1942.cpp",
	MAME_DIR .. "src/mame/capcom/nl_1942.h",

	MAME_DIR .. "src/mame/vtech/gamemachine.cpp",
	MAME_DIR .. "src/mame/vtech/nl_gamemachine.h",
	MAME_DIR .. "src/mame/vtech/nl_gamemachine.cpp",

	MAME_DIR .. "src/mame/nintendo/popeye.cpp",
	MAME_DIR .. "src/mame/nintendo/popeye.h",
	MAME_DIR .. "src/mame/nintendo/popeye_v.cpp",
	MAME_DIR .. "src/mame/nintendo/nl_popeye.cpp",
	MAME_DIR .. "src/mame/nintendo/nl_popeye.h",

	MAME_DIR .. "src/mame/nintendo/mario.cpp",
	MAME_DIR .. "src/mame/nintendo/mario.h",
	MAME_DIR .. "src/mame/nintendo/nl_mario.cpp",
	MAME_DIR .. "src/mame/nintendo/nl_mario.h",
	MAME_DIR .. "src/mame/nintendo/mario_v.cpp",
	MAME_DIR .. "src/mame/nintendo/mario_a.cpp",

	MAME_DIR .. "src/mame/irem/m62.cpp",
	MAME_DIR .. "src/mame/irem/m62.h",
	MAME_DIR .. "src/mame/irem/m62_v.cpp",
	MAME_DIR .. "src/mame/irem/irem.cpp",
	MAME_DIR .. "src/mame/irem/irem.h",
	MAME_DIR .. "src/mame/irem/nl_kidniki.cpp",
	MAME_DIR .. "src/mame/irem/nl_kidniki.h",

	MAME_DIR .. "src/mame/midw8080/mw8080bw.cpp",
	MAME_DIR .. "src/mame/midw8080/mw8080bw.h",
	MAME_DIR .. "src/mame/midw8080/mw8080bw_a.h",
	MAME_DIR .. "src/mame/midw8080/mw8080bw_a.cpp",
	MAME_DIR .. "src/mame/midw8080/mw8080bw_v.cpp",

	MAME_DIR .. "src/mame/midw8080/nl_gunfight.cpp",
	MAME_DIR .. "src/mame/midw8080/nl_gunfight.h",
	MAME_DIR .. "src/mame/midw8080/nl_280zzzap.cpp",
	MAME_DIR .. "src/mame/midw8080/nl_280zzzap.h",

	MAME_DIR .. "src/mame/midway/sspeedr.cpp",
	MAME_DIR .. "src/mame/midway/sspeedr.h",
	MAME_DIR .. "src/mame/midway/sspeedr_v.cpp",

	MAME_DIR .. "src/mame/midway/nl_sspeedr.cpp",
	MAME_DIR .. "src/mame/midway/nl_sspeedr.h",

	MAME_DIR .. "src/mame/univers/cheekyms_a.cpp",
	MAME_DIR .. "src/mame/univers/cheekyms_a.h",
	MAME_DIR .. "src/mame/univers/nl_cheekyms.cpp",
	MAME_DIR .. "src/mame/univers/nl_cheekyms.h",
	MAME_DIR .. "src/mame/univers/cheekyms.cpp",
	MAME_DIR .. "src/mame/univers/cheekyms.h",
	MAME_DIR .. "src/mame/univers/cheekyms_v.cpp",

	MAME_DIR .. "src/mame/cinemat/cinemat.cpp",
	MAME_DIR .. "src/mame/cinemat/cinemat.h",
	MAME_DIR .. "src/mame/cinemat/cinemat_a.cpp",
	MAME_DIR .. "src/mame/cinemat/cinemat_a.h",
	MAME_DIR .. "src/mame/cinemat/cinemat_v.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_armora.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_armora.h",
	MAME_DIR .. "src/mame/cinemat/nl_barrier.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_barrier.h",
	MAME_DIR .. "src/mame/cinemat/nl_boxingb.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_boxingb.h",
	MAME_DIR .. "src/mame/cinemat/nl_cinemat_common.h",
	MAME_DIR .. "src/mame/cinemat/nl_ripoff.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_ripoff.h",
	MAME_DIR .. "src/mame/cinemat/nl_solarq.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_solarq.h",
	MAME_DIR .. "src/mame/cinemat/nl_spacewar.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_spacewar.h",
	MAME_DIR .. "src/mame/cinemat/nl_speedfrk.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_speedfrk.h",
	MAME_DIR .. "src/mame/cinemat/nl_starcas.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_starcas.h",
	MAME_DIR .. "src/mame/cinemat/nl_starhawk.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_starhawk.h",
	MAME_DIR .. "src/mame/cinemat/nl_sundance.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_sundance.h",
	MAME_DIR .. "src/mame/cinemat/nl_tailg.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_tailg.h",
	MAME_DIR .. "src/mame/cinemat/nl_warrior.cpp",
	MAME_DIR .. "src/mame/cinemat/nl_warrior.h",

	MAME_DIR .. "src/mame/galaxian/galaxian.cpp",
	MAME_DIR .. "src/mame/galaxian/galaxian.h",
	MAME_DIR .. "src/mame/galaxian/galaxian_a.cpp",
	MAME_DIR .. "src/mame/galaxian/galaxian_a.h",
	MAME_DIR .. "src/mame/galaxian/galaxian_v.cpp",
	MAME_DIR .. "src/mame/galaxian/nl_konami.h",
	MAME_DIR .. "src/mame/galaxian/nl_konami.cpp",

	MAME_DIR .. "src/mame/shared/cclimber_a.cpp",
	MAME_DIR .. "src/mame/shared/cclimber_a.h",

	MAME_DIR .. "src/mame/zaccaria/nl_zac1b11142.cpp",
	MAME_DIR .. "src/mame/zaccaria/nl_zac1b11142.h",
	MAME_DIR .. "src/mame/zaccaria/zaccaria_a.cpp",
	MAME_DIR .. "src/mame/zaccaria/zaccaria_a.h",
	MAME_DIR .. "src/mame/zaccaria/zaccaria.cpp",
	MAME_DIR .. "src/mame/zaccaria/zaccaria.h",
	MAME_DIR .. "src/mame/zaccaria/zaccaria_v.cpp",

	MAME_DIR .. "src/mame/misc/cocoloco.cpp",
	MAME_DIR .. "src/mame/misc/nl_cocoloco.h",
	MAME_DIR .. "src/mame/misc/nl_cocoloco.cpp",

	MAME_DIR .. "src/mame/skeleton/palestra.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_palestra.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_palestra.h",

	MAME_DIR .. "src/mame/sega/vicdual.cpp",
	MAME_DIR .. "src/mame/sega/vicdual.h",
	MAME_DIR .. "src/mame/sega/vicdual_a.cpp",
	MAME_DIR .. "src/mame/sega/vicdual_a.h",
	MAME_DIR .. "src/mame/sega/nl_brdrline.cpp",
	MAME_DIR .. "src/mame/sega/nl_brdrline.h",
	MAME_DIR .. "src/mame/sega/nl_frogs.cpp",
	MAME_DIR .. "src/mame/sega/nl_frogs.h",
	MAME_DIR .. "src/mame/sega/vicdual-97271p.cpp",
	MAME_DIR .. "src/mame/sega/vicdual-97271p.h",
	MAME_DIR .. "src/mame/sega/vicdual_v.cpp",
	MAME_DIR .. "src/mame/sega/vicdual-97269pb.cpp",
	MAME_DIR .. "src/mame/sega/vicdual-97269pb.h",
	MAME_DIR .. "src/mame/sega/carnival.cpp",
	MAME_DIR .. "src/mame/sega/depthch.cpp",
	MAME_DIR .. "src/mame/sega/invinco.cpp",
	MAME_DIR .. "src/mame/sega/pulsar_a.cpp",

	MAME_DIR .. "src/mame/shared/segacrpt_device.cpp",
	MAME_DIR .. "src/mame/shared/segacrpt_device.h",
	MAME_DIR .. "src/mame/sega/segag80r.cpp",
	MAME_DIR .. "src/mame/sega/segag80r.h",
	MAME_DIR .. "src/mame/sega/segag80_m.cpp",
	MAME_DIR .. "src/mame/sega/segag80_m.h",
	MAME_DIR .. "src/mame/sega/segag80r_a.cpp",
	MAME_DIR .. "src/mame/sega/segag80r_a.h",
	MAME_DIR .. "src/mame/sega/segag80r_v.cpp",
	MAME_DIR .. "src/mame/sega/segag80v.cpp",
	MAME_DIR .. "src/mame/sega/segag80v.h",
	MAME_DIR .. "src/mame/sega/segag80.cpp",
	MAME_DIR .. "src/mame/sega/segag80.h",
	MAME_DIR .. "src/mame/sega/segaspeech.cpp",
	MAME_DIR .. "src/mame/sega/segaspeech.h",
	MAME_DIR .. "src/mame/sega/segausb.cpp",
	MAME_DIR .. "src/mame/sega/segausb.h",
	MAME_DIR .. "src/mame/sega/nl_astrob.cpp",
	MAME_DIR .. "src/mame/sega/nl_astrob.h",
	MAME_DIR .. "src/mame/sega/nl_elim.cpp",
	MAME_DIR .. "src/mame/sega/nl_elim.h",
	MAME_DIR .. "src/mame/sega/nl_spacfury.cpp",
	MAME_DIR .. "src/mame/sega/nl_spacfury.h",
	MAME_DIR .. "src/mame/sega/segag80v_v.cpp",
	MAME_DIR .. "src/mame/sega/zaxxon.cpp",
	MAME_DIR .. "src/mame/sega/zaxxon.h",
	MAME_DIR .. "src/mame/sega/zaxxon_a.cpp",
	MAME_DIR .. "src/mame/sega/zaxxon_v.cpp",

	MAME_DIR .. "src/mame/sega/segas16b.cpp",
	MAME_DIR .. "src/mame/sega/segas16b_isgsm.cpp",
	MAME_DIR .. "src/mame/sega/segas16b.h",
	MAME_DIR .. "src/mame/sega/segas16b_v.cpp",
	MAME_DIR .. "src/mame/sega/nl_segas16b.cpp",
	MAME_DIR .. "src/mame/sega/nl_segas16b.h",
	MAME_DIR .. "src/mame/sega/nl_segausb.cpp",
	MAME_DIR .. "src/mame/sega/nl_segausb.h",
	MAME_DIR .. "src/mame/sega/nl_segaspeech.cpp",
	MAME_DIR .. "src/mame/sega/nl_segaspeech.h",
	MAME_DIR .. "src/mame/sega/315_5195.cpp",
	MAME_DIR .. "src/mame/sega/315_5195.h",
	MAME_DIR .. "src/mame/sega/fd1089.cpp",
	MAME_DIR .. "src/mame/sega/fd1089.h",
	MAME_DIR .. "src/mame/sega/fd1094.cpp",
	MAME_DIR .. "src/mame/sega/fd1094.h",
	MAME_DIR .. "src/mame/sega/segaic16_m.cpp",
	MAME_DIR .. "src/mame/sega/segaic16_m.h",
	MAME_DIR .. "src/mame/sega/sega16sp.cpp",
	MAME_DIR .. "src/mame/sega/sega16sp.h",
	MAME_DIR .. "src/mame/shared/mc8123.cpp",
	MAME_DIR .. "src/mame/shared/mc8123.h",
	MAME_DIR .. "src/mame/sega/segaic16.cpp",
	MAME_DIR .. "src/mame/sega/segaic16.h",
	MAME_DIR .. "src/mame/sega/segaic16_road.cpp",
	MAME_DIR .. "src/mame/sega/segaic16_road.h",

	MAME_DIR .. "src/mame/skeleton/testpat.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_tp1983.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_tp1983.h",
	MAME_DIR .. "src/mame/skeleton/nl_tp1985.cpp",
	MAME_DIR .. "src/mame/skeleton/nl_tp1985.h",

	MAME_DIR .. "src/mame/ramtek/nl_starcrus.h",
	MAME_DIR .. "src/mame/ramtek/nl_starcrus.cpp",
	MAME_DIR .. "src/mame/ramtek/starcrus.cpp",
	MAME_DIR .. "src/mame/ramtek/starcrus.h",
	MAME_DIR .. "src/mame/ramtek/starcrus_v.cpp",

	MAME_DIR .. "src/mame/exidy/nl_carpolo.h",
	MAME_DIR .. "src/mame/exidy/nl_carpolo.cpp",
	MAME_DIR .. "src/mame/exidy/carpolo.cpp",
	MAME_DIR .. "src/mame/exidy/carpolo.h",
	MAME_DIR .. "src/mame/exidy/carpolo_m.cpp",
	MAME_DIR .. "src/mame/exidy/carpolo_v.cpp",

	MAME_DIR .. "src/mame/exidy/nl_fireone.h",
	MAME_DIR .. "src/mame/exidy/nl_fireone.cpp",
	MAME_DIR .. "src/mame/exidy/nl_starfire.h",
	MAME_DIR .. "src/mame/exidy/nl_starfire.cpp",
	MAME_DIR .. "src/mame/exidy/starfire.cpp",
	MAME_DIR .. "src/mame/exidy/starfire.h",
	MAME_DIR .. "src/mame/exidy/starfire_v.cpp",

	MAME_DIR .. "src/mame/ramtek/nl_starcrus.cpp",
	MAME_DIR .. "src/mame/ramtek/nl_starcrus.h",
	MAME_DIR .. "src/mame/ramtek/starcrus.cpp",
	MAME_DIR .. "src/mame/ramtek/starcrus.h",
	MAME_DIR .. "src/mame/ramtek/starcrus_v.cpp",

	MAME_DIR .. "src/mame/atari/dragrace.cpp",
	MAME_DIR .. "src/mame/atari/dragrace.h",
	MAME_DIR .. "src/mame/atari/dragrace_a.cpp",
	MAME_DIR .. "src/mame/atari/dragrace_v.cpp",


	-- Skeletons ...
	MAME_DIR .. "src/mame/misc/a1supply.cpp",
	MAME_DIR .. "src/mame/alliedl/aleisttl.cpp",
	MAME_DIR .. "src/mame/misc/bailey.cpp",
	MAME_DIR .. "src/mame/misc/chicago.cpp",
	MAME_DIR .. "src/mame/misc/crazybal.cpp",
	MAME_DIR .. "src/mame/misc/electra.cpp",
	MAME_DIR .. "src/mame/exidy/exidyttl.cpp",
	MAME_DIR .. "src/mame/misc/fungames.cpp",
	MAME_DIR .. "src/mame/meadows/meadwttl.cpp",
	MAME_DIR .. "src/mame/sega/monacogp.cpp",
	MAME_DIR .. "src/mame/misc/pse.cpp",
	MAME_DIR .. "src/mame/ramtek/ramtek.cpp",
	MAME_DIR .. "src/mame/sega/segattl.cpp",
	MAME_DIR .. "src/mame/taito/taitottl.cpp",
	MAME_DIR .. "src/mame/misc/usbilliards.cpp",

	MAME_DIR .. "src/mame/jpm/jpmsru.cpp",
	MAME_DIR .. "src/mame/jpm/nl_jpmsru.cpp",

	MAME_DIR .. "src/mame/shared/fruitsamples.cpp",
	MAME_DIR .. "src/mame/shared/fruitsamples.h",
	MAME_DIR .. "src/mame/shared/awpvid.cpp",
	MAME_DIR .. "src/mame/shared/awpvid.h",

}
end

function linkProjects_mame_nl(_target, _subtarget)
	links {
		"mame_netlist",
	}
end
