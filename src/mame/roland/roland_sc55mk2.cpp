// license:BSD-3-Clause
// copyright-holders:R. Belmont, superctr
/*************************************************************************************************

    Roland Sound Canvas SC-55mkII

    The Roland SC-55mkII is an expander (synthesizer without the keyboard)
    from 1993.  It has 28 voice polyphony, is 16 part multitimbral, and
    outputs 18-bit stereo samples at 32 kHz.  The synthesis engine is
    straight PCM playback with TVF/TVA and reverb/chorus, all inside the
    GP-4 PCM gate array.

    The front panel includes the power switch, a headphone jack with volume knob,
    a second MIDI IN port, a large LCD, ALL and MUTE buttons, and a group of up/down
    buttons for Part, Level, Reverb, Key Shift, Instrument, Pan, Chorus, and MIDI Channel.

    Main PCB (from the service notes, Apr. 1993):

    X1 24.000 MHz crystal (main CPU), X3 10.000 MHz (sub CPU), X2 455 kHz (remote decoder)
    IC21 Roland R15199848  HD6475328F    Hitachi H8/532 MCU with internal ROM (main CPU)
    IC24 Roland R15199849  M37409M2-FP   Mitsubishi M740 series microcontroller (sub CPU)
    IC30 Roland R15209463  4M MASK ROM   Main program, version 1.00
    IC15 Roland R15209359  16M WAVE ROM
    IC16 Roland R15279813  8M WAVE ROM
    IC28 Roland R15279543  SRM20256SLM10 SRAM (32K x 8-bit non-volatile work RAM)
    IC25 Roland R15179463  TC51832FL-85  PSRAM (32Kword x 8-bit PCM custom chip's work RAM)
    IC9  Roland R15219714  uPD63200GS-E2 D/A converter
    IC26 Roland R15239176  TC6116AF      GP-4 PCM custom
    LCD unit RCM2024T (HD44780 command set)

    MIDI IN 1/2 and the RS-422/RS-232 computer port go to the sub CPU's three UARTs;
    the sub CPU merges them and sends the stream to the main CPU's SCI from UART 2's TxD;
    UART 3's CTS pin doubles as an attention line to the main CPU.
    MIDI OUT comes from the main CPU's SCI. The front panel switch matrix hangs off
    the sub CPU's ports; the two CPUs talk through the sub CPU's shared RAM.

    TODO:
    - remote control receiver, battery test (analog inputs)
    - RS-422/RS-232 computer port (sub CPU UART 1)
*/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8500/h8532.h"
#include "cpu/m6502/m37409.h"
#include "machine/nvram.h"
#include "sound/roland_gp.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "roland_sc55mk2.lh"


namespace {

class sc55mk2_state : public driver_device
{
public:
	sc55mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_keys(*this, "KEY%u", 0U)
		, m_computer_sw(*this, "COMPUTER")
		, m_screen(*this, "screen")
		, m_lcd(*this, "lcd")
		, m_pcm(*this, "pcm")
		, m_nvram(*this, "nvram", 0x8000, ENDIANNESS_BIG)
		, m_led_all(*this, "led_all")
		, m_led_mute(*this, "led_mute")
		, m_led_standby(*this, "led_standby")
	{
	}

	void sc55mk2(machine_config &config);

	void init_sc55mk2();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void sc55mk2_map(address_map &map) ATTR_COLD;
	void lcd_palette(palette_device &palette) const;

	u8 nvram_r(offs_t offset) { return m_nvram[offset]; }
	void nvram_w(offs_t offset, u8 data) { m_nvram[offset] = data; }

	u8 ga_r(offs_t offset);
	void ga_w(offs_t offset, u8 data);
	void sub_cts_w(int state);

	u8 keys_r();
	void key_scan_w(offs_t offset, u8 data, u8 mem_mask);
	u16 analog_r();

