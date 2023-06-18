// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio CZ-101

    Digital Synthesizer

***************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7811.h"
#include "machine/clock.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"

#include "cz101.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cz101_state : public driver_device
{
public:
	cz101_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hd44780(*this, "hd44780"),
		m_keys(*this, "kc%u", 0),
		m_leds(*this, "led_%u", 0U),
		m_port_b(0),
		m_port_c(0),
		m_midi_rx(1)
	{ }

	void cz101(machine_config &config);

	void cz101_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void maincpu_map(address_map &map);

	void port_b_w(uint8_t data);
	void port_c_w(uint8_t data);

	void led_1_w(uint8_t data);
	void led_2_w(uint8_t data);
	void led_3_w(uint8_t data);
	void led_4_w(uint8_t data);
	uint8_t keys_r();
	void sound_w(uint8_t data);

	required_device<upd7810_device> m_maincpu;
	required_device<hd44780_device> m_hd44780;
	required_ioport_array<16> m_keys;
	output_finder<32> m_leds;

	uint8_t m_port_b;
	uint8_t m_port_c;
	uint8_t m_midi_rx;

	uint8_t m_sound_data[2];
	uint8_t m_sound_data_pos;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cz101_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x97ff).noprw(); // rampack
	map(0x9800, 0x9fff).w(FUNC(cz101_state::led_4_w));
	map(0xa000, 0xa7ff).w(FUNC(cz101_state::led_3_w));
	map(0xa800, 0xafff).w(FUNC(cz101_state::led_2_w));
	map(0xb000, 0xb7ff).w(FUNC(cz101_state::led_1_w));
	map(0xb800, 0xbfff).r(FUNC(cz101_state::keys_r));
	map(0xc000, 0xfeff).w(FUNC(cz101_state::sound_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( cz101 )
	PORT_START("kc0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C#2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D#2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F#2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G#2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A#2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C#3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D#3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F#3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G#3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A#3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C#4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D#4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F#4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G#4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A#4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C#5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D#5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F#5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G#5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A#5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C6")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PORTMNT ON/OFF")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PORTMT TIME")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VIBRATO ON/OFF")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BEND RANGE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PRESET")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INTERNAL")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CARTRIDGE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMP/RECALL")

	PORT_START("kc10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SOLO")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE MIX")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRANSPOSE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("WRITE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PROTECT")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SELECT")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")

	PORT_START("kc11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TONE 8")

	PORT_START("kc12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SAVE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOAD")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"CURSOR \u9665") // ◁
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"CURSOR \u9655") // ▷
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ENV \u9661") // ▽
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ENV \u9651") // △
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENV SUST.")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENV END")

	PORT_START("kc13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PACK DETECT")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VIBRAT")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCO1 WAVE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCO1 ENV")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCW1 KEY")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCW1 ENV")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCA1 KEY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCA1 ENV")

	PORT_START("kc14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INITIALIZE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OCTAVE")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCO2 WAVE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCO2 ENV")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCW2 KEY")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCW2 ENV")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCA2 KEY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DCA2 ENV")

	PORT_START("kc15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DETUNE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE SELECT")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RING")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NOISE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"TUNE \u9661") // ▽
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"TUNE \u9651") // △
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("APO ON/OFF")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("AN1")
	PORT_BIT(0xff, 0x7f, IPT_PADDLE) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cz101_state::cz101_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

HD44780_PIXEL_UPDATE( cz101_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 16)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}

void cz101_state::led_4_w(uint8_t data)
{
	m_leds[0] = BIT(~data, 7);
	m_leds[1] = BIT(~data, 6);
	m_leds[2] = BIT(~data, 5);
	m_leds[3] = BIT(~data, 4);
	m_leds[4] = BIT(~data, 3);
	m_leds[5] = BIT(~data, 2);
	m_leds[6] = BIT(~data, 1);
	m_leds[7] = BIT(~data, 0);
}

void cz101_state::led_3_w(uint8_t data)
{
	m_leds[8] = BIT(~data, 7);
	m_leds[9] = BIT(~data, 6);
	m_leds[10] = BIT(~data, 5);
	m_leds[11] = BIT(~data, 4);
	m_leds[12] = BIT(~data, 3);
	m_leds[13] = BIT(~data, 2);
	m_leds[14] = BIT(~data, 1);
	m_leds[15] = BIT(~data, 0);
}

void cz101_state::led_2_w(uint8_t data)
{
	m_leds[16] = BIT(~data, 7);
	m_leds[17] = BIT(~data, 6);
	m_leds[18] = BIT(~data, 5);
	m_leds[19] = BIT(~data, 4);
	m_leds[20] = BIT(~data, 3);
	m_leds[21] = BIT(~data, 2);
	m_leds[22] = BIT(~data, 1);
	m_leds[23] = BIT(~data, 0);
}

