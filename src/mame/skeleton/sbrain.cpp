// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Intertec SuperBrain

2013-08-19 Skeleton

Intertec Compustar terminal appears to have identical hardware. Need roms for it.

Chips: 2x Z80; FD1791; 2x 8251; 8255; BR1941; CRT8002-003; KR3600-056; DP8350; MM5035
Xtals: 16.0, 10.92, 5.0688
RAM: 32K or 64K dynamic (TMS4116-20NL); 1K static disk buffer (MM2114)
Disk parameters: 512 bytes x 10 sectors x 35 tracks. 1 and 2-sided disks supported.
Sound: Beeper
Expansion bus: 40 pins, nearly identical to TRS-80

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
- Improve keyboard.
- Row buffering DMA (DP8350, MM5035) and line-by-line rendering.
- Proper character generator emulation (CRT8002).
- Add expansion bus slot.
- Probably lots of other stuff.

*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/dp8350.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include <algorithm>


namespace {

class sbrain_state : public driver_device
{
public:
	sbrain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_p_chargen(*this, "chargen")
		, m_beep(*this, "beeper")
		, m_crtc(*this, "crtc")
		, m_usart(*this, "usart%u", 0U)
		, m_mainport(*this, "mainport")
		, m_ppi(*this, "ppi")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_boot_prom(*this, "subcpu")
		, m_disk_buffer(*this, "buffer")
		, m_keyboard(*this, "X%u", 0)
		, m_modifiers(*this, "MODIFIERS")
		, m_serial_sw(*this, "BAUDCLOCK")
	{
	}

	void sbrain(machine_config &config);
	void sagafox(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

	u8 ppi_pa_r();
	void ppi_pa_w(u8 data);
	u8 ppi_pb_r();
	void ppi_pb_w(u8 data);
	u8 ppi_pc_r();
	void ppi_pc_w(u8 data);
	u8 char_int_ack_r();
	void char_int_ack_w(u8 data);
	u8 keyboard_r();

	void disk_select_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(kbd_scan);

	void crtc_lrc_w(int state);
	void crtc_vblank_w(int state);
	void crtc_vsync_w(int state);

	void external_txc_w(int state);
	void external_rxc_w(int state);
	void internal_txc_rxc_w(int state);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_mem_map(address_map &map) ATTR_COLD;
	void sub_io_map(address_map &map) ATTR_COLD;
	void sub_mem_map(address_map &map) ATTR_COLD;

	bool m_busak = false;
	u8 m_keydown = 0U;
	u8 m_porta = 0U;
	u8 m_portb = 0U;
	u8 m_portc = 0U;
	u8 m_port10 = 0U;
	u8 m_key_data = 0U;
	u8 m_framecnt = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beep;
	required_device<dp8350_device> m_crtc;
	required_device_array<i8251_device, 2> m_usart;
	required_device<rs232_port_device> m_mainport;
	required_device<i8255_device> m_ppi;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_boot_prom;
	required_shared_ptr<u8> m_disk_buffer;

	required_ioport_array<10> m_keyboard;
	required_ioport m_modifiers;
	required_ioport m_serial_sw;
};

void sbrain_state::main_mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sbrain_state::mem_r), FUNC(sbrain_state::mem_w));
}

void sbrain_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x41).mirror(6).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x48, 0x48).mirror(7).rw(FUNC(sbrain_state::char_int_ack_r), FUNC(sbrain_state::char_int_ack_w)); //chr_int_latch
	map(0x50, 0x50).mirror(7).r(FUNC(sbrain_state::keyboard_r));
	map(0x58, 0x59).mirror(6).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x60).mirror(7).w("brg", FUNC(com8116_device::stt_str_w));
	map(0x68, 0x6b).mirror(4).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void sbrain_state::sub_mem_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0xf000).rom().region("subcpu", 0);
	map(0x8800, 0x8bff).mirror(0x7400).ram().share("buffer");
}

void sbrain_state::sub_io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x08, 0x0b).mirror(4).rw(m_fdc, FUNC(fd1791_device::read), FUNC(fd1791_device::write));
	map(0x10, 0x10).mirror(7).w(FUNC(sbrain_state::disk_select_w));
}


