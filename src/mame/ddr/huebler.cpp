// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Ausbaufaehiger Mikrocomputer mit dem U 880
    electronica Band 227/228

****************************************************************************/

/*

TODO:

    - eprom programmer

Cassette considerations

    - To save a range of memory: S name start end
    - To load it back: L name
    - To save an unnamed file: S "" start end
    - To load an unnamed file: L
    - When loading, if the name doesn't match, there's no message, and no
      way to determine the name.
    - For some reason, the system saves the file 3 times. Maybe it was an
      attempt to guard against the inherent unreliable design.
*/

#include "emu.h"
#include "huebler.h"
#include "speaker.h"
#include "screen.h"
#include "utf8.h"

/* Keyboard */

void amu880_state::scan_keyboard()
{
	uint8_t data = m_key_row[m_key_d6]->read();

	if ((data & 0x0f) != 0x0f)
	{
		m_key_d7 = m_key_d6;
		m_key_a4 = !(BIT(data, 1) && BIT(data, 3));
		m_key_a5 = !(BIT(data, 2) && BIT(data, 3));

		if (!m_key_down)
		{
			m_key_down = true;
			m_key_repeat_timer->adjust(attotime::from_msec(500), 0, attotime::from_hz(30));
		}

		m_key_release_timer->adjust(attotime::from_usec(12700));
	}

	m_key_d6++;

	if (m_key_d6 == 16)
	{
		m_key_d6 = 0;
	}
}

TIMER_CALLBACK_MEMBER(amu880_state::key_release)
{
	m_key_down = false;
	m_key_repeat = true;
	m_key_repeat_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(amu880_state::key_repeat)
{
	m_key_repeat = !m_key_repeat;
}

INPUT_CHANGED_MEMBER(amu880_state::shift_changed)
{
	uint8_t special = m_special->read();

	if (!BIT(special, 2))
		m_shift_lock = true;
	else if (!BIT(special, 1))
		m_shift_lock = false;

	m_shift_led = m_shift_lock;
}

TIMER_DEVICE_CALLBACK_MEMBER(amu880_state::keyboard_tick)
{
	scan_keyboard();
}

/* Read/Write Handlers */

uint8_t amu880_state::keyboard_r(offs_t offset)
{
	/*

	    A0..A3  Y
	    A4      !(X0&X3)
	    A5      !(X2&X3)
	    A6      shift
	    A7      ctrl
	    A8      key pressed
	    A9      AB0

	*/

	uint8_t special = m_special->read();

	int ctrl = BIT(special, 0);
	int shift = BIT(special, 1) && !m_shift_lock;
	int a8 = !(m_key_down && m_key_repeat);
	int ab0 = BIT(offset, 0);

	uint16_t address = (ab0 << 9) | (a8 << 8) | (ctrl << 7) | (shift << 6) | (m_key_a5 << 5) | (m_key_a4 << 4) | m_key_d7;

	return m_kb_rom->base()[address];
}

uint8_t amu880_state::power_off_r()
{
	if (!machine().side_effects_disabled())
		power_off_clock(0);

	return 0xff;
}

void amu880_state::power_off_w(uint8_t data)
{
	power_off_clock(1);
}

void amu880_state::power_off_clock(int rd)
{
	bool const was_pending = m_poweroff_sr == 3;

	m_poweroff_sr = ((m_poweroff_sr << 1) | rd) & 3;

	bool const pending = m_poweroff_sr == 3;

	if (!pending)
		m_poweroff_timer->adjust(attotime::never);
	else if (!was_pending)
		m_poweroff_timer->adjust(attotime::from_seconds(18));

	m_poweroff_led = pending;
}

TIMER_CALLBACK_MEMBER(amu880_state::power_off)
{
	machine().schedule_exit();
}

void amu880_state::tone_w(offs_t offset, uint8_t data)
{
	m_tone_on = (offset & 3) != 0;
}

TIMER_CALLBACK_MEMBER(amu880_state::tone_tick)
{
	m_tone_q = m_tone_on ? !m_tone_q : true;
	m_speaker->level_w(m_tone_q);
}

uint8_t amu880_state::boot_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_boot.disable();

	return m_rom[0xf000 + offset];
}

/* Memory Maps */

