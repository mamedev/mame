// license:BSD-3-Clause
// copyright-holders:AJR, superctr
/****************************************************************************

    Roland SC-88 family of MIDI sound generators.

    The SC-88 is the successor of the SC-55mkII: two 16-part banks with the
    SC-55 and a new native tone set, a second MIDI IN, an equalizer and an
    editable user instrument set, driven by the "XP" PCM chip.

    Main board (service notes, June 1994):
    - IC29 H8/510 (HD6415108) at 20 MHz, mode 4 (16-bit external bus,
      A20/A21 unused); both on-chip SCIs are unused, all serial traffic goes
      through the sub CPU
    - IC27 uPD65622 gate array: chip select decoder, LCD interface, front
      panel LEDs, interrupt and wait control
    - IC11 M38881M2 sub CPU (Roland R00232667, 8 KB ROM, undumped): scans the
      panel switch matrix, merges MIDI IN A/B and the RS-422/RS-232 computer
      port, talks to the main CPU through a shared window on /CS5
    - IC15 MBCS30109 "XP" PCM chip at 24.576 MHz on /CS3, four 2 MB wave
      ROMs, 2 x 256K x 4 effect DRAM, PCM69AU DAC
    - RCM2024T LCD unit: the SC-55mkII glass on an HD44780 command set
    - IC17 512 KB program ROM on /CS0, 2 x 32 KB battery backed SRAM on /CS1

    Interrupts: IRQ0 gate array, IRQ1 XP, IRQ2 sub CPU.  The analog inputs
    read the backup battery (AN0) and the rear COMPUTER selector (AN1-AN3).

    SC-88VL: compact (1U half-rack) version of SC-88 with standby function

    SC-88Pro: the SC-88 with expanded sample ROM and external effect processor
    (LSP) for insertion effects. Address decoding is handled by the LSP rather
    than the gate array.

    VE-GSPro: the SC-88Pro tone generator as a voice expansion board for the
    MC-80 and A-70/A-90. Does not have the sub MCU, rather receives MIDI
    directly using the H8/510 SCIs.

    SK-88Pro: 37-key keyboard with a built in SC-88Pro compatible tone
    generator. Contains a 4M overlay ROM.

****************************************************************************/

#include "emu.h"

#include "sc88_sub.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8500/h8510.h"
#include "machine/nvram.h"
#include "sound/roland_lsp.h"
#include "sound/roland_xp.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "roland_sc88.lh"
#include "roland_sc88pro.lh"

#define LOG_GA      (1U << 1)
#define LOG_PORT    (1U << 2)
#define LOG_SUB     (1U << 3)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class roland_sc88_state : public driver_device
{
public:
	roland_sc88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_xp(*this, "xp")
		, m_lsp(*this, "lsp")
		, m_nvram(*this, "nvram")
		, m_screen(*this, "screen")
		, m_lcd(*this, "lcd")
		, m_keys(*this, "KEY%u", 0U)
		, m_computer_sw(*this, "COMPUTER")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void sc88(machine_config &config);
	void sc88vl(machine_config &config);
	void sc88pro(machine_config &config);
	void vegspro(machine_config &config);

	void init_sc88() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void lcd_palette(palette_device &palette) const;

	void main_map(address_map &map) ATTR_COLD;
	void sc88pro_map(address_map &map) ATTR_COLD;
	void vegspro_map(address_map &map) ATTR_COLD;
	void xp_rom_map(address_map &map) ATTR_COLD;
	void pro_xp_rom_map(address_map &map) ATTR_COLD;

	u8 ga_r(offs_t offset);
	void ga_w(offs_t offset, u8 data);
	void ga_int_update();
	TIMER_CALLBACK_MEMBER(lcd_done);
	// the upper half of the battery RAM is also selected in pages 0 and E
	u16 ram_alias_r(offs_t offset) { return m_nvram[0x4000 + offset]; }
	void ram_alias_w(offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_nvram[0x4000 + offset]); }
	void xp_int_w(int state);
	void lsp_serial_w(offs_t channel, u32 data);
	u32 lsp_serial_r(offs_t channel);
	void lsp_mute_w(u8 data);
	u8 port4_r();
	void port4_w(u8 data);
	u8 port8_r();
	u16 battery_r();
	u16 computer_sw_r();

	required_device<h8510_device> m_maincpu;
	optional_device<sc88_sub_device> m_subcpu;
	required_device<roland_xp_device> m_xp;
	optional_device<roland_lsp_device> m_lsp;
	required_shared_ptr<u16> m_nvram;
	optional_device<screen_device> m_screen;
	optional_device<hd44780_device> m_lcd;
	optional_ioport_array<4> m_keys;
	optional_ioport m_computer_sw;
	output_finder<9> m_leds;

	u8 m_ga_regs[0x100]{};
	u8 m_ga_int_mask = 0;
	u8 m_ga_int_pending = 0;
	u8 m_lcd_fifo[13]{};
	u8 m_lcd_fifo_count = 0;
	bool m_lcd_command_pending = false;
	emu_timer *m_lcd_timer = nullptr;
	bool m_xp_int = false;
	bool m_lsp_mute = true;
};


