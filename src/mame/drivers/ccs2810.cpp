// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CCS Model 2810

2009-12-11 Skeleton driver.
2011-06-02 Connected to a terminal

Chips: INS8250N-B, Z80A, uPD2716D, 82S129. Crystals: 16 MHz, 1.8432MHz

SYSTEM OPERATION:
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

*****************************************************************************

CCS Model 2422B

Chips: UB1793/MB8877, 3 proms and one eprom. Crystal = 16MHz

SYSTEM OPERATION:
Same as above, plus some extra commands

B    Boot from floppy
L    removed
P    Set disk parameters e.g. P0 10 0 = drive A, 10 sectors per track, 1 sided
Q    Set disk position for raw read/write e.g. Q6 0 9 = track 6, side 0, sector 9
Rs f Read absolute disk data (set by P and Q) to memory range s to f
Ws f Write absolute disk data (set by P and Q) from memory range s to f

ToDo:
- fdc hld_r() has no code behind it, to be written
- not sure of the polarities of some of the floppy/fdc signals (not documented)
- no obvious option to change the fdc clock
- currently hard coded for a 20cm drive, not sure how to option select drive sizes
- a few jumpers to be added
- the one disk that exists fails the test:
  Incorrect layout on track 0 head 0, expected_size=41666, current_size=68144

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "machine/ins8250.h"
#include "machine/terminal.h"
#include "machine/wd_fdc.h"

#define TERMINAL_TAG "terminal"

class ccs_state : public driver_device
{
public:
	ccs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_fdc (*this, "fdc"),
		m_floppy0(*this, "fdc:0")
	{
	}

	DECLARE_DRIVER_INIT(ccs2810);
	DECLARE_DRIVER_INIT(ccs2422);
	DECLARE_MACHINE_RESET(ccs);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(port20_r);
	DECLARE_READ8_MEMBER(port25_r);
	DECLARE_READ8_MEMBER(port26_r);
	DECLARE_READ8_MEMBER(port34_r);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port20_w);
	DECLARE_WRITE8_MEMBER(port34_w);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	UINT8 m_26_count;
	bool m_ss;
	bool m_dden;
	bool m_dsize;
	UINT8 m_ds;
	floppy_image_device *m_floppy;
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	optional_device<mb8877_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
};

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
	AM_RANGE(0x04, 0x04) AM_READWRITE(port04_r,port04_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(port20_r,port20_w)
	AM_RANGE(0x25, 0x25) AM_READ(port25_r)
	AM_RANGE(0x26, 0x26) AM_READ(port26_r)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("fdc", mb8877_t, read, write)
	AM_RANGE(0x34, 0x34) AM_READWRITE(port34_r,port34_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs2810 )
INPUT_PORTS_END

//*************************************
//
//  Keyboard
//
//*************************************
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

WRITE8_MEMBER( ccs_state::kbd_put )
{
	m_term_data = data;
}

#if 0
static const ins8250_interface com_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};
#endif

//*************************************
//
//  Status / Control ports
//
//*************************************

/* Status 1
d0 : intrq
d1 : ds0
d2 : ds1
d3 : ds2
d4 : ds3
d5 : hld (1=head loaded)
d6 : autoboot (1=go to monitor)
d7 : drq
*/

READ8_MEMBER( ccs_state::port34_r )
{
	//return (UINT8)m_drq | (m_ds << 1) | ((UINT8)fdc->hld_r() << 5) | 0x40 | ((UINT8)m_intrq << 7);
	return (UINT8)m_fdc->drq_r() | (m_ds << 1) | 0x20 | 0x40 | ((UINT8)m_fdc->intrq_r() << 7); // hld_r doesn't do anything
}

/* Status 2
d0 : trk00 (1=trk00 on 20cm drive; 0=trk00 on 13cm drive)
d1 : drive size (0=20cm, 1=13cm)
d2 : wprt from drive (0=write protected)
d3 : /ss (1=side 0 is selected)
d4 : index hole = 0
d5 : dden (1 = double density)
d6 : double (0 = a double-sided 20cm disk is in the drive)
d7 : drq
*/

