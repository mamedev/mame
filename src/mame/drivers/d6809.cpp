// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

        6809 Portable

        12/05/2009 Skeleton driver.
        21/09/2011 connected to terminal, notes added [Robbbert]

Chips used:
- 6809E CPU
- 6845 CRTC
- 6840 CTC
- 6551 UART Console
- 6551 UART Aux
- 6850 UART Unknown purpose
- uPD765 FDC
- 2764 8K ROM for CPU
- 2732 4K ROM for Chargen (not dumped)
- 6x 6264 RAM
- 3x 5516 RAM
- XTAL: 16MHz

So much for the official documentation.

In practice, it reads/writes to a terminal, and doesn't use most of the other
devices.

'maincpu' (F9DD): unmapped program memory write to 00F0 = 05 & FF
'maincpu' (F9E3): unmapped program memory read from 0001 & FF <----- these 2 are CLR 0001
'maincpu' (F9E3): unmapped program memory write to 0001 = 00 & FF
'maincpu' (F9E6): unmapped program memory read from 0005 & FF <----- these 2 are CLR 0005
'maincpu' (F9E6): unmapped program memory write to 0005 = 00 & FF
'maincpu' (F9E9): unmapped program memory write to 0002 = 0B & FF <-- these 2 are STD 0002
'maincpu' (F9E9): unmapped program memory write to 0003 = 1E & FF
'maincpu' (F9EC): unmapped program memory write to 0006 = 0B & FF <-- these 2 are STD 0006
'maincpu' (F9EC): unmapped program memory write to 0007 = 1E & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF <-- the remainder seems to be disk related
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA41): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA46): unmapped program memory write to 00F0 = 04 & FF
'maincpu' (FA82): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF
'maincpu' (FA4D): unmapped program memory write to 00F2 = 00 & FF
'maincpu' (FA52): unmapped program memory write to 00F3 = 00 & FF
'maincpu' (FA57): unmapped program memory write to 00F4 = 00 & FF
'maincpu' (FA5C): unmapped program memory write to 00F5 = 00 & FF
'maincpu' (FA61): unmapped program memory write to 00F6 = 02 & FF
'maincpu' (FA66): unmapped program memory write to 00F7 = 09 & FF
'maincpu' (FA6B): unmapped program memory write to 00F0 = 01 & FF
'maincpu' (FA6E): unmapped program memory read from 00F0 & FF <-- now it gives up & prints an error


**********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class d6809_state : public driver_device
{
public:
	d6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_WRITE8_MEMBER( kbd_put );
	DECLARE_READ8_MEMBER( term_r );
	DECLARE_WRITE8_MEMBER( term_w );
	UINT8 m_term_data;
	virtual void machine_reset() override;
};

READ8_MEMBER( d6809_state::term_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( d6809_state::term_w )
{
	if ((data > 0) && (data < 0x80))
		m_terminal->write(space, 0, data);
}

static ADDRESS_MAP_START( d6809_mem, AS_PROGRAM, 8, d6809_state )
	ADDRESS_MAP_UNMAP_HIGH
	// 00-FF is for various devices.
	AM_RANGE(0x00ff, 0x00ff) AM_READWRITE(term_r,term_w)
	AM_RANGE(0x1000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( d6809 )
INPUT_PORTS_END


WRITE8_MEMBER( d6809_state::kbd_put )
{
	m_term_data = data;
}

void d6809_state::machine_reset()
{
}


static MACHINE_CONFIG_START( d6809, d6809_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(d6809_mem)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(d6809_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( d6809 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "d6809.rom", 0xe000, 0x2000, CRC(2ceb40b8) SHA1(780111541234b4f0f781a118d955df61daa56e7e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY     FULLNAME       FLAGS */
COMP( 1983, d6809,  0,      0,       d6809,     d6809, driver_device,   0,     "Dunfield", "6809 Portable", MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
