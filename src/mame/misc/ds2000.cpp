// license:BSD-3-Clause
// copyright-holders:O. Galibert

// HAL Communications Corp DS 2000 KSR

// Low-Cost keyboard send-receive terminal

// Does Baudot and ASCII on serial, or Morse on audio lines

// Missing morse on audio input.

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "video/dp8350.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class ds2000_state : public driver_device, public device_sound_interface
{
public:
	ds2000_state(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~ds2000_state();

	void ds2000(machine_config &config);

protected:
	required_device<z80_device> m_maincpu;
	required_device<dp8350_device> m_crtc;
	required_device<clock_device> m_nmi_clock;
	required_device<screen_device> m_screen;
	required_device<speaker_device> m_speaker;
	required_device<microphone_device> m_morse_in;
	required_device<rs232_port_device> m_rs232;

	required_shared_ptr<u8> m_mainram;
	required_region_ptr<u8> m_dm8678;
	required_ioport_array<7> m_keyp;
	required_ioport m_keymeta;
	required_ioport m_config;

	sound_stream *m_stream;

	u8 m_io40;
	u32 m_emit_counter;
	bool m_emit_on;
	int m_rx;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_stream_update(sound_stream &stream) override;
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem(address_map &map);
	void io(address_map &map);
	void dp8350_w(offs_t offset, u8 data);
	u8 kbd_r(offs_t offset);
	u8 io40_r();
	void io40_w(u8 data);
	void io01_w(u8 data);
	void beeper_w(int state);
	void tx_w(int state);
	void rx_w(int state);
	void morse_w(int state);
	void kos_w(int state);
};

ds2000_state::ds2000_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, device_sound_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_crtc(*this, "crtc")
	, m_nmi_clock(*this, "nmi_clock")
	, m_screen(*this, "screen")
	, m_speaker(*this, "speaker")
	, m_morse_in(*this, "morse_in")
	, m_rs232(*this, "rs232")
	, m_mainram(*this, "mainram")
	, m_dm8678(*this, "dm8678")
	, m_keyp(*this, "P%u", 0U)
	, m_keymeta(*this, "PM")
	, m_config(*this, "config")
{
}

ds2000_state::~ds2000_state()
{
}

void ds2000_state::machine_start()
{
	save_item(NAME(m_io40));
	save_item(NAME(m_emit_counter));
	save_item(NAME(m_emit_on));
	save_item(NAME(m_rx));
	m_stream = stream_alloc(1, 1, 48000);
}

void ds2000_state::machine_reset()
{
	m_io40 = 0;
	m_emit_on = 0;
	m_emit_counter = 0;
	m_rx = 1;
}

void ds2000_state::sound_stream_update(sound_stream &stream)
{
	if(m_emit_on) {
		// 800Hz
		for(u32 i = 0; i != stream.samples(); i++) {
			stream.put(0, i, m_emit_counter >= 60 ? 1.0 : -1.0);
			m_emit_counter ++;
			if(m_emit_counter == 120)
				m_emit_counter = 0;
		}
	} else
		stream.fill(0, 0.0);
}

void ds2000_state::dp8350_w(offs_t offset, u8 data)
{
	m_crtc->register_load(util::bitswap<2>(data, 0, 1), offset & 0x7ff);
}

u8 ds2000_state::kbd_r(offs_t offset)
{
	u8 res = m_rx ? 0xff : 0x7f;
	for(int i=0; i != 7; i++)
		if(BIT(offset, i))
			res &= m_keyp[i]->read();
	return res;
}

u8 ds2000_state::io40_r()
{
	return m_keymeta->read() & m_config->read();
}

void ds2000_state::beeper_w(int state)
{
	m_stream->update();
	m_emit_counter = 0;
	m_emit_on = BIT(m_io40, 6);
}

void ds2000_state::kos_w(int state)
{
	logerror("kos_w %d\n", state);
}

void ds2000_state::tx_w(int state)
{
	m_rs232->write_txd(state);
}

void ds2000_state::rx_w(int state)
{
	m_rx = state;
}

void ds2000_state::morse_w(int state)
{
	logerror("morse_w %d\n", state);
}

void ds2000_state::io40_w(u8 data)
{
	// Bit 0/1 not latched, Bit 3 latched but ignore, Bit 7 whole-screen video inverse
	u8 diff = m_io40 ^ data;
	m_io40 = data;
	if(diff & 0x04)
		morse_w(BIT(m_io40, 2));
	if(diff & 0x10)
		kos_w(BIT(m_io40, 4));
	if(diff & 0x20)
		tx_w(BIT(m_io40, 5));
	if(diff & 0x40)
		beeper_w(BIT(m_io40, 6));
}

void ds2000_state::io01_w(u8 data)
{
	logerror("io01_w %02x\n", data);
}

void ds2000_state::mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0).mirror(0x1000);
	map(0x0000, 0x1fff).w(FUNC(ds2000_state::dp8350_w)).mirror(0xc000);
	map(0xe000, 0xe3ff).r(FUNC(ds2000_state::kbd_r)).mirror(0x1000);
	map(0xe400, 0xefff).ram().share(m_mainram).mirror(0x1000);
}

