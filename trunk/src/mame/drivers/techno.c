/*
    Technoplay "2-2C 8008 LS" (68000 CPU)
*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"

class techno_state : public driver_device
{
public:
	techno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( techno_map, AS_PROGRAM, 16, techno_state )
	AM_RANGE(0x0000, 0xffffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( techno )
INPUT_PORTS_END

void techno_state::machine_reset()
{
}

static DRIVER_INIT( techno )
{
}

static MACHINE_CONFIG_START( techno, techno_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(techno_map)
MACHINE_CONFIG_END

ROM_START(xforce)
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD16_BYTE("ic15", 0x000001, 0x8000, CRC(fb8d2853) SHA1(0b0004abfe32edfd3ac15d66f90695d264c97eba))
	ROM_LOAD16_BYTE("ic17", 0x000000, 0x8000, CRC(122ef649) SHA1(0b425f81869bc359841377a91c39f44395502bff))
ROM_END

GAME(1987,  xforce,  0,  techno,  techno,  techno,  ROT0,  "Tecnoplay",    "X Force",     GAME_IS_SKELETON_MECHANICAL)
