#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class hankin_state : public driver_device
{
public:
	hankin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( hankin_map, AS_PROGRAM, 8, hankin_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( hankin )
INPUT_PORTS_END

void hankin_state::machine_reset()
{
}

static DRIVER_INIT( hankin )
{
}

static MACHINE_CONFIG_START( hankin, hankin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, 900000)
	MCFG_CPU_PROGRAM_MAP(hankin_map)
MACHINE_CONFIG_END

/*--------------------------------
/ FJ Holden
/-------------------------------*/
ROM_START(fjholden)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "fj_ic2.mpu", 0x1000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD( "fj_ic3.mpu", 0x1800, 0x0800, CRC(ceaeb7d3) SHA1(9e479b985f8500983e71d6ff33ee94160e99650d))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("fj_ic14.snd", 0x1000, 0x0800, CRC(34fe3587) SHA1(132714675a23c101ceb5a4d544818650ae5ccea2))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("fj_ic3.snd", 0xf000, 0x0200, CRC(09d3f020) SHA1(274be0b94d341ee43357011691da82e83a7c4a00))
ROM_END

/*--------------------------------
/ Howzat!
/-------------------------------*/
ROM_START(howzat)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "hz_ic2.mpu", 0x1000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD( "hz_ic3.mpu", 0x1800, 0x0800, CRC(d13df4bc) SHA1(27a70260698d3eaa7cf7a56edc5dd9a4af3f4103))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hz_ic14.snd", 0x1000, 0x0800, CRC(0e3fdb59) SHA1(cae3c85b2c32a0889785f770ece66b959bcf21e1))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("hz_ic3.snd", 0xf000, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ Orbit 1
/-------------------------------*/
ROM_START(orbit1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "o1_ic2.mpu", 0x1000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD( "o1_ic3.mpu", 0x1800, 0x0800, CRC(fe7b61be) SHA1(c086b0433bb9ab3f2139c705d4372beb1656b27f))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("o1_ic14.snd", 0x1000, 0x0800, CRC(323bfbd5) SHA1(2e89aa4fcd33f9bfeea5c310ffb0a5be45fb70a9))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("o1_ic3.snd", 0xf000, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ Shark
/-------------------------------*/
ROM_START(shark)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "shk_ic2.mpu", 0x1000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD( "shk_ic3.mpu", 0x1800, 0x0800, CRC(c3ef936c) SHA1(14668496d162a77e03c1142bef2956d5b76afc99))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("shk_ic14.snd", 0x1000, 0x0800, CRC(8f8b0e48) SHA1(72d94aa9b32c603b1ca681b0ab3bf8ddbf5c9afe))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("shk_ic3.snd", 0xf000, 0x0200, CRC(dfc57606) SHA1(638853c8e46bf461f2ecde02b8b2aa68c2d414b8))
ROM_END

/*--------------------------------
/ The Empire Strike Back
/-------------------------------*/
ROM_START(empsback)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "sw_ic2.mpu", 0x1000, 0x0800, CRC(b47bc2c7) SHA1(42c985d83a9454fcd08b87e572e5563ebea0d052))
	ROM_LOAD( "sw_ic3.mpu", 0x1800, 0x0800, CRC(837ffe32) SHA1(9affc5d9345ce15394553d3204e5234cc6348d2e))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("sw_ic14.snd", 0x1000, 0x0800, CRC(c1eeb53b) SHA1(7a800dd0a8ae392e14639e1819198d4215cc2251))
	ROM_RELOAD( 0xf800, 0x0800)
	ROM_LOAD("sw_ic3.snd", 0xf000, 0x0200, CRC(db214f65) SHA1(1a499cf2059a5c0d860d5a4251a89a5735937ef8))
ROM_END


GAME(1978,  fjholden,  0,  hankin,  hankin,  hankin,  ROT0,  "Hankin",    "FJ Holden",			GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  howzat,    0,  hankin,  hankin,  hankin,  ROT0,  "Hankin",    "Howzat!",				GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  orbit1,    0,  hankin,  hankin,  hankin,  ROT0,  "Hankin",    "Orbit 1",				GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  shark,     0,  hankin,  hankin,  hankin,  ROT0,  "Hankin",    "Shark",				GAME_IS_SKELETON_MECHANICAL)
GAME(1981,  empsback,  0,  hankin,  hankin,  hankin,  ROT0,  "Hankin",    "The Empire Strike Back",GAME_IS_SKELETON_MECHANICAL)
