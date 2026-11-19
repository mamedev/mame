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
    (Menu, DEVICE CONFIG: I/O PARALEL or SERIAL, with BAUDRATE, ROBUST XON
    and DTR POLARITY for the serial port).  This device is the printer with
    both interfaces as callbacks and inputs; bus/centronics/hp_lj3p.cpp and
    bus/rs232/hp_lj3p.cpp connect it to a computer, hp/laserjet3p.cpp runs
    it on its own.  On a serial port, select I/O SERIAL and the computer's
    baud rate in the control panel (the settings stay in the EEPROM).

    Emulated: firmware, EEPROM, control panel display (HD44780) with keys and
    indicators, both host interfaces, the serial command/status link to the
    Canon engine with the MP tray and the optional lower cassette as
    configuration settings, the glyph/rule merger and the band video output.
    Printed pages are saved as PNG files (300 dpi) in the snapshot
    directory and shown in a paper view; the layout "Paper and Control
    Panel" puts the control panel (display, six keys, On Line and Form Feed
    indicators) under the paper.  Connected to a computer, the view
    "Computer, Paper and Control Panel" adds the computer's screen on the
    left: the screen tagged "screen" at the root of the machine, assumed
    4:3 (the order of the screens depends on the computer, so it is not
    taken by index); without such a screen MAME leaves this view out.  The
    computer's own views stay selectable; as the only view with all
    screens, MAME selects it by default.

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
      MARGIN_OFFSET converts it to the paper edge (see hp_lj3p.h)

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

#include "emuopts.h"
#include "fileio.h"
#include "video.h"

#include "png.h"

#include <algorithm>
#include <bit>
#include <iterator>

#include "hplj3p.lh"
#include "hplj3p_host.lh"

#define VERBOSE 0
#include "logmacro.h"


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


DEFINE_DEVICE_TYPE(HP_LJ3P, hp_lj3p_device, "hplj3p", "HP LaserJet IIIP")

hp_lj3p_device::hp_lj3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, HP_LJ3P, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_eeprom(*this, "eeprom"),
	m_lcdc(*this, "lcdc"),
	m_acia(*this, "acia"),
	m_acia_clock(*this, "acia_clock"),
	m_palette(*this, "palette"),
	m_config(*this, "CONFIG"),
	m_keys(*this, "KEYS"),
	m_online_led(*this, "hplj3p_online_led"),
	m_formfeed_led(*this, "hplj3p_formfeed_led"),
	m_busy_cb(*this),
	m_ack_cb(*this),
	m_perror_cb(*this),
	m_fault_cb(*this),
	m_select_cb(*this),
	m_txd_cb(*this),
	m_dtr_cb(*this),
	m_data(0)
{
}

const tiny_rom_entry *hp_lj3p_device::device_rom_region() const
{
	return ROM_NAME( hp_lj3p );
}

ioport_constructor hp_lj3p_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hp_lj3p );
}


u16 hp_lj3p_device::unk_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
		LOG("%s: read %06x & %04x\n", machine().describe_context(), offset << 1, mem_mask);
	return 0;
}

void hp_lj3p_device::unk_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: write %06x = %04x & %04x\n", machine().describe_context(), offset << 1, data, mem_mask);
}