void ds2000_state::io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01^0xff, 0x01^0xff).w(FUNC(ds2000_state::io01_w));
	map(0x40^0xff, 0x40^0xff).rw(FUNC(ds2000_state::io40_r), FUNC(ds2000_state::io40_w));
}

u32 ds2000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(!BIT(m_io40, 7) ? 0xffffff : 0, cliprect);
	u16 ma = m_crtc->top_of_page();
	u16 cr = m_crtc->cursor_address();
	for(u32 y=0; y != 24; y++) {
		for(u32 x=0; x != 80; x++) {
			u8 inv = (ma == cr) ^ !BIT(m_io40, 7) ? 0xff : 0;
			u8 c = ma < 0x400 ? 0xff : m_mainram[ma - 0x400];
			const u8 *cdata = m_dm8678 + 8 * (c & 0x3f);
			for(u32 yy = 0; yy != 7; yy++) {
				u32 *base = &bitmap.pix(cliprect.top() + 10*y + yy, cliprect.left() + 7*x);
				u8 cd = inv ^ *cdata++;
				u32 on = BIT(c, 7) ? 0x909090 : 0xffffff;
				for(u32 xx = 0; xx != 5; xx++)
					*base++ = BIT(cd, 4-xx) ? on : 0;
			}
			ma = (ma + 1) & 0xfff;
		}
	}
	return 0;
}

void ds2000_state::ds2000(machine_config &config)
{
	constexpr XTAL MC = 10.92_MHz_XTAL;
	Z80(config, m_maincpu, MC/5);
	m_maincpu->set_addrmap(AS_PROGRAM, &ds2000_state::mem);
	m_maincpu->set_addrmap(AS_IO, &ds2000_state::io);

	CLOCK(config, m_nmi_clock).set_period(attotime::from_ticks(10240, MC)).signal_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	DP8350(config, m_crtc, MC);
	m_crtc->set_screen(m_screen);
	m_crtc->refresh_control(1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(ds2000_state::screen_update));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(ds2000_state::rx_w));

	SPEAKER(config, m_speaker, 1).front_center();
	add_route(0, m_speaker, 1.0, 0);

	MICROPHONE(config, m_morse_in, 1).front_center();
	m_morse_in->add_route(0, *this, 1.0, 0);
}

static INPUT_PORTS_START(ds2000)
	PORT_START("config")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME(0x40, 0x40, "Number of columns")
	PORT_CONFSETTING(0x40, "72")
	PORT_CONFSETTING(0x00, "69")
	PORT_CONFNAME(0x80, 0x80, "D jumper")
	PORT_CONFSETTING(0x80, "Absent")
	PORT_CONFSETTING(0x00, "Present")

	PORT_START("PM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L \\ FF") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V SYN") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R DC2") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F ACK") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')  PORT_CHAR('-')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B STX") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T DC4") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G BEL") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"I \u2192") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y EM") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"H \u2190") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ident") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z SUB") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q DCI") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A SOH") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Linefeed") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rubout") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X CAN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W ETB") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S DC3") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('P') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C ETX") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E WRU") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D EOT") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"K [ \u2191") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('[') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U NAK") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(']')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"J \u2193") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

ROM_START(ds2000)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("hal_ds2000_ep0.bin", 0x000, 0x400, CRC(0aa0e88f) SHA1(a24ca674baa8efdeb266d7dcb639dcbb152f919f))
	ROM_LOAD("hal_ds2000_ep1.bin", 0x400, 0x400, CRC(8295c22d) SHA1(6f9a448241ffb3ed1155ba1811c8e509a992f9d6))
	ROM_LOAD("hal_ds2000_ep2.bin", 0x800, 0x400, CRC(300bb8d0) SHA1(b0894cbd3c0646bab291659568a5faf1627cb4dc))
	ROM_LOAD("hal_ds2000_ep3.bin", 0xc00, 0x400, CRC(2e508b56) SHA1(2ea563ce05cdcae10a851172469c841cd3915c89))

	ROM_REGION(0x200, "dm8678", 0)
	ROM_LOAD( "dm8678cab.bin", 0x0000, 0x0200, CRC(8da502e7) SHA1(30d2dd9658823cdc2b2f6ef37f5a05d6f3e0db76))
ROM_END


COMP(1981, ds2000, 0, 0, ds2000, ds2000, ds2000_state, empty_init, "HAL Communications Corp", "DS 2000 KSR", MACHINE_SUPPORTS_SAVE)
