// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio RZ-1

    Sampling drum machine

    Hardware:
    - uPD7811G-120
    - HN4872128G25 (program rom)
    - uPD4364C (data ram, battery backed)
    - 2x uPD934G (percussion generator)
    - 2x HN613256P (sample rom)
    - 2x uPD4364C-15L (sample ram, battery backed)
    - EXK-F19Z2064 (10-bit DAC)

    Notes:
    - Each sample ROM holds 1.49s of sounds in the following format:
      PCM data, signed 8-bit, mono, 20,000 Hz
    - Holding EDIT/RECORD, DELETE, INSERT/AUTO-COMPENSATE and
      CHAIN/BEAT at startup causes the system to go into a RAM test.

    TODO:
    - Metronome
    - Make audio input generic (core support needed)

***************************************************************************/

#include "emu.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7810.h"
#include "formats/trs_cas.h"
#include "imagedev/cassette.h"
#include "machine/nvram.h"
#include "sound/upd934g.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "rz1.lh"


namespace {


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
		m_pg(*this, "pg%u", 0U),
		m_toms(*this, "tom%u", 1U),
		m_bd(*this, "bd"),
		m_cassette(*this, "cassette"),
		m_linein(*this, "linein"),
		m_keys(*this, "kc%u", 0U),
		m_foot(*this, "foot"),
		m_led_sampling(*this, "led_sampling"),
		m_led_song(*this, "led_song"),
		m_led_pattern(*this, "led_pattern"),
		m_led_startstop(*this, "led_startstop"),
		m_port_a(0),
		m_port_b(0xff),
		m_midi_rx(1)
	{ }

	void rz1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<upd7811_device> m_maincpu;
	required_device<hd44780_device> m_hd44780;
	required_device_array<upd934g_device, 2> m_pg;
	required_device_array<speaker_device, 3> m_toms;
	required_device<speaker_device> m_bd;
	required_device<cassette_image_device> m_cassette;
	required_device<cassette_image_device> m_linein;
	required_ioport_array<8> m_keys;
	required_ioport m_foot;

	output_finder<> m_led_sampling;
	output_finder<> m_led_song;
	output_finder<> m_led_pattern;
	output_finder<> m_led_startstop;

	void map(address_map &map) ATTR_COLD;
	void pg0_map(address_map &map) ATTR_COLD;
	void pg1_map(address_map &map) ATTR_COLD;

	uint8_t key_r();

	void rz1_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void leds_w(uint8_t data);

	void upd934g_c_w(offs_t offset, uint8_t data);
	void upd934g_b_w(offs_t offset, uint8_t data);
	uint8_t analog_r();

	uint8_t port_a_r();
	void port_a_w(uint8_t data);
	void port_b_w(uint8_t data);
	uint8_t port_c_r();
	void port_c_w(uint8_t data);

	uint8_t m_port_a;
	uint8_t m_port_b;
	int m_midi_rx;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void rz1_state::map(address_map &map)
{
//  map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x2000, 0x3fff).ram().share("dataram");
	map(0x4000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x8fff).w(FUNC(rz1_state::upd934g_c_w));
	map(0x9000, 0x9fff).rw(FUNC(rz1_state::key_r), FUNC(rz1_state::upd934g_b_w));
	map(0xa000, 0xbfff).ram().share("sample1");
	map(0xc000, 0xdfff).ram().share("sample2");
	map(0xe000, 0xe001).w(FUNC(rz1_state::leds_w));
}

void rz1_state::pg0_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("sample1");
	map(0xa000, 0xbfff).ram().share("sample2");
}

void rz1_state::pg1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
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

	PORT_START("foot")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Foot Switch")        PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD
//**************************************************************************

uint8_t rz1_state::key_r()
{
	uint8_t data = 0;

	for (int i = 0; i < 8; i++)
		if (BIT(m_port_a, i) == 0)
			data |= m_keys[i]->read();

	return data;
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void rz1_state::rz1_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88));    // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE( rz1_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 1 && pos < 16)
		bitmap.pix(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
}

