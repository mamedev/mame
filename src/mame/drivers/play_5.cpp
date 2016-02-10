// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    Playmatic MPU 5
*/


#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class play_5_state : public driver_device
{
public:
	play_5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cosmac_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override;
public:
	DECLARE_DRIVER_INIT(play_5);
};


static ADDRESS_MAP_START( play_5_map, AS_PROGRAM, 8, play_5_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( play_5 )
INPUT_PORTS_END

void play_5_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(play_5_state,play_5)
{
}

static MACHINE_CONFIG_START( play_5, play_5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_5_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ KZ-26 (1984)
/-------------------------------------------------------------------*/
ROM_START(kz26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("kz26.cpu", 0x0000, 0x2000, CRC(8030a699) SHA1(4f86b325801d8ce16011f7b6ba2f3633e2f2af35))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sound1.su3", 0x0000, 0x2000, CRC(8ad1a804) SHA1(6177619f09af4302ffddd8c0c1b374dab7f47e91))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("sound2.su4", 0x2000, 0x0800, CRC(355dc9ad) SHA1(eac8bc27157afd908f9bc5b5a7c40be5b9427269))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Spain 82 (10/82)
/-------------------------------------------------------------------*/
ROM_START(spain82)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spaic12.bin", 0x0000, 0x1000, CRC(cd37ecdc) SHA1(ff2d406b6ac150daef868121e5857a956aabf005))
	ROM_RELOAD(0x4000, 0x1000)
	ROM_RELOAD(0x8000, 0x1000)
	ROM_RELOAD(0xc000, 0x1000)
	ROM_LOAD("spaic11.bin", 0x1000, 0x0800, CRC(c86c0801) SHA1(1b52539538dae883f9c8fe5bc6454f9224780d11))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("spasnd.bin", 0x0000, 0x2000, CRC(62412e2e) SHA1(9e48dc3295e78e1024f726906be6e8c3fe3e61b1))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
ROM_END

/*-------------------------------------------------------------------
/ ??/84 Nautilus
/-------------------------------------------------------------------*/
ROM_START(nautilus)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nautilus.rom", 0x0000, 0x2000, CRC(197e5492) SHA1(0f83fc2e742fd0cca0bd162add4bef68c6620067))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("nautilus.snd", 0x0000, 0x2000, CRC(413d110f) SHA1(8360f652296c46339a70861efb34c41e92b25d0e))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
ROM_END

/*-------------------------------------------------------------------
/ ??/84 The Raid
/-------------------------------------------------------------------*/
ROM_START(theraid)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("theraid.rom", 0x0000, 0x2000, CRC(97aa1489) SHA1(6b691b287138cc78cfc1010f380ff8c66342c39b))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("theraid.snd", 0x0000, 0x2000, CRC(e33f8363) SHA1(e7f251c334b15e12b1eb7e079c2e9a5f64338052))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
ROM_END

/*-------------------------------------------------------------------
/ 11/84 UFO-X
/-------------------------------------------------------------------*/
ROM_START(ufo_x)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ufoxcpu.rom", 0x0000, 0x2000, CRC(cf0f7c52) SHA1(ce52da05b310ac84bdd57609e21b0401ee3a2564))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("ufoxu3.rom", 0x0000, 0x2000, CRC(6ebd8ee1) SHA1(83522b76a755556fd38d7b292273b4c68bfc0ddf))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("ufoxu4.rom", 0x2000, 0x0800, CRC(aa54ede6) SHA1(7dd7e2852d42aa0f971936dbb84c7708727ce0e7))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Rock 2500
/-------------------------------------------------------------------*/
ROM_START(rock2500)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("r2500cpu.rom", 0x0000, 0x2000, CRC(9c07e373) SHA1(5bd4e69d11e69fdb911a6e65b3d0a7192075abc8))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("r2500snd.rom", 0x0000, 0x2000, CRC(24fbaeae) SHA1(20ff35ed689291f321e483287a977c02e84d4524))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Star Fire
/-------------------------------------------------------------------*/
ROM_START(starfirp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starfcpu.rom", 0x0000, 0x2000, CRC(450ddf20) SHA1(c63c4e3833ffc1f69fcec39bafecae9c80befb2a))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

ROM_START(starfirpa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.rom", 0x0000, 0x2000, CRC(29bac350) SHA1(ab3e3ea4881be954f7fa7278800ffd791c4581da))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ ??/86 Flash Dragon
/-------------------------------------------------------------------*/
ROM_START(fldragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fldrcpu1.rom", 0x0000, 0x2000, CRC(e513ded0) SHA1(64ed3dcff53311fb93bd50d105a4c1186043fdd7))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("fldrcpu2.rom", 0x2000, 0x2000, CRC(6ff2b276) SHA1(040b614f0b0587521ef5550b5587b94a7f3f178b))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, NO_DUMP)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, NO_DUMP)
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Stop Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ ??/87 Skill Flight
/-------------------------------------------------------------------*/
ROM_START(sklflite)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("skflcpu1.rom", 0x0000, 0x2000, CRC(8f833b55) SHA1(1729203582c22b51d1cc401aa8f270aa5cdadabe))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("skflcpu2.rom", 0x2000, 0x2000, CRC(ffc497aa) SHA1(3e88539ae1688322b9268f502d8ca41cffb28df3))
	ROM_RELOAD(0x6000, 0x2000)
	ROM_RELOAD(0xa000, 0x2000)
	ROM_RELOAD(0xe000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sfsndu3.rom", 0x0000, 0x2000, NO_DUMP)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("sfsndu4.rom", 0x2000, 0x0800, NO_DUMP)
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Phantom Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Trailer (1985)
/-------------------------------------------------------------------*/
ROM_START(trailer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trcpu.rom", 0x0000, 0x2000, CRC(cc81f84d) SHA1(7a3282a47de271fde84cfddbaceb118add0df116))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("trsndu3.rom", 0x0000, 0x2000, CRC(05975c29) SHA1(e54d3a5613c3e39fc0338a53dbadc2e91c09ffe3))
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x8000, 0x2000)
	ROM_RELOAD(0xc000, 0x2000)
	ROM_LOAD("trsndu4.rom", 0x2000, 0x0800, CRC(bda2a735) SHA1(134b5abb813ed8bf2eeac0861b4c88c7176582d8))
	ROM_RELOAD(0x6000, 0x0800)
	ROM_RELOAD(0xa000, 0x0800)
	ROM_RELOAD(0xe000, 0x0800)
ROM_END

GAME(1982,  spain82,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Spain '82",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  nautilus,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Nautilus",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  theraid,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "The Raid",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  ufo_x,      0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "UFO-X",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  kz26,       0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "KZ-26",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  rock2500,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Rock 2500",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  starfirp,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Star Fire",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  starfirpa,  starfirp,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Star Fire (alternate set)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  trailer,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Trailer",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986,  fldragon,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Flash Dragon",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  sklflite,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Skill Flight (Playmatic)",      MACHINE_IS_SKELETON_MECHANICAL)
