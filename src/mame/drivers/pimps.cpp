// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        P.I.M.P.S. (Personal Interactive MicroProcessor System)

        06/12/2009 Skeleton driver.

Commands:
A Assemble Code
D Dump Memory
E Enter the Text Editor
F Full Duplex Host Operation
G Go To
H Half Duplex Host Operation
M Memory
P Port
S Substitute
T Transparent Mode
U Usart Parameters for Host
V Virtual Memory

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class pimps_state : public driver_device
{
public:
	pimps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_terminal(*this, TERMINAL_TAG),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER(term_status_r);
	DECLARE_READ8_MEMBER(term_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset();

	required_device<generic_terminal_device> m_terminal;
	required_device<cpu_device> m_maincpu;
};


// should be 8251 UART


READ8_MEMBER( pimps_state::term_status_r )
{
	return (m_term_data) ? 3 : 1;
}

READ8_MEMBER( pimps_state::term_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

static ADDRESS_MAP_START(pimps_mem, AS_PROGRAM, 8, pimps_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pimps_io, AS_IO, 8, pimps_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf0, 0xf0) AM_READ(term_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xf1, 0xf1) AM_READ(term_status_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pimps )
INPUT_PORTS_END


void pimps_state::machine_reset()
{
	m_term_data = 0;
}

WRITE8_MEMBER( pimps_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( pimps, pimps_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(pimps_mem)
	MCFG_CPU_IO_MAP(pimps_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(pimps_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pimps )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pimps.bin", 0xf000, 0x1000, CRC(5da1898f) SHA1(d20e31d0981a1f54c83186dbdfcf4280e49970d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 197?, pimps,  0,      0,       pimps,     pimps, driver_device,   0, "Henry Colford", "P.I.M.P.S.", MACHINE_NO_SOUND_HW)