u8 sbrain_state::mem_r(offs_t offset)
{
	switch (offset & 0xc000)
	{
	case 0x0000:
		// PPIC-2 set selects boot PROM
		if (BIT(m_portc, 2))
			return m_boot_prom[offset & 0x7ff];

		// lowest 16K of RAM is disabled when PPIC-0 is set
		if (BIT(m_portc, 0))
			return 0xff;
		break;

	case 0x8000:
		// resetting PPIC-4 selects disk buffer RAM
		if (!BIT(m_portc, 4))
			return m_disk_buffer[offset & 0x3ff];

		break;
	}

	return m_ram->read(offset);
}

void sbrain_state::mem_w(offs_t offset, u8 data)
{
	// any memory write affects CRTC when PPIC-0 is set
	if (BIT(m_portc, 0))
		m_crtc->register_load(bitswap<2>(data, 0, 1), offset & 0xfff);

	switch (offset & 0xc000)
	{
	case 0x0000:
		// can't write to boot PROM
		if (BIT(m_portc, 2))
			return;

		// RAS is disabled for bank 0 when PPIC-0 is set
		if (BIT(m_portc, 0))
			return;

		break;

	case 0x8000:
		// resetting PPIC-4 selects disk buffer RAM
		if (!BIT(m_portc, 4))
		{
			m_disk_buffer[offset & 0x3ff] = data;
			break;
		}

		break;
	}

	m_ram->write(offset, data);
}

u8 sbrain_state::char_int_ack_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

void sbrain_state::char_int_ack_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

u8 sbrain_state::keyboard_r()
{
	return m_key_data;
}

/* Misc disk functions
d0 : busy signal to master CPU
d1 : SEL A (drive 0?)
d2 : SEL B (drive 1?)
d3 : SEL C
d4 : SEL D
d5 : side select
d6,7 : not used
*/
void sbrain_state::disk_select_w(u8 data)
{
	m_port10 = data | 0xc0;

	floppy_image_device *floppy = nullptr;
	for (int d = 0; d < 4; d++)
		if (BIT(m_port10, d + 1))
		{
			floppy = m_floppy[d]->get_device();
			if (floppy)
				break;
		}

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(m_port10, 5));
}

u8 sbrain_state::ppi_pa_r()
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
void sbrain_state::ppi_pa_w(u8 data)
{
	m_crtc->refresh_control(BIT(data, 6));

	m_porta = data;
}

/* Inputs
d0 : data ready from keyboard
d1 : key held down
d2 : Vert Blank
d3 : not used
d4 : /capslock
d5 : disk status: 1 = busy, 0 = ready
d6 : Ring Indicator line from main rs232 port, 1=normal, 0=set
d7 : cpu2 /busak line
*/
u8 sbrain_state::ppi_pb_r()
{
	u8 vertsync = m_crtc->vblank_r() << 2;
	u8 capslock = BIT(m_modifiers->read(), 0) << 4; // bit 4, capslock
	u8 p10d0 = BIT(m_port10, 0) << 5; // bit 5
	u8 ri = m_mainport->ri_r() << 6;
	u8 busak = m_busak ? 128 : 0; // bit 7
	return busak | ri | p10d0 | capslock | vertsync | m_keydown;
}

void sbrain_state::ppi_pb_w(u8 data)
{
	m_portb = data & 8;
}

u8 sbrain_state::ppi_pc_r()
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
void sbrain_state::ppi_pc_w(u8 data)
{
	m_portc = data;
	m_beep->set_state(BIT(data, 6));
	if (!BIT(data, 7))
		m_keydown &= 2; // ack DR

	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);
	m_fdc->mr_w(!BIT(data, 3));
	if (BIT(data, 3))
		disk_select_w(0);
	m_subcpu->set_input_line(Z80_INPUT_LINE_BUSRQ, BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE); // ignored in z80.cpp
	m_busak = BIT(data, 5);
}

