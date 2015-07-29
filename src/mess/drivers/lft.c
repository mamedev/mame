// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

2013-09-09 Skeleton of LFT computer system. A search on the net produces
           no finds.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/terminal.h"


class lft_state : public driver_device
{
public:
	lft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE16_MEMBER(term_w);
	DECLARE_READ16_MEMBER(keyin_r);
	DECLARE_READ16_MEMBER(status_r);

private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(lft_mem, AS_PROGRAM, 16, lft_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x5ffff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(lft_io, AS_IO, 16, lft_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// screen 1
	AM_RANGE(0x00, 0x01) AM_READNOP
	AM_RANGE(0x04, 0x05) AM_READWRITE(keyin_r,term_w)
	// screen 2
	AM_RANGE(0x02, 0x03) AM_READNOP
	AM_RANGE(0x06, 0x07) AM_WRITENOP
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( lft )
INPUT_PORTS_END

READ16_MEMBER( lft_state::keyin_r )
{
	UINT16 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ16_MEMBER( lft_state::status_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( lft_state::kbd_put )
{
	m_term_data = data;
}

WRITE16_MEMBER( lft_state::term_w )
{
	m_terminal->write(space, 0, data & 0x7f); // fix backspace
}

void lft_state::machine_reset()
{
	m_term_data = 0;
}

static MACHINE_CONFIG_START( lft, lft_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80186, 4000000) // no idea
	MCFG_CPU_PROGRAM_MAP(lft_mem)
	MCFG_CPU_IO_MAP(lft_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(lft_state, kbd_put))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( lft1230 )
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD16_BYTE( "1230lf29", 0x0000, 0x2000, CRC(11c87367) SHA1(0879650aa98e19a4e6ca7b6ee7874f81c9c8ccfa) )
	ROM_LOAD16_BYTE( "1230lf42", 0x0001, 0x2000, CRC(ab82b620) SHA1(8c7d93950703f348e5ce0f9e376d157dd6098c6a) )
ROM_END

ROM_START( lft1510 )
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD16_BYTE( "1510lfev", 0x2000, 0x1000, CRC(47dbb290) SHA1(b557e9a54a30d9a16edfdef4a6b12a5393d30bf3) )
	ROM_IGNORE(0x1000)
	ROM_LOAD16_BYTE( "1510lfod", 0x2001, 0x1000, CRC(ba8c23fc) SHA1(d4b82f69fccd653b31e7bd05ee884b323ff0007b) )
	ROM_IGNORE(0x1000)
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE     INPUT    CLASS      INIT    COMPANY   FULLNAME     FLAGS */
COMP( ????, lft1510,   0,       0,    lft,       lft, driver_device,  0,     "LFT",   "LFT 1510", MACHINE_IS_SKELETON)
COMP( ????, lft1230, lft1510,   0,    lft,       lft, driver_device,  0,     "LFT",   "LFT 1230", MACHINE_IS_SKELETON)