// The wave ROMs are stored as dumped; the board wiring permutes address lines
// within each 256 KB block and the data lines.
void roland_sc88_state::init_sc88()
{
	memory_region *region = memregion("waverom");
	u8 *rom = region->base();
	const u32 size = region->bytes();

	static const u8 address_lines[18] = { 0, 4, 2, 3, 1, 13, 7, 12, 5, 10, 16, 9, 6, 8, 14, 17, 11, 15 };
	static const u8 data_lines[8] = { 2, 0, 4, 5, 7, 6, 3, 1 };

	std::vector<u8> scrambled(rom, rom + size);
	for (u32 i = 0; i < size; i++)
	{
		u32 address = i & ~0x3ffff;
		for (int bit = 0; bit < 18; bit++)
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

void roland_sc88_state::machine_start()
{
	m_lcd_timer = timer_alloc(FUNC(roland_sc88_state::lcd_done), this);

	save_item(NAME(m_ga_regs));
	save_item(NAME(m_ga_int_mask));
	save_item(NAME(m_ga_int_pending));
	save_item(NAME(m_lcd_fifo));
	save_item(NAME(m_lcd_fifo_count));
	save_item(NAME(m_lcd_command_pending));
	save_item(NAME(m_xp_int));
	save_item(NAME(m_lsp_mute));
}

void roland_sc88_state::machine_reset()
{
	std::fill(std::begin(m_ga_regs), std::end(m_ga_regs), 0);
	m_ga_int_mask = 0xff;
	m_ga_int_pending = 0;
	m_lcd_fifo_count = 0;
	m_lcd_command_pending = false;
	m_lcd_timer->adjust(attotime::never);

}


//-------------------------------------------------
//  gate array: LEDs, interrupt aggregator on IRQ0 and the LCD interface.
//  The LCD is written through a command register and a data FIFO; the
//  strobe transfers both and raises interrupt source 0 when done.
//
//    00     LEDs                 04     1 + pending interrupt source (read)
//    01-02  ?                    05     interrupt mask, bit n masks source n
//    03/07  LCD setup            1e     LCD strobe    1f  LCD command
//                                20-2c  LCD data FIFO
//-------------------------------------------------

void roland_sc88_state::ga_int_update()
{
	m_maincpu->set_input_line(0, (m_ga_int_pending & ~m_ga_int_mask) ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(roland_sc88_state::lcd_done)
{
	m_ga_int_pending |= 1;
	ga_int_update();
}

u8 roland_sc88_state::ga_r(offs_t offset)
{
	u8 data = m_ga_regs[offset];
	switch (offset)
	{
	case 0x04:
	{
		// 1 + number of the lowest pending source, 0 when none; reading acknowledges it
		const u8 active = m_ga_int_pending & ~m_ga_int_mask;
		data = 0;
		for (int source = 0; source < 8 && !data; source++)
			if (BIT(active, source))
			{
				data = source + 1;
				if (!machine().side_effects_disabled())
				{
					m_ga_int_pending &= ~(1 << source);
					ga_int_update();
				}
			}
		return data;
	}
	default:
		break;
	}
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_GA, "%s: gate array read %02x = %02x\n", machine().describe_context(), offset, data);
	return data;
}

void roland_sc88_state::ga_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_GA, "%s: ga w %02x = %02x\n", machine().describe_context(), offset, data);
	m_ga_regs[offset] = data;
	switch (offset)
	{
	case 0x00:
	case 0x01:
	{
		// 00 = LED data (ALL, MUTE, INST MAP, EQ, EDIT (upper, middle, lower),
		// USER INST), gated by the common in 01 bit 0 (active low).  01 bit 1
		// drives the SC-88Pro's second lens die directly (red; the green one is
		// on the USER INST data line, and both lit show orange in EFX mode).
		const u8 data = m_ga_regs[0x00];
		const u8 commons = m_ga_regs[0x01];
		for (int i = 0; i < 8; i++)
			m_leds[i] = BIT(data, i) && !BIT(commons, 0);
		m_leds[8] = BIT(commons, 1);
		break;
	}
	case 0x05:
		m_ga_int_mask = data;
		ga_int_update();
		break;
	case 0x1e:
	{
		int bytes = m_lcd_fifo_count;
		if (m_lcd_command_pending)
		{
			m_lcd->write(0, m_ga_regs[0x1f]);
			bytes++;
		}
		for (int i = 0; i < m_lcd_fifo_count; i++)
			m_lcd->write(1, m_lcd_fifo[i]);
		m_lcd_command_pending = false;
		m_lcd_fifo_count = 0;
		m_lcd_timer->adjust(attotime::from_usec(40 * (bytes + 1)));
		break;
	}
	case 0x1f:
		m_lcd_command_pending = true;
		break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25:
	case 0x26: case 0x27: case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c:
		if (m_lcd_fifo_count < 13)
			m_lcd_fifo[m_lcd_fifo_count++] = data;
		break;
	default:
		LOGMASKED(LOG_GA, "%s: gate array write %02x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}


//-------------------------------------------------
//  XP PCM chip on IRQ1; the interrupt handler polls the pin through port 8
//-------------------------------------------------

void roland_sc88_state::xp_int_w(int state)
{
	m_xp_int = state;
	m_maincpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  the XP clocks the LSP: SDOB carries the EFX send to TRR, TRS0 comes back on SDIA5
//-------------------------------------------------

void roland_sc88_state::lsp_serial_w(offs_t channel, u32 data)
{
	// the XP presents its word one bit up; the return comes back nine bits down and inverted. A linear
	// effect only ever sees the product of the two, so it took a clipping one to tell the split apart
	m_lsp->ser_w(channel, s32(data));
	if (channel)
		m_lsp->run_once();
}

// the pair comes back a half-frame rotated
u32 roland_sc88_state::lsp_serial_r(offs_t channel)
{
	return m_lsp_mute ? 0 : u32(-(m_lsp->ser_r(channel ^ 1) >> 9));
}

// P3-7 is LSPMUTE, the second input of the gate the return passes through: the firmware holds it low
// while it uploads a program, and for ten seconds over the boot.
void roland_sc88_state::lsp_mute_w(u8 data)
{
	m_lsp_mute = !BIT(data, 7);
}

//-------------------------------------------------
//  port 4: the LCD contrast ladder and the audio mute
//-------------------------------------------------

// bit 4-7: LCD contrast
// bit 3: timer output
// bit 2: analog mute
// bit 1: timer clock in
// bit 0: sub-CPU reset (88VL only)
u8 roland_sc88_state::port4_r()
{
	return 0xff;
}

void roland_sc88_state::port4_w(u8 data)
{
	LOGMASKED(LOG_PORT, "%s: port 4 = %02x (contrast %2d, mute %s)\n", machine().describe_context(),
			data, ~data >> 4 & 0x0f, BIT(data, 2) ? "off" : "on");
	if (m_subcpu.found())
		m_subcpu->reset_w(BIT(data, 0));
}

u8 roland_sc88_state::port8_r()
{
	return m_xp_int ? 0xfd : 0xff;
}


//-------------------------------------------------
//  analog inputs
//-------------------------------------------------

u16 roland_sc88_state::battery_r()
{
	return 0x2a0;
}

// the rear selector is a resistor ladder into three inputs; until the
// thresholds are known each position pulls one input high
u16 roland_sc88_state::computer_sw_r()
{
	return m_computer_sw.read_safe(0) ? 0x3ff : 0;
}


//-------------------------------------------------
//  memory maps
//-------------------------------------------------

void roland_sc88_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("progrom", 0);
	map(0x008000, 0x00ffff).rw(FUNC(roland_sc88_state::ram_alias_r), FUNC(roland_sc88_state::ram_alias_w));
	map(0x080000, 0x08ffff).ram().share(m_nvram);
	map(0x0e8000, 0x0effff).rw(FUNC(roland_sc88_state::ram_alias_r), FUNC(roland_sc88_state::ram_alias_w));
	map(0x0e0000, 0x0e3fff).rw(m_xp, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0x0f0000, 0x0f00ff).rw(m_subcpu, FUNC(sc88_sub_device::read), FUNC(sc88_sub_device::write));
	map(0x0fc100, 0x0fc1ff).rw(FUNC(roland_sc88_state::ga_r), FUNC(roland_sc88_state::ga_w));
}

void roland_sc88_state::sc88pro_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("progrom", 0);
	map(0xc00000, 0xc0ffff).ram().share(m_nvram);
	map(0xc80000, 0xc83fff).rw(m_xp, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0xe00000, 0xe000ff).rw(m_subcpu, FUNC(sc88_sub_device::read), FUNC(sc88_sub_device::write));
	map(0xefc100, 0xefc1ff).rw(FUNC(roland_sc88_state::ga_r), FUNC(roland_sc88_state::ga_w));
	map(0xf00000, 0xf000ff).rw(m_lsp, FUNC(roland_lsp_device::host_r), FUNC(roland_lsp_device::host_w));
}

void roland_sc88_state::vegspro_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("progrom", 0);
	map(0xc00000, 0xc0ffff).ram().share(m_nvram);
	map(0xc80000, 0xc83fff).rw(m_xp, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0xefc100, 0xefc1ff).rw(FUNC(roland_sc88_state::ga_r), FUNC(roland_sc88_state::ga_w));
	map(0xf00000, 0xf000ff).rw(m_lsp, FUNC(roland_lsp_device::host_r), FUNC(roland_lsp_device::host_w));
}

// the XP's region number: bits 6-4 select the ROM chip, bits 3-0 the 1 MB page in it
void roland_sc88_state::xp_rom_map(address_map &map)
{
	map(0x0000000, 0x01fffff).rom().region("waverom", 0x000000);
	map(0x1000000, 0x11fffff).rom().region("waverom", 0x200000);
	map(0x2000000, 0x21fffff).rom().region("waverom", 0x400000);
	map(0x3000000, 0x31fffff).rom().region("waverom", 0x600000);
}

// five 32 Mbit ROMs, XWCS0-4 = IC20-IC24 (service notes, main circuit diagram)
void roland_sc88_state::pro_xp_rom_map(address_map &map)
{
	map(0x0000000, 0x03fffff).rom().region("waverom", 0x000000);
	map(0x1000000, 0x13fffff).rom().region("waverom", 0x400000);
	map(0x2000000, 0x23fffff).rom().region("waverom", 0x800000);
	map(0x3000000, 0x33fffff).rom().region("waverom", 0xc00000);
	map(0x4000000, 0x43fffff).rom().region("waverom", 0x1000000);
}


//-------------------------------------------------
//  LCD: the RCM2024T glass of the SC-55mkII, see roland_sc55mk2.cpp
//-------------------------------------------------

void roland_sc88_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xf8, 0xc8, 0x40)); // background
	palette.set_pen_color(1, rgb_t(0x20, 0x10, 0x00)); // segment on
}

