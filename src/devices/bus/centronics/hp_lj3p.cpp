// license:BSD-3-Clause
// copyright-holders:rfka01
/***************************************************************************

HP Laserjet IIIP, Hewlett Packard, April 1991
Hardware Info By Guru
---------------------

The LaserJet IIIP (product# 33481A) was the Resolution Enhancement Technology version of the LaserJet IIP.
Features include...
4 Pages Per Minute.
1MB RAM as Standard. RAM expansions in 1MB or 2MB modules (maximum 2, maximum 5MB).
300DPI Resolution (8" x 7" bitmap graphic with standard RAM, 1 full-page bitmap with additional 1MB RAM)
Built in Fonts (14 Bitmapped Fonts, 8 Scalable Fonts). Many optional font carts were available.
PCL5 Printer Language (Postscript requires an additional 1MB RAM).
Serial and Parallel Interface (optional 10BaseT, JetDirect, LocalTalk)

This is the main controller board. PCB# RH6-3107 33481-80003 REV A
Main components are....
- 32MHz Oscillator
- Toshiba TMP68HC000T-16 CPU. Clock Input 16MHz [32/2]
- Samsung KM44C256 256kB x4-bit CMOS DRAM with Fast-Page Mode (x8 chips, total 1024kB DRAM)
  Specification shows RAM was expandable up to maximum 5MBytes with 2x 2MB RAM Boards.
- Toshiba TC5316200AF-F125 2MB SOP44 mask ROM
- 93C46 SOIC8 EEPROM
- National SD9212AL (PLCC84). HP Part# 1820-6339.
- HP 1TW7-0301 92171 EPG56 (QFP120). Most likely this is the Print Engine, possibly related to Canon's CX Print Engine.
- HP 1TU3-0201 92147 (PLCC28)
- 19.401MHz Crystal, connected to PLCC28 chip on pins 23 and 25.
- Some LS245 Logic Chips.
- ST TL7705 Reset Chip.
- Slot for Font Cartridge.
- Several Connectors.

Additional notes
----------------

    - mask ROM 1818-5008 (16 bit)
    - the 93C46 EEPROM holds the control panel settings and the page count
    - the serial interface works like an MC6850 ACIA and is emulated as one
      (which chip contains it is not known)
    - control panel: HD44780 compatible LCD controller, keys and indicators

    Host interfaces (user's manual, appendix D): a Centronics parallel port
    and an RS-232C serial port; the control panel selects one of them
    (Menu, CONFIG: I/O PARALEL or SERIAL, with BAUDRATE, ROBUST XON and
    DTR POLARITY for the serial port).  The device can be plugged into a
    Centronics or an RS-232 port; on a serial port, select I/O SERIAL and the
    computer's baud rate in the control panel (the settings stay in the
    EEPROM).

    Emulated: firmware, EEPROM, control panel display (HD44780) with keys and
    indicators, both host interfaces, the serial command/status link to the
    Canon engine with the MP tray and the optional lower cassette as
    configuration settings, the glyph/rule merger and the band video output.
    Printed pages are saved as PNG files (300 dpi) in the snapshot
    directory and shown in a paper view, which the layout shows next to
    the control panel (display, six keys, On Line and Form Feed indicators).

    Implementation notes:
    - serial port: MC6850 compatible ACIA at 300000 on IRQ 6; the baud rate
      clock comes from 3e0006 bits 13-15 (19200 Hz doubled per step; the
      firmware sets the ACIA to divide by 64 for 300 and 600 baud, by 16
      above)
    - merger: a write to the x position register (3c0000) moves the write
      pointer within the current row; the HP-GL/2 span fill routines set
      it for every row
    - the band registers are double buffered: the firmware writes the
      blank top margin and the first band back to back
    - the left margin register counts pixels from the beam detect signal;
      MARGIN_OFFSET converts it to the paper edge (PCL logical page 75
      pixels from the edge on Letter, 71 on A4)

    Not verified against hardware: the engine timing (TOP after PRNT, line
    period), the merger's raster operations, which are modelled as ROP3
    codes from the values the firmware uses, and the meaning of the
    parallel status bits (taken from the firmware's routines and its
    behaviour off line and without paper).

    Paper handling (service manual): the MP tray has no size sensor, its size
    is set in the control panel (MP SIZE); the lower cassette reports its size
    with three switches. PS2 senses paper in the MP tray, PS6 in the lower
    cassette.

***************************************************************************/

#include "emu.h"
#include "hp_lj3p.h"

#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/eepromser.h"
#include "video/hd44780.h"

#include "emuopts.h"
#include "emupal.h"
#include "fileio.h"
#include "screen.h"
#include "video.h"

#include "png.h"

#include <bit>

#define VERBOSE 0
#include "logmacro.h"

#include "hplj3p.lh"


namespace {

class hp_lj3p_base : public device_t
{
protected:
	hp_lj3p_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// the host interfaces
	virtual void out_busy(int state) { }
	virtual void out_ack(int state) { }
	virtual void out_perror(int state) { }
	virtual void out_fault(int state) { }
	virtual void out_select(int state) { }
	virtual void out_rxd(int state) { }     // serial data to the computer
	virtual void out_dtr(int state) { }