	required_device<h8532_device> m_maincpu;
	required_device<m37409_device> m_subcpu;
	required_ioport_array<3> m_keys;
	required_ioport m_computer_sw;
	required_device<screen_device> m_screen;
	required_device<hd44780_device> m_lcd;
	required_device<roland_gp4_device> m_pcm;
	memory_share_creator<u8> m_nvram;
	output_finder<> m_led_all;
	output_finder<> m_led_mute;
	output_finder<> m_led_standby;

	// gate array interrupt aggregator on IRQ1 (source 5 = sub CPU attention line)
	u8 m_ga_int_enable = 0;
	u8 m_ga_int_trigger = 0;
	u8 m_ga_int5_state = 0;
	u8 m_lcd_control = 0;
	u8 m_key_scan = 0xff;
};


void sc55mk2_state::machine_start()
{

	save_item(NAME(m_ga_int_enable));
	save_item(NAME(m_ga_int_trigger));
	save_item(NAME(m_ga_int5_state));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_key_scan));
}

void sc55mk2_state::machine_reset()
{
	m_ga_int_enable = 0;
	m_ga_int_trigger = 0;
	m_ga_int5_state = 0;
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

// The wave ROMs are stored as dumped; the board wiring permutes address lines
// within each 1 MB block and the data lines.
void sc55mk2_state::init_sc55mk2()
{
	memory_region *region = memregion("waverom");
	u8 *rom = region->base();
	const u32 size = region->bytes();

	static const u8 address_lines[20] = { 2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19 };
	static const u8 data_lines[8] = { 2, 0, 4, 5, 7, 6, 3, 1 };

	std::vector<u8> scrambled(rom, rom + size);
	for (u32 i = 0; i < size; i++)
	{
		u32 address = i & ~0xfffff;
		for (int bit = 0; bit < 20; bit++)
			if (BIT(i, bit))
				address |= 1 << address_lines[bit];

		const u8 source = scrambled[address];
		u8 data = 0;
		for (int bit = 0; bit < 8; bit++)
			if (BIT(source, data_lines[bit]))
				data |= 1 << bit;
		rom[i] = data;
	}
}


//-------------------------------------------------
//  gate array: LCD strobe and interrupt aggregator
//-------------------------------------------------

u8 sc55mk2_state::ga_r(offs_t offset)
{
	switch (offset)
	{
	case 2:
	{
		// number of the pending interrupt source; reading acknowledges it
		const u8 trigger = m_ga_int_trigger;
		if (!machine().side_effects_disabled())
		{
			m_ga_int_trigger = 0;
			m_maincpu->set_input_line(1, CLEAR_LINE);
		}
		return trigger;
	}
	case 4: case 5:
		return m_lcd->read(offset & 1);
	default:
		return 0xff;
	}
}

void sc55mk2_state::ga_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 1:
		// bit 0 low enables the LCD, bits 2-3 select the analog multiplexer input
		m_lcd_control = data;
		break;
	case 2:
		// interrupt enable mask, bit n enables source n+1; writing also
		// acknowledges the pending interrupt
		m_ga_int_enable = data;
		m_ga_int_trigger = 0;
		m_maincpu->set_input_line(1, CLEAR_LINE);
		break;
	case 4: case 5:
		m_lcd->write(offset & 1, data);
		break;
	default:
		logerror("%s: unknown gate array write %d = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

// the sub CPU pulses its CTS3 pin low as an output to signal the main CPU;
// source 5 triggers on the falling edge when enabled
void sc55mk2_state::sub_cts_w(int state)
{
	const u8 active = !state;
	if (active && !m_ga_int5_state && BIT(m_ga_int_enable, 4) && m_ga_int_trigger == 0)
	{
		m_ga_int_trigger = 5;
		m_maincpu->set_input_line(1, ASSERT_LINE);
	}
	m_ga_int5_state = active;
}


// AN7 reads the BU4051 multiplexer: battery voltage, n/c, the rear COMPUTER
// selector (a resistor ladder) or the remote control receiver
u16 sc55mk2_state::analog_r()
{
	switch ((m_lcd_control >> 2) & 3)
	{
	case 0:
		return 0x2a0;
	case 2:
		return m_computer_sw->read() * 0x155;
	default:
		return 0;
	}
}


//-------------------------------------------------
//  front panel: switch matrix on the sub CPU's ports, scanned by the main
//  CPU through the port passthrough (P0 walking zero, P1 columns); the
//  ALL, MUTE and STANDBY LEDs hang off P0 bits 6, 5 and 4 (active low)
//-------------------------------------------------

void sc55mk2_state::key_scan_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_key_scan = data;
	m_led_all = BIT(~data, 6);
	m_led_mute = BIT(~data, 5);
	m_led_standby = BIT(~data, 4);
}

u8 sc55mk2_state::keys_r()
{
	u8 data = 0xff;
	for (int row = 0; row < 3; row++)
		if (!BIT(m_key_scan, row))
			data &= m_keys[row]->read();
	return data;
}


//-------------------------------------------------
//  address map
//-------------------------------------------------

void sc55mk2_state::sc55mk2_map(address_map &map)
{
	// 0x00000-0x07fff: internal ROM
	map(0x08000, 0x0dfff).rw(FUNC(sc55mk2_state::nvram_r), FUNC(sc55mk2_state::nvram_w));
	map(0x0e000, 0x0e03f).mirror(0x3c0).rw(m_pcm, FUNC(roland_gp4_device::read), FUNC(roland_gp4_device::write));
	map(0x0e400, 0x0e407).rw(FUNC(sc55mk2_state::ga_r), FUNC(sc55mk2_state::ga_w));
	map(0x0ec00, 0x0ecff).rw(m_subcpu, FUNC(m37409_device::system_r), FUNC(m37409_device::system_w));
	// the program ROM's A18 is driven by CPU A19; pages 1-4 see its lower half, 8-9 and 14-15 the upper
	map(0x10000, 0x3ffff).rom().region("progrom", 0x10000);
	map(0x40000, 0x4ffff).rom().region("progrom", 0x00000);
	map(0x80000, 0x9ffff).rom().region("progrom", 0x40000);
	map(0xa0000, 0xa7fff).mirror(0x18000).ram().share("nvram");
	map(0xe0000, 0xfffff).rom().region("progrom", 0x60000);
}


//-------------------------------------------------
//  LCD
//-------------------------------------------------

void sc55mk2_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xff, 0xa0, 0x20)); // orange backlight
	palette.set_pen_color(1, rgb_t(0x20, 0x10, 0x00));
	palette.set_pen_color(2, rgb_t::black()); // LCD power off
}

