// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Skeleton driver for Televideo TS803

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class ts803_state : public driver_device
{
public:
	ts803_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_reset();
};

static ADDRESS_MAP_START(ts803_mem, AS_PROGRAM, 8, ts803_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ts803_io, AS_IO, 8, ts803_state)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ts803 )
INPUT_PORTS_END


void ts803_state::machine_reset()
{
}

static MACHINE_CONFIG_START( ts803, ts803_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(ts803_mem)
	MCFG_CPU_IO_MAP(ts803_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts803h )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "180001-37 rev d 803 5 23 84", 0x0000, 0x2000, CRC(0aa658a7) SHA1(42d0a89c2ff9b6588cd88bdb1f800fac540dccbb) )

	ROM_REGION(0x10000, "proms", 0)
	ROM_LOAD( "8000134.bin", 0x000, 0x100, CRC(231fe6d6) SHA1(3c052ba4b74547e0e2451fa1ae67bbcb83a18bab) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 19??, ts803h,  0,   0,     ts803,     ts803, driver_device,  0,  "Televideo", "TS803H", MACHINE_IS_SKELETON )