	void strobe_w(int state);
	void init_w(int state);
	void txd_w(int state) { m_acia->write_rxd(state); }

	u8 m_data;              // Centronics data lines

private:
	required_device<m68000_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hd44780_device> m_lcdc;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
	required_ioport m_config;
	required_ioport m_keys;
	output_finder<> m_online_led;
	output_finder<> m_formfeed_led;

	// video output: the formatter streams bands of the page from DRAM, one line
	// per engine BD pulse; 380000 high byte: words per line, 380000 low byte and
	// 380002: start address (word aligned, CPU address - 0x800000), 380004: left
	// margin, 380006: bits 0-11 line count, bit 12 stop, bit 14 data (else
	// blank lines), bit 15 set for the bands before the last one
	emu_timer *m_line_timer;
	TIMER_CALLBACK_MEMBER(video_line);
	bitmap_rgb32 m_page;
	int m_page_y;
	bool m_page_dirty;
	// the band registers are double buffered: a band written
	// while another one is output waits and is taken over when the current
	// band ends; the end of every band requests IRQ 5.  The firmware writes
	// the blank top margin and the first band with data back to back.
	struct band_t
	{
		u32 addr;
		u16 words;
		s16 x0;
		u16 lines;
		bool data;
	};
	band_t m_cur;
	band_t m_next;
	bool m_cur_active;
	bool m_next_valid;
	bool m_stop_pending;
	band_t latch_band() const;
	// Left margin offset: 380004 holds the start of the image in pixels after
	// the beam detect signal of each laser line, not after the paper edge.
	// The firmware writes 0x8d; with the paper edge 91 pixels after the
	// signal the PCL logical page starts 71 pixels (0.24 inch) from the edge
	// of an A4 sheet as specified, and a Letter image starts 50 pixels in.
	// Taken from printed rules, not measured on a printer; without it the
	// image is 141 pixels to the right.
	static constexpr int MARGIN_OFFSET = 91;
	// line period: about 50 mm/s at 300 lines per inch (not measured)
	static inline const attotime LINE_PERIOD = attotime::from_usec(1690);
	void eject_page();

	// paper view: the page being printed (or the last one) at a quarter of
	// the resolution, each pixel the coverage of a 4 x 4 dot cell
	static constexpr int VIEW_W = 2550 / 4;
	static constexpr int VIEW_H = 3508 / 4;
	bitmap_rgb32 m_view;
	bool m_view_clear;
	void view_dot(int x, int y);
	u32 paper_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void save_page();

	// glyph and rule merger: 3a0004 destination (CPU address, bits 0-22),
	// 3e0006 low byte line stride in words,
	// 3c0000 x position in pixels (bits 0-3 shift, bits 4-15 word offset), 3c0002 mask, 3c0004 mode,
	// 3e0000 data word inside a row, 3e0002 last data word of a row
	u32 m_blt_dest;
	u32 m_blt_row;
	u32 m_blt_ptr;
	u16 m_blt_ctrl;
	u16 m_blt_mask;
	u16 m_blt_mode;
	u16 m_blt_spill;
	u32 m_blt_pat;      // 3a0000: shading pattern, one word per row
	int m_blt_patrow;
	void blt_dest_w(offs_t offset, u16 data, u16 mem_mask);
	void blt_pat_w(offs_t offset, u16 data, u16 mem_mask);
	void blt_ctrl_w(offs_t offset, u16 data, u16 mem_mask);
	void blt_data_w(offs_t offset, u16 data, u16 mem_mask);
	void blt_out(u16 data);

	// parallel host interface: data latch at 200001, latch full on 3e0006 bit 10
	// and IRQ 2.  200000 written (the firmware's routines at 143e0-14494):
	// bit 15 ready for data (BUSY while clear or while the latch is full),
	// bit 7 ACK pulse, bit 6 /PERROR, bit 5 /FAULT, bit 4 SELECT (inverted
	// and active low as the routines set and clear them; bit 6 clears without
	// paper, bit 4 off line), bits 0, 1 and 3 serial interface outputs
	// (bit 0 DTR through an inverting driver, the others not known)
	u16 m_pctrl;
	u8 m_pdata;
	bool m_pfull;
	int m_strobe;
	emu_timer *m_ack_timer;
	TIMER_CALLBACK_MEMBER(ack_end);
	void update_parallel();
	void pctrl_w(offs_t offset, u16 data, u16 mem_mask);
	u16 pdata_r(offs_t offset, u16 mem_mask);

	u16 m_panel;
	void panel_w(offs_t offset, u16 data, u16 mem_mask);
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void main_map(address_map &map) ATTR_COLD;