static constexpr int LCD_CHAR_PITCH = 35;
static constexpr int LCD_DOT_PITCH = 6;
static constexpr int LCD_DOT_SIZE = 5;
static constexpr int LCD_COL0_X = 24;
static constexpr int LCD_COL1_X = 143;
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

HD44780_PIXEL_UPDATE(roland_sc88_state::lcd_pixel_update)
{
	if (pos >= 20 && pos < 24 && line < 2)
	{
		int const bar = (pos - 20) * 5 + x;
		if (bar < 16)
			lcd_fill(bitmap, LCD_BAR_X + bar * LCD_BAR_PITCH_X, LCD_BAR_Y + (line * 8 + y) * LCD_BAR_PITCH_Y, LCD_BAR_W, LCD_BAR_H, state);
		return;
	}

	if (line == 1 && pos == 18)
	{
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

// front panel switch matrix: SSC0-SSC3 strobes select the columns, SD0-SD7 are
// the rows (switch board, service notes p.15); PREVIEW is the volume knob's
// push switch on the VR board, on SSC0/SD7.
static INPUT_PORTS_START(sc88)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("EQ")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SC-55 Map")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument <")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument >")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Mute")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("All")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Preview")

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

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("User Inst Edit On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("User Inst Edit Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Rate / Attack / Delay <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Rate / Attack / Delay >")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Depth / Cutoff / Decay <")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Depth / Cutoff / Decay >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Delay / Resonance / Release <")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Delay / Resonance / Release >")

	// rear panel selector, read through the analog inputs as a resistor ladder
	PORT_START("COMPUTER")
	PORT_CONFNAME(0x03, 0x00, "Computer Switch")
	PORT_CONFSETTING(0x00, "MIDI")
	PORT_CONFSETTING(0x01, "PC-1")
	PORT_CONFSETTING(0x02, "PC-2")
	PORT_CONFSETTING(0x03, "Mac")