// the LCD unit and its backlight are powered through a regulator the CPU switches off in standby
u32 sc55mk2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_lcd_control, 0))
	{
		bitmap.fill(2, cliprect);
		return 0;
	}
	return m_lcd->screen_update(screen, bitmap, cliprect);
}

// The RCM2024T display unit is a custom glass driven by an HD44780-compatible
// controller in 2x40 mode.  Only part of the DDRAM maps onto visible
// character cells; the rest of the glass is a 16 x 16 bar graph (part level
// meters) and a stereo "L R" mark, both lit from character generator RAM.
//
//   line 0, pos  0- 2  part number          line 1, pos  0- 2  level
//   line 0, pos  3-18  instrument name      line 1, pos  3- 5  pan
//   line 0/1, pos 20-23 bar graph            line 1, pos  6- 8  chorus
//     (5 columns per cell = 5 parts,        line 1, pos  9-11  reverb
//      cell 23 column 0 = part 16,          line 1, pos 12-14  key shift
//      line 0 = upper 8 segments)           line 1, pos 15-17  MIDI channel
//                                           line 1, pos 18     L/R mark (row 0, bit 0)

static constexpr int LCD_CHAR_PITCH = 35;
static constexpr int LCD_DOT_PITCH = 6;
static constexpr int LCD_DOT_SIZE = 5;
static constexpr int LCD_COL0_X = 24;      // part number, level, reverb, key shift
static constexpr int LCD_COL1_X = 143;     // instrument, pan, chorus, MIDI channel
static constexpr int LCD_ROW_Y[4] = { 14, 78, 142, 206 };
static constexpr int LCD_BAR_X = 283;
static constexpr int LCD_BAR_Y = 74;
static constexpr int LCD_BAR_PITCH_X = 26;
static constexpr int LCD_BAR_PITCH_Y = 11;
static constexpr int LCD_BAR_W = 24;
static constexpr int LCD_BAR_H = 9;
static constexpr int LCD_LR_X = 254;
static constexpr int LCD_LR_Y[2] = { 74, 234 };

