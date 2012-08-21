/***************************************************************************

        CCS Model 2810

        11/12/2009 Skeleton driver.
        02/06/2011 Connected to a terminal

Press Enter to start the system.
All commands are in uppercase.

A    Assign logical device
Dn,n Dump memory
E    Punch End-of-File to paper tape
F    Fill
G    Go
H    Hex arithmetic
I    In
L    Punch Leader to paper tape
M    Move
O    Out
Q    Query logical devices
R    Read a file from paper tape
S    Edit memory
T    Test memory
V    Verify (compare 2 blocks of memory)
W    Write a file to paper tape
X    Examine Registers
Y    Set Baud rate of i8250
Z    Zleep (lock terminal). Press control+G twice to unlock.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/terminal.h"

#define MACHINE_RESET_MEMBER(name) void name::machine_reset()

class ccs2810_state : public driver_device
{
public:
	ccs2810_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER( ccs2810_20_r );
	DECLARE_READ8_MEMBER( ccs2810_25_r );
	DECLARE_READ8_MEMBER( ccs2810_26_r );
	DECLARE_WRITE8_MEMBER( ccs2810_20_w );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	UINT8 m_26_count;
	virtual void machine_reset();
};

READ8_MEMBER( ccs2810_state::ccs2810_20_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ccs2810_state::ccs2810_25_r )
{
	return (m_term_data) ? 0x21 : 0x20;
}

READ8_MEMBER( ccs2810_state::ccs2810_26_r )
{
	if (m_26_count) m_26_count--;
	return m_26_count;
}

WRITE8_MEMBER( ccs2810_state::ccs2810_20_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

static ADDRESS_MAP_START(ccs2810_mem, AS_PROGRAM, 8, ccs2810_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ccs2810_io, AS_IO, 8, ccs2810_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_READWRITE(ccs2810_20_r,ccs2810_20_w)
	AM_RANGE(0x25, 0x25) AM_READ(ccs2810_25_r)
	AM_RANGE(0x26, 0x26) AM_READ(ccs2810_26_r)
	//AM_RANGE(0x20, 0x27) AM_DEVREADWRITE_LEGACY("ins8250", ins8250_r, ins8250_w )
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs2810 )
INPUT_PORTS_END

WRITE8_MEMBER( ccs2810_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ccs2810_state, kbd_put)
};

MACHINE_RESET_MEMBER(ccs2810_state)
{
	cpu_set_reg(m_maincpu, Z80_PC, 0xf000);
	m_26_count = 0x41;
}

static const ins8250_interface ccs2810_com_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( ccs2810, ccs2810_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ccs2810_mem)
	MCFG_CPU_IO_MAP(ccs2810_io)

	MCFG_INS8250_ADD( "ins8250", ccs2810_com_interface, XTAL_1_8432MHz )

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs2810 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ccs2810_u8.bin", 0xf000, 0x0800, CRC(0c3054ea) SHA1(c554b7c44a61af13decb2785f3c9b33c6fc2bfce))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1980, ccs2810,  0,    0,       ccs2810,   ccs2810, driver_device,  0,   "California Computer Systems", "CCS Model 2810", GAME_NO_SOUND_HW)
