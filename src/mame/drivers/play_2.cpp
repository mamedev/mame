// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    Playmatic MPU 2
*/


#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class play_2_state : public driver_device
{
public:
	play_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cosmac_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(play_2);
};

static ADDRESS_MAP_START( play_2_map, AS_PROGRAM, 8, play_2_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( play_2 )
INPUT_PORTS_END

void play_2_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(play_2_state,play_2)
{
}

static MACHINE_CONFIG_START( play_2, play_2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_2_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Antar (11/79)
/-------------------------------------------------------------------*/
ROM_START(antar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("antar08.bin", 0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_RELOAD(0x4000, 0x0400)
	ROM_RELOAD(0x8000, 0x0400)
	ROM_RELOAD(0xc000, 0x0400)
	ROM_LOAD("antar09.bin", 0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_RELOAD(0x4400, 0x0400)
	ROM_RELOAD(0x8400, 0x0400)
	ROM_RELOAD(0xc400, 0x0400)
	ROM_LOAD("antar10.bin", 0x0800, 0x0400, CRC(a6ce5667) SHA1(85ecd4fce94dc419e4c210262f867310b0889cd3))
	ROM_RELOAD(0x4800, 0x0400)
	ROM_RELOAD(0x8800, 0x0400)
	ROM_RELOAD(0xc800, 0x0400)
	ROM_LOAD("antar11.bin", 0x0c00, 0x0400, CRC(6474b17f) SHA1(e4325ceff820393b06eb2e8e4a85412b0d01a385))
	ROM_RELOAD(0x4c00, 0x0400)
	ROM_RELOAD(0x8c00, 0x0400)
	ROM_RELOAD(0xcc00, 0x0400)
ROM_END

ROM_START(antar2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("antar08.bin", 0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_RELOAD(0x4000, 0x0400)
	ROM_RELOAD(0x8000, 0x0400)
	ROM_RELOAD(0xc000, 0x0400)
	ROM_LOAD("antar09.bin", 0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_RELOAD(0x4400, 0x0400)
	ROM_RELOAD(0x8400, 0x0400)
	ROM_RELOAD(0xc400, 0x0400)
	ROM_LOAD("antar10a.bin", 0x0800, 0x0400, CRC(520eb401) SHA1(1d5e3f829a7e7f38c7c519c488e6b7e1a4d34321))
	ROM_RELOAD(0x4800, 0x0400)
	ROM_RELOAD(0x8800, 0x0400)
	ROM_RELOAD(0xc800, 0x0400)
	ROM_LOAD("antar11a.bin", 0x0c00, 0x0400, CRC(17ad38bf) SHA1(e2c9472ed8fbe9d5965a5c79515a1b7ea9edaa79))
	ROM_RELOAD(0x4c00, 0x0400)
	ROM_RELOAD(0x8c00, 0x0400)
	ROM_RELOAD(0xcc00, 0x0400)
ROM_END


/*-------------------------------------------------------------------
/ Evil Fight (03/80)
/-------------------------------------------------------------------*/
ROM_START(evlfight)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("evfg08.bin", 0x0000, 0x0400, CRC(2cc2e79a) SHA1(17440512c419b3bb2012539666a5f052f3cd8c1d))
	ROM_RELOAD(0x4000, 0x0400)
	ROM_RELOAD(0x8000, 0x0400)
	ROM_RELOAD(0xc000, 0x0400)
	ROM_LOAD("evfg09.bin", 0x0400, 0x0400, CRC(5232dc4c) SHA1(6f95a578e9f09688e6ce8b0a622bcee887936c82))
	ROM_RELOAD(0x4400, 0x0400)
	ROM_RELOAD(0x8400, 0x0400)
	ROM_RELOAD(0xc400, 0x0400)
	ROM_LOAD("evfg10.bin", 0x0800, 0x0400, CRC(de2f754d) SHA1(0287a9975095bcbf03ddb2b374ff25c080c8020f))
	ROM_RELOAD(0x4800, 0x0400)
	ROM_RELOAD(0x8800, 0x0400)
	ROM_RELOAD(0xc800, 0x0400)
	ROM_LOAD("evfg11.bin", 0x0c00, 0x0400, CRC(5eb8ac02) SHA1(31c80e74a4272becf7014aa96eaf7de555e26cd6))
	ROM_RELOAD(0x4c00, 0x0400)
	ROM_RELOAD(0x8c00, 0x0400)
	ROM_RELOAD(0xcc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Mad Race (??/85?)
/-------------------------------------------------------------------*/
ROM_START(madrace)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("madrace.2a0", 0x0000, 0x0800, CRC(ab487c79) SHA1(a5df29b2af4c9d94d8bf54c5c91d1e9b5ca4d065))
	ROM_RELOAD(0x4000, 0x0800)
	ROM_RELOAD(0x8000, 0x0800)
	ROM_RELOAD(0xc000, 0x0800)
	ROM_LOAD("madrace.2b0", 0x0800, 0x0800, CRC(dcb54b39) SHA1(8e2ca7180f5ea3a28feb34b01f3387b523dbfa3b))
	ROM_RELOAD(0x4800, 0x0800)
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xc800, 0x0800)
	ROM_LOAD("madrace.2c0", 0x1000, 0x0800, CRC(b24ea245) SHA1(3f868ccbc4bfb77c40c4cc05dcd8eeca85ecd76f))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("madrace1.snd", 0x0000, 0x2000, CRC(49e956a5) SHA1(8790cc27a0fda7b8e07bee65109874140b4018a2))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("madrace2.snd", 0x2000, 0x0800, CRC(c19283d3) SHA1(42f9770c46030ef20a80cc94fdbe6548772aa525))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END


/*-------------------------------------------------------------------
/ Attack (10/80)
/-------------------------------------------------------------------*/
ROM_START(attack)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("attack8.bin", 0x0000, 0x0400, CRC(a5204b58) SHA1(afb4b81720f8d56e88f47fc842b23313824a1085))
	ROM_RELOAD(0x4000, 0x0400)
	ROM_RELOAD(0x8000, 0x0400)
	ROM_RELOAD(0xc000, 0x0400)
	ROM_LOAD("attack9.bin", 0x0400, 0x0400, CRC(bbd086b4) SHA1(6fc94b94beea482d8c8f5b3c69d3f218e2b2dfc4))
	ROM_RELOAD(0x4400, 0x0400)
	ROM_RELOAD(0x8400, 0x0400)
	ROM_RELOAD(0xc400, 0x0400)
	ROM_LOAD("attack10.bin", 0x0800, 0x0400, CRC(764925e4) SHA1(2f207ef87786d27d0d856c5816a570a59d89b718))
	ROM_RELOAD(0x4800, 0x0400)
	ROM_RELOAD(0x8800, 0x0400)
	ROM_RELOAD(0xc800, 0x0400)
	ROM_LOAD("attack11.bin", 0x0c00, 0x0400, CRC(972157b4) SHA1(23c90f23a34b34acfe445496a133b6022a749ccc))
	ROM_RELOAD(0x4c00, 0x0400)
	ROM_RELOAD(0x8c00, 0x0400)
	ROM_RELOAD(0xcc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Black Fever (12/80)
/-------------------------------------------------------------------*/
ROM_START(blkfever)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("blackf8.bin", 0x0000, 0x0400, CRC(916b8ed8) SHA1(ddc7e09b68e3e1a033af5dc5ec32ab5b0922a833))
	ROM_RELOAD(0x4000, 0x0400)
	ROM_RELOAD(0x8000, 0x0400)
	ROM_RELOAD(0xc000, 0x0400)
	ROM_LOAD("blackf9.bin", 0x0400, 0x0400, CRC(ecb72fdc) SHA1(d3598031b7170fab39727b3402b7053d4f9e1ca7))
	ROM_RELOAD(0x4400, 0x0400)
	ROM_RELOAD(0x8400, 0x0400)
	ROM_RELOAD(0xc400, 0x0400)
	ROM_LOAD("blackf10.bin", 0x0800, 0x0400, CRC(b3fae788) SHA1(e14e09cc7da1098abf2f60f26a8ec507e123ff7c))
	ROM_RELOAD(0x4800, 0x0400)
	ROM_RELOAD(0x8800, 0x0400)
	ROM_RELOAD(0xc800, 0x0400)
	ROM_LOAD("blackf11.bin", 0x0c00, 0x0400, CRC(5a97c1b4) SHA1(b9d7eb0dd55ef6d959c0fab48f710e4b1c8d8003))
	ROM_RELOAD(0x4c00, 0x0400)
	ROM_RELOAD(0x8c00, 0x0400)
	ROM_RELOAD(0xcc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ Zira (??/80)
/-------------------------------------------------------------------*/
ROM_START(zira)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("zira_u8.bin", 0x0000, 0x0800, CRC(53f8bf17) SHA1(5eb74f27bc65374a85dd44bbc8f6142488c226a2))
	ROM_RELOAD(0x4000, 0x0800)
	ROM_RELOAD(0x8000, 0x0800)
	ROM_RELOAD(0xc000, 0x0800)
	ROM_LOAD("zira_u9.bin", 0x0800, 0x0800, CRC(d50a2419) SHA1(81b157f579a433389506817b1b6e02afaa2cf0d5))
	ROM_RELOAD(0x4800, 0x0800)
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xc800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("zira.snd", 0x0000, 0x0400, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ Cerberus (03/82)
/-------------------------------------------------------------------*/
ROM_START(cerberup)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cerb8.cpu", 0x0000, 0x0800, CRC(021d0452) SHA1(496010e6892311b1cabcdac62296cd6aa0782c5d))
	ROM_RELOAD(0x4000, 0x0800)
	ROM_RELOAD(0x8000, 0x0800)
	ROM_RELOAD(0xc000, 0x0800)
	ROM_LOAD("cerb9.cpu", 0x0800, 0x0800, CRC(0fd41156) SHA1(95d1bf42c82f480825e3d907ae3c87b5f994fd2a))
	ROM_RELOAD(0x4800, 0x0800)
	ROM_RELOAD(0x8800, 0x0800)
	ROM_RELOAD(0xc800, 0x0800)
	ROM_LOAD("cerb10.cpu", 0x1000, 0x0800, CRC(785602e0) SHA1(f38df3156cd14ab21752dbc849c654802079eb33))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("cerb.snd", 0x0000, 0x2000, CRC(8af53a23) SHA1(a80b57576a1eb1b4544b718b9abba100531e3942))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
ROM_END

// ??/84 Nautilus
// ??/84 The Raid
// ??/85 Stop Ship
// ??/86 Flash Dragon
// ??/87 Phantom Ship
// ??/87 Skill Flight


GAME(1979,  antar,     0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Antar (set 1)",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  antar2,    antar,  play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Antar (set 2)",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  evlfight,  0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Evil Fight",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  attack,    0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Attack",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  blkfever,  0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Black Fever",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1982,  cerberup,  0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Cerberus (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  madrace,   0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Mad Race",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1980,  zira,      0,      play_2,  play_2, play_2_state,  play_2,  ROT0,  "Playmatic",    "Zira",           MACHINE_IS_SKELETON_MECHANICAL)