INPUT_PORTS_END

// same matrix positions as the SC-88; SC-88 MAP replaces EQ, and the lower
// row doubles as the EFX controls
static INPUT_PORTS_START(sc88pro)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SC-88 Map")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SC-55 Map")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument <")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Instrument >")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Mute")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("All")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Preview")

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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Shift / Delay <")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Shift / Delay >")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reverb <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reverb >")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Level <")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Level >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Part <")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("User Inst / EFX")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Select / EFX On/Off")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Rate / Attack / EFX Type <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Rate / Attack / EFX Type >")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Depth / Cutoff / Decay / EFX Param <")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Depth / Cutoff / Decay / EFX Param >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Delay / Resonance / Release / EFX Value <")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vib Delay / Resonance / Release / EFX Value >")

	// rear panel selector, read through the analog inputs as a resistor ladder
	PORT_START("COMPUTER")
	PORT_CONFNAME(0x03, 0x00, "Computer Switch")
	PORT_CONFSETTING(0x00, "MIDI")
	PORT_CONFSETTING(0x01, "PC-1")
	PORT_CONFSETTING(0x02, "PC-2")
	PORT_CONFSETTING(0x03, "Mac")
INPUT_PORTS_END

void roland_sc88_state::sc88(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::main_map);
	m_maincpu->read_adc<0>().set(FUNC(roland_sc88_state::battery_r));
	m_maincpu->read_adc<1>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_adc<2>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_adc<3>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_port4().set(FUNC(roland_sc88_state::port4_r));
	m_maincpu->write_port4().set(FUNC(roland_sc88_state::port4_w));
	m_maincpu->read_port8().set(FUNC(roland_sc88_state::port8_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2 x SRM2A256 + battery

	SC88_SUB(config, m_subcpu, 20_MHz_XTAL / 2); // M38881M2, clocked from the H8's phi output
	m_subcpu->int_callback().set_inputline(m_maincpu, 2); // IRQ2
	m_subcpu->keys_callback<0>().set_ioport("KEY0");
	m_subcpu->keys_callback<1>().set_ioport("KEY1");
	m_subcpu->keys_callback<2>().set_ioport("KEY2");
	m_subcpu->keys_callback<3>().set_ioport("KEY3");
	m_subcpu->tx_callback().set("mdout", FUNC(midi_port_device::write_txd));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_subcpu, FUNC(sc88_sub_device::rxd_w<0>));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin2", midiin_slot, "midiin"));
	mdin2.rxd_handler().set(m_subcpu, FUNC(sc88_sub_device::rxd_w<1>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(80);
	m_screen->set_screen_update("lcd", FUNC(hd44780_device::screen_update));
	m_screen->set_size(720, 272);
	m_screen->set_visarea_full();
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_sc88_state::lcd_palette), 2);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 40);
	m_lcd->set_pixel_update_cb(FUNC(roland_sc88_state::lcd_pixel_update));

	config.set_default_layout(layout_roland_sc88);

	SPEAKER(config, "speaker", 2).front();

	ROLAND_XP(config, m_xp, 24.576_MHz_XTAL);
	m_xp->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc88_state::xp_rom_map);
	m_xp->int_callback().set(FUNC(roland_sc88_state::xp_int_w));
	m_xp->add_route(1, "speaker", 1.0, 0);
	m_xp->add_route(4, "speaker", 1.0, 1);
}