	u16 m_ctrl;
	u16 m_ctrl2;        // 3e0006 written
	u16 m_irqctl;       // 3e0004 written
	bool m_irq4;        // engine sync interrupt (acknowledged with 3e0004 bit 7)
	bool m_irq5;        // video band interrupt (acknowledged with 3e0004 bit 6)
	bool m_top;         // engine TOP input (3e0006 bit 9)
	bool m_sheet;       // a sheet is passing the drum
	emu_timer *m_print_timer;
	TIMER_CALLBACK_MEMBER(print_step);
	emu_timer *m_tick_timer;
	TIMER_CALLBACK_MEMBER(tick) { m_maincpu->set_input_line(M68K_IRQ_3, HOLD_LINE); }
	void irqctl_w(offs_t offset, u16 data, u16 mem_mask);
	void video_w(offs_t offset, u16 data, u16 mem_mask);
	u16 m_video[4];

	// Canon engine command/status link (bit-banged by the firmware)
	u8 m_eng_cmd;
	u8 m_eng_sts;
	int m_eng_bits;
	int m_eng_cbits;    // command bits received
	bool m_eng_sbsy;    // engine has a status byte ready (formatter input 0, 280001 bit 1 low)
	bool m_eng_out;     // STS line state (1 = bit set)
	emu_timer *m_eng_timer;
	TIMER_CALLBACK_MEMBER(eng_reply);
	u8 eng_status(u8 cmd);
	void ctrl2_w(offs_t offset, u16 data, u16 mem_mask);
	u16 ctrl2_r(offs_t offset, u16 mem_mask);

	void ctrl_w(offs_t offset, u16 data, u16 mem_mask);
	u16 status_r(offs_t offset, u16 mem_mask);

	u16 unk_r(offs_t offset, u16 mem_mask);
	void unk_w(offs_t offset, u16 data, u16 mem_mask);
};


ROM_START( hp_lj3p )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "1818-5008.bin", 0x000000, 0x200000, CRC(900d4ea8) SHA1(c28333147b125bf332ce034e817df82887539cb3) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.bin", 0x00, 0x80, CRC(f8bf50ea) SHA1(1257d5b3912f8696f6843a13ac7e7bc7afdcef4e) )
ROM_END


static INPUT_PORTS_START( hp_lj3p )
	// control panel: the secondary functions (Reset, Continue, Print Fonts,
	// Test, minus) are selected by holding Alt while clicking the key
	PORT_START("KEYS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet On Line / Reset") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet Form Feed / Continue") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet Menu / Print Fonts") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet Enter / Test") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet + / -") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LaserJet Alt") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x00, "Lower cassette (LC)" )
	PORT_CONFSETTING(    0x00, "Not installed" )
	PORT_CONFSETTING(    0x01, "Letter" )
	PORT_CONFSETTING(    0x02, "Legal" )
	PORT_CONFSETTING(    0x03, "A4" )
	PORT_CONFSETTING(    0x04, "Executive" )
	PORT_CONFNAME( 0x08, 0x08, "Paper in MP tray" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x08, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x10, 0x10, "Paper in lower cassette" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x10, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x20, 0x00, "Paper path door" )
	PORT_CONFSETTING(    0x00, "Closed" )
	PORT_CONFSETTING(    0x20, "Open" )
INPUT_PORTS_END


hp_lj3p_base::hp_lj3p_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_data(0),
	m_maincpu(*this, "maincpu"),
	m_eeprom(*this, "eeprom"),
	m_lcdc(*this, "lcdc"),
	m_acia(*this, "acia"),
	m_acia_clock(*this, "acia_clock"),
	m_config(*this, "CONFIG"),
	m_keys(*this, "KEYS"),
	m_online_led(*this, "hplj3p_online_led"),
	m_formfeed_led(*this, "hplj3p_formfeed_led")
{
}

const tiny_rom_entry *hp_lj3p_base::device_rom_region() const
{
	return ROM_NAME( hp_lj3p );
}

ioport_constructor hp_lj3p_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( hp_lj3p );
}


u16 hp_lj3p_base::unk_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
		LOG("%s: read %06x & %04x\n", machine().describe_context(), offset << 1, mem_mask);
	return 0;
}

void hp_lj3p_base::unk_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: write %06x = %04x & %04x\n", machine().describe_context(), offset << 1, data, mem_mask);
}

