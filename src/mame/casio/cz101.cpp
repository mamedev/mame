// license: BSD-3-Clause
// copyright-holders: Dirk Best, Devin Acker
/***************************************************************************

    Casio CZ-101

    Digital Synthesizer

    Misc. notes:

    To run a (currently undumped) test/diagnostic/debug cartridge:
    Hold "env step +", "env step -", "initialize", and "write" all at once, then press "load".
    If a cartridge is inserted that begins with the 8 bytes "5a 96 5a 96 5a 96 5a 96", then
    the CZ-101 firmware will copy the first 2kb of the cart to $8800-8fff and then call $8810.
    This works even when the normal "cart detect" signal isn't present.
    (Note that this will wipe out all patches saved to the internal RAM.)

    Unused input matrix bits:
    Bit 7 of KC15 is normally unused, but if pulled low using a diode, then bits 2-5 of KC8
    (also normally unused) become DIP switches that override the normal MIDI "basic channel"
    setting from the front panel (possibly planned for use on a screenless MIDI module).

***************************************************************************/

#include "emu.h"

#include "ra3.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/upd933.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

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
		m_upd933(*this, "upd933"),
		m_cart(*this, "cart"),
		m_keys(*this, "kc%u", 0),
		m_leds(*this, "led_%u", 0U),
		m_led_env(*this, "led_env%u.%u", 0U, 0U),
		m_led_tone(*this, "led_tone%u.%u", 0U, 0U),
		m_power(0),
		m_port_b(0),
		m_port_c(0),
		m_midi_rx(1)
	{ }

	void cz101(machine_config &config);

	void cz101_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	DECLARE_INPUT_CHANGED_MEMBER(power_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void maincpu_map(address_map &map) ATTR_COLD;

	void port_b_w(uint8_t data);
	void port_c_w(uint8_t data);

	void led_1_w(uint8_t data);
	void led_2_w(uint8_t data);
	void led_3_w(uint8_t data);
	void led_4_w(uint8_t data);
	uint8_t keys_r();

	required_device<upd7810_device> m_maincpu;
	required_device<hd44780_device> m_hd44780;
	required_device<upd933_device> m_upd933;
	required_device<casio_ram_cart_device> m_cart;
	required_ioport_array<16> m_keys;
	output_finder<16> m_leds;
	output_finder<3, 4> m_led_env;
	output_finder<3, 3> m_led_tone;

	uint8_t m_power;
	uint8_t m_port_b;
	uint8_t m_port_c;
	uint8_t m_midi_rx;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cz101_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x8fff).ram().share("nvram");
	map(0x9000, 0x97ff).rw(m_cart, FUNC(casio_ram_cart_device::read), FUNC(casio_ram_cart_device::write));
	map(0x9800, 0x9fff).w(FUNC(cz101_state::led_4_w));
	map(0xa000, 0xa7ff).w(FUNC(cz101_state::led_3_w));
	map(0xa800, 0xafff).w(FUNC(cz101_state::led_2_w));
	map(0xb000, 0xb7ff).w(FUNC(cz101_state::led_1_w));
	map(0xb800, 0xbfff).r(FUNC(cz101_state::keys_r));
	map(0xc000, 0xfeff).rw(m_upd933, FUNC(upd933_device::read), FUNC(upd933_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( cz101 )
	PORT_START("kc0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C#2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D#2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F#2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G#2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A#2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B2")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C#3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D#3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F#3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G#3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A#3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B3")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C#4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D#4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F#4")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G4")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G#4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A#4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C#5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D#5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F#5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G#5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A#5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B5")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C6")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kc9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Portamento On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Portamento Time")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Vibrato On/Off")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Bend Range")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Preset")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Internal")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Cartridge")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Compare/Recall")

	PORT_START("kc10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E)     PORT_NAME("Solo")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R)     PORT_NAME("Tone Mix")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T)     PORT_NAME("Key Transpose")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)     PORT_NAME("Write")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_U)     PORT_NAME("MIDI")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER)  PORT_TOGGLE              PORT_NAME("Memory Protect")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Select")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MEMORY_RESET)                    PORT_NAME("P (Reset RAM)")

	PORT_START("kc11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Tone 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Tone 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Tone 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Tone 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Tone 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Tone 6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Tone 7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Tone 8")

	PORT_START("kc12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN)   PORT_NAME(u8"Value \u25bd / Save") // ▽
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)     PORT_NAME(u8"Value \u25b3 / Load") // △
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)   PORT_NAME(u8"Cursor \u25c1") // ◁
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT)  PORT_NAME(u8"Cursor \u25b7") // ▷
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_END)    PORT_NAME(u8"Env. Step \u25bd") // ▽
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME)   PORT_NAME(u8"Env. Step \u25b3") // △
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Env. Point Sustain")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL)    PORT_NAME("Env. Point End")

	PORT_START("kc13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("cart", casio_ram_cart_device, exists)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Vibrato")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("DCO1 Wave Form")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("DCO1 Envelope")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("DCW1 Key Follow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("DCW1 Envelope")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("DCA1 Key Follow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("DCA1 Envelope")

	PORT_START("kc14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Initialize")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z)     PORT_NAME("Octave")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X)     PORT_NAME("DCO2 Wave Form")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C)     PORT_NAME("DCO2 Envelope")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V)     PORT_NAME("DCW2 Key Follow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B)     PORT_NAME("DCW2 Envelope")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_N)     PORT_NAME("DCA2 Key Follow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_M)     PORT_NAME("DCA2 Envelope")

	PORT_START("kc15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K)      PORT_NAME("Detune")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L)      PORT_NAME("Line Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP)   PORT_NAME("Ring Modulation")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH)  PORT_NAME("Noise Modulation")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS)  PORT_NAME(u8"Master Tune \u25bd") // ▽
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME(u8"Master Tune \u25b3") // △
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER)  PORT_TOGGLE               PORT_NAME("Auto Power Off")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PB")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("upd933", upd933_device, rq_r)
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_NAME("Power") PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, cz101_state, power_w, 0)

	PORT_START("AN1")
	PORT_BIT(0xff, 0x7f, IPT_PADDLE) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN2")
	PORT_CONFNAME(0xff, 0xff, "Battery Level")
	PORT_CONFSETTING(   0xff, "Normal")
	PORT_CONFSETTING(   0x00, "Low")
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cz101_state::cz101_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 63,  59,  62)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