void rz1_state::leds_w(uint8_t data)
{
	// 76------  unknown
	// --5-----  sampling led
	// ---4----  start/stop led
	// ----3---  pattern led red
	// -----2--  pattern led green
	// ------1-  song led red
	// -------0  song led green

	m_led_song = BIT(~data, 0, 2);
	m_led_pattern = BIT(~data, 2, 2);
	m_led_startstop = BIT(~data, 4);
	m_led_sampling = BIT(~data, 5);
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void rz1_state::upd934g_c_w(offs_t offset, uint8_t data)
{
	m_pg[0]->write(offset >> 8, data);
}

void rz1_state::upd934g_b_w(offs_t offset, uint8_t data)
{
	m_pg[1]->write(offset >> 8, data);
}

uint8_t rz1_state::analog_r()
{
	return uint8_t(int8_t(m_linein->input() * 127.0) + 127);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t rz1_state::port_a_r()
{
	if ((BIT(m_port_b, 7) == 0) && (BIT(m_port_b, 6) == 1))
	{
		// Not clear why, but code expects to read busy flag from PA5 rather than PA7
		return bitswap<8>(m_hd44780->read(BIT(m_port_b, 5)), 5, 6, 7, 4, 3, 2, 1, 0);
	}

	logerror("port_a_r (PB = %02x)\n", m_port_b);
	return 0;
}

void rz1_state::port_a_w(uint8_t data)
{
	m_port_a = data;

	if ((BIT(m_port_b, 7) == 0) && (BIT(m_port_b, 6) == 0))
		m_hd44780->write(BIT(m_port_b, 5), data);
}

void rz1_state::port_b_w(uint8_t data)
{
	// 7-------  lcd e
	// -6------  lcd rw
	// --5-----  lcd rs
	// ---4----  percussion generator reset
	// ----3---  metronome trigger
	// -----2--  power-on mute for line-out
	// ------1-  change-over signal tom3/bd
	// -------0  change-over signal tom1/tom2

	if (0)
		logerror("port_b_w: %02x\n", data);

	m_port_b = data;

	m_toms[0]->set_input_gain(0, BIT(data, 0) ? 1.0 : 0.0);
	m_toms[1]->set_input_gain(0, BIT(data, 0) ? 0.0 : 1.0);
	m_toms[2]->set_input_gain(0, BIT(data, 1) ? 1.0 : 0.0);
	m_bd->set_input_gain(0, BIT(data, 1) ? 0.0 : 1.0);
}

uint8_t rz1_state::port_c_r()
{
	// 7-------  foot-sustain input
	// -6------  cassette data out
	// --5-----  cassette remote control
	// ---4----  change-over signal for sampling ram
	// ----3---  cassette data in
	// -----2--  control signal for percussion generator c
	// ------1-  midi in (handled elsewhere: cpu rxd)
	// -------0  midi out (handled elsewhere: cpu txd)

	uint8_t data = 0;

	data |= (m_cassette->input() > 0 ? 0 : 1) << 3;
	data |= m_foot->read() << 7;

	return data;
}

void rz1_state::port_c_w(uint8_t data)
{
	m_cassette->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_cassette->output(BIT(data, 6) ? -1.0 : 1.0);

	logerror("port_c_w: %02x\n", data);
}

void rz1_state::machine_start()
{
	// resolve output finders
	m_led_sampling.resolve();
	m_led_song.resolve();
	m_led_pattern.resolve();
	m_led_startstop.resolve();

	// register for save states
	save_item(NAME(m_port_a));
	save_item(NAME(m_port_b));
	save_item(NAME(m_midi_rx));
}

void rz1_state::machine_reset()
{
	m_midi_rx = 1;
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
	m_maincpu->rxd_func().set([this]() { return m_midi_rx; });
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));
	m_maincpu->an0_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an1_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an2_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an3_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an4_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an5_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an6_func().set(FUNC(rz1_state::analog_r));
	m_maincpu->an7_func().set(FUNC(rz1_state::analog_r));

	NVRAM(config, "dataram", nvram_device::DEFAULT_NONE);
	NVRAM(config, "sample1", nvram_device::DEFAULT_NONE);
	NVRAM(config, "sample2", nvram_device::DEFAULT_NONE);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*16+1, 10);
	screen.set_visarea(0, 6*16, 0, 10-1);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(rz1_state::rz1_palette), 3);

	HD44780(config, m_hd44780, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_hd44780->set_lcd_size(1, 16);
	m_hd44780->set_pixel_update_cb(FUNC(rz1_state::lcd_pixel_update));

	config.set_default_layout(layout_rz1);

	// audio hardware

	// individual drum outputs
	SPEAKER(config, "tom1").front_center();
	SPEAKER(config, "tom2").front_center();
	SPEAKER(config, "tom3").front_center();
	SPEAKER(config, "bd").front_center();
	SPEAKER(config, "rim_and_sd").front_center();
	SPEAKER(config, "hihat").front_center();
	SPEAKER(config, "claps_and_ride").front_center();
	SPEAKER(config, "cowbell_and_crash").front_center();
	SPEAKER(config, "sample_1_and_2").front_center();
	SPEAKER(config, "sample_3_and_4").front_center();
	// for tape / line in
	SPEAKER(config, "speaker").front_center();

	UPD934G(config, m_pg[0], 1333000);
	m_pg[0]->set_addrmap(0, &rz1_state::pg0_map);
	m_pg[0]->add_route(0, "claps_and_ride", 1.0);
	m_pg[0]->add_route(1, "cowbell_and_crash", 1.0);
	m_pg[0]->add_route(2, "sample_1_and_2", 1.0);
	m_pg[0]->add_route(3, "sample_3_and_4", 1.0);

	UPD934G(config, m_pg[1], 1280000);
	m_pg[1]->set_addrmap(0, &rz1_state::pg1_map);
	// tom1/tom2 and tom3/bd are multiplexed together (see port_b_w)
	m_pg[1]->add_route(0, "tom1", 1.0);
	m_pg[1]->add_route(0, "tom2", 1.0);
	m_pg[1]->add_route(1, "tom3", 1.0);
	m_pg[1]->add_route(1, "bd", 1.0);
	m_pg[1]->add_route(2, "rim_and_sd", 1.0);
	m_pg[1]->add_route(3, "hihat", 1.0);

	// midi
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set([this](int state) { m_midi_rx = state; });
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");

	// mt (magnetic tape)
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(trs80l2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("rz1_cass");
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);

	// should be a generic audio input port, using cassette for now
	CASSETTE(config, m_linein);
	m_linein->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_linein->set_interface("audio_cass");
	m_linein->add_route(ALL_OUTPUTS, "speaker", 0.05);

	SOFTWARE_LIST(config, "cass_list").set_original("rz1_cass");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rz1 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811g-120.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("program.bin", 0x0000, 0x4000, CRC(b44b2652) SHA1(b77f8daece9adb177b6ce1ef518fc3238b8c0a9c))

	// Clap, Ride, Cowbell and Crash
	ROM_REGION(0x8000, "pg0", 0)
	ROM_LOAD("sound_b.cm6", 0x0000, 0x8000, CRC(ee5b703e) SHA1(cbf2e92c68901f236678d704e9e695a5c84ff49e))

	// Toms 1~3, Kick, Snare, Rimshot, Closed Hi-Hat, Open Hi-Hat and Metronome Click
	ROM_REGION(0x8000, "pg1", 0)
	ROM_LOAD("sound_a.cm5", 0x0000, 0x8000, CRC(c643ff24) SHA1(e886314d22a9a5473bfa2cb237ecafcf0daedfc1))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME  FLAGS
CONS( 1986, rz1,  0,      0,      rz1,     rz1,   rz1_state, empty_init, "Casio", "RZ-1",   MACHINE_SUPPORTS_SAVE )
