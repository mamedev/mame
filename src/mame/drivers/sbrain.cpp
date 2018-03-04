// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Intertec SuperBrain

2013-08-19 Skeleton

Intertec Compustar terminal appears to have identical hardware. Need roms for it.

Chips: 2x Z80; FD1791; 2x 8251; 8255; BR1941; CRT8002; KR3600; DP8350
Xtals: 16.0, 10.92, 5.0688
Disk parameters: 512 bytes x 10 sectors x 35 tracks. 1 and 2-sided disks supported.
Sound: Beeper

The boot prom is shared between both cpus. This feat is accomplished by holding the sub cpu
 in reset, until the main cpu has prepared a few memory locations. The first thing in the rom
 is to check these locations, and then program flow splits into 2 sections, one for each cpu.
The main cpu does a busreq to gain access to the sub cpu's 1k static ram. When the sub cpu
 responds with busack, then the main cpu switches bank2. In emulation, it isn't actually
 necessary to stop the sub cpu because of other handshaking. Our Z80 emulation doesn't
 support the busack signal anyway, so we just assume it is granted immediately.
When booted, the time (corrupted) is displayed at top right. You need to run TIME hh:mm:ss
 to set the time (TIME.COM must be on the disk).

The schematic in parts is difficult to read. Some assumptions have been made.

To Do:
- Without a disk in, it should display a message to insert a disk. Doesn't happen. May
  attempt to execute empty memory instead. (-bios 1, -bios 2)
- Improve keyboard.
- Video chips need to be emulated (CRT8002 and DP8350), attributes etc.
- Probably lots of other stuff.
- PC0 when high will swap out lower 16k of RAM for the DP8350 device. The value of HL (the
  "memory" address) is the data (cursor location, top of screen, etc). The byte "written"
  to this address (d0,d1) is the command for the CRTC.

*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"


class sbrain_state : public driver_device
{
public:
	sbrain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_beep(*this, "beeper")
		, m_brg(*this, "brg")
		, m_u0(*this, "uart0")
		, m_u1(*this, "uart1")
		, m_ppi(*this, "ppi")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_vs(*this, "VS")
		, m_bankr0(*this, "bankr0")
		, m_bankw0(*this, "bankw0")
		, m_bank2(*this, "bank2")
		, m_keyboard(*this, "X%u", 0)
		{}

	DECLARE_DRIVER_INIT(sbrain);
	DECLARE_MACHINE_RESET(sbrain);
	DECLARE_READ8_MEMBER(ppi_pa_r);
	DECLARE_WRITE8_MEMBER(ppi_pa_w);
	DECLARE_READ8_MEMBER(ppi_pb_r);
	DECLARE_WRITE8_MEMBER(ppi_pb_w);
	DECLARE_READ8_MEMBER(ppi_pc_r);
	DECLARE_WRITE8_MEMBER(ppi_pc_w);
	DECLARE_READ8_MEMBER(port48_r);
	DECLARE_READ8_MEMBER(port50_r);
	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(baud_w);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(kbd_scan);

	void sbrain(machine_config &config);
	void sbrain_io(address_map &map);
	void sbrain_mem(address_map &map);
	void sbrain_subio(address_map &map);
	void sbrain_submem(address_map &map);
private:
	bool m_busak;
	u8 m_keydown;
	u8 m_porta;
	u8 m_portb;
	u8 m_portc;
	u8 m_port10;
	u8 m_term_data;
	u8 m_framecnt;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beep;
	required_device<com8116_device> m_brg;
	required_device<i8251_device> m_u0;
	required_device<i8251_device> m_u1;
	required_device<i8255_device> m_ppi;
	required_device<fd1791_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_ioport m_vs;
	required_memory_bank m_bankr0;
	required_memory_bank m_bankw0;
	required_memory_bank m_bank2;
	required_ioport_array<10> m_keyboard;
};

ADDRESS_MAP_START(sbrain_state::sbrain_mem)
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x4000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xbfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xc000, 0xf7ff ) AM_RAM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

ADDRESS_MAP_START(sbrain_state::sbrain_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	AM_RANGE(0x41, 0x41) AM_MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	AM_RANGE(0x48, 0x4f) AM_READ(port48_r) //chr_int_latch
	AM_RANGE(0x50, 0x57) AM_READ(port50_r)
	AM_RANGE(0x58, 0x58) AM_MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x59, 0x59) AM_MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x60, 0x67) AM_WRITE(baud_w)
	AM_RANGE(0x68, 0x6b) AM_MIRROR(4) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(sbrain_state::sbrain_submem)
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	AM_RANGE( 0x8800, 0x8bff ) AM_RAM AM_REGION("subcpu", 0x8800)
ADDRESS_MAP_END

ADDRESS_MAP_START(sbrain_state::sbrain_subio)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("fdc", fd1791_device, read, write)
	AM_RANGE(0x10, 0x10) AM_READWRITE(port10_r,port10_w)
ADDRESS_MAP_END