static void lcd_fill(bitmap_ind16 &bitmap, int x, int y, int w, int h, int state)
{
	for (int yy = y; yy < y + h; yy++)
		for (int xx = x; xx < x + w; xx++)
			bitmap.pix(yy, xx) = state;
}

HD44780_PIXEL_UPDATE(sc55mk2_state::lcd_pixel_update)
{
	if (pos >= 20 && pos < 24 && line < 2)
	{
		// bar graph: one CG column per part, one CG row per segment
		int const bar = (pos - 20) * 5 + x;
		if (bar < 16)
			lcd_fill(bitmap, LCD_BAR_X + bar * LCD_BAR_PITCH_X, LCD_BAR_Y + (line * 8 + y) * LCD_BAR_PITCH_Y, LCD_BAR_W, LCD_BAR_H, state);
		return;
	}

	if (line == 1 && pos == 18)
	{
		// L/R mark: single segment lit by bit 0 of the glyph's first row
		if (y == 0 && x == 4)
		{
			static constexpr u16 L_SHAPE[12] = { 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x7ff, 0x7ff };
			static constexpr u16 R_SHAPE[12] = { 0x7fc, 0x7fe, 0x606, 0x606, 0x606, 0x7fe, 0x7fc, 0x630, 0x618, 0x60c, 0x606, 0x603 };
			for (int i = 0; i < 12; i++)
				for (int j = 0; j < 11; j++)
				{
					if (BIT(L_SHAPE[i], 10 - j))
						bitmap.pix(LCD_LR_Y[0] + i, LCD_LR_X + j) = state;
					if (BIT(R_SHAPE[i], 10 - j))
						bitmap.pix(LCD_LR_Y[1] + i, LCD_LR_X + j) = state;
				}
		}
		return;
	}

	if (y >= 7)
		return;

	int cx, cy;
	if (line == 0)
	{
		if (pos < 3)
			cx = LCD_COL0_X + pos * LCD_CHAR_PITCH;
		else if (pos < 19)
			cx = LCD_COL1_X + (pos - 3) * LCD_CHAR_PITCH;
		else
			return;
		cy = LCD_ROW_Y[0];
	}
	else if (line == 1 && pos < 18)
	{
		int const field = pos / 3;
		static constexpr int FIELD_COL[6] = { 0, 1, 1, 0, 0, 1 };
		static constexpr int FIELD_ROW[6] = { 1, 1, 2, 2, 3, 3 };
		cx = (FIELD_COL[field] ? LCD_COL1_X : LCD_COL0_X) + (pos % 3) * LCD_CHAR_PITCH;
		cy = LCD_ROW_Y[FIELD_ROW[field]];
	}
	else
		return;

	lcd_fill(bitmap, cx + x * LCD_DOT_PITCH, cy + y * LCD_DOT_PITCH, LCD_DOT_SIZE, LCD_DOT_SIZE, state);
}


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