READ8_MEMBER( ccs_state::port04_r )
{
	bool trk00=1,wprt=0,dside=1;
	int idx=1;
	if (m_floppy)
	{
		trk00 = !m_floppy->trk00_r();
		wprt = !m_floppy->wpt_r();
		idx = m_floppy->idx_r()^1;
		dside = m_floppy->twosid_r();
	}
	return (UINT8)trk00 | 0 | ((UINT8)wprt << 2) | ((UINT8)m_ss << 3) |
		idx << 4 | ((UINT8)m_dden << 5) | ((UINT8)dside << 6) | ((UINT8)m_fdc->drq_r() << 7);
}

/* Control 1
d0 : ds0
d1 : ds1
d2 : ds2
d3 : ds3
d4 : drive size (adjusts fdc clock)
d5 : mon (1=motor on)
d6 : dden
d7 : autowait (0=ignore drq)
*/

WRITE8_MEMBER( ccs_state::port34_w )
{
	m_ds = data & 15;
	m_dsize = BIT(data, 4);
	m_dden = BIT(data, 6);

	m_floppy = nullptr;
	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(!m_dden);

	if (m_floppy)
	{
		m_floppy->mon_w(!BIT(data, 5));
	}
}

/* Control 2
d2 : remote eject for persci drive (1=eject)
d4 : fast seek mode (1=on)
d6 : /ss (0=side 1 selected)
d7 : rom enable (1=firmware enabled)
other bits not used
*/

WRITE8_MEMBER( ccs_state::port04_w )
{
	m_ss = BIT(data, 6);
	if (m_floppy)
		m_floppy->ss_w(!m_ss);
}


//*************************************
//
//  Machine
//
//*************************************
WRITE8_MEMBER( ccs_state::port40_w )
{
	membank("bankr0")->set_entry( (data) ? 1 : 0);
}

MACHINE_RESET_MEMBER( ccs_state, ccs )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_26_count = 0x41;
	m_maincpu->set_state_int(Z80_PC, 0xf000);
}

DRIVER_INIT_MEMBER( ccs_state, ccs2810 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

DRIVER_INIT_MEMBER( ccs_state, ccs2422 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

//*************************************
//
//  Disk
//
//*************************************

static SLOT_INTERFACE_START( ccs_floppies )
	SLOT_INTERFACE( "8sssd", FLOPPY_8_SSSD )
SLOT_INTERFACE_END

	//SLOT_INTERFACE( "525dd", FLOPPY_525_DD )

static MACHINE_CONFIG_START( ccs2810, ccs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ccs2810_mem)
	MCFG_CPU_IO_MAP(ccs2810_io)
	MCFG_MACHINE_RESET_OVERRIDE(ccs_state, ccs)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ccs_state, kbd_put))

	/* Devices */
	//MCFG_INS8250_ADD( "ins8250", com_intf, XTAL_1_8432MHz )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ccs2422, ccs_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ccs2810_mem)
	MCFG_CPU_IO_MAP(ccs2422_io)
	MCFG_MACHINE_RESET_OVERRIDE(ccs_state, ccs)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ccs_state, kbd_put))

	/* Devices */
	MCFG_MB8877_ADD("fdc", XTAL_16MHz / 8) // UB1793 or MB8877
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ccs_floppies, "8sssd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
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

/*    YEAR  NAME      PARENT   COMPAT   MACHINE    INPUT    CLASS       INIT          COMPANY                        FULLNAME       FLAGS */
COMP( 1980, ccs2810,  0,       0,       ccs2810,   ccs2810, ccs_state,  ccs2810,   "California Computer Systems", "CCS Model 2810 CPU card", MACHINE_NO_SOUND_HW)
COMP( 1980, ccs2422,  ccs2810, 0,       ccs2422,   ccs2810, ccs_state,  ccs2422,   "California Computer Systems", "CCS Model 2422B FDC card", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
