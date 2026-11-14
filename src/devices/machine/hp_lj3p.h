// license:BSD-3-Clause
// copyright-holders:rfka01
/***************************************************************************

    HP LaserJet IIIP laser printer

    The printer with its Centronics parallel and RS-232C serial interfaces;
    see bus/centronics/hp_lj3p.h and bus/rs232/hp_lj3p.h for the devices
    that plug into a computer's ports, and hp/laserjet3p.cpp for the
    printer on its own.

***************************************************************************/

#ifndef MAME_MACHINE_HP_LJ3P_H
#define MAME_MACHINE_HP_LJ3P_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/eepromser.h"
#include "video/hd44780.h"

#include "screen.h"


class hp_lj3p_device : public device_t
{
public:
	hp_lj3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// parallel interface: data, /STROBE and /INIT from the computer, the
	// status lines to it
	void data_w(u8 data) { m_data = data; }
	void strobe_w(int state);
	void init_w(int state);
	auto busy_cb() { return m_busy_cb.bind(); }
	auto ack_cb() { return m_ack_cb.bind(); }
	auto perror_cb() { return m_perror_cb.bind(); }
	auto fault_cb() { return m_fault_cb.bind(); }
	auto select_cb() { return m_select_cb.bind(); }

	// serial interface: data from the computer arrives on the printer's RxD,
	// TxD and DTR go to the computer
	void rxd_w(int state);
	auto txd_cb() { return m_txd_cb.bind(); }
	auto dtr_cb() { return m_dtr_cb.bind(); }

	// default layout for a port device that holds the printer as "printer":
	// the computer's first screen, the paper and the control panel
	static void host_layout(machine_config &config) ATTR_COLD;

protected:

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	devcb_write_line m_busy_cb;
	devcb_write_line m_ack_cb;
	devcb_write_line m_perror_cb;
	devcb_write_line m_fault_cb;
	devcb_write_line m_select_cb;
	devcb_write_line m_txd_cb;
	devcb_write_line m_dtr_cb;

	u8 m_data;              // Centronics data lines

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
	// bit 7 ACK pulse, bit 6 PERROR (set without paper and while off line),
	// bit 5 /FAULT, bit 4 set while off line or initialising (SELECT low),
	// bit 3 DTR through an inverting driver (the firmware applies DTR
	// POLARITY: with HI it is clear while on line, set while initialising
	// and off line), bits 0 and 1 not known (bit 0 pulses while the serial
	// port is set up)
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

DECLARE_DEVICE_TYPE(HP_LJ3P, hp_lj3p_device)

#endif // MAME_MACHINE_HP_LJ3P_H
