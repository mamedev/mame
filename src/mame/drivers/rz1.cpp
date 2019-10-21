// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio RZ-1

    Sampling drum machine

    Sound ROM info:

    Each ROM chip holds 1.49s of sounds and can be opened as raw
    PCM data: signed 8-bit, mono, 20,000 Hz.

    * Sound A: Toms 1~3, Kick, Snare, Rimshot, Closed Hi-Hat, Open Hi-Hat,
        and Metronome Click (in that order).

    * Sound B: Clap, Ride, Cowbell, and Crash (in that order).

    Note: Holding EDIT/RECORD, DELETE, INSERT/AUTO-COMPENSATE and
    CHAIN/BEAT at startup causes the system to go into a RAM test.

    TODO:
    - Metronome
    - MIDI
    - Cassette
    - Audio input

***************************************************************************/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "cpu/upd7810/upd7811.h"
#include "video/hd44780.h"
#include "sound/upd934g.h"
#include "speaker.h"

#include "rz1.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rz1_state : public driver_device
{
public:
	rz1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hd44780(*this, "hd44780"),
		m_pg{ {*this, "upd934g_c"}, {*this, "upd934g_b"} },
		m_samples{ {*this, "samples_a"}, {*this, "samples_b"} },
		m_keys(*this, "kc%u", 0),
		m_led_song(*this, "led_song"),
		m_led_pattern(*this, "led_pattern"),
		m_led_startstop(*this, "led_startstop"),
		m_port_a(0),
		m_port_b(0xff)
	{ }

	void rz1(machine_config &config);

	void rz1_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<upd7811_device> m_maincpu;
	required_device<hd44780_device> m_hd44780;
	required_device<upd934g_device> m_pg[2];
	required_memory_region m_samples[2];
	required_ioport_array<8> m_keys;

	output_finder<> m_led_song;
	output_finder<> m_led_pattern;
	output_finder<> m_led_startstop;

	void map(address_map &map);

	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_c_w);

	DECLARE_READ8_MEMBER(upd934g_c_data_r);
	DECLARE_WRITE8_MEMBER(upd934g_c_w);
	DECLARE_READ8_MEMBER(upd934g_b_data_r);
	DECLARE_WRITE8_MEMBER(upd934g_b_w);
	DECLARE_READ8_MEMBER(key_r);
	DECLARE_WRITE8_MEMBER(leds_w);

	uint8_t m_port_a;
	uint8_t m_port_b;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void rz1_state::map(address_map &map)
{
//  map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x8fff).w(FUNC(rz1_state::upd934g_c_w));
	map(0x9000, 0x9fff).rw(FUNC(rz1_state::key_r), FUNC(rz1_state::upd934g_b_w));
	map(0xa000, 0xbfff).ram(); // sample ram 1
	map(0xc000, 0xdfff).ram(); // sample ram 2
	map(0xe000, 0xe001).w(FUNC(rz1_state::leds_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( rz1 )
	PORT_START("kc0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TOM1")               PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TOM3")               PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIM")                PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("OPEN HH")            PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLAPS")              PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COWBELL")            PORT_CODE(KEYCODE_F6)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TOM2")               PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B D")                PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S D")                PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLOSED HH")          PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIDE")               PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CRASH")              PORT_CODE(KEYCODE_F12)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMPLE 1")           PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMPLE 2")           PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMPLE 3")           PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMPLE 4")           PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MUTE")               PORT_CODE(KEYCODE_M)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ACCENT")             PORT_CODE(KEYCODE_A)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("START/STOP")         PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTINUE START")     PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SAMPLING")           PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TEMPO \xe2\x96\xb3") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TEMPO \xe2\x96\xbd") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET/COPY")         PORT_CODE(KEYCODE_R)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PATTERN")                PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SONG")                   PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EDIT/RECORD")            PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DELETE")                 PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INSERT/AUTO-COMPENSATE") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHAIN/BEAT")             PORT_CODE(KEYCODE_B)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MT SAVE")            PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MT LOAD")            PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MIDI CH")            PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MIDI CLOCK")         PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 (1/2)")            PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 (1/4)")            PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 (1/6)")            PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 (1/8)")            PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 (1/12)")           PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 (1/16)")           PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("kc7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 (1/24)")           PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 (1/32)")           PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (1/48)")           PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (1/96)")           PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\xe2\x96\xb3 (YES)") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\xe2\x96\xbd (NO)")  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

HD44780_PIXEL_UPDATE( rz1_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 1 && pos < 16)
		bitmap.pix16(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
}

READ8_MEMBER( rz1_state::upd934g_c_data_r )
{
	if (offset < 0x8000)
		return m_samples[1]->base()[offset];
	else
	{
		if (offset < 0xc000)
			return m_maincpu->space(AS_PROGRAM).read_byte(offset + 0x2000);
		else
			return 0;
	}
}

READ8_MEMBER( rz1_state::upd934g_b_data_r )
{
	if (offset < 0x8000)
		return m_samples[0]->base()[offset];
	else
		return 0;
}

WRITE8_MEMBER( rz1_state::upd934g_c_w )
{
	m_pg[0]->write(space, offset >> 8, data);
}

WRITE8_MEMBER( rz1_state::upd934g_b_w )
{
	m_pg[1]->write(space, offset >> 8, data);
}

