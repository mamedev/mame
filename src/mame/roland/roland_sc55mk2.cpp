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
    MIDI OUT comes from the main CPU's SCI. The front panel switch matrix hangs off
    the sub CPU's ports; the two CPUs talk through the sub CPU's shared RAM.

    TODO:
    - M37409M2 sub CPU: MIDI in, panel switches, IPC (currently a stub)
    - remote control receiver, battery test (analog inputs)
    - custom LCD glass layout
*/

#include "emu.h"

#include "bus/midi/midioutport.h"
#include "cpu/h8500/h8532.h"
#include "machine/nvram.h"
#include "sound/roland_gp.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sc55mk2_state : public driver_device
{
public:
	sc55mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_lcd(*this, "lcd")
		, m_pcm(*this, "pcm")
		, m_nvram(*this, "nvram", 0x8000, ENDIANNESS_BIG)
	{
	}

	void sc55mk2(machine_config &config);

	void init_sc55mk2();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

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
	void ga_int_w(int source, int state);
	void pcm_int_w(int state);

	u8 ipc_r(offs_t offset);
	void ipc_w(offs_t offset, u8 data);

	required_device<h8532_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<hd44780_device> m_lcd;
	required_device<roland_gp4_device> m_pcm;
	memory_share_creator<u8> m_nvram;

	// gate array interrupt aggregator (source 5 = PCM and sub CPU UART 3)
	u8 m_ga_int_enable = 0;
	u8 m_ga_int_trigger = 0;
	u8 m_ga_int_state = 0;
	u8 m_lcd_control = 0;

	// sub CPU shared RAM and mailbox, stubbed until the M37409 is emulated
	u8 m_ipc_ram[0xc0] = { };
	u8 m_ipc_semaphore = 0;
};


void sc55mk2_state::machine_start()
{
	save_item(NAME(m_ga_int_enable));
	save_item(NAME(m_ga_int_trigger));
	save_item(NAME(m_ga_int_state));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_ipc_ram));
	save_item(NAME(m_ipc_semaphore));
}

void sc55mk2_state::machine_reset()
{
	m_ga_int_enable = 0;
	m_ga_int_trigger = 0;
	m_ga_int_state = 0;
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
		// bit 0 low enables the LCD
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

void sc55mk2_state::ga_int_w(int source, int state)
{
	const u8 mask = 1 << source;
	if (state && !(m_ga_int_state & mask) && BIT(m_ga_int_enable, source - 1) && m_ga_int_trigger == 0)
	{
		m_ga_int_trigger = source;
		m_maincpu->set_input_line(1, ASSERT_LINE);
	}
	if (state)
		m_ga_int_state |= mask;
	else
		m_ga_int_state &= ~mask;
}

void sc55mk2_state::pcm_int_w(int state)
{
	ga_int_w(5, state);
}


//-------------------------------------------------
//  sub CPU interface (stub)
//-------------------------------------------------

u8 sc55mk2_state::ipc_r(offs_t offset)
{
	if (offset < 0xc0)
		return m_ipc_ram[offset];
	switch (offset)
	{
	case 0xff:
		return m_ipc_semaphore;
	default:
		return 0;
	}
}

void sc55mk2_state::ipc_w(offs_t offset, u8 data)
{
	if (offset < 0xc0)
		m_ipc_ram[offset] = data;
	else if (offset == 0xff)
		m_ipc_semaphore = (m_ipc_semaphore & ~0x1f) | (data & 0x1f);
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
	map(0x0ec00, 0x0ecff).rw(FUNC(sc55mk2_state::ipc_r), FUNC(sc55mk2_state::ipc_w));
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
}

HD44780_PIXEL_UPDATE(sc55mk2_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 40)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

static INPUT_PORTS_START( sc55mk2 )
INPUT_PORTS_END

void sc55mk2_state::sc55mk2(machine_config &config)
{
	HD6435328(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc55mk2_state::sc55mk2_map);
	m_maincpu->read_port9().set_constant(0x02); // bit 1: 1 = SC-55mkII, 0 = SC-155mkII
	m_maincpu->write_sci_tx<0>().set("mdout", FUNC(midi_port_device::write_txd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM20256 (IC28) + CR2032 battery

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	ROLAND_GP4(config, m_pcm, 24_MHz_XTAL);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->int_callback().set(FUNC(sc55mk2_state::pcm_int_w));

	SPEAKER(config, "speaker", 2).front();
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(80);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(sc55mk2_state::lcd_palette), 2);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 40);
	m_lcd->set_pixel_update_cb(FUNC(sc55mk2_state::lcd_pixel_update));

	m_screen->set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	m_screen->set_size(6 * 40, 8 * 2);
	m_screen->set_visarea_full();
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