void sbrain_state::external_txc_w(int state)
{
	if (!BIT(m_serial_sw->read(), 0))
		m_usart[1]->write_txc(state);
}

void sbrain_state::external_rxc_w(int state)
{
	if (!BIT(m_serial_sw->read(), 2))
		m_usart[1]->write_rxc(state);
}

void sbrain_state::internal_txc_rxc_w(int state)
{
	if (!BIT(m_serial_sw->read(), 1))
		m_usart[1]->write_txc(state);
	if (!BIT(m_serial_sw->read(), 3))
		m_usart[1]->write_rxc(state);
	if (!BIT(m_serial_sw->read(), 4))
		m_mainport->write_etc(state);
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
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Insert") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("X2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

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

	PORT_START("BAUDCLOCK")
	PORT_DIPNAME(0x03, 0x01, "Main TX Clock") PORT_DIPLOCATION("SW:1,3")
	PORT_DIPSETTING(0x01, "Internal")
	PORT_DIPSETTING(0x02, "External")
	PORT_DIPNAME(0x0c, 0x04, "Main RX Clock") PORT_DIPLOCATION("SW:2,4")
	PORT_DIPSETTING(0x04, "Internal")
	PORT_DIPSETTING(0x08, "External")
	PORT_DIPNAME(0x10, 0x10, "Internal Baud Clock to Main Port") PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(sbrain_state::kbd_scan)
{
	// m_keydown: d0 = 1 after key pressed, and is reset by pc7; d1 = 1 while a key is down.
	m_keydown &= 1;
	u8 i, j, keyin, mods = m_modifiers->read() & 6;
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
					m_keydown = (m_key_data == pressed) ? (m_keydown | 2) : 3;
					m_key_data = pressed;
					return;
				}
			}
		}
	}
	m_key_data = 0xff;
}

void sbrain_state::crtc_lrc_w(int state)
{
	// TODO: actually triggered by BUSACK and taken after DMA burst finishes
	if (state && !m_crtc->lbre_r() && !m_crtc->vblank_r())
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

void sbrain_state::crtc_vblank_w(int state)
{
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

void sbrain_state::crtc_vsync_w(int state)
{
	// TODO: internal to the CRT8002
	if (!state)
		m_framecnt++;
}

void sbrain_state::machine_start()
{
	std::fill_n(m_ram->pointer(), m_ram->size(), 0x00);

	m_usart[0]->write_cts(0);

	save_item(NAME(m_busak));
	save_item(NAME(m_keydown));
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
	save_item(NAME(m_port10));
	save_item(NAME(m_key_data));
	save_item(NAME(m_framecnt));
}

static void sbrain_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void sbrain_state::machine_reset()
{
	m_keydown = 0;

	for (auto &floppy : m_floppy)
	{
		floppy_image_device *device = floppy->get_device();
		if (device != nullptr)
			device->mon_w(0); // motors run all the time
	}

	// PPI resets to input mode, which causes PPIC-3 to be pulled up to +5V, resetting disk CPU
}

u32 sbrain_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// PPIC-3 blanks entire display
	if (BIT(m_portc, 3))
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	uint16_t sy=0;

	// Where attributes come from:
	// - Most systems use ram for character-based attributes, but this one uses strictly hardware which would seem cumbersome
	// - d0,1 graphics from porta d0,d1
	// - d2 strike-through from porta d5
	// - d3 underline from porta d2
	// - d4 reverse-video from porta d7
	// - d5 blank from PC1 (scan-line based)
	// - d6 flash from bit 7 of each character

	uint16_t ma = m_crtc->top_of_page();
	uint16_t cr = m_crtc->cursor_address();
	uint8_t *videoram = &m_ram->pointer()[m_ram->size() - 0x800];
	for (uint8_t y = 0; y < 24; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint32_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 80; x++)
			{
				uint8_t gfx = 0;
				if (ra > 0)
				{
					uint8_t chr = videoram[(x + ma) & 0x7ff];

					if (!BIT(chr, 7) || BIT(m_framecnt, 5))
					{
						chr &= 0x7f;

						gfx = m_p_chargen[(chr<<4) | ra];
					}
				}

				if (((x + ma) & 0xfff) == cr)
					gfx ^= 0xfe;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 6) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 5) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 4) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 3) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 2) ? rgb_t::white() : rgb_t::black();
				*p++ = BIT(gfx, 1) ? rgb_t::white() : rgb_t::black();
			}
		}
		ma = (ma + 80) & 0xfff;
	}
	return 0;
}