void hp_lj3p_device::device_start()
{
	m_maincpu->space(AS_PROGRAM).specific(m_program);

	m_print_timer = timer_alloc(FUNC(hp_lj3p_device::print_step), this);
	m_line_timer = timer_alloc(FUNC(hp_lj3p_device::video_line), this);
	m_ack_timer = timer_alloc(FUNC(hp_lj3p_device::ack_end), this);
	m_eng_timer = timer_alloc(FUNC(hp_lj3p_device::eng_reply), this);
	// system tick on level 3 (source not known): the firmware converts
	// milliseconds to ticks as ms * 100 / 167 and back (routines at 12f00 and
	// 12ecc), i.e. 1.67 ms per tick.  Its software timers count every second
	// tick; with this rate a sheet counts as delivered about 21 seconds after
	// it was started (which turns the Form Feed indicator off and lets On Line
	// pressed during printing take effect) and the indicators flash at 1 Hz
	// (assumed rate, not measured)
	m_tick_timer = timer_alloc(FUNC(hp_lj3p_device::tick), this);
	attotime const tick = attotime::from_hz(600);
	m_tick_timer->adjust(tick, 0, tick);

	m_page = make_unique_clear<u8 []>(PAGE_STRIDE * PAGE_H);
	m_view = make_unique_clear<u8 []>(VIEW_W * VIEW_H);
	std::fill_n(m_view.get(), VIEW_W * VIEW_H, 0xff);
	m_view_clear = false;
	m_strobe = true;

	// the host interface register's reset value is not known; this is the
	// firmware's first write (not ready, DTR inactive), so that a computer
	// does not send before the firmware runs
	m_pctrl = 0x0078;
	m_pdata = 0;
	m_pfull = false;
	m_lines = status_lines();
	m_busy_cb(BIT(m_lines, 0));
	m_select_cb(BIT(m_lines, 1));
	m_perror_cb(BIT(m_lines, 2));
	m_fault_cb(BIT(m_lines, 3));
	m_dtr_cb(BIT(m_lines, 4));
	m_ack_cb(1);

	save_item(NAME(m_data));
	save_item(NAME(m_video));
	save_pointer(NAME(m_page), PAGE_STRIDE * PAGE_H);
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
	save_pointer(NAME(m_view), VIEW_W * VIEW_H);
	save_item(NAME(m_view_clear));
	save_item(NAME(m_blt_dest));
	save_item(NAME(m_blt_row));
	save_item(NAME(m_blt_ptr));
	save_item(NAME(m_blt_ctrl));
	save_item(NAME(m_blt_mask));
	save_item(NAME(m_blt_mode));
	save_item(NAME(m_blt_spill));
	save_item(NAME(m_blt_pat));
	save_item(NAME(m_blt_patrow));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_pdata));
	save_item(NAME(m_pfull));
	save_item(NAME(m_strobe));
	save_item(NAME(m_lines));
	save_item(NAME(m_panel));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_ctrl2));
	save_item(NAME(m_irqctl));
	save_item(NAME(m_irq3));
	save_item(NAME(m_irq4));
	save_item(NAME(m_irq5));
	save_item(NAME(m_top));
	save_item(NAME(m_sheet));
	save_item(NAME(m_eng_cmd));
	save_item(NAME(m_eng_sts));
	save_item(NAME(m_eng_bits));
	save_item(NAME(m_eng_cbits));
	save_item(NAME(m_eng_sbsy));
	save_item(NAME(m_eng_out));
}

void hp_lj3p_device::device_reset()
{
	// the firmware starts over: a page in progress is dropped
	m_print_timer->adjust(attotime::never);
	m_line_timer->adjust(attotime::never);
	m_ack_timer->adjust(attotime::never);
	m_eng_timer->adjust(attotime::never);

	std::fill(std::begin(m_video), std::end(m_video), 0);
	clear_page();
	m_cur = m_next = band_t{ 0, 0, 0, 0, false };
	m_cur_active = m_next_valid = m_stop_pending = false;

	m_blt_dest = m_blt_row = m_blt_ptr = 0;
	m_blt_ctrl = m_blt_mask = m_blt_mode = m_blt_spill = 0;
	m_blt_pat = 0;
	m_blt_patrow = 0;

	m_panel = 0;
	m_ctrl = 0;
	m_ctrl2 = 0;
	m_irqctl = 0;
	m_irq3 = m_irq4 = m_irq5 = m_top = m_sheet = false;
	m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

	m_eng_cmd = m_eng_sts = 0;
	m_eng_bits = m_eng_cbits = 0;
	m_eng_sbsy = m_eng_out = false;

	m_acia->write_cts(0);
	m_acia->write_dcd(0);
	m_acia_clock->set_unscaled_clock(19200);

	// the host interface register returns to its initial state (see
	// device_start); a pending /ACK pulse ends
	m_pctrl = 0x0078;
	m_pdata = 0;
	m_pfull = false;
	update_parallel();
	m_ack_cb(1);
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
u8 hp_lj3p_device::eng_status(u8 cmd)
{
	u8 const cfg = m_config->read();
	u8 const lc = cfg & 0x07;           // lower cassette size, 0 = not installed
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
		static constexpr u8 CODES[] = { 0x00, 0x04, 0x0c, 0x01, 0x0d };
		data = (lc < std::size(CODES) ? CODES[lc] : 0) << 1;
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
	if (!(std::popcount(data) & 1))
		data |= 0x01;
	return data;
}

TIMER_CALLBACK_MEMBER(hp_lj3p_device::eng_reply)
{
	m_eng_sts = eng_status(m_eng_cmd);
	m_eng_bits = 0;
	m_eng_sbsy = true;
	LOG("engine cmd %02x -> sts %02x\n", m_eng_cmd, m_eng_sts);
}

// 3e0006: bits 0-7 merger line stride in words, bit 8 SCLK (the formatter
// clocks commands out and status in), bits 13-15 serial clock select
void hp_lj3p_device::ctrl2_w(offs_t offset, u16 data, u16 mem_mask)
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
		int const sel = std::min(m_ctrl2 >> 13, 4);
		m_acia_clock->set_unscaled_clock(19200 << sel);
		LOG("%s: serial clock %d Hz\n", machine().describe_context(), 19200 << sel);
	}
	if ((m_ctrl2 ^ old) & 0x1e00)
		LOG("%s: 3e0006 %04x\n", machine().describe_context(), m_ctrl2);
}