void amu880_state::amu880_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("video_ram");
	map(0xf000, 0xfbff).rom();
	map(0xfc00, 0xffff).ram();
	map(0x0000, 0xffff).view(m_boot);
	m_boot[0](0x0000, 0xefff).lr8(NAME([] () -> uint8_t { return 0x00; })).nopw();
	m_boot[0](0xf000, 0xfbff).r(FUNC(amu880_state::boot_r));
}

void amu880_state::amu880_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(amu880_state::power_off_r), FUNC(amu880_state::power_off_w));
	map(0x04, 0x07).w(FUNC(amu880_state::tone_w));
	map(0x08, 0x09).mirror(0x02).r(FUNC(amu880_state::keyboard_r));
	map(0x0c, 0x0f).rw(Z80PIO2_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x10, 0x13).rw(Z80PIO1_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x14, 0x17).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x18, 0x1b).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

/* Input Ports */

static INPUT_PORTS_START( amu880 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_END)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) // <-|

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('_') PORT_CHAR('|')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CASSETTE")
	PORT_DIPNAME( 0x01, 0x00, "Playback Phase" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Inverted" )
	PORT_DIPNAME( 0x02, 0x02, "Monoflop Preset A" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPNAME( 0x04, 0x00, "Monoflop Preset B" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPNAME( 0x08, 0x08, "Monoflop Preset C" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPNAME( 0x10, 0x00, "Monoflop Preset D" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(amu880_state::shift_changed), 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(amu880_state::shift_changed), 0)
INPUT_PORTS_END

/* Video */

uint32_t amu880_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pen = m_palette->pens();

	for (int y = 0; y < 240; y++)
	{
		int const line = y % 10;

		for (int sx = 0; sx < 64; sx++)
		{
			uint16_t const videoram_addr = ((y / 10) * 64) + sx;
			uint8_t const videoram_data = m_video_ram[videoram_addr & 0x7ff];

			uint16_t const charrom_addr = ((videoram_data & 0x7f) << 3) | line;
			uint8_t data = m_char_rom->base()[charrom_addr & 0x3ff];

			for (int x = 0; x < 6; x++)
			{
				int const color = ((line > 7) ? 0 : BIT(data, 7)) ^ BIT(videoram_data, 7);

				bitmap.pix(y, (sx * 6) + x) = pen[color];

				data <<= 1;
			}
		}
	}

	return 0;
}

/* Z80-CTC Interface */

void amu880_state::ctc_z0_w(int state)
{
}

void amu880_state::ctc_z2_w(int state)
{
	if (state && !m_cass_t)
	{
		uint8_t const config = m_cass_config->read();

		if (m_rx_mono)
		{
			if (m_rx_count)
			{
				m_rx_count--;
			}
			else
			{
				m_rx_mono = false;
				m_rx_count = BIT(config, 1, 4);
			}
		}

		double const level = m_cassette->input();

		bool rxd = m_rx_d;
		if (level > 0.04)
			rxd = false;
		else if (level < -0.04)
			rxd = true;

		if (rxd != m_rx_d)
		{
			m_rx_d = rxd;
			m_rx_mono = true;
		}

		m_sio->rxa_w(m_rx_d == BIT(config, 0));
		m_sio->rxca_w(BIT(m_rx_count, 2));

		m_tx_count = (m_tx_count + 1) & 7;
		m_txc = BIT(m_tx_count, 2);
		m_sio->txca_w(m_txc);
		update_cassette();
	}

	m_cass_t = state;
}

/* Z80-SIO Interface */

void amu880_state::cassette_w(int state)
{
	m_cassbit = state;
	update_cassette();
}

void amu880_state::update_cassette()
{
	m_cassette->output((m_cassbit != m_txc) ? 1.0 : -1.0);
}

/* Z80 Daisy Chain */

static const z80_daisy_config amu880_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ nullptr }
};

/* Machine Initialization */

void amu880_state::machine_start()
{
	m_key_release_timer = timer_alloc(FUNC(amu880_state::key_release), this);
	m_key_repeat_timer = timer_alloc(FUNC(amu880_state::key_repeat), this);
	m_tone_timer = timer_alloc(FUNC(amu880_state::tone_tick), this);
	m_poweroff_timer = timer_alloc(FUNC(amu880_state::power_off), this);

	m_tone_timer->adjust(attotime::from_hz(1300), 0, attotime::from_hz(1300));

	/* register for state saving */
	save_item(NAME(m_key_d6));
	save_item(NAME(m_key_d7));
	save_item(NAME(m_key_a4));
	save_item(NAME(m_key_a5));
	save_item(NAME(m_key_down));
	save_item(NAME(m_key_repeat));
	save_item(NAME(m_shift_lock));
	save_item(NAME(m_poweroff_sr));
	save_item(NAME(m_tone_on));
	save_item(NAME(m_tone_q));
	save_item(NAME(m_cass_t));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_txc));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_rx_d));
	save_item(NAME(m_rx_mono));
	save_item(NAME(m_rx_count));
}

