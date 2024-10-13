// license:BSD-3-Clause
// copyright-holders: Robbbert
/**********************************************************************************

TRS-80 DT-1

Tandy's Data Terminal.

Skeleton driver commenced on 2017-10-25.

Core bugs noted:
- If region() used to locate the main rom in another region, validation
  complains that region ':maincpu' not found.
- If region 'maincpu' changed to 0x1000 (same size as the rom), a fatal error
  of duplicate save state occurs at start.


ToDo:
- Serial printer + (P1.1, P1.2, P3.5)
- Fix cpu bug with timer interrupt and then remove hack.
- Check the existing serial comms and LPTR that polarities are correct.

You can get into the setup menu by pressing Ctrl+Shift+Enter.

Note: The printer and serial interfaces are cheap and nasty. Connecting things
      up wrongly will lead to malfunction. Read the user manual for more info.

The LPTR (parallel printer) works by sending everything on the databus to the
printer, then asserting STROBE only for data that needs to print. The address-map
mechanism can't really handle this, but after investigation, it turns out that
the printer data goes to B800 which is a spare address range in the real machine.

**********************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/i8275.h"
#include "machine/7474.h"
#include "machine/x2212.h"
#include "sound/beep.h"
#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class trs80dt1_state : public driver_device
{
public:
	trs80dt1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_nvram(*this,"nvram")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_buzzer(*this, "buzzer")
		, m_7474(*this, "7474")
		, m_rs232(*this, "rs232")
		, m_centronics(*this, "centronics")
	{ }

	void trs80dt1(machine_config &config);

private:
	u8 dma_r(offs_t offset);
	u8 key_r(offs_t offset);
	u8 port1_r();
	u8 port3_r();
	void store_w(u8 data);
	void port1_w(u8 data);
	void port3_w(u8 data);
	void rx_w(int state);
	I8275_DRAW_CHARACTER_MEMBER(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	bool m_bow = false;
	bool m_cent_busy = false;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<i8051_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<i8276_device> m_crtc;
	required_device<x2210_device> m_nvram;
	required_ioport_array<9> m_io_keyboard;
	required_device<beep_device> m_buzzer;
	required_device<ttl7474_device> m_7474;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;

	u8 m_port3;
};

void trs80dt1_state::machine_reset()
{
	m_bow = 0;
	m_7474->preset_w(1);
	// line is active low in the real chip
	m_nvram->recall(1);
	m_nvram->recall(0);
	m_port3 = 0;
}

u8 trs80dt1_state::dma_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_crtc->dack_w(m_p_videoram[offset]); // write to /BS pin
	return 0x7f;
}

u8 trs80dt1_state::key_r(offs_t offset)
{
	offset &= 15;
	if (offset < 9)
		return m_io_keyboard[offset]->read();
	else
		return 0xff;
}

void trs80dt1_state::store_w(u8 data)
{
	// line is active low in the real chip
	m_nvram->store(1);
	m_nvram->store(0);
}

u8 trs80dt1_state::port1_r()
{
	u8 data = m_cent_busy << 6;
	return data;
}

/*
d0 : /PSTRB (centronics strobe)
d1 : TRPRT (for serial printer)
d2 : /SP BUSY (for serial printer)
d3 : /RTS
d4 : BOW (applies reverse video to entire screen)
d5 : /DTR
d6 : PP BUSY (parallel printer busy - input)
d7 : n/c */
void trs80dt1_state::port1_w(u8 data)
{
	m_centronics->write_strobe(BIT(data, 0));
	m_bow = BIT(data, 4);
	m_rs232->write_dtr(BIT(data, 5));
	m_rs232->write_rts(BIT(data, 3));
}

/*
d4 : beeper
d5 : Printer enable */
void trs80dt1_state::port3_w(u8 data)
{
	m_rs232->write_txd(BIT(data, 1));
	m_buzzer->set_state(BIT(data, 4));

	m_port3 = (m_port3 & 1) | (data & ~1);
}

u8 trs80dt1_state::port3_r()
{
	return m_port3;
}

void trs80dt1_state::rx_w(int state)
{
	if (state)
		m_port3 |= 1;
	else
		m_port3 &= ~1;
}

void trs80dt1_state::prg_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x27ff).r(FUNC(trs80dt1_state::dma_r));
}

void trs80dt1_state::io_map(address_map &map)
{
	map.global_mask(0xbfff); // A14 not used
	map(0xa000, 0xa7ff).ram().share("videoram");
	map(0xa800, 0xa83f).mirror(0x3c0).rw(m_nvram, FUNC(x2210_device::read), FUNC(x2210_device::write)); // X2210
	map(0xac00, 0xafff).r(FUNC(trs80dt1_state::key_r));
	map(0xb000, 0xb3ff).portr("X9");
	map(0xb400, 0xb7ff).w(FUNC(trs80dt1_state::store_w));
	map(0xb800, 0xbbff).w("cent_data_out", FUNC(output_latch_device::write));
	map(0xbc00, 0xbc01).mirror(0x3fe).rw(m_crtc, FUNC(i8276_device::read), FUNC(i8276_device::write)); // i8276
}

