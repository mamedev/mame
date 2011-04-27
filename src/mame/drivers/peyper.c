#include "emu.h"
#include "cpu/z80/z80.h"

extern const char layout_pinball[];

class peyper_state : public driver_device
{
public:
	peyper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};

static ADDRESS_MAP_START( peyper_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x5FFF) AM_ROM
	AM_RANGE(0x6000, 0x67FF) AM_RAM //AM_BASE_GENERIC(nvram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( peyper_io, AS_IO, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( peyper )
INPUT_PORTS_END

static MACHINE_RESET( peyper )
{
}

static DRIVER_INIT( peyper )
{
}

static MACHINE_CONFIG_START( peyper, peyper_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(peyper_map)
	MCFG_CPU_IO_MAP(peyper_io)

	MCFG_MACHINE_RESET( peyper )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Gamatron (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Hang-On (1988)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Night Fever (1979)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Odisea Paris-Dakar (1987)
/-------------------------------------------------------------------*/
ROM_START(odisea)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("odiseaa.bin", 0x0000, 0x2000, CRC(29a40242) SHA1(321e8665df424b75112589fc630a438dc6f2f459))
	ROM_LOAD("odiseab.bin", 0x2000, 0x2000, CRC(8bdf7c17) SHA1(7202b4770646fce5b2ba9e3b8ca097a993123b14))
	ROM_LOAD("odiseac.bin", 0x4000, 0x2000, CRC(832dee5e) SHA1(9b87ffd768ab2610f2352adcf22c4a7880de47ab))
ROM_END

/*-------------------------------------------------------------------
/ Odin De Luxe (1985)
/-------------------------------------------------------------------*/
ROM_START(odin_dlx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1a.bin", 0x0000, 0x2000, CRC(4fca9bfc) SHA1(05dce75919375d01a306aef385bcaac042243695))
	ROM_LOAD("2a.bin", 0x2000, 0x2000, CRC(46744695) SHA1(fdbd8a93b3e4a9697e77e7d381759829b86fe28b))
ROM_END

/*-------------------------------------------------------------------
/ Pole Position (1987)
/-------------------------------------------------------------------*/
ROM_START(poleposn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(fdd37f6d) SHA1(863fef32ab9b5f3aca51788b6be9373a01fa0698))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(967cb72b) SHA1(adef17018e2caf65b64bbfef72fe159b9704c409))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(461fe9ca) SHA1(01bf35550e2c55995f167293746f355cfd484af1))
ROM_END

/*-------------------------------------------------------------------
/ Solar Wars (1986)
/-------------------------------------------------------------------*/
ROM_START(solarwap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("solarw1c.bin", 0x0000, 0x2000, CRC(aa6bf0cd) SHA1(7332a4b1679841283d846f3e4f1792cb8e9529bf))
	ROM_LOAD("solarw2.bin",  0x2000, 0x2000, CRC(95e2cbb1) SHA1(f9ab3222ca0b9e0796030a7a618847a4e8f77957))
ROM_END

/*-------------------------------------------------------------------
/ Star Wars (1987)
/-------------------------------------------------------------------*/
ROM_START(sonstwar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sw1.bin", 0x0000, 0x2000, CRC(a2555d92) SHA1(5c82be85bf097e94953d11c0d902763420d64de4))
	ROM_LOAD("sw2.bin", 0x2000, 0x2000, CRC(c2ae34a7) SHA1(0f59242e3aec5da7111e670c4d7cf830d0030597))
	ROM_LOAD("sw3.bin", 0x4000, 0x2000, CRC(aee516d9) SHA1(b50e54d4d5db59e3fb71fb000f9bc5e34ff7de9c))
ROM_END

/*-------------------------------------------------------------------
/ Wolf Man (1987)
/-------------------------------------------------------------------*/
ROM_START(wolfman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("memoriaa.bin", 0x0000, 0x2000, CRC(1fec83fe) SHA1(5dc887d0fa00129ae31451c03bfe442f87dd2f54))
	ROM_LOAD("memoriab.bin", 0x2000, 0x2000, CRC(62a1e3ec) SHA1(dc472c7c9d223820f8f1031c92e36890c1fcba7d))
	ROM_LOAD("memoriac.bin", 0x4000, 0x2000, CRC(468f16f0) SHA1(66ce0464d82331cfc0ac1f6fbd871066e4e57262))
ROM_END

GAME( 1987, odisea,   0, peyper, peyper, peyper, ROT0, "Peyper", "Odisea Paris-Dakar",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, wolfman,  0, peyper, peyper, peyper, ROT0, "Peyper", "Wolf Man",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1985, odin_dlx, 0, peyper, peyper, peyper, ROT0, "Sonic", "Odin De Luxe",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1986, solarwap, 0, peyper, peyper, peyper, ROT0, "Sonic", "Solar Wars (Sonic)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, poleposn, 0, peyper, peyper, peyper, ROT0, "Sonic", "Pole Position (Sonic)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1987, sonstwar, 0, peyper, peyper, peyper, ROT0, "Sonic", "Star Wars (Sonic)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
