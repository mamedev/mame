// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Roland JV-80, JV-880 and related synthesizers.

    JV-880: H8/532 main CPU, GP-4 PCM chip (TC6116, also the interrupt
    aggregator and the panel strobe/LCD glue), HD44780 LCD, 2 x 2 MB wave
    ROMs, 32K battery-backed RAM for the user memory.  The front panel is
    a 3 x 5 switch matrix strobed from the gate array and read on port 7;
    the DATA knob is an encoder delivering direction-separated pulses to
    two interrupt sources.  TONE SWITCH 4 doubles as ENTER.

    The firmware trusts a blank NVRAM (after a transient "Internal RAM
    read error"), leaving the internal performances empty and MIDI silent,
    so the default NVRAM image holds the user memory after UTILITY ->
    Factory preset.

    TODO: panel LEDs, PCM/DATA card slots, expansion board.
    RD-500: keyboard, panel, sub board.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"
#include "cpu/h8500/h8532.h"
#include "machine/nvram.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "sound/roland_gp.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

// The wave ROMs are stored as dumped; the board wiring permutes address lines
// within each 1 MB block and the data lines.
void descramble_waverom(memory_region &region)
{
	u8 *rom = region.base();
	const u32 size = region.bytes();

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

class roland_jv80_state : public driver_device
{
public:
	roland_jv80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_lcd(*this, "lcd")
		, m_pcm(*this, "pcm")
		, m_keys(*this, "KEY%u", 0U)
		, m_dial(*this, "DIAL")
	{
	}

	void jv880(machine_config &config);
	void init_jv880();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void jv880_mem_map(address_map &map) ATTR_COLD;

	void jv_palette(palette_device &palette) const;

	u8 ga_r(offs_t offset);
	void ga_w(offs_t offset, u8 data);
	void ga_int(int source);
	void pcm_int_w(int state);
	u8 port7_r();
	TIMER_CALLBACK_MEMBER(dial_poll);

	required_device<h8532_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<hd44780_device> m_lcd;
	required_device<roland_gp4_device> m_pcm;
	required_ioport_array<3> m_keys;
	required_ioport m_dial;

	// gate array interrupt aggregator (sources: 3/4 = encoder, 5 = PCM)
	u8 m_ga_int_enable = 0;
	u8 m_ga_int_trigger = 0;
	u8 m_panel_sel = 0;
	int m_pcm_int_state = 0;
	u16 m_dial_last = 0;
	emu_timer *m_dial_timer = nullptr;
};

void roland_jv80_state::init_jv880()
{
	descramble_waverom(*memregion("waverom"));
}

void roland_jv80_state::machine_start()
{
	m_dial_timer = timer_alloc(FUNC(roland_jv80_state::dial_poll), this);
	m_dial_timer->adjust(attotime::from_msec(2), 0, attotime::from_msec(2));

	save_item(NAME(m_ga_int_enable));
	save_item(NAME(m_ga_int_trigger));
	save_item(NAME(m_panel_sel));
	save_item(NAME(m_pcm_int_state));
	save_item(NAME(m_dial_last));
}

u8 roland_jv80_state::ga_r(offs_t offset)
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
			m_maincpu->set_input_line(0, CLEAR_LINE);
		}
		return trigger;
	}

	default:
		return 0;
	}
}

void roland_jv80_state::ga_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 1:
		// panel scan strobe (walking zero); bit 0 doubles as an LCD control
		m_panel_sel = data;
		break;

	case 2:
		// interrupt enable mask, bit n enables source n+1; writing also
		// acknowledges the pending interrupt
		m_ga_int_enable = data;
		m_ga_int_trigger = 0;
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;

	default:
		break;
	}
}