void roland_sc88_state::sc88vl(machine_config &config)
{
	sc88(config);
}

void roland_sc88_state::sc88pro(machine_config &config)
{
	sc88(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::sc88pro_map);
	m_xp->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc88_state::pro_xp_rom_map);

	// two DAC pairs: OUTPUT1 takes words 02/03, OUTPUT2 words 06/07
	SPEAKER(config, "output2", 2).front();
	m_xp->reset_routes();
	m_xp->add_route(2, "speaker", 1.0, 0);
	m_xp->add_route(3, "speaker", 1.0, 1);
	m_xp->add_route(6, "output2", 1.0, 0);
	m_xp->add_route(7, "output2", 1.0, 1);

	ROLAND_LSP(config, m_lsp, 24.576_MHz_XTAL);

	// the EFX send leaves in RAM words 01/05; TRS0 comes back into the DSP's own serial capture
	m_xp->set_serial_output_words(0x01, 0x05);
	m_xp->serial_out_callback().set(FUNC(roland_sc88_state::lsp_serial_w));
	m_xp->serial_in_callback().set(FUNC(roland_sc88_state::lsp_serial_r));
	m_maincpu->write_port3().set(FUNC(roland_sc88_state::lsp_mute_w));

	config.set_default_layout(layout_roland_sc88pro);
}

