// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        CCS Model 300

        2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

There's no info available on this system, however the bankswitching
appears to be the same as their other systems.

Early on, it does a read from port F2. If bit 3 is low, the system becomes
a Model 400.

Since IM2 is used, it is assumed there are Z80 peripherals on board.
There's unknown usage of ports 11 thru 1B, 34, and F0 thru F2.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class ccs300_state : public driver_device
{
public:
	ccs300_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_DRIVER_INIT(ccs300);
	DECLARE_MACHINE_RESET(ccs300);
	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_READ8_MEMBER(port11_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(ccs300_mem, AS_PROGRAM, 8, ccs300_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ccs300_io, AS_IO, 8, ccs300_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READWRITE(port10_r,port10_w)
	AM_RANGE(0x11, 0x11) AM_READ(port11_r)
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs300 )
INPUT_PORTS_END

//*************************************
//
//  Keyboard
//
//*************************************
READ8_MEMBER( ccs300_state::port10_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ccs300_state::port11_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( ccs300_state::port10_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

WRITE8_MEMBER( ccs300_state::kbd_put )
{
	m_term_data = data;
}

//*************************************
//
//  Machine
//
//*************************************
WRITE8_MEMBER( ccs300_state::port40_w )
{
	membank("bankr0")->set_entry( (data) ? 1 : 0);
}

MACHINE_RESET_MEMBER( ccs300_state, ccs300 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( ccs300_state, ccs300 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static MACHINE_CONFIG_START( ccs300, ccs300_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ccs300_mem)
	MCFG_CPU_IO_MAP(ccs300_io)
	MCFG_MACHINE_RESET_OVERRIDE(ccs300_state, ccs300)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ccs300_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs300 )
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ccs300.rom", 0x10000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT   MACHINE    INPUT    CLASS          INIT          COMPANY                          FULLNAME       FLAGS */
COMP( 19??, ccs300, ccs2810, 0,       ccs300,    ccs300,  ccs300_state,  ccs300,   "California Computer Systems", "CCS Model 300", MACHINE_IS_SKELETON )