// 3e0006 read: bit 8 /STS, bit 9 engine input, bit 10 parallel data latch full
u16 hp_lj3p_device::ctrl2_r()
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
void hp_lj3p_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
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
u16 hp_lj3p_device::status_r()
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

u8 hp_lj3p_device::status_lines() const
{
	return
			((m_pfull || !BIT(m_pctrl, 15)) ? 0x01 : 0x00) |  // BUSY
			(BIT(m_pctrl, 4) ? 0x00 : 0x02) |                 // SELECT
			(BIT(m_pctrl, 6) << 2) |                          // PERROR
			(BIT(m_pctrl, 5) << 3) |                          // /FAULT
			(BIT(m_pctrl, 3) << 4);                           // DTR
}

void hp_lj3p_device::update_parallel()
{
	// the interrupt is only requested while the formatter accepts data
	m_maincpu->set_input_line(M68K_IRQ_2, (m_pfull && BIT(m_pctrl, 15)) ? ASSERT_LINE : CLEAR_LINE);

	// the status lines to the computer, each passed on when it changes
	u8 const lines = status_lines();
	u8 const changed = lines ^ m_lines;
	m_lines = lines;
	if (BIT(changed, 0))
		m_busy_cb(BIT(lines, 0));
	if (BIT(changed, 1))
		m_select_cb(BIT(lines, 1));
	if (BIT(changed, 2))
		m_perror_cb(BIT(lines, 2));
	if (BIT(changed, 3))
		m_fault_cb(BIT(lines, 3));
	if (BIT(changed, 4))
		m_dtr_cb(BIT(lines, 4));
}

void hp_lj3p_device::strobe_w(int state)
{
	// the data is latched on the falling edge of /STROBE
	if (m_strobe && !state && !m_pfull)
	{
		m_pdata = m_data;
		m_pfull = true;
		update_parallel();
	}
	m_strobe = state != 0;
}

void hp_lj3p_device::rxd_w(int state)
{
	m_acia->write_rxd(state);
}

void hp_lj3p_device::init_w(int state)
{
	if (!state)
		LOG("%s: /INIT from the host (not handled)\n", machine().describe_context());
}

TIMER_CALLBACK_MEMBER(hp_lj3p_device::ack_end)
{
	m_ack_cb(1);
}

u16 hp_lj3p_device::pdata_r()
{
	return m_pdata;
}

void hp_lj3p_device::pctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_pctrl;
	COMBINE_DATA(&m_pctrl);
	if (BIT(m_pctrl, 7) && !BIT(old, 7))
	{
		// acknowledge: the latch is free again, /ACK pulses low
		m_pfull = false;
		m_ack_cb(0);
		m_ack_timer->adjust(attotime::from_usec(5));
	}
	update_parallel();
	if ((m_pctrl ^ old) & 0x7f7f)
		LOG("%s: parallel control %04x\n", machine().describe_context(), m_pctrl);
}

TIMER_CALLBACK_MEMBER(hp_lj3p_device::print_step)
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

void hp_lj3p_device::irqctl_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 const old = m_irqctl;
	COMBINE_DATA(&m_irqctl);
	// the firmware sets bit 5 to acknowledge the system tick but clears it only
	// in its copy of the register (routine at 12e6a), so a write with bit 5 set
	// is taken as the acknowledge, not its rising edge (assumed)
	if (BIT(m_irqctl, 5) && m_irq3)
	{
		m_irq3 = false;
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
	}
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

TIMER_CALLBACK_MEMBER(hp_lj3p_device::tick)
{
	// the level 3 handler (c06) acknowledges with 3e0004 bit 5 (see irqctl_w)
	m_irq3 = true;
	m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
}