void cz101_state::led_1_w(uint8_t data)
{
	m_leds[24] = BIT(~data, 7);
	m_leds[25] = BIT(~data, 6);
	m_leds[26] = BIT(~data, 5);
	m_leds[27] = BIT(~data, 4);
	m_leds[28] = BIT(~data, 3);
	m_leds[29] = BIT(~data, 2);
	m_leds[30] = BIT(~data, 1);
	m_leds[31] = BIT(~data, 0);
}

uint8_t cz101_state::keys_r()
{
	return m_keys[m_port_b & 0x0f]->read();
}

void cz101_state::sound_w(uint8_t data)
{
	if (m_sound_data_pos >= 2)
	{
		logerror("sound reg write: %02x %02x %02x\n", m_sound_data[0], m_sound_data[1], data);
		m_sound_data_pos = 0;
	}
	else
	{
		m_sound_data[m_sound_data_pos++] = data;
	}
}

// 7-------  power switch input (also connected to /NMI)
// -6------  music lsi write enable
// --5-----  music lsi chip select
// ---4----  music lsi irq input
// ----3210  key select output

void cz101_state::port_b_w(uint8_t data)
{
	LOG("port_b_w: %02x\n", data);

	m_port_b = data;
}

// 7-------  lcd e
// -6------  lcd rw
// --5-----  lcd rs
// ---4----  not used
// ----3---  power down detection output
// -----2--  midi clock
// ------1-  midi input
// -------0  midi output

void cz101_state::port_c_w(uint8_t data)
{
	LOG("port_c_w: %02x\n", data);

	m_port_c = data;
	m_hd44780->e_w(!BIT(data, 7));
	m_hd44780->rw_w(BIT(data, 6));
	m_hd44780->rs_w(BIT(data, 5));
}

void cz101_state::machine_start()
{
	m_leds.resolve();

	// register for save states
	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_midi_rx));
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_data_pos));
}

void cz101_state::machine_reset()
{
	m_sound_data[0] = m_sound_data[1] = 0;
	m_sound_data_pos = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void cz101_state::cz101(machine_config &config)
{
	UPD7810(config, m_maincpu, 10_MHz_XTAL); // actually 7811, but internal ROM disabled
	m_maincpu->set_addrmap(AS_PROGRAM, &cz101_state::maincpu_map);
	m_maincpu->pa_in_cb().set(m_hd44780, FUNC(hd44780_device::db_r));
	m_maincpu->pa_out_cb().set(m_hd44780, FUNC(hd44780_device::db_w));
	m_maincpu->pb_in_cb().set_constant(0x80); // TODO: power switch
	m_maincpu->pb_out_cb().set(FUNC(cz101_state::port_b_w));
	m_maincpu->pc_out_cb().set(FUNC(cz101_state::port_c_w));
	m_maincpu->an1_func().set_ioport("AN1");
	m_maincpu->set_pc_pullups(0x03);

	CLOCK(config, "midi_clock", 2_MHz_XTAL).signal_handler().set(m_maincpu, FUNC(upd7810_device::sck_w));

	midi_port_device& mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set([this] (int state) { m_midi_rx = state; });
	m_maincpu->rxd_func().set([this] () { return m_midi_rx; });

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16 + 1, 19);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(cz101_state::cz101_palette), 3);

	HD44780(config, m_hd44780, 0);
	m_hd44780->set_lcd_size(2, 16);
	m_hd44780->set_function_set_at_any_time();
	m_hd44780->set_pixel_update_cb(FUNC(cz101_state::lcd_pixel_update));

	config.set_default_layout(layout_cz101);

	//UPD933(config, "music", 8.96_MHz_XTAL / 2);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( cz101 )
	ROM_REGION(0x1000, "maincpu", 0) // same internal ROM as RZ-1 and SZ-1, but disabled by mode pins
	ROM_LOAD("upd7811g-120.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	/*
	* according to a 1986 Casio service bulletin (T.N. #84 / 84A), three ROM revisions were released to fix MIDI bugs:
	* - "Version I":   HN613256PN-26 mask ROM
	* - "Version II":  HN613256PS-40 mask ROM (misprinted as N-40 in service bulletin)
	* - "Version III": uPD27C256D-20 EPROM
	*/
	ROM_REGION(0x8000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v2", "Version II" )
	ROMX_LOAD("hn613256ps40.bin", 0x0000, 0x8000, CRC(c417bc57) SHA1(2aa5bfb76dc0a56797cf5dd547197816cedfa370), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1", "Version I" )
	ROMX_LOAD("hn613256pn26.bin", 0x0000, 0x8000, CRC(e6c7780e) SHA1(52ff2c280392e104e84b05288c2b952fc29b50f4), ROM_BIOS(1))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
CONS( 1984, cz101, 0,      0,      cz101,   cz101, cz101_state, empty_init, "Casio", "CZ-101", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
