/***************************************************************************

    Skeleton driver for 68k Single Board Computer

    29/03/2011

    http://www.kmitl.ac.th/~kswichit/68k/68k.html

    TODO:
    - Add RTC (type DS12887)
    - Add UART (type mc6850)

    All of the address and i/o decoding is done by a pair of XC9536
    mask-programmed custom devices.

    There are some chips used for unclear purposes (GPI, GPO, LCD).

    This computer has no sound, and no facility for saving or loading programs.

    When started, press ? key for a list of commands.

    Has 1MB of flash (which is actually just 12k of program),
    and 1MB of RAM. Memory map should probably be corrected.


****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

#define MACHINE_RESET_MEMBER(name) void name::machine_reset()

class c68ksbc_state : public driver_device
{
public:
	c68ksbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ16_MEMBER( c68ksbc_status_r );
	DECLARE_READ16_MEMBER( c68ksbc_terminal_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	virtual void machine_reset();
};

//bit 0 high = a key is ready; bit 1 high = ready to output to terminal
READ16_MEMBER( c68ksbc_state::c68ksbc_status_r )
{
	return (m_term_data) ? 3 : 2;
}

READ16_MEMBER( c68ksbc_state::c68ksbc_terminal_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}


static ADDRESS_MAP_START(c68ksbc_mem, AS_PROGRAM, 16, c68ksbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x002fff) AM_ROM
	AM_RANGE(0x003000, 0x5fffff) AM_RAM
	AM_RANGE(0x600000, 0x600001) AM_READ(c68ksbc_status_r)
	AM_RANGE(0x600002, 0x600003) AM_READ(c68ksbc_terminal_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0xff)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( c68ksbc )
INPUT_PORTS_END


MACHINE_RESET_MEMBER( c68ksbc_state )
{
	m_term_data = 0;
}

WRITE8_MEMBER( c68ksbc_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(c68ksbc_state, kbd_put)
};

static MACHINE_CONFIG_START( c68ksbc, c68ksbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) // text says 8MHz, schematic says 10MHz
	MCFG_CPU_PROGRAM_MAP(c68ksbc_mem)

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( 68ksbc )
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD( "t68k.bin", 0x0000, 0x2f78, CRC(20a8d0d0) SHA1(544fd8bd8ed017115388c8b0f7a7a59a32253e43) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                FULLNAME               FLAGS */
COMP( 2002, 68ksbc,   0,       0,    c68ksbc,   c68ksbc, driver_device,  0,  "Ichit Sirichote", "68k Single Board Computer", GAME_NO_SOUND_HW)
