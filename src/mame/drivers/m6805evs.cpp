// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************

Motorola M68HC05EVS evaluation system

Chips:
Main board: XC68HC26P, R65C52P2, MS62256l-70PC, MS6264L-70PC, eprom. Xtal = 3.6864MHz
Emulator board: MC68C705P9CP, undumped 28-pin prom. Xtal = 4MHz

R65C52 = Dual ACIA with inbuilt baud rate divider, uses 8 addresses, uses the 3.6864MHz crystal
XC68HC26P = PPI (3 ports), uses 8 addresses.

2014-01-12 Skeleton driver

The rom is larger than the available address space, but not all of it is programmed. The code
ranges are 800-18FF,1FF0-1FFF. There must be a banking scheme in use.

Memory map guess
000-07F Stack (and user ram?)
080-0FF RAM (or devices?)
100-FFF ROM

ToDo:
- Everything

******************************************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"


class m6805evs_state : public driver_device
{
public:
	m6805evs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override;
};


static ADDRESS_MAP_START( m6805evs_mem, AS_PROGRAM, 8, m6805evs_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x07ff) AM_ROM AM_REGION("roms", 0x1100)
	AM_RANGE(0x0800, 0x0fef) AM_ROM AM_REGION("roms", 0x0800)
	AM_RANGE(0x0ff0, 0x0fff) AM_ROM AM_REGION("roms", 0x1ff0)
ADDRESS_MAP_END

static INPUT_PORTS_START( m6805evs )
INPUT_PORTS_END

void m6805evs_state::machine_reset()
{
}

static MACHINE_CONFIG_START( m6805evs, m6805evs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68705, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(m6805evs_mem)
MACHINE_CONFIG_END

ROM_START(m6805evs)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "evsbug12.bin", 0x0000, 0x2000, CRC(8b581aef) SHA1(eacf425cc8a042085ccc4097cc61570b633b1e38) )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS         INIT    COMPANY      FULLNAME */
COMP( 1990, m6805evs, 0,        0,      m6805evs, m6805evs, driver_device, 0,    "Motorola",  "M68HC05EVS", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