void hp_lj3p_base::device_start()
{

	m_ctrl = 0;
	m_ctrl2 = 0;
	m_irqctl = 0;
	m_irq4 = m_irq5 = m_top = m_sheet = false;
	std::fill(std::begin(m_video), std::end(m_video), 0);
	m_print_timer = timer_alloc(FUNC(hp_lj3p_base::print_step), this);
	m_line_timer = timer_alloc(FUNC(hp_lj3p_base::video_line), this);
	// system tick on level 3: the firmware counts 167 ticks per second (source and acknowledge not known)
	m_tick_timer = timer_alloc(FUNC(hp_lj3p_base::tick), this);
	attotime const tick = attotime::from_hz(32_MHz_XTAL / 192000);
	m_tick_timer->adjust(tick, 0, tick);
	m_blt_dest = m_blt_row = m_blt_ptr = 0;
	m_blt_ctrl = m_blt_mask = m_blt_mode = m_blt_spill = 0;
	m_blt_pat = 0;
	m_blt_patrow = 0;
	m_page.allocate(2550, 3508);   // 8.5 x 11.69 inches: Letter width, A4 length
	m_page.fill(0xffffff);
	m_view.allocate(VIEW_W, VIEW_H);
	m_view.fill(0xffffff);
	m_view_clear = false;
	save_item(NAME(m_view));
	save_item(NAME(m_view_clear));
	m_page_y = 0;
	m_page_dirty = false;
	m_cur = m_next = band_t{ 0, 0, 0, 0, false };
	m_cur_active = m_next_valid = m_stop_pending = false;
	m_panel = 0;
	m_pctrl = 0;
	m_pdata = 0;
	m_pfull = false;
	m_strobe = 1;
	m_ack_timer = timer_alloc(FUNC(hp_lj3p_base::ack_end), this);
	m_eng_cmd = 0;
	m_eng_sts = 0;
	m_eng_bits = 0;
	m_eng_cbits = 0;
	m_eng_sbsy = false;
	m_eng_out = false;
	m_eng_timer = timer_alloc(FUNC(hp_lj3p_base::eng_reply), this);

	save_item(NAME(m_data));
	save_item(NAME(m_sheet));
	save_item(NAME(m_blt_pat));
	save_item(NAME(m_blt_patrow));
	save_item(NAME(m_blt_dest));
	save_item(NAME(m_blt_row));
	save_item(NAME(m_blt_ptr));
	save_item(NAME(m_blt_ctrl));
	save_item(NAME(m_blt_mask));
	save_item(NAME(m_blt_mode));
	save_item(NAME(m_blt_spill));
	save_item(NAME(m_page));
	save_item(NAME(m_page_y));
	save_item(NAME(m_page_dirty));
	save_item(NAME(m_cur.addr));
	save_item(NAME(m_cur.words));
	save_item(NAME(m_cur.x0));
	save_item(NAME(m_cur.lines));
	save_item(NAME(m_cur.data));
	save_item(NAME(m_next.addr));
	save_item(NAME(m_next.words));
	save_item(NAME(m_next.x0));
	save_item(NAME(m_next.lines));
	save_item(NAME(m_next.data));
	save_item(NAME(m_cur_active));
	save_item(NAME(m_next_valid));
	save_item(NAME(m_stop_pending));
	save_item(NAME(m_irqctl));
	save_item(NAME(m_irq4));
	save_item(NAME(m_irq5));
	save_item(NAME(m_top));
	save_item(NAME(m_video));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_pdata));
	save_item(NAME(m_pfull));
	save_item(NAME(m_strobe));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_ctrl2));
	save_item(NAME(m_panel));
	save_item(NAME(m_eng_cmd));
	save_item(NAME(m_eng_sts));
	save_item(NAME(m_eng_bits));
	save_item(NAME(m_eng_cbits));
	save_item(NAME(m_eng_sbsy));
	save_item(NAME(m_eng_out));
}

void hp_lj3p_base::device_reset()
{
	m_acia->write_cts(0);
	m_acia->write_dcd(0);
	m_acia_clock->set_unscaled_clock(19200);
	update_parallel();
	out_ack(1);
}


// Status bytes as the firmware evaluates them (odd parity in bit 0, bit 7 set
// with bit 6 clear: command not supported by this engine). The firmware tells
// the engine variants apart by the commands they reject; the IIP/IIIP engine
// rejects 13 and 15 and has the MP tray plus the optional lower cassette (LC).
//   01  general status: bit 3 warming up, bit 4 printing error (41 ERROR)
//   02  bit 1 engine test, bit 2 door open or no EP cartridge (12),
//       bit 3 paper jam (13), bit 4 paper out
//   04  bit 4 52 ERROR, bit 5 51 ERROR, bit 6 50 SERVICE
//   0b  lower cassette size code in bits 6-1 (from switches SW901-SW903)
//   25  bit 5 lower cassette installed
//   2a  bit 6 paper in the MP tray (PS2), bit 5 paper in the lower cassette (PS6)
u8 hp_lj3p_base::eng_status(u8 cmd)
{
	u8 const cfg = m_config->read();
	unsigned const lc = cfg & 0x07;     // lower cassette size, 0 = not installed
	u8 data = 0x00;
	switch (cmd)
	{
	case 0x02:
		if (BIT(cfg, 5))
			data |= 0x04;
		break;
	case 0x0b:
	{
		// engine codes for Letter, Legal, A4 and Executive as translated by the
		// firmware's table at 0x5968
		static u8 const codes[] = { 0x00, 0x04, 0x0c, 0x01, 0x0d };
		data = (lc < std::size(codes) ? codes[lc] : 0) << 1;
		break;
	}
	case 0x13:
	case 0x15:
		data = 0x80;
		break;
	case 0x25:
		if (lc)
			data |= 0x20;
		break;
	case 0x2a:
		if (BIT(cfg, 3))
			data |= 0x40;
		if (lc && BIT(cfg, 4))
			data |= 0x20;
		break;
	}
	if (!(std::popcount(unsigned(data)) & 1))
		data |= 0x01;
	return data;
}