void roland_jv80_state::ga_int(int source)
{
	if (BIT(m_ga_int_enable, source - 1) && m_ga_int_trigger == 0)
	{
		m_ga_int_trigger = source;
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

void roland_jv80_state::pcm_int_w(int state)
{
	// the PCM interrupt is gate array source 5
	if (state && !m_pcm_int_state)
		ga_int(5);
	m_pcm_int_state = state;
}

// switch matrix: the strobe register's bits 2-4 select a row, the columns
// come back on P7-0..P7-4 (pulled up); the push switches of the DATA and
// VOLUME knobs form a fifth column
u8 roland_jv80_state::port7_r()
{
	u8 result = 0xff;
	for (int row = 0; row < 3; row++)
		if (!BIT(m_panel_sel, 2 + row))
			result &= m_keys[row]->read();
	return result;
}

// the DATA knob delivers direction-separated pulses to gate array sources
// 3 and 4; the firmware just counts them
TIMER_CALLBACK_MEMBER(roland_jv80_state::dial_poll)
{
	const u16 value = m_dial->read();
	const s8 delta = s8(u8(value - m_dial_last));
	m_dial_last = value;
	if (delta > 0)
		ga_int(4);
	else if (delta < 0)
		ga_int(3);
}


void roland_jv80_state::jv880_mem_map(address_map &map)
{
	map(0x0'0000, 0x0'7fff).rom().region("maincpu", 0);
	map(0x0'8000, 0x0'dfff).ram().mirror(0xa0000);
	map(0x0'f000, 0x0'f3ff).rw(m_pcm, FUNC(roland_gp4_device::read), FUNC(roland_gp4_device::write));
	map(0x0'f400, 0x0'f403).rw(FUNC(roland_jv80_state::ga_r), FUNC(roland_jv80_state::ga_w));
	map(0x0'f404, 0x0'f405).rw(m_lcd, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x0'f406, 0x0'f407).nopw(); // unknown gate array registers
	map(0x1'0000, 0x3'ffff).rom().region("progrom", 0x10000);
	map(0x4'0000, 0x4'ffff).rom().region("progrom", 0);
	map(0xa'0000, 0xb'ffff).ram();
	map(0xc'0000, 0xc'7fff).mirror(0x10000).ram().share("nvram");
	map(0xe'0000, 0xf'7fff).lr8(NAME([]() { return u8(0xff); })); // DATA card slot (pulled up when empty)
}

void roland_jv80_state::jv_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(88, 247, 0)); // bright green
	palette.set_pen_color(1, rgb_t(3, 3, 60));   // dark blue
}

HD44780_PIXEL_UPDATE(roland_jv80_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 24)
	{
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
	}
}

static INPUT_PORTS_START(jv880)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor <")     PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor >")     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Select")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone 1 / Mute")       PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DATA (push)")  PORT_CODE(KEYCODE_D)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone 2 / Monitor")    PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone 3 / Info / Compare") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone 4 / Enter")      PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Utility")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Preview (push VOLUME knob)") PORT_CODE(KEYCODE_V)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Perform/Patch") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Edit")         PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("System")       PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Rhythm")       PORT_CODE(KEYCODE_R)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DIAL")
	PORT_BIT(0x00ff, 0x0000, IPT_DIAL) PORT_NAME("DATA") PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)
INPUT_PORTS_END

void roland_jv80_state::jv880(machine_config &config)
{
	HD6435328(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_jv80_state::jv880_mem_map);
	m_maincpu->write_sci_tx<0>().set("mdout", FUNC(midi_port_device::write_txd));
	m_maincpu->read_port7().set(FUNC(roland_jv80_state::port7_r));
	m_maincpu->read_adc<0>().set_constant(0x3ff); // +15 V rail monitor (divider, saturates)
	m_maincpu->read_adc<1>().set_constant(0x266); // backup battery, 3 V

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(h8532_device::sci_rx_w<0>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // LC36256AML-10 (IC18) + CR2032 battery

	SPEAKER(config, "speaker", 2).front();
	ROLAND_GP4(config, m_pcm, 23.2_MHz_XTAL);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->int_callback().set(FUNC(roland_jv80_state::pcm_int_w));
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(80);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_jv80_state::jv_palette), 2);

	HD44780(config, m_lcd, 4.9152_MHz_XTAL / 4); // HD44780
	m_lcd->set_lcd_size(2, 24);
	m_lcd->set_pixel_update_cb(FUNC(roland_jv80_state::lcd_pixel_update));

	m_screen->set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	m_screen->set_size(6 * 24, 8 * 2);
	m_screen->set_visarea_full();
}

class roland_rd500_state : public driver_device
{
public:
	roland_rd500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void rd500(machine_config &config);
	void init_rd500();

private:
	void rd500_mem_map(address_map &map) ATTR_COLD;

	u8 keyscan_r(offs_t offset);
	void keyscan_w(offs_t offset, u8 data);

	required_device<h8510_device> m_maincpu;
	required_device<roland_gp4_device> m_pcm;
};

void roland_rd500_state::init_rd500()
{
	descramble_waverom(*memregion("waverom"));
}

