// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Casio CTK-2000 keyboard (and related models)

    - CTK-2000 (2008)
      Basic 61-key model
    - CTK-3000
      Adds velocity-sensitive keys and pitch wheel
    - CTK-2100 (2009)
      More flexible sampling feature, lesson buttons double as voice/drum pads
      (based on CTK-2000, not 3000)

    Main board (M800-MDA1):

    IC1: CPU (NEC uPD800468)
        Custom chip (ARM-based), built in peripheral controllers & sound generator

    LSI2: 16Mbit ROM (OKI MR27T1602L)

    Console PCB (M800-CNA):

    IC401: LCD controller (Sitronix ST7066U-0A, HD44780 compatible)

    CTK-2000 service manual with schematics, pinouts, etc.:
    https://www.manualslib.com/manual/933451/Casio-Ctk-2000.html

 */

#include "emu.h"

#include "cpu/arm7/upd800468.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class ctk2000_state : public driver_device
{
public:
	ctk2000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
	{
	}

	void ctk2000(machine_config &config);

	ioport_value lcd_r() { return m_lcdc->db_r() >> 4; }
	void lcd_w(int state) { m_lcdc->db_w(state << 4); }

	void apo_w(int state);

private:
	void ctk2000_map(address_map &map) ATTR_COLD;

	virtual void driver_start() override;
	virtual void driver_reset() override;

	HD44780_PIXEL_UPDATE(lcd_update);
	void palette_init(palette_device &palette);

	required_device<upd800468_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

void ctk2000_state::apo_w(int state)
{
	logerror("apo_w: %x\n", state);
	/* TODO: when 0, this should turn off the LCD, speakers, etc. */
}

HD44780_PIXEL_UPDATE(ctk2000_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void ctk2000_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(255, 255, 255));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

void ctk2000_state::ctk2000_map(address_map &map)
{
	// ROM is based at 0x18000000, but needs to be mirrored to the beginning of memory in order to boot
	map(0x00000000, 0x001fffff).rom().mirror(0x18e00000);
}

void ctk2000_state::driver_start()
{
}

void ctk2000_state::driver_reset()
{
}

void ctk2000_state::ctk2000(machine_config &config)
{
	// CPU
	UPD800468(config, m_maincpu, 48'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ctk2000_state::ctk2000_map);
	m_maincpu->port_in_cb<2>().set_ioport("P2_R");
	m_maincpu->port_out_cb<2>().set_ioport("P2_W");
	m_maincpu->port_out_cb<3>().set_ioport("P3");
	// ADCs 0, 5, 6, 7 are connected to the mic input
	// ADC 4 is connected to the pitch wheel (for ctk3000)
	m_maincpu->adc_cb<1>().set_ioport("AIN1");
	m_maincpu->adc_cb<2>().set_ioport("AIN2");
	m_maincpu->adc_cb<3>().set_ioport("AIN3");

	// LCD
	HD44780(config, m_lcdc, 270'000); // TODO: Wrong device type, should be ST7066U_0A; clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(ctk2000_state::lcd_update));

	// screen (for testing only)
	// TODO: the actual LCD with custom segments
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 8, 8 * 2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ctk2000_state::palette_init), 2);
}

INPUT_PORTS_START(ctk2100)
	PORT_START("maincpu:kbd:FI0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C2#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D2#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F2#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G2")

	PORT_START("maincpu:kbd:FI1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G2#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A2#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C3#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D3#")

	PORT_START("maincpu:kbd:FI2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F3#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G3#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A3#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B3")

	PORT_START("maincpu:kbd:FI3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C4#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D4#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F4#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G4")

	PORT_START("maincpu:kbd:FI4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G4#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A4#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C5#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D5#")

	PORT_START("maincpu:kbd:FI5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F5#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G5")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G5#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A5#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B5")

	PORT_START("maincpu:kbd:FI6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C6#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("D6#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("E6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("F6#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G6")

	PORT_START("maincpu:kbd:FI7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("G6#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("A6#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("B6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("C7")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("maincpu:kbd:FI8")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("maincpu:kbd:FI9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("maincpu:kbd:FI10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Pad 5 / Auto")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Synchro / Ending / Pause")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Accomp On/Off / Chord / Part Select")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Sampling")

	PORT_START("maincpu:kbd:KI0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("maincpu:kbd:KI1")
	// "song bank" and "tone" seem to be swapped in the schematic
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Song Bank")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tone")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tempo Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Set Select / Music Challenge")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Pad 4 / Next")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Play / Stop")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Metronome")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Intro / Repeat")

	PORT_START("maincpu:kbd:KI2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Rhythm")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Tempo Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Pad 2 / Watch")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Pad 3 / Remember")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Voice Pad 1 / Listen")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Normal / Fill In / Rewind")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Function")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Variation / Fill In / Fast Forward")

	PORT_START("P2_R")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )  PORT_CUSTOM_MEMBER(ctk2000_state, lcd_r)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_W")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(ctk2000_state, lcd_w)
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, e_w)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_MEMBER(ctk2000_state, apo_w)

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, rw_w)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT )  PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", hd44780_device, rs_w)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("AIN1")
	PORT_BIT( 0x3ff, IP_ACTIVE_LOW, IPT_OTHER )  PORT_NAME("Pedal")

	PORT_START("AIN2")
	PORT_CONFNAME( 0x3ff, 0x000, "Power Source" )
	PORT_CONFSETTING(     0x000, "AC Adapter" )
	PORT_CONFSETTING(     0x3ff, "Battery" )

	PORT_START("AIN3")
	PORT_BIT( 0x3ff, IP_ACTIVE_LOW, IPT_UNUSED )   PORT_CONDITION("AIN2", 0x3ff, EQUALS, 0)
	PORT_CONFNAME( 0x3ff, 0x3ff, "Battery Level" ) PORT_CONDITION("AIN2", 0x3ff, NOTEQUALS, 0)
	// values here are somewhat arbitrary - ctk2100 only checks if the value is above/below a certain threshold
	PORT_CONFSETTING(     0x100, "Low" )
	PORT_CONFSETTING(     0x3ff, "Normal" )

INPUT_PORTS_END

ROM_START(ctk2100)
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("ctk2100.ic2", 0x000000, 0x200000, CRC(daf62f81) SHA1(4edc76ea04b59090d02646d92f9fc635a43140e9))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME     FLAGS
SYST( 2009, ctk2100, 0,      0,      ctk2000, ctk2100, ctk2000_state, empty_init, "Casio", "CTK-2100",  MACHINE_NO_SOUND | MACHINE_NODEVICE_MICROPHONE | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