/* Input ports */
static INPUT_PORTS_START( trs80dt1 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x7f)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(23)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(11)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(5)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(17)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(12)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(26)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(18)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(24)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(20)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(19)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHAR('\\')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(25)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(22)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHAR('`')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(21)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(6)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('.') PORT_CHAR('|')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(7)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(10)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(14)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('}') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(15)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('{') PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(16)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) // Jumper - LOW for 60Hz, high for 50Hz
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // No Connect
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rs232", rs232_port_device, dcd_r)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rs232", rs232_port_device, cts_r)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rs232", rs232_port_device, dsr_r)
INPUT_PORTS_END

void trs80dt1_state::machine_start()
{
	m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00)); // black
	m_palette->set_pen_color(1, rgb_t(0x00,0xa0,0x00)); // normal
	m_palette->set_pen_color(2, rgb_t(0x00,0xff,0x00)); // highlight

	save_item(NAME(m_bow));
	save_item(NAME(m_cent_busy));
}

const gfx_layout trs80dt1_charlayout =
{
	8, 8,             /* 8x16 characters - the last 8 lines are always blank */
	128,                /* 128 characters */
	1,              /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0,1,2,3,4,5,6,7},
	{0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16                /* space between characters */
};

static GFXDECODE_START( gfx_trs80dt1 )
	GFXDECODE_ENTRY( "chargen", 0x0000, trs80dt1_charlayout, 0, 1 )
GFXDECODE_END


I8275_DRAW_CHARACTER_MEMBER( trs80dt1_state::crtc_update_row )
{
	charcode &= 0x7f;
	linecount &= 15;

	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 gfx = 0;

	using namespace i8275_attributes;
	if (BIT(attrcode, LTEN)) // underline attr
		gfx = 0xff;
	else if (BIT(attrcode, GPA0, 2) == 0 && !BIT(attrcode, VSP)) // blinking and invisible attributes
		gfx = m_p_chargen[linecount | (charcode << 4)];

	if (BIT(attrcode, RVV)) // reverse video attr
		gfx ^= 0xff;

	if (m_bow) // black-on-white
		gfx ^= 0xff;

	bool hlgt = BIT(attrcode, HLGT);
	for(u8 i=0; i<8; i++)
		bitmap.pix(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
}


void trs80dt1_state::trs80dt1(machine_config &config)
{
	/* basic machine hardware */
	I8051(config, m_maincpu, 7.3728_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80dt1_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &trs80dt1_state::io_map);
	m_maincpu->port_out_cb<1>().set(FUNC(trs80dt1_state::port1_w));
	m_maincpu->port_in_cb<1>().set(FUNC(trs80dt1_state::port1_r));
	m_maincpu->port_out_cb<3>().set(FUNC(trs80dt1_state::port3_w));
	m_maincpu->port_in_cb<3>().set(FUNC(trs80dt1_state::port3_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("crtc", FUNC(i8276_device::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(40*12, 16*16);
	screen.set_visarea(0, 40*12-1, 0, 16*16-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_trs80dt1);

	I8276(config, m_crtc, 12.48_MHz_XTAL / 8);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(trs80dt1_state::crtc_update_row));
	m_crtc->drq_wr_callback().set_inputline(m_maincpu, MCS51_INT0_LINE); // BRDY pin goes through inverter to /INT0, so we don't invert
	m_crtc->irq_wr_callback().set(m_7474, FUNC(ttl7474_device::clear_w)); // INT pin
	m_crtc->irq_wr_callback().append(m_7474, FUNC(ttl7474_device::d_w));
	m_crtc->vrtc_wr_callback().set(m_7474, FUNC(ttl7474_device::clock_w));
	m_crtc->set_screen("screen");

	PALETTE(config, "palette").set_entries(3);

	X2210(config, "nvram");

	TTL7474(config, m_7474, 0);
	m_7474->comp_output_cb().set_inputline(m_maincpu, MCS51_INT1_LINE).invert(); // /Q connects directly to /INT1, so we need to invert

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_buzzer, 2000).add_route(ALL_OUTPUTS, "mono", 0.50);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(trs80dt1_state::rx_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this] (bool state) { m_cent_busy = state; });

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}

ROM_START( trs80dt1 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "trs80dt1.u12", 0x0000, 0x1000, CRC(04e8a53f) SHA1(7b5d5047319ef8f230b82684d97a918b564d466e) )
	ROM_FILL(0x9a,1,0xd4) // fix for timer0 problem

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "8045716.u8",   0x0000, 0x0800, CRC(e2c5e59b) SHA1(0d571888d5f9fea4e565486ea8d3af8998ca46b1) )
ROM_END

} // anonymous namespace

COMP( 1982, trs80dt1, 0, 0, trs80dt1, trs80dt1, trs80dt1_state, empty_init, "Radio Shack", "TRS-80 DT-1 Data Terminal", MACHINE_SUPPORTS_SAVE )