TIMER_CALLBACK_MEMBER(hp_lj3p_base::eng_reply)
{
	m_eng_sts = eng_status(m_eng_cmd);
	m_eng_bits = 0;
	m_eng_sbsy = true;
	LOG("engine cmd %02x -> sts %02x\n", m_eng_cmd, m_eng_sts);
}

// 3e0006: bits 0-7 merger line stride in words, bit 8 SCLK (the formatter
// clocks commands out and status in), bits 13-15 serial clock select
void hp_lj3p_base::ctrl2_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_ctrl2;
	COMBINE_DATA(&m_ctrl2);
	if (BIT(m_ctrl2, 8) && !BIT(old, 8))
	{
		if (!BIT(m_ctrl, 11))
		{
			// /CBSY low: command bit on 240000 bit 10 (active low), MSB first
			m_eng_cmd = (m_eng_cmd << 1) | (BIT(m_ctrl, 10) ? 0 : 1);
			m_eng_cbits++;
		}
		else if (m_eng_sbsy)
		{
			m_eng_out = BIT(m_eng_sts, 7 - m_eng_bits);
			if (++m_eng_bits == 8)
				m_eng_sbsy = false;
		}
	}
	if ((m_ctrl2 ^ old) & 0xe000)
	{
		// The firmware's baud rate routine (140a8) selects 0-4 here and the
		// ACIA divider /64 for 300 and 600 baud, /16 above: the clock is
		// 19200 Hz (300 x 64) doubled per step
		unsigned const sel = std::min(m_ctrl2 >> 13, 4);
		m_acia_clock->set_unscaled_clock(19200 << sel);
		LOG("%s: serial clock %d Hz\n", machine().describe_context(), 19200 << sel);
	}
	if ((m_ctrl2 ^ old) & 0x1e00)
		LOG("%s: 3e0006 %04x\n", machine().describe_context(), m_ctrl2);
}

// 3e0006 read: bit 8 /STS, bit 9 engine input, bit 10 parallel data latch full
u16 hp_lj3p_base::ctrl2_r(offs_t offset, u16 mem_mask)
{
	u16 data = m_eng_out ? 0x0000 : 0x0100;
	if (m_top)
		data |= 0x0200;
	if (m_pfull)
		data |= 0x0400;
	return data;
}

// 240000: bit 7 EEPROM CS, bit 8 CLK, bit 9 DI; engine: bit 10 /CMD, bit 11 /CBSY,
// bits 12-14 further engine outputs
void hp_lj3p_base::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_ctrl;
	COMBINE_DATA(&m_ctrl);
	if (!BIT(old, 11) && BIT(m_ctrl, 11) && m_eng_cbits == 8)
		m_eng_timer->adjust(attotime::from_usec(200));   // CBSY released: command complete
	if (BIT(old, 11) && !BIT(m_ctrl, 11))
	{
		m_eng_cmd = 0;
		m_eng_cbits = 0;
	}
	m_eeprom->di_write(BIT(m_ctrl, 9));
	m_eeprom->cs_write(BIT(m_ctrl, 7));
	m_eeprom->clk_write(BIT(m_ctrl, 8));
	if ((m_ctrl ^ old) & 0x7000)
		LOG("%s: ctrl %04x\n", machine().describe_context(), m_ctrl);
	if (BIT(old, 13) && !BIT(m_ctrl, 13))
		m_print_timer->adjust(attotime::from_seconds(2), 0);    // initial rotation (time not measured), then TOP
}

// 280000: bit 0 EEPROM DO, bit 1 /SBSY (engine status ready), bits 2/3 engine inputs
u16 hp_lj3p_base::status_r(offs_t offset, u16 mem_mask)
{
	u16 data = m_eeprom->do_read() ? 0x0001 : 0x0000;
	data |= m_keys->read() << 8;    // control panel keys, active low
	data |= m_eng_sbsy ? 0x0000 : 0x0002;
	// bit 2 /RDY and bit 3 /PPRDY (engine powered): both held active (low) for an
	// engine that is switched on and ready; the firmware waits for /PPRDY before it
	// starts talking to the engine.  Bits 4 and 5 sense an RS-422 adapter on the
	// serial port (routine at 1502a), none fitted.
	return data;
}

void hp_lj3p_base::update_parallel()
{
	// the interrupt is only requested while the formatter accepts data
	m_maincpu->set_input_line(M68K_IRQ_2, (m_pfull && BIT(m_pctrl, 15)) ? ASSERT_LINE : CLEAR_LINE);
	out_busy((m_pfull || !BIT(m_pctrl, 15)) ? 1 : 0);
	out_select(BIT(m_pctrl, 4));
	out_perror(BIT(m_pctrl, 6) ? 0 : 1);
	out_fault(BIT(m_pctrl, 5));
	out_dtr(BIT(m_pctrl, 0));
}

void hp_lj3p_base::strobe_w(int state)
{
	// the data is latched on the falling edge of /STROBE
	if (m_strobe && !state && !m_pfull)
	{
		m_pdata = m_data;
		m_pfull = true;
		update_parallel();
	}
	m_strobe = state;
}

