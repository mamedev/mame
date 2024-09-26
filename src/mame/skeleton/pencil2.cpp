// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Hanimex Pencil II
    Manufactured by Soundic, Hong Kong.

    2012-11-06 [Robbbert]

    Computer kindly donated for MAME by Ian Farquhar.

    Accessories:
    - PEN-216 : 16k RAM expansion
    - PEN-264 : 64k RAM expansion
    - PEN-511 : Data Cassette Recorder
    - ???     : Printer
    - ???     : Floppy Disk Drive (5.25)
    - ???     : Floppy Disk Controller
    - ???     : RS-232C Serial Interface
    - ???     : Coleco Adapter*
    - PEN-8xx : Various software on Cassette or Floppy Disk
    - ???     : Game Controller (joystick and 14 buttons)
    - PEN-7xx : Various software in Cartridge format
    - PEN-430 : Modem
    - PEN-902 : Computer power supply
    - PEN-962 : Monitor cable

    * The cart slot is the same as that found on the Colecovision console. By plugging the
      Coleco Adapter into the expansion slot, Colecovision cartridges can be plugged into the
      cart slot and played.

Information found by looking inside the computer
------------------------------------------------
Main Board PEN-002 11-50332-10

J1 Expansion slot
J2 Cart slot
J3 Memory expansion slot
J4 Printer slot
J5,J6 Joystick ports

XTAL 10.738 MHz

Output is to a TV on Australian Channel 1.

U1     uPD780C-1 (Z80A)
U2     Video chip with heatsink stuck on top, TMS9929
U3     SN76489AN
U4     2764 bios rom
U5     uPD4016C-2 (assumed to be equivalent of 6116 2K x 8bit static RAM)
U6     74LS04
U7     74LS74A
U8-10  74LS138
U11    74LS00
U12    74LS273
U13    74LS74A
U14-21 TMM416P-3 (4116-3 16k x 1bit dynamic RAM)
U22    74LS05
U23-24 SN74LS541


SD-BASIC usage:
All commands must be in uppercase, which is the default at boot.
The 'capslock' is done by pressing Shift and Esc together, and the
cursor will change to a checkerboard pattern.

The above key combination is not available to natural keyboard, so
press ~ instead. Natural keyboard & Paste assume this has been done,
to enable lowercase.


MEMORY MAP
0000-1FFF bios rom
2000-5FFF available for expansion
6000-7FFF static RAM (2K mirrored)
8000-FFFF cart slot

The 16k dynamic RAM holds the BASIC program and the video/gfx etc
but is banked out of view of a BASIC program.


ToDo:
- Joysticks (no info)

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

class pencil2_state : public driver_device
{
public:
	pencil2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_centronics(*this, "centronics")
		, m_cass(*this, "cassette")
		, m_cart(*this, "cartslot")
	{}

	void pencil2(machine_config &config);

	int printer_ready_r();
	int printer_ack_r();

private:
	void port10_w(u8 data);
	void port30_w(u8 data);
	void port80_w(u8 data);
	void portc0_w(u8 data);
	u8 porte2_r();
	void write_centronics_ack(int state);
	void write_centronics_busy(int state);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	int m_centronics_busy = 0;
	int m_centronics_ack = 0;
	bool m_cass_state = false;
	required_device<cpu_device> m_maincpu;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cass;
	required_device<generic_slot_device> m_cart;
};

void pencil2_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x5fff).nopw();  // stop error log filling up
	map(0x6000, 0x67ff).mirror(0x1800).ram();
	//map(0x8000, 0xffff)      // mapped by the cartslot
}

void pencil2_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x0f).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x10, 0x1f).w(FUNC(pencil2_state::port10_w));
	map(0x30, 0x3f).w(FUNC(pencil2_state::port30_w));
	map(0x80, 0x9f).w(FUNC(pencil2_state::port80_w));
	map(0xa0, 0xa1).mirror(0x1e).rw("tms9928a", FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0xc0, 0xdf).w(FUNC(pencil2_state::portc0_w));
	map(0xe0, 0xff).w("sn76489a", FUNC(sn76489a_device::write));
	map(0xe0, 0xe0).portr("E0");
	map(0xe1, 0xe1).portr("E1");
	map(0xe2, 0xe2).r(FUNC(pencil2_state::porte2_r));
	map(0xe3, 0xe3).portr("E3");
	map(0xe4, 0xe4).portr("E4");
	map(0xe6, 0xe6).portr("E6");
	map(0xe8, 0xe8).portr("E8");
	map(0xea, 0xea).portr("EA");
	map(0xf0, 0xf0).portr("F0");
	map(0xf2, 0xf2).portr("F2");
}

u8 pencil2_state::porte2_r()
{
	return (m_cass->input() > 0.1) ? 0xff : 0x7f;
}

void pencil2_state::port10_w(u8 data)
{
	m_centronics->write_strobe(BIT(data, 0));
}

void pencil2_state::port30_w(u8 data)
{
	m_cass_state ^= 1;
	m_cass->output( m_cass_state ? -1.0 : +1.0);
}

void pencil2_state::port80_w(u8 data)
{
}

void pencil2_state::portc0_w(u8 data)
{
}

void pencil2_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

int pencil2_state::printer_ready_r()
{
	return m_centronics_busy;
}

void pencil2_state::write_centronics_ack(int state)
{
	m_centronics_ack = state;
}

int pencil2_state::printer_ack_r()
{
	return m_centronics_ack;
}


/* Input ports */
static INPUT_PORTS_START( pencil2 )
	PORT_START("E0")
	// port_custom MUST be ACTIVE_HIGH to work
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(pencil2_state, printer_ready_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(pencil2_state, printer_ack_r)

	PORT_START("E1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) PORT_CHAR('~') // natural keyboard: press ~ to enable lowercase
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("E8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('/')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("F0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("F2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void pencil2_state::machine_start()
{
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_cass_state));

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xffff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));
}

void pencil2_state::pencil2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pencil2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pencil2_state::io_map);

	/* video hardware */
	tms9929a_device &vdp(TMS9929A(config, "tms9928a", XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, "sn76489a", XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 1.00); // guess

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pencil2_cart");

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pencil2_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pencil2_state::write_centronics_busy));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pencil2");
}

/* ROM definition */
ROM_START( pencil2 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "mt.u4", 0x0000, 0x2000, CRC(338d7b59) SHA1(2f89985ac06971e00210ff992bf1e30a296d10e7) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME     FLAGS
COMP( 1983, pencil2, 0,      0,      pencil2, pencil2, pencil2_state, empty_init, "Hanimex", "Pencil II", MACHINE_SUPPORTS_SAVE )
