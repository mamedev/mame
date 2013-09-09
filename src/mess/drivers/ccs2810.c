/***************************************************************************

CCS Model 2810

2009-12-11 Skeleton driver.
2011-06-02 Connected to a terminal

Chips: INS8250N-B, Z80A, uPD2716D, 82S129. Crystals: 16 MHz, 1.8432MHz


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
#include "machine/wd_fdc.h"


class ccs_state : public driver_device
{
public:
	ccs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	DECLARE_DRIVER_INIT(ccs);
	DECLARE_MACHINE_RESET(ccs);
	DECLARE_READ8_MEMBER(port20_r);
	DECLARE_READ8_MEMBER(port25_r);
	DECLARE_READ8_MEMBER(port26_r);
	DECLARE_WRITE8_MEMBER(port20_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	UINT8 m_26_count;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	optional_device<mb8877_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
};

READ8_MEMBER( ccs_state::port20_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ccs_state::port25_r )
{
	return (m_term_data) ? 0x21 : 0x20;
}

READ8_MEMBER( ccs_state::port26_r )
{
	if (m_26_count) m_26_count--;
	return m_26_count;
}

WRITE8_MEMBER( ccs_state::port20_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

static ADDRESS_MAP_START(ccs2810_mem, AS_PROGRAM, 8, ccs_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf7ff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ccs2810_io, AS_IO, 8, ccs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_READWRITE(port20_r,port20_w)
	AM_RANGE(0x25, 0x25) AM_READ(port25_r)
	AM_RANGE(0x26, 0x26) AM_READ(port26_r)
	//AM_RANGE(0x20, 0x27) AM_DEVREADWRITE("ins8250", ins8250_device, ins8250_r, ins8250_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START(ccs2422_io, AS_IO, 8, ccs_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x04, 0x04) AM_READWRITE(port04_r,port04_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(port20_r,port20_w)
	AM_RANGE(0x25, 0x25) AM_READ(port25_r)
	AM_RANGE(0x26, 0x26) AM_READ(port26_r)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("fdc", mb8877_t, read, write)
	//AM_RANGE(0x34, 0x34) AM_READWRITE(port34_r,port34_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs2810 )
INPUT_PORTS_END

WRITE8_MEMBER( ccs_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ccs_state, kbd_put)
};

MACHINE_RESET_MEMBER( ccs_state, ccs )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_26_count = 0x41;
	m_maincpu->set_state_int(Z80_PC, 0xf000);
}

DRIVER_INIT_MEMBER( ccs_state, ccs )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
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

static SLOT_INTERFACE_START( ccs_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( ccs2810, ccs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ccs2810_mem)
	MCFG_CPU_IO_MAP(ccs2810_io)
	MCFG_MACHINE_RESET_OVERRIDE(ccs_state, ccs)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	/* Devices */
	//MCFG_INS8250_ADD( "ins8250", ccs2810_com_interface, XTAL_1_8432MHz )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ccs2422, ccs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ccs2810_mem)
	MCFG_CPU_IO_MAP(ccs2422_io)
	MCFG_MACHINE_RESET_OVERRIDE(ccs_state, ccs)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	/* Devices */
	MCFG_MB8877x_ADD("fdc", XTAL_16MHz / 16) // UB1793 or MB8877
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ccs_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs2810 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	ROM_LOAD( "ccs2810.u8",   0x10000, 0x0800, CRC(0c3054ea) SHA1(c554b7c44a61af13decb2785f3c9b33c6fc2bfce))
ROM_END

ROM_START( ccs2422 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	ROM_LOAD( "2422.u24",  0x10000, 0x0800, CRC(6b47586b) SHA1(73ba779a659da4a1f0e22a3fa351a2b36d8456a0))

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD_OPTIONAL( "2422.u23",  0x0000, 0x0100, CRC(b279cada) SHA1(6cc6e00ec49ba2245c8836d6f09266b09d6e7648))
	ROM_LOAD_OPTIONAL( "2422.u22",  0x0100, 0x0100, CRC(e41858bb) SHA1(0be53725032ebea16e32cb720f099551a357e761))
	ROM_LOAD_OPTIONAL( "2422.u21",  0x0200, 0x0100, NO_DUMP )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT    CLASS       INIT     COMPANY                        FULLNAME       FLAGS */
COMP( 1980, ccs2810,  0,       0,       ccs2810,   ccs2810, ccs_state,  ccs,   "California Computer Systems", "CCS Model 2810 CPU card", GAME_NO_SOUND_HW)
COMP( 1980, ccs2422,  ccs2810, 0,       ccs2422,   ccs2810, ccs_state,  ccs,   "California Computer Systems", "CCS Model 2422B FDC card", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
