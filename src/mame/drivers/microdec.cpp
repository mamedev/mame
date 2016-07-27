// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************************

Morrow Designs Micro Decision

2009-12-10 Skeleton driver.

Although it looks like an ordinary CP/M computer, the monitor and keyboard are actually a
dumb terminal plugged into the base unit. Therefore the roms and details of the terminal are
needed.

Board design changes depending on bios version of base unit:
In earliest design, F7 sets up VFO and F8 selects motor on, while fdc does drive select.
Later version gets rid of these, and F7 now does motor on and drive select; also addition of
i8253 timer, a centronics port, and a diagnostic jumper. F8 is unused.

Currently (as at 2016-07-17), memory test works, rom banking works, disk does NOT boot.
Ver 1 roms say "Not found", Ver 2 roms hang after pressing enter, Ver 3 rom hangs after memory test.

ToDo:
- Make the floppy boot
- Add i8253 timer chip
- Add centronics parts
- Different hardware for different bios versions (only earliest design is coded)

***********************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/terminal.h"
#include "softlist.h"


class microdec_state : public driver_device
{
public:
	microdec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_terminal(*this, "terminal")
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy(nullptr)
	{
	}

	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(portf5_r);
	DECLARE_READ8_MEMBER(portf6_r);
	DECLARE_WRITE8_MEMBER(portf6_w);
	DECLARE_READ8_MEMBER(portf7_r);
	DECLARE_WRITE8_MEMBER(portf7_w);
	DECLARE_WRITE8_MEMBER(portf8_w);
	DECLARE_DRIVER_INIT(microdec);
private:
	UINT8 m_term_data;
	UINT8 m_portf8;
	bool m_fdc_rdy;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<generic_terminal_device> m_terminal;
	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	floppy_image_device *m_floppy;
};


READ8_MEMBER( microdec_state::status_r )
{
	return (m_term_data) ? 3 : 1;
}