void roland_sc88_state::vegspro(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::vegspro_map);
	m_maincpu->read_adc<0>().set(FUNC(roland_sc88_state::battery_r));
	// the expansion board has no rear selector, but the SC-88Pro's firmware still polls the ladder
	m_maincpu->read_adc<1>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_adc<2>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_adc<3>().set(FUNC(roland_sc88_state::computer_sw_r));
	m_maincpu->read_port4().set(FUNC(roland_sc88_state::port4_r));
	m_maincpu->write_port4().set(FUNC(roland_sc88_state::port4_w));
	m_maincpu->read_port8().set(FUNC(roland_sc88_state::port8_r));
	m_maincpu->write_sci_tx<0>().set("mdout", FUNC(midi_port_device::write_txd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// MIDI IN A/B go straight to the H8's serial ports, MIDI OUT comes from SCI1
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(hd6415108_device::sci_rx_w<0>));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin2", midiin_slot, "midiin"));
	mdin2.rxd_handler().set(m_maincpu, FUNC(hd6415108_device::sci_rx_w<1>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "output2", 2).front();

	ROLAND_XP(config, m_xp, 24.576_MHz_XTAL);
	m_xp->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc88_state::pro_xp_rom_map);
	m_xp->int_callback().set(FUNC(roland_sc88_state::xp_int_w));
	m_xp->add_route(2, "speaker", 1.0, 0);
	m_xp->add_route(3, "speaker", 1.0, 1);
	m_xp->add_route(6, "output2", 1.0, 0);
	m_xp->add_route(7, "output2", 1.0, 1);

	ROLAND_LSP(config, m_lsp, 24.576_MHz_XTAL);

	// the EFX send leaves in RAM words 01/05; TRS0 comes back into the DSP's own serial capture
	m_xp->set_serial_output_words(0x01, 0x05);
	m_xp->serial_out_callback().set(FUNC(roland_sc88_state::lsp_serial_w));
	m_xp->serial_in_callback().set(FUNC(roland_sc88_state::lsp_serial_r));
	m_maincpu->write_port3().set(FUNC(roland_sc88_state::lsp_mute_w));
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))
#define ROM_LOAD16_WORD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

