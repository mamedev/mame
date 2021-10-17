// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
	Casio CTK-551 keyboard (and related models)

	Casio released several keyboard models with the same main board.
	As usual, some of them were also rebranded by Radio Shack.

	- CTK-531, CTK-533
	  Basic 61-key model
	- CTK-541, Optimus MD-1150
	  Adds velocity-sensitive keys
	- CTK-551, CTK-558, Radio Shack MD-1160
	  Adds pitch wheel and different selection of demo songs

	Main board (JCM456-MA1M):

	LSI1: CPU (Casio GT913F)
	      Custom chip based on H8/300 instruction set, built in peripheral controllers & sound generator

	LSI2: 8Mbit ROM (OKI MSM538002E)

	LSI3: LCD controller (HD44780 compatible)
	      May be either a Samsung KS0066U-10B or Epson SED1278F2A.

	IC1:  stereo DAC (NEC uPD6379GR)

	CTK-541 service manual with schematics, pinouts, etc.:
	https://revenant1.net/ctk541.pdf

	To access the test mode (not mentioned in the service manual):
	Hold the "Start/Stop" and keypad 0 buttons together when turning on the keyboard.
	Afterwards, press one of these buttons:
	- Tone: LCD test (press repeatedly)
	- Keypad 0: switch test (press all front panel buttons in a specific order, generally left to right)
	- Keypad 1 or Rhythm: pedal and key test
	- Keypad 2: ROM test
	- Keypad 4/5/6: sound volume test
	- Keypad 7/8: stereo test
	- Keypad 9: MIDI loopback test
	- Keypad + or Song Bank: power source test
	- Keypad -: pitch wheel test
	- FFWD: exit test mode
	- Stop: power off

 */

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/gt913.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class ctk551_state : public driver_device
{
public:
	ctk551_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_led_touch(*this, "led_touch")
	{
	}

	void ctk551(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(lcd_r)     { return m_lcdc->db_r() >> 4; }
	DECLARE_WRITE_LINE_MEMBER(lcd_w)       { m_lcdc->db_w(state << 4); }

	DECLARE_CUSTOM_INPUT_MEMBER(switch_r)  { return m_switch; }
	DECLARE_INPUT_CHANGED_MEMBER(switch_w);

	DECLARE_WRITE_LINE_MEMBER(led_touch_w) { m_led_touch = state; }
	DECLARE_WRITE_LINE_MEMBER(apo_w);

private:
	void ctk551_io_map(address_map &map);

	virtual void driver_start() override;

	HD44780_PIXEL_UPDATE(lcd_update);
	void palette_init(palette_device &palette);

	required_device<gt913_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;

	output_finder<> m_led_touch;

	ioport_value m_switch;
};


INPUT_CHANGED_MEMBER(ctk551_state::switch_w)
{
	logerror("switch_w: %x\n", param);
	if (!oldval && newval)
	{
		if (m_switch == 0x1 && param != m_switch)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		else
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

		m_switch = param;
	}
}

WRITE_LINE_MEMBER(ctk551_state::apo_w)
{
	logerror("apo_w: %x\n", state);
	/* TODO: when 1, this should turn off the LCD, speakers, etc.
	the CPU will go to sleep until the power switch triggers a NMI */
}

HD44780_PIXEL_UPDATE(ctk551_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void ctk551_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(255, 255, 255));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

void ctk551_state::ctk551_io_map(address_map &map)
{
	map(h8_device::PORT_1, h8_device::PORT_1).portr("P1_R").portw("P1_W").umask16(0x00ff);
	map(h8_device::PORT_2, h8_device::PORT_2).portrw("P2").umask16(0x00ff);
	map(h8_device::PORT_3, h8_device::PORT_3).portrw("P3").umask16(0x00ff);
	map(h8_device::ADC_0,  h8_device::ADC_0).portr("AN0");
	map(h8_device::ADC_1,  h8_device::ADC_1).portr("AN1");
}

void ctk551_state::driver_start()
{
	m_led_touch.resolve();

	m_switch = 0x2;

	save_item(NAME(m_switch));
}

void ctk551_state::ctk551(machine_config &config)
{
	// CPU
	GT913(config, m_maincpu, 30'000'000);
	m_maincpu->set_addrmap(AS_IO, &ctk551_state::ctk551_io_map);

	// MIDI
	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set("maincpu:sci", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->subdevice<h8_sci_device>("sci")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));

	// LCD
	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(ctk551_state::lcd_update));

	// screen (for testing only)
	// TODO: the actual LCD with custom segments
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 8, 8 * 2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ctk551_state::palette_init), 2);
}

INPUT_PORTS_START(ctk551)
	PORT_START("maincpu:kbd:KO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C2#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D2#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F2#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G2")

	PORT_START("maincpu:kbd:KO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G2#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A2#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C3#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D3#")

	PORT_START("maincpu:kbd:KO2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F3#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G3#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A3#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B3")

	PORT_START("maincpu:kbd:KO3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C4#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D4#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F4#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G4")

	PORT_START("maincpu:kbd:KO4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G4#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A4#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C5#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D5#")

	PORT_START("maincpu:kbd:KO5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F5#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G5")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G5#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A5#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B5")

	PORT_START("maincpu:kbd:KO6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C6#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D6#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F6#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G6")

	PORT_START("maincpu:kbd:KO7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G6#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A6#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C7")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("maincpu:kbd:KO8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Touch Response")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Song Bank")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tone")
	
	PORT_START("maincpu:kbd:KO9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Accomp Volume")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Transpose / Tune / MIDI")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Chord Book")

	PORT_START("maincpu:kbd:KO10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Play / Pause")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rewind")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Start / Stop")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Sync / Fill In")

	PORT_START("maincpu:kbd:KO11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Right Hand On/Off")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Left Hand On/Off")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Fast Forward")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tempo Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Volume Down")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tempo Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Volume Up")

	PORT_START("maincpu:kbd:KO12")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(ctk551_state, switch_r)

	PORT_START("maincpu:kbd:VELOCITY")
	PORT_BIT( 0x7f, 0x7f, IPT_POSITIONAL ) PORT_NAME("Key Velocity") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Power Off")   PORT_CHANGED_MEMBER(DEVICE_SELF, ctk551_state, switch_w, 0x1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Normal")      PORT_CHANGED_MEMBER(DEVICE_SELF, ctk551_state, switch_w, 0x2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Casio Chord") PORT_CHANGED_MEMBER(DEVICE_SELF, ctk551_state, switch_w, 0x4)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Fingered")    PORT_CHANGED_MEMBER(DEVICE_SELF, ctk551_state, switch_w, 0x8)

	PORT_START("P1_R")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER )   PORT_NAME("Pedal")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_CUSTOM )  PORT_CUSTOM_MEMBER(ctk551_state, lcd_r)

	PORT_START("P1_W")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(ctk551_state, led_touch_w)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT )  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, e_w)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(ctk551_state, lcd_w)

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, rs_w)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, rw_w)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(ctk551_state, apo_w)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("AN0")
	PORT_CONFNAME( 0xff, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "AC Adapter" )
	PORT_CONFSETTING(    0xff, "Battery" )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff)

INPUT_PORTS_END

ROM_START(ctk551)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP("ctk551.lsi2", 0x000000, 0x100000, CRC(66fc34cd) SHA1(47e9559edc106132f8a83462ed17a6c5c3872157))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME     FLAGS
SYST( 1999, ctk551,  0,      0,      ctk551,  ctk551, ctk551_state, empty_init, "Casio", "CTK-551",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
