#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class rowamet_state : public driver_device
{
public:
	rowamet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( rowamet_map, AS_PROGRAM, 8, rowamet_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( rowamet )
INPUT_PORTS_END

void rowamet_state::machine_reset()
{
}

static DRIVER_INIT( rowamet )
{
}

static MACHINE_CONFIG_START( rowamet, rowamet_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 1888888)
	MCFG_CPU_PROGRAM_MAP(rowamet_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Conan (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Heavy Metal (????)
/-------------------------------------------------------------------*/
ROM_START(heavymtl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hvymtl_c.bin", 0x0000, 0x1000, CRC(8f36d3da) SHA1(beec79c5d794ede96d95105bad7466b67762606d))
	ROM_LOAD("hvymtl_b.bin", 0x1000, 0x1000, CRC(357f1252) SHA1(ddc55ded0dc1c8632c31d809bfadfb45ae248cfd))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hvymtl_s.bin", 0x0000, 0x1000, CRC(c525e6cb) SHA1(144e06fbbdd1f3e45ccca8bace6b04f876b1312c))
ROM_END

/*-------------------------------------------------------------------
/ Vulcan IV (1982)
/-------------------------------------------------------------------*/


GAME(198?,  heavymtl,  0,  rowamet,  rowamet,  rowamet,  ROT0,  "Rowamet",    "Heavy Metal",      GAME_IS_SKELETON_MECHANICAL)