hp_lj3p_device::band_t hp_lj3p_device::latch_band() const
{
	return band_t{
			((u32(m_video[0] & 0x7f) << 16) | (m_video[1] & 0xfffe)) + 0x80'0000,
			u16(m_video[0] >> 8),
			s16((m_video[2] & 0x0fff) - MARGIN_OFFSET),
			u16(m_video[3] & 0x0fff),
			bool(BIT(m_video[3], 14)) };
}

void hp_lj3p_device::video_w(offs_t offset, u16 data, u16 mem_mask)
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

TIMER_CALLBACK_MEMBER(hp_lj3p_device::video_line)
{
	if (!m_cur_active)
		return;
	if (m_cur.data && m_page_y < PAGE_H)
	{
		u8 *const line = &m_page[m_page_y * PAGE_STRIDE];
		for (int w = 0; w < m_cur.words; w++)
		{
			u16 const d = m_program.read_word(m_cur.addr + w * 2);
			m_program.write_word(m_cur.addr + w * 2, 0);   // the band memory is cleared as it is read out (the firmware never clears it)
			for (int b = 0; b < 16; b++)
			{
				int const x = m_cur.x0 + w * 16 + b;
				if (BIT(d, 15 - b) && x >= 0 && x < PAGE_W)
				{
					line[x >> 3] |= 0x80 >> (x & 7);
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

void hp_lj3p_device::blt_pat_w(offs_t offset, u16 data, u16 mem_mask)
{
	// offset 0 high word, offset 1 low word
	u16 word = offset ? u16(m_blt_pat) : u16(m_blt_pat >> 16);
	COMBINE_DATA(&word);
	m_blt_pat = offset ? ((m_blt_pat & 0xffff'0000) | word) : ((m_blt_pat & 0x0000'ffff) | (u32(word) << 16));
	m_blt_patrow = 0;
}

void hp_lj3p_device::blt_dest_w(offs_t offset, u16 data, u16 mem_mask)
{
	// offset 0 high word, offset 1 low word
	u16 word = offset ? u16(m_blt_dest) : u16(m_blt_dest >> 16);
	COMBINE_DATA(&word);
	m_blt_dest = offset ? ((m_blt_dest & 0xffff'0000) | word) : ((m_blt_dest & 0x0000'ffff) | (u32(word) << 16));
	m_blt_row = m_blt_dest;
	m_blt_ptr = m_blt_row + (m_blt_ctrl >> 4) * 2;
	m_blt_spill = 0;
}

void hp_lj3p_device::blt_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
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

void hp_lj3p_device::blt_out(u16 data)
{
	if (m_blt_dest == 0xffff'ffff || m_blt_ptr < 0xb0'0000 || m_blt_ptr > 0xff'fffe)
		return;
	u16 const d = m_program.read_word(m_blt_ptr);
	u16 const pat = m_program.read_word((m_blt_pat & 0xff'fffe) + (m_blt_patrow & 15) * 2);
	// raster operation from the low byte of 3c0004, indexed by data, pattern and
	// destination bits like a ROP3 code (fa: OR the data into the page, ea: OR the
	// data masked with the shading pattern)
	u8 const rop = m_blt_mode & 0xff;
	u16 r = 0;
	for (int b = 0; b < 16; b++)
		if (BIT(rop, (BIT(data, b) << 2) | (BIT(pat, b) << 1) | BIT(d, b)))
			r |= 1 << b;
	m_program.write_word(m_blt_ptr, (d & ~m_blt_mask) | (r & m_blt_mask));
	m_blt_ptr += 2;
}

void hp_lj3p_device::blt_data_w(offs_t offset, u16 data, u16 mem_mask)
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

void hp_lj3p_device::view_dot(int x, int y)
{
	if (m_view_clear)
	{
		std::fill_n(m_view.get(), VIEW_W * VIEW_H, 0xff);
		m_view_clear = false;
	}
	if (x / 4 < VIEW_W && y / 4 < VIEW_H)
	{
		u8 &p = m_view[(y / 4) * VIEW_W + x / 4];
		p = std::max(p - 16, 0);
	}
}

void hp_lj3p_device::clear_page()
{
	std::fill_n(m_page.get(), PAGE_STRIDE * PAGE_H, 0);
	m_page_y = 0;
	m_page_dirty = false;
}

void hp_lj3p_device::save_page()
{
	// each page goes to a PNG file in the snapshot directory (300 dpi)
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = machine().video().open_next(file, "png");
	if (filerr)
		return;
	bitmap_ind16 page(PAGE_W, PAGE_H);
	for (int y = 0; y < PAGE_H; y++)
	{
		u8 const *const src = &m_page[y * PAGE_STRIDE];
		u16 *const dst = &page.pix(y);
		for (int x = 0; x < PAGE_W; x++)
			dst[x] = BIT(src[x >> 3], ~x & 7) ? 0 : 1;
	}
	static constexpr rgb_t PAPER[2] = { rgb_t::black(), rgb_t::white() };
	util::png_write_bitmap(file, nullptr, page, 2, PAPER);
	machine().popmessage("printer page saved as %s", file.filename());
}

u32 hp_lj3p_device::paper_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u8 const *const src = &m_view[y * VIEW_W];
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			dst[x] = rgb_t(src[x], src[x], src[x]);
	}
	return 0;
}

void hp_lj3p_device::eject_page()
{
	m_view_clear = true;   // the next page replaces the view
	if (m_page_dirty || m_page_y)
	{
		LOG("page complete, %d lines\n", m_page_y);
		save_page();
	}
	clear_page();
	// PRNT still active: the next sheet follows
	m_sheet = false;
	if (!BIT(m_ctrl, 13))
		m_print_timer->adjust(attotime::from_seconds(1), 0);
}

// 280000 written: control panel HD44780 in 4 bit mode, bits 0-3 D4-D7,
// bit 6 RS, bit 7 E; bit 4 Form Feed and bit 5 On Line indicators (active low)
void hp_lj3p_device::panel_w(offs_t offset, u16 data, u16 mem_mask)
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

HD44780_PIXEL_UPDATE(hp_lj3p_device::lcd_pixel_update)
{
	// two rows of eight characters (user's manual)
	if (x < 5 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(line * 9 + y, pos * 6 + x) = state;
}

void hp_lj3p_device::main_map(address_map &map)
{
	map(0x00'0000, 0xff'ffff).rw(FUNC(hp_lj3p_device::unk_r), FUNC(hp_lj3p_device::unk_w));
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x200001).rw(FUNC(hp_lj3p_device::pdata_r), FUNC(hp_lj3p_device::pctrl_w));
	map(0x240000, 0x240001).w(FUNC(hp_lj3p_device::ctrl_w));
	map(0x280000, 0x280001).rw(FUNC(hp_lj3p_device::status_r), FUNC(hp_lj3p_device::panel_w));
	map(0x300000, 0x300003).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x380000, 0x380007).w(FUNC(hp_lj3p_device::video_w));
	map(0x3a0000, 0x3a0003).w(FUNC(hp_lj3p_device::blt_pat_w));
	map(0x3a0004, 0x3a0007).w(FUNC(hp_lj3p_device::blt_dest_w));
	map(0x3c0000, 0x3c0005).w(FUNC(hp_lj3p_device::blt_ctrl_w));
	map(0x3e0000, 0x3e0003).w(FUNC(hp_lj3p_device::blt_data_w));
	map(0x3e0004, 0x3e0005).w(FUNC(hp_lj3p_device::irqctl_w));
	map(0x3e0006, 0x3e0007).rw(FUNC(hp_lj3p_device::ctrl2_r), FUNC(hp_lj3p_device::ctrl2_w));
	map(0xb00000, 0xbfffff).ram();
}

void hp_lj3p_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp_lj3p_device::main_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	// serial interface: the receive interrupt handler (b3d86) is installed on level 6
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (int state) { m_txd_cb(state); });
	m_acia->irq_handler().set_inputline(m_maincpu, M68K_IRQ_6);
	CLOCK(config, m_acia_clock, 19200);
	m_acia_clock->signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	m_acia_clock->signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	screen_device &paper(SCREEN(config, "paper"));
	paper.set_refresh_hz(10);
	paper.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	paper.set_size(VIEW_W, VIEW_H);
	paper.set_visarea_full();
	paper.set_screen_update(FUNC(hp_lj3p_device::paper_update));

	config.set_default_layout(layout_hplj3p);

	screen_device &screen(SCREEN(config, "lcd").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(8 * 6, 2 * 9 - 1);
	screen.set_visarea_full();
	screen.set_palette(m_palette);
	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	HD44780(config, m_lcdc, 270'000);   // clock not measured
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(hp_lj3p_device::lcd_pixel_update));
}

void hp_lj3p_device::host_layout(machine_config &config)
{
	// added to the computer's views; it shows all screens, so MAME selects it
	// by default
	config.set_default_layout(layout_hplj3p_host);
}
