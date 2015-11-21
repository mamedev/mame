// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/********************************************************************************

    Pinball
    Bally Kiss 8035 prototype

*********************************************************************************/


#include "emu.h"
#include "cpu/mcs48/mcs48.h"

class kissp_state : public driver_device
{
public:
	kissp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(kissp);
};


static ADDRESS_MAP_START( kissp_map, AS_PROGRAM, 8, kissp_state )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( kissp )
INPUT_PORTS_END

void kissp_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(kissp_state,kissp)
{
}

static MACHINE_CONFIG_START( kissp, kissp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8035, 6000000/15)
	MCFG_CPU_PROGRAM_MAP(kissp_map)
MACHINE_CONFIG_END

ROM_START(kissp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "kiss8755.bin", 0x4000, 0x0800, CRC(894c1052) SHA1(579ce3c8ec374f2cd17928ab92311f035ecee341))
	ROM_RELOAD( 0x0000, 0x0800)
	ROM_LOAD( "kissprot.u5", 0x1000, 0x1000, CRC(38a2ef5a) SHA1(4ffdb2e9aa30417d506af3bc4b6835ba1dc80e4f))
	ROM_LOAD( "kissprot.u6", 0x2000, 0x1000, CRC(bcdfaf1d) SHA1(d21bebbf702b400eb71f8c88be50a180a5ac260a))
	ROM_LOAD( "kissprot.u7", 0x3000, 0x0800, CRC(d97da1d3) SHA1(da771a08969a12105c7adc9f9e3cbd1677971e79))
	ROM_RELOAD( 0x4800, 0x0800)
ROM_END


GAME( 1979,  kissp,  kiss,  kissp,  kissp, kissp_state,  kissp,  ROT0,  "Bally", "Kiss (prototype)", MACHINE_IS_SKELETON_MECHANICAL )