READ8_MEMBER( microdec_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

/*
d0-2 : motor on signals from f8
d3   : ack (cent)
d4   : ready (fdd)
d5   : diag jumper (md3 only) */
READ8_MEMBER( microdec_state::portf5_r )
{
	m_fdc->set_ready_line_connected(m_fdc_rdy);

	UINT8 data = m_portf8 | ioport("DIAG")->read() | 0xc0;
	return data;
}

// disable eprom
READ8_MEMBER( microdec_state::portf6_r )
{
	membank("bankr0")->set_entry(0); // point at ram
	return 0xff;
}

// TC pin on fdc
READ8_MEMBER( microdec_state::portf7_r )
{
	m_fdc->tc_w(1);
	return 0xff;
}

// enable eprom
WRITE8_MEMBER( microdec_state::portf6_w )
{
	membank("bankr0")->set_entry(1); // point at rom
}

// sets up VFO stuff
WRITE8_MEMBER( microdec_state::portf7_w )
{
	m_fdc_rdy = BIT(data,2);
}

/*
d0-2 : motor on for drive sockets
d3   : precomp */
WRITE8_MEMBER( microdec_state::portf8_w )
{
	m_portf8 = data & 7;
	/* code for motor on per drive goes here */
	m_floppy = m_floppy0->get_device();
	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
		m_floppy->mon_w(0);
}

static ADDRESS_MAP_START(microdec_mem, AS_PROGRAM, 8, microdec_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x1000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(microdec_io, AS_IO, 8, microdec_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf5, 0xf5) AM_READ(portf5_r)
	AM_RANGE(0xf6, 0xf6) AM_READWRITE(portf6_r,portf6_w)
	AM_RANGE(0xf7, 0xf7) AM_READWRITE(portf7_r,portf7_w)
	AM_RANGE(0xf8, 0xf8) AM_WRITE(portf8_w)
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("fdc", upd765a_device, map)
	AM_RANGE(0xfc, 0xfc) AM_READ(keyin_r) AM_DEVWRITE("terminal", generic_terminal_device, write)
	AM_RANGE(0xfd, 0xfd) AM_READ(status_r)
	// AM_RANGE(0xf0, 0xf3) 8253 PIT (md3 only) used as a baud rate generator for serial ports
	// AM_RANGE(0xf4, 0xf4) Centronics data
	// AM_RANGE(0xf5, 0xf5) motor check (md1/2)
	// AM_RANGE(0xf5, 0xf5) Centronics status (md3) read bit 3 (ack=1); read bit 4 (busy=1); write bit 7 (stb=0)
	// AM_RANGE(0xf6, 0xf6) rom enable (w=enable; r=disable)
	// AM_RANGE(0xf7, 0xf7) VFO Count set
	// AM_RANGE(0xf8, 0xf8) Motor and Shift control
	// AM_RANGE(0xfa, 0xfb) uPD765C fdc FA=status; FB=data
	// AM_RANGE(0xfc, 0xfd) Serial Port 1 (terminal) FC=data FD=status
	// AM_RANGE(0xfe, 0xff) Serial Port 2 (printer) FE=data FF=status
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( microdec )
	PORT_START("DIAG")
	PORT_DIPNAME( 0x20, 0x20, "Diagnostics" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void microdec_state::machine_start()
{
}

void microdec_state::machine_reset()
{
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_maincpu->set_input_line_vector(0, 0x7f);
	m_term_data = 0;
}

WRITE8_MEMBER( microdec_state::kbd_put )
{
	m_term_data = data;
}

static SLOT_INTERFACE_START( microdec_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

DRIVER_INIT_MEMBER( microdec_state, microdec )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x1000]);
	membank("bankw0")->configure_entry(0, &main[0x1000]);
}

static MACHINE_CONFIG_START( microdec, microdec_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(microdec_mem)
	MCFG_CPU_IO_MAP(microdec_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(microdec_state, kbd_put))

	MCFG_UPD765A_ADD("fdc", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", microdec_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	//MCFG_FLOPPY_DRIVE_ADD("fdc:1", microdec_floppies, "525hd", floppy_image_device::default_floppy_formats)
	//MCFG_FLOPPY_DRIVE_SOUND(true)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "md2_flop")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( md2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v13", "v1.3" )
	ROMX_LOAD( "md2-13.bin",  0x0000, 0x0800, CRC(43f4c9ab) SHA1(48a35cbee4f341310e9cba5178c3fd6e74ef9748), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v13a", "v1.3a" )
	ROMX_LOAD( "md2-13a.bin", 0x0000, 0x0800, CRC(d7fcddfd) SHA1(cae29232b737ebb36a27b8ad17bc69e9968f1309), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v13b", "v1.3b" )
	ROMX_LOAD( "md2-13b.bin", 0x0000, 0x1000, CRC(a8b96835) SHA1(c6b111939aa7e725da507da1915604656540b24e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "v20", "v2.0" )
	ROMX_LOAD( "md2-20.bin",  0x0000, 0x1000, CRC(a604735c) SHA1(db6e6e82a803f5cbf4f628f5778a93ae3e211fe1), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "v23", "v2.3" )
	ROMX_LOAD( "md2-23.bin",  0x0000, 0x1000, CRC(49bae273) SHA1(00381a226fe250aa3636b0b740df0af63efb0d18), ROM_BIOS(5))
ROM_END

ROM_START( md3 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v23a", "v2.3a" )
	ROMX_LOAD( "md3-23a.bin", 0x0000, 0x1000, CRC(95d59980) SHA1(ae65a8e8e2823cf4cf6b1d74c0996248e290e9f1), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v25", "v2.5" )
	ROMX_LOAD( "md3-25.bin",  0x0000, 0x1000, CRC(14f86bc5) SHA1(82fe022c85f678744bb0340ca3f88b18901fdfcb), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v31", "v3.1" )
	ROMX_LOAD( "md3-31.bin",  0x0000, 0x1000, CRC(bd4014f6) SHA1(5b33220af34c64676756177db4915f97840b2996), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     CLASS             INIT       COMPANY                  FULLNAME       FLAGS */
COMP( 1982, md2,    0,      0,       microdec,  microdec, microdec_state, microdec,  "Morrow Designs", "Micro Decision MD-2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1982, md3,    md2,    0,       microdec,  microdec, microdec_state, microdec,  "Morrow Designs", "Micro Decision MD-3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