void hp_lj3p_base::init_w(int state)
{
	if (!state)
		LOG("%s: /INIT from the host (not handled)\n", machine().describe_context());
}

TIMER_CALLBACK_MEMBER(hp_lj3p_base::ack_end)
{
	out_ack(1);
}

u16 hp_lj3p_base::pdata_r(offs_t offset, u16 mem_mask)
{
	return m_pdata;
}

void hp_lj3p_base::pctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_pctrl;
	COMBINE_DATA(&m_pctrl);
	if (BIT(m_pctrl, 7) && !BIT(old, 7))
	{
		// acknowledge: the latch is free again, /ACK pulses low
		m_pfull = false;
		out_ack(0);
		m_ack_timer->adjust(attotime::from_usec(5));
	}
	update_parallel();
	if ((m_pctrl ^ old) & 0x7f7f)
		LOG("%s: parallel control %04x\n", machine().describe_context(), m_pctrl);
}

TIMER_CALLBACK_MEMBER(hp_lj3p_base::print_step)
{
	switch (param)
	{
	case 0:
		// TOP for the next sheet: PRNT (240000 bit 13) and bit 12 active and the
		// previous sheet finished
		if (BIT(m_ctrl, 13))
			break;
		if (BIT(m_ctrl, 12) || m_sheet)
		{
			m_print_timer->adjust(attotime::from_msec(100), 0);
			break;
		}
		LOG("engine TOP\n");
		m_sheet = true;
		m_top = true;
		m_irq4 = true;
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
		m_print_timer->adjust(attotime::from_msec(10), 1);
		break;
	case 1:
		m_top = false;
		break;
	}
}

void hp_lj3p_base::irqctl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_irqctl;
	COMBINE_DATA(&m_irqctl);
	if (BIT(m_irqctl, 7) && !BIT(old, 7) && m_irq4)
	{
		m_irq4 = false;
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	}
	if (BIT(m_irqctl, 6) && !BIT(old, 6) && m_irq5)
	{
		m_irq5 = false;
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
	}
	if ((m_irqctl ^ old) & 0xff1f)
		LOG("%s: 3e0004 %04x\n", machine().describe_context(), m_irqctl);
}

hp_lj3p_base::band_t hp_lj3p_base::latch_band() const
{
	band_t b;
	b.addr = ((u32(m_video[0] & 0x7f) << 16) | (m_video[1] & 0xfffe)) + 0x800000;
	b.words = m_video[0] >> 8;
	b.x0 = (m_video[2] & 0x0fff) - MARGIN_OFFSET;
	b.lines = m_video[3] & 0x0fff;
	b.data = BIT(m_video[3], 14);
	return b;
}

void hp_lj3p_base::video_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_video[offset]);
	LOG("%s: video %06x = %04x\n", machine().describe_context(), 0x380000 + offset * 2, m_video[offset]);
	if (offset != 3)
		return;
	if (BIT(m_video[3], 12))
	{
		// stop: the page ends after the bands already queued
		if (!m_cur_active && !m_next_valid)
			eject_page();
		else
			m_stop_pending = true;
	}
	else if (m_video[3] & 0x0fff)
	{
		m_stop_pending = false;
		if (!m_cur_active)
		{
			m_cur = latch_band();
			m_cur_active = true;
			m_line_timer->adjust(LINE_PERIOD);
		}
		else
		{
			m_next = latch_band();
			m_next_valid = true;
		}
	}
}

TIMER_CALLBACK_MEMBER(hp_lj3p_base::video_line)
{
	if (!m_cur_active)
		return;
	if (m_cur.data && m_page_y < m_page.height())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		u32 *const line = &m_page.pix(m_page_y);
		for (int w = 0; w < m_cur.words; w++)
		{
			u16 const d = space.read_word(m_cur.addr + w * 2);
			space.write_word(m_cur.addr + w * 2, 0);   // the band memory is cleared as it is read out (the firmware never clears it)
			for (int b = 0; b < 16; b++)
			{
				int const x = m_cur.x0 + w * 16 + b;
				if (BIT(d, 15 - b) && x >= 0 && x < m_page.width())
				{
					line[x] = 0x000000;
					view_dot(x, m_page_y);
					m_page_dirty = true;
				}
			}
		}
		m_cur.addr += m_cur.words * 2;
	}
	m_page_y++;
	if (--m_cur.lines)
	{
		m_line_timer->adjust(LINE_PERIOD);
		return;
	}
	LOG("band done, page line %d\n", m_page_y);
	m_irq5 = true;
	m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
	if (m_next_valid)
	{
		m_cur = m_next;
		m_next_valid = false;
		m_line_timer->adjust(LINE_PERIOD);
	}
	else
	{
		m_cur_active = false;
		if (m_stop_pending)
		{
			m_stop_pending = false;
			eject_page();
		}
	}
}

