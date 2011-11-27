#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"

class vd_state : public driver_device
{
public:
	vd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
};


static ADDRESS_MAP_START( vd_map, AS_PROGRAM, 8, vd_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( vd )
INPUT_PORTS_END

void vd_state::machine_reset()
{
}

static DRIVER_INIT( vd )
{
}

static MACHINE_CONFIG_START( vd, vd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(vd_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Break '86 (1986)
/-------------------------------------------------------------------*/
ROM_START(break86)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("break1.cpu", 0x0000, 0x2000, CRC(c187d263) SHA1(1790566799ccc41cd5445936e86f945150e24e8a))
	ROM_LOAD("break2.cpu", 0x2000, 0x2000, CRC(ed8f84ab) SHA1(ff5d7e3c373ca345205e8b92c6ce7b02f36a3d95))
	ROM_LOAD("break3.cpu", 0x4000, 0x2000, CRC(3cdfedc2) SHA1(309fd04c81b8facdf705e6297c0f4d507957ae1f))
ROM_END

/*-------------------------------------------------------------------
/ Papillon (1986)
/-------------------------------------------------------------------*/


GAME(1986,  break86,  0,    vd,  vd,  vd,  ROT0,  "Videodens",    "Break '86",     GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