READ8_MEMBER( sbrain_state::port48_r )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

READ8_MEMBER( sbrain_state::port50_r )
{
	return m_term_data;
}

READ8_MEMBER( sbrain_state::port10_r )
{
	return m_port10;
}

/* Misc disk functions
d0 : ?
d1 : SEL A (drive 0?)
d2 : SEL B (drive 1?)
d3 : SEL C
d4 : SEL D
d5 : side select
d6,7 : not used
*/
WRITE8_MEMBER( sbrain_state::port10_w )
{
	m_port10 = data | 0xc0;

	floppy_image_device *floppy = nullptr;
	if (BIT(m_port10, 1)) floppy = m_floppy0->get_device();
	if (BIT(m_port10, 2)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(m_port10, 5));

	m_floppy0->get_device()->mon_w(0); // motors run all the time
	m_floppy1->get_device()->mon_w(0);
}

WRITE8_MEMBER( sbrain_state::baud_w )
{
	m_brg->str_w(data & 0x0f);
	m_brg->stt_w(data >> 4);
}

READ8_MEMBER( sbrain_state::ppi_pa_r )
{
	return m_porta;
}

/* Video functions:
d0,1 : 11 = alphanumeric; 10 = external ;other = graphics
d2 : Underline
d3,4 : not used
d5 : strike through
d6 : 1=60hz 0=50hz
d7 : reverse video
*/
WRITE8_MEMBER( sbrain_state::ppi_pa_w )
{
	m_porta = data;
}

/* Inputs
d0 : data ready from keyboard
d1 : key held down
d2 : Vert Blank
d3 : not used
d4 : /capslock
d5 : ?
d6 : Ring Indicator line from main rs232 port, 1=normal, 0=set
d7 : cpu2 /busak line
*/
READ8_MEMBER( sbrain_state::ppi_pb_r )
{
	u8 vertsync = m_vs->read(); // bit 2
	u8 capslock = BIT(ioport("MODIFIERS")->read(), 0) << 4; // bit 4, capslock
	u8 p10d0 = BIT(m_port10, 0) << 5; // bit 5
	u8 busak = m_busak ? 128 : 0; // bit 7
	return busak | p10d0 | capslock | vertsync | m_keydown;
}

WRITE8_MEMBER( sbrain_state::ppi_pb_w )
{
	m_portb = data & 8;
}

READ8_MEMBER( sbrain_state::ppi_pc_r )
{
	return m_portc;
}

/* System
d0 : 1 = bank 0 disabled
d1 : character blanking
d2 : 1=enable rom, 0=enable ram bank 0
d3 : cpu2 reset line
d4 : 1=enable ram bank 2, 0=bank 2 uses disk buffer
d5 : cpu2 /busreq line
d6 : beeper
d7 : keyboard, 1=enable comms, 0=reset
*/
WRITE8_MEMBER( sbrain_state::ppi_pc_w )
{
	m_portc = data;
	m_beep->set_state(BIT(data, 6));
	m_bankr0->set_entry(BIT(data, 2));
	m_bank2->set_entry(BIT(data, 4));
	if (!BIT(data, 7))
		m_keydown &= 2; // ack DR

	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(Z80_INPUT_LINE_BUSRQ, BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE); // ignored in z80.cpp
	m_busak = BIT(data, 5);
}

u8 translate_table[3][10][8] = {
	// unshifted
	{
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x2e, 0x2c, 0x2d, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x09, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
		{ 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f },
		{ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 },
		{ 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x7b, 0x03, 0x7f },
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x2d, 0x3d, 0x5c, 0x08, 0x0a, 0x0d },
		{ 0x1b, 0x00, 0x20, 0x3b, 0x27, 0x2c, 0x2e, 0x2f }
	},
	// shift
	{
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x2e, 0x2c, 0x2d, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x09, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 },
		{ 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f },
		{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 },
		{ 0x58, 0x59, 0x5a, 0x5d, 0x7c, 0x7d, 0x03, 0x7f },
		{ 0x29, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26 },
		{ 0x2a, 0x28, 0x5f, 0x2b, 0x7e, 0x08, 0x0a, 0x0d },
		{ 0x1b, 0x00, 0x20, 0x3a, 0x22, 0x3c, 0x3e, 0x3f }
	},
	// ctrl
	{
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x2e, 0x2c, 0x2d, 0x00, 0x00, 0x00 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 },
		{ 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
		{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 },
		{ 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x03, 0x1f },
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x2d, 0x3d, 0x1e, 0x08, 0x0a, 0x0d },
		{ 0x1b, 0x00, 0x20, 0x3b, 0x27, 0x2c, 0x2e, 0x2f }
	}
};