void hp_lj3p_base::blt_pat_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset)
		m_blt_pat = (m_blt_pat & 0xffff0000) | data;
	else
		m_blt_pat = (m_blt_pat & 0x0000ffff) | (u32(data) << 16);
	m_blt_patrow = 0;
}

void hp_lj3p_base::blt_dest_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset)
		m_blt_dest = (m_blt_dest & 0xffff0000) | (data & mem_mask) | (m_blt_dest & ~mem_mask & 0xffff);
	else
		m_blt_dest = (m_blt_dest & 0x0000ffff) | (u32(data & mem_mask) << 16) | (m_blt_dest & (u32(~mem_mask & 0xffff) << 16));
	m_blt_row = m_blt_dest;
	m_blt_ptr = m_blt_row + (m_blt_ctrl >> 4) * 2;
	m_blt_spill = 0;
}

void hp_lj3p_base::blt_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
	case 0:
		// x position: the row continues from this word (the span fill
		// routines for HP-GL/2 areas and vectors set it for every row)
		COMBINE_DATA(&m_blt_ctrl);
		m_blt_ptr = m_blt_row + (m_blt_ctrl >> 4) * 2;
		m_blt_spill = 0;
		break;
	case 1: COMBINE_DATA(&m_blt_mask); break;
	case 2:
	{
		u16 const old = m_blt_mode;
		COMBINE_DATA(&m_blt_mode);
		if ((old ^ m_blt_mode) & 0xff00)
			LOG("%s: merger mode %04x\n", machine().describe_context(), m_blt_mode);
		break;
	}
	}
}

void hp_lj3p_base::blt_out(u16 data)
{
	if (m_blt_dest == 0xffffffff || m_blt_ptr < 0xb00000 || m_blt_ptr > 0xfffffe)
		return;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u16 const d = space.read_word(m_blt_ptr);
	u16 const pat = space.read_word((m_blt_pat & 0xfffffe) + (m_blt_patrow & 15) * 2);
	// raster operation from the low byte of 3c0004, indexed by data, pattern and
	// destination bits like a ROP3 code (fa: OR the data into the page, ea: OR the
	// data masked with the shading pattern)
	u8 const rop = m_blt_mode & 0xff;
	u16 r = 0;
	for (int b = 0; b < 16; b++)
		if (BIT(rop, (BIT(data, b) << 2) | (BIT(pat, b) << 1) | BIT(d, b)))
			r |= 1 << b;
	space.write_word(m_blt_ptr, (d & ~m_blt_mask) | (r & m_blt_mask));
	m_blt_ptr += 2;
}

void hp_lj3p_base::blt_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	int const sh = m_blt_ctrl & 0x0f;
	u16 const out = m_blt_spill | (data >> sh);
	m_blt_spill = sh ? u16(data << (16 - sh)) : 0;
	blt_out(out);
	if (offset)
	{
		// last word of the row: flush and go to the next row
		if (sh)
			blt_out(m_blt_spill);
		m_blt_spill = 0;
		m_blt_row += (m_ctrl2 & 0xff) * 2;   // line stride in words: 3e0006 low byte
		m_blt_ptr = m_blt_row + (m_blt_ctrl >> 4) * 2;
		m_blt_patrow++;
	}
}

void hp_lj3p_base::view_dot(int x, int y)
{
	if (m_view_clear)
	{
		m_view.fill(0xffffff);
		m_view_clear = false;
	}
	if (x / 4 < VIEW_W && y / 4 < VIEW_H)
	{
		u32 &p = m_view.pix(y / 4, x / 4);
		u32 const v = std::max<int>(int(p & 0xff) - 16, 0);
		p = (v << 16) | (v << 8) | v;
	}
}

void hp_lj3p_base::save_page()
{
	// each page goes to a PNG file in the snapshot directory (300 dpi)
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = machine().video().open_next(file, "png");
	if (!filerr)
		util::png_write_bitmap(file, nullptr, m_page, 0, nullptr);
}

u32 hp_lj3p_base::paper_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_view, 0, 0, 0, 0, cliprect);
	return 0;
}

void hp_lj3p_base::eject_page()
{
	m_view_clear = true;   // the next page replaces the view
	if (m_page_dirty || m_page_y)
	{
		LOG("page complete, %d lines\n", m_page_y);
		save_page();
	}
	m_page.fill(0xffffff);
	m_page_y = 0;
	m_page_dirty = false;
	// PRNT still active: the next sheet follows
	m_sheet = false;
	if (!BIT(m_ctrl, 13))
		m_print_timer->adjust(attotime::from_seconds(1), 0);
}

// 280000 written: control panel HD44780 in 4 bit mode, bits 0-3 D4-D7,
// bit 6 RS, bit 7 E; bit 4 Form Feed and bit 5 On Line indicators (active low)
void hp_lj3p_base::panel_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_panel;
	COMBINE_DATA(&m_panel);
	m_lcdc->db_w((m_panel & 0x0f) << 4);
	m_lcdc->rs_w(BIT(m_panel, 6));
	m_lcdc->rw_w(0);
	m_lcdc->e_w(BIT(m_panel, 7));
	m_online_led = BIT(m_panel, 5) ? 0 : 1;      // active low
	m_formfeed_led = BIT(m_panel, 4) ? 0 : 1;
	if ((m_panel ^ old) & 0xff30)
		LOG("%s: panel %04x\n", machine().describe_context(), m_panel);
}