HD44780_PIXEL_UPDATE( cz101_state::lcd_pixel_update )
{
	// char size is 5x8
	if (!m_power || x > 4 || y > 7)
		return;

	if (line < 2 && pos < 16)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}

INPUT_CHANGED_MEMBER(cz101_state::power_w)
{
	if (!newval)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		m_power = 1;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

void cz101_state::led_4_w(uint8_t data)
{
	if (!m_power) data = 0xff;

	for (int i = 0; i < 7; i++)
		m_leds[i] = BIT(~data, i);
}

void cz101_state::led_3_w(uint8_t data)
{
	if (!m_power) data = 0xff;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			m_led_env[i][j] = BIT(data, 4+i) & BIT(~data, j);
}

void cz101_state::led_2_w(uint8_t data)
{
	if (!m_power) data = 0xff;

	m_leds[7] = BIT(~data, 7);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m_led_tone[i][j] = BIT(data, 1+i) & BIT(~data, 4+j);
}

void cz101_state::led_1_w(uint8_t data)
{
	if (!m_power) data = 0xff;

	for (int i = 0; i < 8; i++)
		m_leds[8+i] = BIT(~data, i);
}

uint8_t cz101_state::keys_r()
{
	return m_keys[m_port_b & 0x0f]->read();
}

// 7-------  power switch input (also connected to /NMI)
// -6------  music lsi write enable
// --5-----  music lsi chip select
// ---4----  music lsi irq input
// ----3210  key select output

void cz101_state::port_b_w(uint8_t data)
{
	LOG("port_b_w: %02x\n", data);

	m_upd933->cs_w(BIT(data, 5));

	m_port_b = data;
}

// 7-------  lcd e
// -6------  lcd rw
// --5-----  lcd rs
// ---4----  not used / debug? (MIDI CC #7 writes data to port A and then toggles this bit)
// ----3---  power down output
// -----2--  midi clock
// ------1-  midi input
// -------0  midi output

void cz101_state::port_c_w(uint8_t data)
{
	LOG("port_c_w: %02x\n", data);

	m_port_c = data;
	if (BIT(data, 3))
	{
		m_hd44780->e_w(BIT(~data, 7));
		m_hd44780->rw_w(BIT(data, 6));
		m_hd44780->rs_w(BIT(data, 5));
	}
	else
	{
		m_power = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_hd44780->reset();
		m_upd933->reset();
		led_1_w(0xff);
		led_2_w(0xff);
		led_3_w(0xff);
		led_4_w(0xff);
	}
}

void cz101_state::machine_start()
{
	m_leds.resolve();
	m_led_env.resolve();
	m_led_tone.resolve();

	// register for save states
	save_item(NAME(m_power));
	save_item(NAME(m_port_b));
	save_item(NAME(m_port_c));
	save_item(NAME(m_midi_rx));
}

void cz101_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void cz101_state::cz101(machine_config &config)
{
	UPD7811(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cz101_state::maincpu_map);
	m_maincpu->pa_in_cb().set(m_hd44780, FUNC(hd44780_device::db_r));
	m_maincpu->pa_out_cb().set(m_hd44780, FUNC(hd44780_device::db_w));
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pb_out_cb().set(FUNC(cz101_state::port_b_w));
	m_maincpu->pc_out_cb().set(FUNC(cz101_state::port_c_w));
	m_maincpu->an1_func().set_ioport("AN1");
	m_maincpu->an2_func().set_ioport("AN2");

	CLOCK(config, "midi_clock", 2_MHz_XTAL).signal_handler().set(m_maincpu, FUNC(upd7810_device::sck_w));

	NVRAM(config, "nvram"); // backed by external battery when RAM cartridge is inserted
	CASIO_RA3(config, m_cart);

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

	HD44780(config, m_hd44780, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_hd44780->set_lcd_size(2, 16);
	m_hd44780->set_function_set_at_any_time();
	m_hd44780->set_pixel_update_cb(FUNC(cz101_state::lcd_pixel_update));

	config.set_default_layout(layout_cz101);

	SPEAKER(config, "speaker").front_center();

	UPD933(config, m_upd933, 8.96_MHz_XTAL / 2);
	m_upd933->irq_cb().set_inputline(m_maincpu, UPD7810_INTF1);
	m_upd933->add_route(0, "speaker", 1.0);
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

	ROM_REGION(0x1000, "nvram", 0)
	ROM_LOAD("init_ram.bin", 0x0000, 0x1000, CRC(11010fa5) SHA1(a5f33408cc2852c8429b828849675d231c370831))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
CONS( 1984, cz101, 0,      0,      cz101,   cz101, cz101_state, empty_init, "Casio", "CZ-101", MACHINE_SUPPORTS_SAVE )