void amu880_state::machine_reset()
{
	m_boot.select(0);

	m_poweroff_sr = 0;
	m_poweroff_timer->adjust(attotime::never);
	m_poweroff_led = 0;
	m_tone_on = false;

	m_rx_mono = false;
	m_rx_count = BIT(m_cass_config->read(), 1, 4);
}

/* Machine Driver */

/* F4 Character Displayer */
static const gfx_layout amu880_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_amu880 )
	GFXDECODE_ENTRY( "chargen", 0x0000, amu880_charlayout, 0, 1 )
GFXDECODE_END


void amu880_state::amu880(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000)/4); // U880D
	m_maincpu->set_addrmap(AS_PROGRAM, &amu880_state::amu880_mem);
	m_maincpu->set_addrmap(AS_IO, &amu880_state::amu880_io);
	m_maincpu->set_daisy_config(amu880_daisy_chain);

	TIMER(config, "keyboard").configure_periodic(FUNC(amu880_state::keyboard_tick), attotime::from_hz(1500));

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_screen_update(FUNC(amu880_state::screen_update));
	screen.set_raw(9000000, 576, 0*6, 64*6, 320, 0*10, 24*10);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_amu880);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	z80ctc_device& ctc(Z80CTC(config, Z80CTC_TAG, XTAL(10'000'000)/4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(amu880_state::ctc_z0_w));
	ctc.zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	ctc.zc_callback<2>().set(FUNC(amu880_state::ctc_z2_w));

	z80pio_device& pio1(Z80PIO(config, Z80PIO1_TAG, XTAL(10'000'000)/4));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio2(Z80PIO(config, Z80PIO2_TAG, XTAL(10'000'000)/4));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, XTAL(10'000'000)/4); // U856
	m_sio->out_txda_callback().set(FUNC(amu880_state::cassette_w));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROMs */

ROM_START( amu880 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v21" )
	ROM_SYSTEM_BIOS( 0, "v21", "H.MON v2.1" )
	ROMX_LOAD( "mon21a.bin", 0xf000, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21b.bin", 0xf400, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21c.bin", 0xf800, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21.bin", 0xf000, 0x0bdf, BAD_DUMP CRC(ba905563) SHA1(1fa0aeab5428731756bdfa74efa3c664898bf083), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v30", "H.MON v3.0" )
	ROMX_LOAD( "mon30.bin", 0xf000, 0x1000, CRC(033f8112) SHA1(0c6ae7b9d310dec093652db6e8ae84f8ebfdcd29), ROM_BIOS(1) )

	ROM_REGION( 0x4800, "hbasic", 0 )
	ROM_LOAD( "mon30p_hbasic33p.bin", 0x0000, 0x4800, CRC(c927e7be) SHA1(2d1f3ff4d882c40438a1281872c6037b2f07fdf2) )

	ROM_REGION( 0x400, "chargen", 0 )
	ROM_LOAD( "hemcfont.bin", 0x0000, 0x0400, CRC(1074d103) SHA1(e558279cff5744acef4eccf30759a9508b7f8750) )

	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "keyboard.bin", 0x0000, 0x0400, BAD_DUMP CRC(daa06361) SHA1(b0299bd8d1686e05dbeeaed54f6c41ae543be20c) ) // typed in from manual
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT        COMPANY                   FULLNAME                                      FLAGS */
COMP( 1983, amu880, 0,      0,      amu880, amu880, amu880_state, empty_init, "Militaerverlag der DDR", "Ausbaufaehiger Mikrocomputer mit dem U 880", MACHINE_IMPERFECT_SOUND )