HD44780_PIXEL_UPDATE(hp_lj3p_base::lcd_pixel_update)
{
	// two rows of eight characters (user's manual)
	if (x < 5 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(line * 9 + y, pos * 6 + x) = state;
}

void hp_lj3p_base::main_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(hp_lj3p_base::unk_r), FUNC(hp_lj3p_base::unk_w));
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x200001).rw(FUNC(hp_lj3p_base::pdata_r), FUNC(hp_lj3p_base::pctrl_w));
	map(0x240000, 0x240001).w(FUNC(hp_lj3p_base::ctrl_w));
	map(0x280000, 0x280001).rw(FUNC(hp_lj3p_base::status_r), FUNC(hp_lj3p_base::panel_w));
	map(0x300000, 0x300003).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x380000, 0x380007).w(FUNC(hp_lj3p_base::video_w));
	map(0x3a0000, 0x3a0003).w(FUNC(hp_lj3p_base::blt_pat_w));
	map(0x3a0004, 0x3a0007).w(FUNC(hp_lj3p_base::blt_dest_w));
	map(0x3c0000, 0x3c0005).w(FUNC(hp_lj3p_base::blt_ctrl_w));
	map(0x3e0000, 0x3e0003).w(FUNC(hp_lj3p_base::blt_data_w));
	map(0x3e0004, 0x3e0005).w(FUNC(hp_lj3p_base::irqctl_w));
	map(0x3e0006, 0x3e0007).rw(FUNC(hp_lj3p_base::ctrl2_r), FUNC(hp_lj3p_base::ctrl2_w));
	map(0xb00000, 0xbfffff).ram();
}

void hp_lj3p_base::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp_lj3p_base::main_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	// serial interface: the receive interrupt handler (b3d86) is installed on level 6
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (int state) { out_rxd(state); });
	m_acia->irq_handler().set_inputline(m_maincpu, M68K_IRQ_6);
	CLOCK(config, m_acia_clock, 19200);
	m_acia_clock->signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	m_acia_clock->signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	screen_device &paper(SCREEN(config, "paper"));
	paper.set_refresh_hz(10);
	paper.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	paper.set_size(VIEW_W, VIEW_H);
	paper.set_visarea_full();
	paper.set_screen_update(FUNC(hp_lj3p_base::paper_update));

	config.set_default_layout(layout_hplj3p);

	screen_device &screen(SCREEN(config, "lcd").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(8 * 6, 2 * 9 - 1);
	screen.set_visarea_full();
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	HD44780(config, m_lcdc, 270'000);   // clock not measured
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(hp_lj3p_base::lcd_pixel_update));
}


//**************************************************************************
//  INTERFACES
//**************************************************************************

// one device for both built-in interfaces
class hp_lj3p_device : public hp_lj3p_base, public device_centronics_peripheral_interface, public device_rs232_port_interface
{
public:
	hp_lj3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		hp_lj3p_base(mconfig, HP_LJ3P, tag, owner, clock),
		device_centronics_peripheral_interface(mconfig, *this),
		device_rs232_port_interface(mconfig, *this),
		m_parallel(dynamic_cast<centronics_device *>(owner) != nullptr),
		m_serial(dynamic_cast<rs232_port_device *>(owner) != nullptr)
	{
	}

	// parallel interface
	virtual void input_strobe(int state) override { strobe_w(state); }
	virtual void input_init(int state) override { init_w(state); }
	virtual void input_data0(int state) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }

	// serial interface: data from the computer arrives on the printer's RxD
	virtual void input_txd(int state) override { txd_w(state); }

protected:
	virtual bool supports_pin35_5v() override { return true; }

	virtual void device_reset() override ATTR_COLD
	{
		hp_lj3p_base::device_reset();
		if (m_serial)
		{
			output_rxd(1);
			output_dcd(0);
		}
	}

	virtual void out_busy(int state) override { if (m_parallel) output_busy(state); }
	virtual void out_ack(int state) override { if (m_parallel) output_ack(state); }
	virtual void out_perror(int state) override { if (m_parallel) output_perror(state); }
	virtual void out_fault(int state) override { if (m_parallel) output_fault(state); }
	virtual void out_select(int state) override { if (m_parallel) output_select(state); }
	virtual void out_rxd(int state) override { if (m_serial) output_rxd(state); }
	virtual void out_dtr(int state) override
	{
		// the printer's DTR (pin 20) goes to the computer as DSR and CTS; the
		// line driver inverts, so a clear bit asserts DTR (the firmware keeps it
		// clear when it is ready in serial mode)
		if (m_serial)
		{
			output_dsr(state);
			output_cts(state);
		}
	}

private:
	bool const m_parallel;
	bool const m_serial;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(HP_LJ3P, device_t, hp_lj3p_device, "hplj3p", "HP LaserJet IIIP")