static INPUT_PORTS_START( sbrain )
	PORT_START("X0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("X1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Insert") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("X2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)

	PORT_START("X3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("X4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("X5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("X6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ ]") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("{ }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(3)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)

	PORT_START("X7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("X8")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('~')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(10)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X9")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Here Is") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("MODIFIERS")
	PORT_BIT(0x01,IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CapsLock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	/* vblank */
	PORT_START("VS")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(sbrain_state::kbd_scan)
{
	// m_keydown: d0 = 1 after key pressed, and is reset by pc7; d1 = 1 while a key is down.
	m_keydown &= 1;
	u8 i, j, keyin, mods = ioport("MODIFIERS")->read() & 6;
	u8 translate_set = 0;
	if (BIT(mods, 1))
		translate_set = 1;
	else
	if (BIT(mods, 2))
		translate_set = 2;

	for (i = 0; i < 10; i++)
	{
		keyin = m_keyboard[i]->read();
		if (keyin)
		{
			for (j = 0; j < 8; j++)
			{
				if (BIT(keyin, j))
				{
					u8 pressed = translate_table[translate_set][i][j];
					m_keydown = (m_term_data == pressed) ? (m_keydown | 2) : 3;
					m_term_data = pressed;
					return;
				}
			}
		}
	}
	m_term_data = 0xff;
}

DRIVER_INIT_MEMBER( sbrain_state, sbrain )
{
	u8 *main = memregion("maincpu")->base();
	u8 *sub = memregion("subcpu")->base();

	m_bankr0->configure_entry(0, &main[0x0000]);
	m_bankr0->configure_entry(1, &sub[0x0000]);
	m_bankw0->configure_entry(0, &main[0x0000]);
	m_bank2->configure_entry(0, &sub[0x8000]);
	m_bank2->configure_entry(1, &main[0x8000]);
}

static SLOT_INTERFACE_START( sbrain_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

MACHINE_RESET_MEMBER( sbrain_state, sbrain )
{
	m_keydown = 0;
	m_bankr0->set_entry(1); // point at rom
	m_bankw0->set_entry(0); // always write to ram
	m_bank2->set_entry(1); // point at maincpu bank
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // hold subcpu in reset
}

u32 sbrain_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;
	m_framecnt++;

	// Where attributes come from:
	// - Most systems use ram for character-based attributes, but this one uses strictly hardware which would seem cumbersome
	// - d0,1 graphics from porta d0,d1
	// - d2 strike-through from porta d5
	// - d3 underline from porta d2
	// - d4 reverse-video from porta d7
	// - d5 blank from PC1 (scan-line based)
	// - d6 flash from bit 7 of each character

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x+ma];

					if (!BIT(chr, 7) || BIT(m_framecnt, 5))
					{
						chr &= 0x7f;

						if (chr)
							gfx = m_p_chargen[(chr<<4) | ra ];
					}
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}

MACHINE_CONFIG_START(sbrain_state::sbrain)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL(16'000'000) / 4)
	MCFG_CPU_PROGRAM_MAP(sbrain_mem)
	MCFG_CPU_IO_MAP(sbrain_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sbrain_state, irq0_line_hold)

	MCFG_CPU_ADD("subcpu", Z80, XTAL(16'000'000) / 4)
	MCFG_CPU_PROGRAM_MAP(sbrain_submem)
	MCFG_CPU_IO_MAP(sbrain_subio)

	MCFG_MACHINE_RESET_OVERRIDE(sbrain_state, sbrain)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::amber())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(sbrain_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 800)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(sbrain_state, ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sbrain_state, ppi_pa_w))
	MCFG_I8255_IN_PORTB_CB(READ8(sbrain_state, ppi_pb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sbrain_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(sbrain_state, ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sbrain_state, ppi_pc_w))

	MCFG_DEVICE_ADD("uart0", I8251, 0)

	MCFG_DEVICE_ADD("uart1", I8251, 0)

	MCFG_DEVICE_ADD("brg", COM8116, XTAL(5'068'800)) // BR1941L
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("uart0", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart0", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))

	MCFG_FD1791_ADD("fdc", XTAL(16'000'000) / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", sbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", sbrain_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_a", sbrain_state, kbd_scan, attotime::from_hz(15))
MACHINE_CONFIG_END

ROM_START( sbrain )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "subcpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "4_003", "4.003" )
	ROMX_LOAD( "4_003_vc8001.z69", 0x0000, 0x0800, CRC(3ce3cd53) SHA1(fb6ade6bd67de3d9f911a1a48481ca619bda65ae), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "3_1", "3.1" )
	ROMX_LOAD( "3_1.z69", 0x0000, 0x0800, CRC(b6a2e6a5) SHA1(a646faaecb9ac45ee1a42764628e8971524d5c13), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "3_05", "3.05" )
	ROMX_LOAD( "qd_3_05.z69", 0x0000, 0x0800, CRC(aedbe777) SHA1(9ee9ca3f05e11ceb80896f06c3a3ae352db214dc), ROM_BIOS(3) )
	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

COMP( 1981, sbrain, 0, 0, sbrain, sbrain, sbrain_state, sbrain, "Intertec", "Superbrain", MACHINE_NOT_WORKING )