void roland_rd500_state::rd500_mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("progrom", 0);
	map(0x900000, 0x90ffff).ram();
	map(0xa00000, 0xa0ffff).rw(m_pcm, FUNC(roland_gp4_device::read), FUNC(roland_gp4_device::write));
	map(0xc00000, 0xc0ffff).rw(FUNC(roland_rd500_state::keyscan_r), FUNC(roland_rd500_state::keyscan_w));
}

u8 roland_rd500_state::keyscan_r(offs_t offset)
{
	return 0;
}

void roland_rd500_state::keyscan_w(offs_t offset, u8 data)
{
}

static INPUT_PORTS_START(rd500)
INPUT_PORTS_END

void roland_rd500_state::rd500(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_rd500_state::rd500_mem_map);
	m_maincpu->write_sci_tx<0>().set("mdout", FUNC(midi_port_device::write_txd));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(h8510_device::sci_rx_w<0>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	SPEAKER(config, "speaker", 2).front();
	ROLAND_GP4(config, m_pcm, 23.2_MHz_XTAL);
	m_pcm->set_device_rom_tag("waverom");
	m_pcm->add_route(0, "speaker", 1.0, 0);
	m_pcm->add_route(1, "speaker", 1.0, 1);
}

ROM_START(jv880)
	ROM_DEFAULT_BIOS("v1.0.1")
	ROM_SYSTEM_BIOS(0, "v1.0.0", "ROM Version 1.0.0")
	ROM_SYSTEM_BIOS(1, "v1.0.1", "ROM Version 1.0.1")

	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("roland_r15199810_6435328b86f.ic16", 0x0000, 0x8000, CRC(b3d611b1) SHA1(6c1b59905b361ac3c4803d39589406bc3e1d0647))

	ROM_REGION(0x40000, "progrom", 0)
	ROMX_LOAD("roland_jv-880_v1.00.ic17", 0x00000, 0x40000, CRC(b266efe9) SHA1(282116e8e8471053cf159d22675931592b7f7c8f), ROM_BIOS(0))
	ROMX_LOAD("roland_jv-880_v1.01.ic17", 0x00000, 0x40000, CRC(5f19c95f) SHA1(38ec496f16dfa02d35f934cf32d8302aaf5f236e), ROM_BIOS(1))

	ROM_REGION(0x8000, "nvram", 0) // user memory after "Factory preset", generated in emulation
	ROM_LOAD("jv880_nvram.bin", 0x0000, 0x8000, CRC(9795ef6c) SHA1(0d5902f15287f7a98f62fb69a310c5a65c115b67))

	ROM_REGION(0x400000, "waverom", 0)
	ROM_LOAD("roland-a_r15209312_lh5375n2.ic27", 0x000000, 0x200000, CRC(1348c0dc) SHA1(37e28498351fb502f6d43398d288a026c02b446d))
	ROM_LOAD("roland-b_r15209313_lh5375n3.ic25", 0x200000, 0x200000, CRC(d55fcf90) SHA1(963ce75b6668dab377d3a2fd895630a745491be5))
ROM_END

ROM_START(rd500)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_LOAD16_WORD_SWAP("rd500_rom.bin", 0x00000, 0x80000, CRC(668fc7e9) SHA1(59e28d3e2190902dd6fd02a9820f96e383781178))

	ROM_REGION(0x800000, "waverom", 0)
	ROM_LOAD("roland-a_r00342978.ic4", 0x000000, 0x200000, CRC(c885bf4f) SHA1(e14f0f4a8181e09fae7db10130e4ed3cd6bf5a34))
	ROM_LOAD("roland-b_r00343012.ic5", 0x200000, 0x200000, CRC(ceb02d33) SHA1(9f6969d94598c68902188085d0c91fb8b300d762))
	ROM_LOAD("roland-c_r00343023.ic6", 0x400000, 0x200000, CRC(f627cdb7) SHA1(7b834fee5db5a7377ec7f66172d0fa3096cefbc9))
	ROM_LOAD("roland-d_r00343034.ic7", 0x600000, 0x200000, CRC(c06be973) SHA1(2e7ce8a91a6f92648f73d7ff8c1d608f62df9aab))
ROM_END

} // anonymous namespace


//SYST(1992, jv80, 0, 0, jv80, jv80, roland_jv80_state, empty_init, "Roland", "JV-80 Multi Timbral Synthesizer", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1992, jv880, 0, 0, jv880, jv880, roland_jv80_state, init_jv880, "Roland", "JV-880 Multi Timbral Synthesizer Module", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
SYST(1994, rd500, 0, 0, rd500, rd500, roland_rd500_state, init_rd500, "Roland", "RD-500 Digital Piano", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
