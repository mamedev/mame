/*
    DataEast/Sega Version 1
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6800/m6800.h"

class de_1_state : public driver_device
{
public:
	de_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( de_1_map, AS_PROGRAM, 8, de_1_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( de_1 )
INPUT_PORTS_END

void de_1_state::machine_reset()
{
}

static DRIVER_INIT( de_1 )
{
}

static MACHINE_CONFIG_START( de_1, de_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(de_1_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Laser War - CPU Rev 1 /Alpha Type 1 - 32K ROM - 32/64K Sound Roms
/-------------------------------------------------------------------*/
ROM_START(lwar_a83)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar8-3.c5", 0x8000, 0x8000, CRC(eee158ee) SHA1(54db2342bdd15b16fee906dc65f183a957fd0012))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("lwar_e7.snd", 0x20000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END

ROM_START(lwar_e90)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar9-0.e5", 0x8000, 0x8000, CRC(b596151f) SHA1(10dade79ded71625770ec7e21ea50b7aa64023d0))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_RELOAD( 0x10000, 0x10000)
	ROM_LOAD("lwar_e7.snd", 0x20000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
	ROM_RELOAD( 0x30000, 0x10000)
ROM_END


GAME(1987,  lwar_a83,  0,         de_1,  de_1,  de_1,  ROT0,  "Data East",    "Laser War (8.3)",           GAME_IS_SKELETON_MECHANICAL)
GAME(1987,  lwar_e90,  lwar_a83,  de_1,  de_1,  de_1,  ROT0,  "Data East",    "Laser War (9.0 Europe)",    GAME_IS_SKELETON_MECHANICAL)