ROM_START(sc88)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_DEFAULT_BIOS("104")
	ROM_SYSTEM_BIOS(0, "102", "Control ROM 1.0.2")	// v1.02, 1994-06-16, 18:43 (offset 0x7cff6)
	ROM_LOAD16_WORD_SWAP_BIOS(0, "roland_sc88-control-1.02-hn27c4096h.ic17", 0x00000, 0x80000, CRC(9e9c56f9) SHA1(9291a4e4ac0fea9fb5a39777be501ab3d34877c8))
	ROM_SYSTEM_BIOS(1, "103", "Control ROM 1.0.3")	// v1.03, 1994-07-01, 10:48 (offset 0x7d136)
	ROM_LOAD16_WORD_SWAP_BIOS(1, "roland_sc88-control-1.03-hn27c4096hg-85.ic17", 0x00000, 0x80000, CRC(cf953f71) SHA1(6a25d46b608b317d2604b6424fea7b50978b5d32)) // HN27C4096HG-85
	ROM_SYSTEM_BIOS(2, "104", "Control ROM 1.0.4")	// v1.04, 1994-08-04, 12:41 (offset 0x7d1a6)
	// This ROM dump is circulating labelled as "Roland SC88 Version 1.01", but it really is 1.04.
	ROM_LOAD16_WORD_BIOS(2, "roland_sc88-control-1.04.ic17", 0x00000, 0x80000, CRC(979b6c09) SHA1(3bc9a68703bd09459283b6b45d01d08feaffb744)) // dumper comment: original chip was a 27C4096

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("subcpu.ic11", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x800000, "waverom", 0)
	ROM_LOAD("sc88-pcm-ic-325.ic14", 0x000000, 0x200000, CRC(f9dd9e49) SHA1(860dcd9804ded4cd46aab38bfc3764a1ad7a6b65))
	ROM_LOAD("sc88-pcm-ic-326.ic8",  0x200000, 0x200000, CRC(05f939f2) SHA1(ac95c26c46c40aacb944f5d45634c93bee9e6d90))
	ROM_LOAD("sc88-pcm-ic-327.ic7",  0x400000, 0x200000, CRC(a6fc7393) SHA1(d88bf13c3d74097991b783295d95ccfae2c9282d))
	ROM_LOAD("sc88-pcm-ic-328.ic6",  0x600000, 0x200000, CRC(7bc514aa) SHA1(03e70ae2efd190f41af24be01b1abaa84bfa93d9))
ROM_END

ROM_START(sc88vl)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_LOAD16_WORD_SWAP("roland_sc88_vl-1.04.ic29", 0x00000, 0x80000, CRC(66aa5762) SHA1(3a20f8f8cefd0d5e1edb103046f6fe94bb73ac7a))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("roland-r00232667-m38881m2-150gp.ic23", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x800000, "waverom", 0)
	// the VL's own mask ROMs (roland-r00785356-hn624316fbc25.ic10, roland-r00785367-hn624316fbc26.ic7,
	// roland-r00788489-hn624316fbc27.ic4, roland-r00788490-hn624316fbc28.ic2) are undumped;
	// the sample data should be identical to the SC-88's, so use the parent's images
	ROM_LOAD("sc88-pcm-ic-325.ic14", 0x000000, 0x200000, BAD_DUMP CRC(f9dd9e49) SHA1(860dcd9804ded4cd46aab38bfc3764a1ad7a6b65))
	ROM_LOAD("sc88-pcm-ic-326.ic8",  0x200000, 0x200000, BAD_DUMP CRC(05f939f2) SHA1(ac95c26c46c40aacb944f5d45634c93bee9e6d90))
	ROM_LOAD("sc88-pcm-ic-327.ic7",  0x400000, 0x200000, BAD_DUMP CRC(a6fc7393) SHA1(d88bf13c3d74097991b783295d95ccfae2c9282d))
	ROM_LOAD("sc88-pcm-ic-328.ic6",  0x600000, 0x200000, BAD_DUMP CRC(7bc514aa) SHA1(03e70ae2efd190f41af24be01b1abaa84bfa93d9))
ROM_END

ROM_START(sc88pro)
	ROM_REGION16_BE(0x100000, "progrom", 0)
	ROM_DEFAULT_BIOS("104")
	ROM_SYSTEM_BIOS(0, "102", "Control ROM 1.0.2")	// v1.02, 1996-09-17, 08:01 (offset 0xcf554)
	ROM_LOAD16_WORD_BIOS(0, "roland_sc88pro-1.02.ic26", 0x00000, 0x100000, CRC(7b0d392d) SHA1(19906eea7655bc105ef5077ce4fac10c21678225))
	ROM_SYSTEM_BIOS(1, "104", "Control ROM 1.0.4")	// v1.04, 1997-02-18, 12:11 (offset 0xcf45c)
	ROM_LOAD16_WORD_BIOS(1, "roland_sc88pro-1.04.ic26", 0x00000, 0x100000, CRC(820824d2) SHA1(6d06f00a1f8195a76b3af600ca708003f9a3abdc))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("subcpu.ic10", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x1400000, "waverom", 0)
	// Wave ROM reconstructed from vegspro (same sample data, wider chips)
	ROM_LOAD("roland-r01017834-378.ic20", 0x0000000, 0x400000, BAD_DUMP CRC(84dfea65) SHA1(1af429154193dfcda17d609fd73719750c133220))
	ROM_LOAD("roland-r01017845-379.ic21", 0x0400000, 0x400000, BAD_DUMP CRC(60210227) SHA1(5a453a1bd93d1ddee7cc809430768d4a08739d2b))
	ROM_LOAD("roland-r01124778-519.ic22", 0x0800000, 0x400000, BAD_DUMP CRC(5f883ddd) SHA1(78b17450d969c23fba5e9e3f8e4cbc4178096674))
	ROM_LOAD("roland-r01124789-520.ic23", 0x0c00000, 0x400000, BAD_DUMP CRC(ecb4dd39) SHA1(a6b29144b165248162bef8ba4cded49efa4f5b90))
	ROM_LOAD("roland-r01124790-521.ic24", 0x1000000, 0x400000, BAD_DUMP CRC(93541e95) SHA1(e2ad5f0b2e5bdad84e42d080fc2e2fad523cb84b))