READ8_MEMBER( rz1_state::port_a_r )
{
	if ((BIT(m_port_b, 7) == 0) && (BIT(m_port_b, 6) == 1))
	{
		// Not clear why, but code expects to read busy flag from PA5 rather than PA7
		return bitswap<8>(m_hd44780->read(BIT(m_port_b, 5)), 5, 6, 7, 4, 3, 2, 1, 0);
	}

	logerror("port_a_r (PB = %02x)\n", m_port_b);
	return 0;
}

WRITE8_MEMBER( rz1_state::port_a_w )
{
	m_port_a = data;

	if ((BIT(m_port_b, 7) == 0) && (BIT(m_port_b, 6) == 0))
		m_hd44780->write(BIT(m_port_b, 5), data);
}

// 7-------  lcd e
// -6------  lcd rw
// --5-----  lcd rs
// ---4----  percussion generator reset
// ----3---  metronome trigger
// -----2--  power-on mute for line-out
// ------1-  change-over signal tom3/bd
// -------0  change-over signal tom1/tom2

WRITE8_MEMBER( rz1_state::port_b_w )
{
	if (0)
		logerror("port_b_w: %02x\n", data);

	m_port_b = data;
}

// 7-------  foot-sustain input
// -6------  cassette data out
// --5-----  cassette remote control
// ---4----  change-over signal for sampling ram
// ----3---  cassette data in
// -----2--  control signal for percussion generator c
// ------1-  midi in
// -------0  midi out

READ8_MEMBER( rz1_state::port_c_r )
{
	return 0;
}

WRITE8_MEMBER( rz1_state::port_c_w )
{
	logerror("port_c_w: %02x\n", data);
}

READ8_MEMBER( rz1_state::key_r )
{
	uint8_t data = 0;

	if (BIT(m_port_a, 0) == 0) data |= m_keys[0]->read();
	if (BIT(m_port_a, 1) == 0) data |= m_keys[1]->read();
	if (BIT(m_port_a, 2) == 0) data |= m_keys[2]->read();
	if (BIT(m_port_a, 3) == 0) data |= m_keys[3]->read();
	if (BIT(m_port_a, 4) == 0) data |= m_keys[4]->read();
	if (BIT(m_port_a, 5) == 0) data |= m_keys[5]->read();
	if (BIT(m_port_a, 6) == 0) data |= m_keys[6]->read();
	if (BIT(m_port_a, 7) == 0) data |= m_keys[7]->read();

	return data;
}

WRITE8_MEMBER( rz1_state::leds_w )
{
	m_led_song =      BIT(data, 0) == 0 ? 1 : BIT(data, 1) == 0 ? 2 : 0;
	m_led_pattern =   BIT(data, 2) == 0 ? 1 : BIT(data, 3) == 0 ? 2 : 0;
	m_led_startstop = BIT(data, 4) == 0 ? 1 : 0;
}

void rz1_state::machine_start()
{
	// resolve output finders
	m_led_song.resolve();
	m_led_pattern.resolve();
	m_led_startstop.resolve();

	// register for save states
	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
}

void rz1_state::machine_reset()
{
}

void rz1_state::rz1_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88));    // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void rz1_state::rz1(machine_config &config)
{
	UPD7811(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rz1_state::map);
	m_maincpu->pa_in_cb().set(FUNC(rz1_state::port_a_r));
	m_maincpu->pa_out_cb().set(FUNC(rz1_state::port_a_w));
	m_maincpu->pb_out_cb().set(FUNC(rz1_state::port_b_w));
	m_maincpu->pc_in_cb().set(FUNC(rz1_state::port_c_r));
	m_maincpu->pc_out_cb().set(FUNC(rz1_state::port_c_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*16+1, 10);
	screen.set_visarea(0, 6*16, 0, 10-1);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(rz1_state::rz1_palette), 3);

	HD44780(config, m_hd44780, 0);
	m_hd44780->set_lcd_size(1, 16);
	m_hd44780->set_pixel_update_cb(FUNC(rz1_state::lcd_pixel_update), this);

	config.set_default_layout(layout_rz1);

	SPEAKER(config, "speaker").front_center();
	UPD934G(config, m_pg[0], 1333000);
	m_pg[0]->data_callback().set(FUNC(rz1_state::upd934g_c_data_r));
	m_pg[0]->add_route(ALL_OUTPUTS, "speaker", 1.0);
	UPD934G(config, m_pg[1], 1280000);
	m_pg[1]->data_callback().set(FUNC(rz1_state::upd934g_b_data_r));
	m_pg[1]->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rz1 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("program.bin", 0x0000, 0x4000, CRC(b44b2652) SHA1(b77f8daece9adb177b6ce1ef518fc3238b8c0a9c))

	ROM_REGION(0x8000, "samples_a", 0)
	ROM_LOAD("sound_a.cm5", 0x0000, 0x8000, CRC(c643ff24) SHA1(e886314d22a9a5473bfa2cb237ecafcf0daedfc1)) // HN613256P

	ROM_REGION(0x8000, "samples_b", 0)
	ROM_LOAD("sound_b.cm6", 0x0000, 0x8000, CRC(ee5b703e) SHA1(cbf2e92c68901f236678d704e9e695a5c84ff49e)) // HN613256P
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS
CONS( 1986, rz1,  0,      0,      rz1,     rz1,   rz1_state, empty_init, "Casio", "RZ-1",   MACHINE_SUPPORTS_SAVE )
