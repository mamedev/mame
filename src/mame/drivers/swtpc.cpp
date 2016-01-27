// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SWTPC 6800

        10/12/2009 Skeleton driver.

        http://www.swtpc.com/mholley/swtpc_6800.htm

    bios 0 (SWTBUG) is made for a PIA (parallel) interface.
    bios 1 (MIKBUG) is made for a ACIA (serial) interface at the same address.
    MIKBUG will actually read the bits as they arrive and assemble a byte.

    Since the interface is optional, it is not on the schematics, so I've
    looked at the code and come up with something horrible that works.

    Note: All commands must be in uppercase. See the SWTBUG manual.

    ToDo:
        - Add PIA and work out the best way to hook up the keyboard. As can be
          seen from the code below, it might be tricky.

        - Finish conversion to modern.

Commands:
B Breakpoint
C Clear screen
D Disk boot
E End of tape
F Find a byte
G Goto
J Jump
L Ascii Load
M Memory change (enter to quit, - to display next byte)
O Optional Port
P Ascii Punch
R Register dump
Z Goto Prom (0xC000)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class swtpc_state : public driver_device
{
public:
	swtpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(swtpc_status_r);
	DECLARE_READ8_MEMBER(swtpc_terminal_r);
	DECLARE_READ8_MEMBER(swtpc_tricky_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset() override;
};

// bit 0 - ready to receive a character; bit 1 - ready to send a character to the terminal
READ8_MEMBER( swtpc_state::swtpc_status_r )
{
	return (m_term_data) ? 3 : 0x82;
}

READ8_MEMBER( swtpc_state::swtpc_terminal_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( swtpc_state::swtpc_tricky_r )
{
	UINT8 ret = m_term_data;
	return ret;
}

static ADDRESS_MAP_START(swtpc_mem, AS_PROGRAM, 8, swtpc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x8004, 0x8004 ) AM_READ(swtpc_status_r)
	AM_RANGE( 0x8005, 0x8005 ) AM_READ(swtpc_terminal_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE( 0x8007, 0x8007 ) AM_READ(swtpc_tricky_r)
	AM_RANGE( 0xa000, 0xa07f ) AM_RAM
	AM_RANGE( 0xe000, 0xe3ff ) AM_MIRROR(0x1c00) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( swtpc )
INPUT_PORTS_END


void swtpc_state::machine_reset()
{
}

WRITE8_MEMBER( swtpc_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( swtpc, swtpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(swtpc_mem)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(swtpc_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swtpc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "swt", "SWTBUG" )
	ROMX_LOAD( "swtbug.bin", 0xe000, 0x0400, CRC(f9130ef4) SHA1(089b2d2a56ce9526c3e78ce5d49ce368b9eabc0c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "mik", "MIKBUG" )
	ROMX_LOAD( "mikbug.bin", 0xe000, 0x0400, CRC(e7f4d9d0) SHA1(5ad585218f9c9c70f38b3c74e3ed5dfe0357621c), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1975, swtpc,  0,       0,      swtpc,     swtpc, driver_device,    0, "Southwest Technical Products Corporation", "SWTPC 6800", MACHINE_NO_SOUND)