void sbrain_state::sbrain(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &sbrain_state::main_mem_map);
	m_maincpu->set_addrmap(AS_IO, &sbrain_state::main_io_map);

	Z80(config, m_subcpu, 16_MHz_XTAL / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &sbrain_state::sub_mem_map);
	m_subcpu->set_addrmap(AS_IO, &sbrain_state::sub_io_map);

	RAM(config, m_ram).set_default_size("64K").set_extra_options("32K");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_screen_update(FUNC(sbrain_state::screen_update));

	DP8350(config, m_crtc, 10.92_MHz_XTAL).set_screen("screen"); // XTAL not directly connected
	m_crtc->character_generator_program(1);
	m_crtc->lrc_callback().set(FUNC(sbrain_state::crtc_lrc_w));
	m_crtc->vblank_callback().set(FUNC(sbrain_state::crtc_vblank_w));
	m_crtc->vsync_callback().set(FUNC(sbrain_state::crtc_vsync_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 800).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* Devices */
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(sbrain_state::ppi_pa_r));
	m_ppi->out_pa_callback().set(FUNC(sbrain_state::ppi_pa_w));
	m_ppi->in_pb_callback().set(FUNC(sbrain_state::ppi_pb_r));
	m_ppi->out_pb_callback().set(FUNC(sbrain_state::ppi_pb_w));
	m_ppi->in_pc_callback().set(FUNC(sbrain_state::ppi_pc_r));
	m_ppi->out_pc_callback().set(FUNC(sbrain_state::ppi_pc_w));

	I8251(config, m_usart[0], 16_MHz_XTAL / 8);
	m_usart[0]->txd_handler().set("auxport", FUNC(rs232_port_device::write_txd));

	I8251(config, m_usart[1], 16_MHz_XTAL / 8);
	m_usart[1]->txd_handler().set(m_mainport, FUNC(rs232_port_device::write_txd));
	m_usart[1]->rts_handler().set(m_mainport, FUNC(rs232_port_device::write_rts));
	m_usart[1]->dtr_handler().set(m_mainport, FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_mainport, default_rs232_devices, nullptr);
	m_mainport->rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	m_mainport->cts_handler().set(m_usart[1], FUNC(i8251_device::write_cts));
	m_mainport->dsr_handler().set(m_usart[1], FUNC(i8251_device::write_dsr));
	m_mainport->txc_handler().set(FUNC(sbrain_state::external_txc_w));
	m_mainport->rxc_handler().set(FUNC(sbrain_state::external_rxc_w));

	rs232_port_device &auxport(RS232_PORT(config, "auxport", default_rs232_devices, nullptr));
	auxport.rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	auxport.dsr_handler().set(m_usart[0], FUNC(i8251_device::write_dsr));

	com8116_device &brg(COM8116(config, "brg", 5.0688_MHz_XTAL)); // BR1941L
	brg.fr_handler().set(m_usart[0], FUNC(i8251_device::write_txc));
	brg.fr_handler().append(m_usart[0], FUNC(i8251_device::write_rxc));
	brg.ft_handler().set(FUNC(sbrain_state::internal_txc_rxc_w));

	FD1791(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, "fdc:0", sbrain_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", sbrain_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", sbrain_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", sbrain_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	TIMER(config, "timer_a", 0).configure_periodic(FUNC(sbrain_state::kbd_scan), attotime::from_hz(15));

	SOFTWARE_LIST(config, "flop_list").set_original("sbrain");
}

void sbrain_state::sagafox(machine_config& config)
{
	sbrain(config);

	subdevice<software_list_device>("flop_list")->set_original("sagafox");
}