ROM_END

/*
ROM_START(sk88pro)
	ROM_REGION16_BE(0x100000, "progrom", 0)
	ROM_LOAD("roland-r01348078.ic8", 0x00000, 0x100000, NO_DUMP)
	ROM_LOAD("roland_r01454223.ic7", 0x00000, 0x80000, NO_DUMP) // overlay

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("roland-r01235734-m38881m2-152gp.ic5", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x1400000, "waverom", 0)
	ROM_LOAD("roland-r01348723.ic13", 0x0000000, 0x800000, BAD_DUMP CRC(5754EE2E) SHA1(5cc1700b3f41921ed81fe3b92ba9ec28bd6649c9))
	ROM_LOAD("roland-r01348734.ic15", 0x0800000, 0x800000, BAD_DUMP CRC(E2B57861) SHA1(78b37ce0e735ad2c3e51338e74219a103d7fadba))
	ROM_LOAD("roland-r01233667-314.ic17", 0x1000000, 0x400000, CRC(93541E95) SHA1(e2ad5f0b2e5bdad84e42d080fc2e2fad523cb84b))
ROM_END
*/

ROM_START(vegspro)
	ROM_REGION16_BE(0x100000, "progrom", 0)
	// "SC88PRO (MK03B)" v1.02, 1997-05-19
	ROM_LOAD16_WORD_SWAP("roland-r01780078.bin", 0x00000, 0x100000, CRC(6cf8fc8b) SHA1(c07cbbd026781428ae83f4b897a51cb7bcdafeb6))

	ROM_REGION(0x1400000, "waverom", 0)
	// dumped as "R01567167 301 (Samples A).bin", "R01567178 302 (Samples B).BIN", "R01233667 314 (Samples C).BIN"
	ROM_LOAD("roland-r01567167-301.bin", 0x0000000, 0x800000, CRC(5754EE2E) SHA1(5cc1700b3f41921ed81fe3b92ba9ec28bd6649c9))
	ROM_LOAD("roland-r01567178-302.bin", 0x0800000, 0x800000, CRC(E2B57861) SHA1(78b37ce0e735ad2c3e51338e74219a103d7fadba))
	ROM_LOAD("roland-r01233667-314.bin", 0x1000000, 0x400000, CRC(93541E95) SHA1(e2ad5f0b2e5bdad84e42d080fc2e2fad523cb84b))
ROM_END

} // anonymous namespace


SYST(1994, sc88, 0, 0, sc88, sc88, roland_sc88_state, init_sc88, "Roland", "Sound Canvas SC-88", 0)
SYST(1995, sc88vl, sc88, 0, sc88vl, sc88, roland_sc88_state, init_sc88, "Roland", "Sound Canvas SC-88VL", MACHINE_NOT_WORKING) // needs its own panel layout and input assignments
SYST(1996, sc88pro, 0, 0, sc88pro, sc88pro, roland_sc88_state, init_sc88, "Roland", "Sound Canvas SC-88Pro", 0)
//SYST(1998, sk88pro, 0, 0, sk88pro, sc88, roland_sc88_state, init_sc88, "Roland", "Sound Canvas SK-88Pro", MACHINE_NOT_WORKING)
SYST(1998, vegspro, 0, 0, vegspro, 0, roland_sc88_state, init_sc88, "Roland", "VE-GSPro Voice Expansion Board", MACHINE_NOT_WORKING)