static INPUT_PORTS_START( sc55mk2 )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Power")
	PORT_BIT(0x06, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument <")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument >")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Mute")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("All")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MIDI Ch <")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MIDI Ch >")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Chorus <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Chorus >")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pan <")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pan >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Part >")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Shift <")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Shift >")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reverb <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reverb >")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Level <")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Level >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Part <")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// rear panel selector, read through the analog multiplexer as a resistor ladder
	PORT_START("COMPUTER")
	PORT_CONFNAME(0x03, 0x03, "Computer Switch")
	PORT_CONFSETTING(0x03, "MIDI")
	PORT_CONFSETTING(0x02, "RS-232C-2")
	PORT_CONFSETTING(0x01, "RS-232C-1")
	PORT_CONFSETTING(0x00, "RS-422")
INPUT_PORTS_END

void sc55mk2_state::sc55mk2(machine_config &config)
{
	HD6435328(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc55mk2_state::sc55mk2_map);
	m_maincpu->read_port9().set_constant(0x02); // bit 1: 1 = SC-55mkII, 0 = SC-155mkII
	m_maincpu->read_adc<7>().set(FUNC(sc55mk2_state::analog_r));
	m_maincpu->write_sci_tx<0>().set("mdout", FUNC(midi_port_device::write_txd));

	M37409(config, m_subcpu, 10_MHz_XTAL);
	m_subcpu->write_p<0>().set(FUNC(sc55mk2_state::key_scan_w));
	m_subcpu->read_p<1>().set(FUNC(sc55mk2_state::keys_r));
	m_subcpu->cts_handler<2>().set(FUNC(sc55mk2_state::sub_cts_w));
	m_subcpu->txd_handler<1>().set(m_maincpu, FUNC(h8532_device::sci_rx_w<0>)); // merged MIDI stream to the main CPU

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM20256 (IC28) + CR2032 battery

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_subcpu, FUNC(m37409_device::rxd_w<1>));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin2", midiin_slot, "midiin"));
	mdin2.rxd_handler().set(m_subcpu, FUNC(m37409_device::rxd_w<2>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	ROLAND_GP4(config, m_pcm, 24_MHz_XTAL);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->int_callback().set_inputline(m_maincpu, 0); // IRQ0

	SPEAKER(config, "speaker", 2).front();
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(80);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(sc55mk2_state::lcd_palette), 3);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 40);
	m_lcd->set_pixel_update_cb(FUNC(sc55mk2_state::lcd_pixel_update));

	m_screen->set_screen_update(FUNC(sc55mk2_state::screen_update));
	m_screen->set_size(720, 272);
	m_screen->set_visarea_full();

	config.set_default_layout(layout_roland_sc55mk2);
}

ROM_START( sc55mk2 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )  // H8/532 main code
	ROM_LOAD("r15199858_main_mcu.bin", 0x000000, 0x008000, CRC(9b66631f) SHA1(b91bb1d9dccffe831b7cfde7800a3fe32b2fbda6))

	ROM_REGION( 0x1000, "subcpu", 0 )   // M37409M2 sub-CPU with 3 UARTs, clocked at 10 MHz
	ROM_LOAD("r15199880_secondary_mcu.bin", 0x000000, 0x001000, CRC(702c0a82) SHA1(4d48578d811a762a8e7bfaf18989bcac70ae1ba4))

	ROM_REGION( 0x80000, "progrom", 0 ) // additional H8/532 code and patch data
	ROM_LOAD("r00233567_control.bin", 0x000000, 0x080000, CRC(fcee1e8e) SHA1(078cb5feea05e80bb9a1bb857a2163ee434fd053))

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD("r15209359_pcm_1.bin", 0x000000, 0x200000, CRC(1519d3b3) SHA1(96708cb21381c2fd03de4babbf7aea301c7594a6))
	ROM_LOAD("r15279813_pcm_2.bin", 0x200000, 0x100000, CRC(0f826c7f) SHA1(4d91cdeaed048d653dbf846a221003c3a3f08279))
ROM_END

} // anonymous namespace


SYST( 1993, sc55mk2, 0, 0, sc55mk2, sc55mk2, sc55mk2_state, init_sc55mk2, "Roland", "Sound Canvas SC-55mkII", MACHINE_NOT_WORKING )