ROM_START( sbrain )
	ROM_REGION( 0x0800, "subcpu", ROMREGION_ERASEFF ) // only the second CPU has its own ROM
	ROM_SYSTEM_BIOS( 0, "4_2", "4.2")
	ROMX_LOAD("sbii_sb4_2.z69", 0x0000, 0x0800, CRC(89313e26) SHA1(755d494934099a4488abc44a8566c18d7d4fdea3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "4_003", "4.003" )
	ROMX_LOAD("4_003_vc8001.z69", 0x0000, 0x0800, CRC(3ce3cd53) SHA1(fb6ade6bd67de3d9f911a1a48481ca619bda65ae), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "3_1", "3.1" )
	ROMX_LOAD("3_1.z69", 0x0000, 0x0800, CRC(b6a2e6a5) SHA1(a646faaecb9ac45ee1a42764628e8971524d5c13), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "3_05", "3.05" )
	ROMX_LOAD("qd_3_05.z69", 0x0000, 0x0800, CRC(aedbe777) SHA1(9ee9ca3f05e11ceb80896f06c3a3ae352db214dc), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "4_2_50", "4.2 (50Hz hack)")
	ROMX_LOAD("sbii_sb4_2_50hz.z69", 0x0000, 0x0800, CRC(285a894b) SHA1(694fef446fe19c0962f79951aa4d464489a9d161), ROM_BIOS(4))
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD("crt8002-003.bin", 0x0000, 0x0800, BAD_DUMP CRC(5181d324) SHA1(7aa2d084947bcc0e3d31568f4de84c23b84abfff)) // hand-crafted
ROM_END

ROM_START( sagafox )
	ROM_REGION( 0x0800, "subcpu", 0 )
	ROM_LOAD("1b02.bin", 0x0000, 0x0800, CRC(1cd6e4d2) SHA1(585d2a52840804a3771077bcc77e249b28ea1602))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD("1cg0.bin", 0x0000, 0x0800, CRC(508c56ef) SHA1(9ba2cceeb3b9f72503693372ae76f54d5919f024))

	ROM_REGION( 0x0400, "keyboard", 0 ) // use with KR3600-PRO
	ROM_LOAD("1kb1.bin", 0x0000, 0x0400, CRC(92b00014) SHA1(0669ecfc24bf010735a9c14791409e80390a01f7))
ROM_END

ROM_START( sagafoxf80 )
	ROM_REGION( 0x0800, "subcpu", 0 )
	ROM_LOAD("f80_6ms.bin", 0x0000, 0x0800, CRC(81ed0c24) SHA1(411fd7bbb85dfd8c793f0871041f64febf5571e0))

	ROM_REGION( 0x0800, "chargen", 0 ) // minor differences to sagafox, most notable on '6', suffering from bitrot?
	ROM_LOAD("1cg0.bin", 0x0000, 0x0800, CRC(fba860fc) SHA1(7cd467fe2c11861daf1ed5f9f002874eabe2a47a))

	ROM_REGION( 0x0400, "keyboard", 0 ) // use with KR3600-PRO
	ROM_LOAD("1kb1.bin", 0x0000, 0x0400, CRC(92b00014) SHA1(0669ecfc24bf010735a9c14791409e80390a01f7))

	ROM_REGION( 0x0800, "oam", 0 ) // software protection module?
	ROM_LOAD("oam120.bin", 0x0000, 0x0800, CRC(880a8e36) SHA1(c6bee88a294090f039161fe20ce36a4ada3b10d3))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE   INPUT   CLASS         INIT        COMPANY                                FULLNAME                            FLAGS
COMP( 1981, sbrain,     0,      0,     sbrain,   sbrain, sbrain_state, empty_init, "Intertec Data Systems",               "SuperBrain Video Computer System", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1980, sagafox,    sbrain, 0,     sagafox,  sbrain, sbrain_state, empty_init, "Sistemi Avanzati Gestione Aziendale", "Saga Fox",                         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1980, sagafoxf80, sbrain, 0,     sagafox,  sbrain, sbrain_state, empty_init, "Sistemi Avanzati Gestione Aziendale", "Saga Fox/F80",                     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
