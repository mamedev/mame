// license:BSD-3-Clause
// copyright-holders: 68bit
/****************************************************************************

Motorola EXORterm 155 (M68SDS)

****************************************************************************/

#ifndef MAME_MACHINE_EXORTERM_H
#define MAME_MACHINE_EXORTERM_H

#pragma once

#include "cpu/m6800/m6800.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mc14411.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"
#include "sound/spkrdev.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"

#include "machine/terminal.h"


INPUT_PORTS_EXTERN(exorterm155);

class exorterm155_device : public device_t
{
public:
	exorterm155_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// Interface to a RS232 connection.
	auto rs232_conn_txd_handler() { return m_rs232_conn_txd_handler.bind(); }
	auto rs232_conn_dtr_handler() { return m_rs232_conn_dtr_handler.bind(); }
	auto rs232_conn_rts_handler() { return m_rs232_conn_rts_handler.bind(); }
	void rs232_conn_dcd_w(int state);
	void rs232_conn_dsr_w(int state);
	void rs232_conn_ri_w(int state);
	void rs232_conn_cts_w(int state);
	void rs232_conn_rxd_w(int state);

protected:
	exorterm155_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<m6800_cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<acia6850_device> m_acia;
	required_device<mc14411_device> m_brg;
	required_ioport m_rs232_baud;
	required_ioport m_disable_fac;
	required_ioport m_display_fac;
	required_device<pia6821_device> m_pia_kbd;
	required_device<pia6821_device> m_pia_cfg;
	required_device<pia6821_device> m_pia_disp;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<clock_device> m_sys_timer_clock;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_chargen;
	required_device<beep_device> m_beeper;
	output_finder<> m_online_led;
	output_finder<> m_auto_lf_led;
	output_finder<> m_page_mode_led;
	output_finder<> m_insert_char_led;

	void mem_map(address_map &map) ATTR_COLD;

	void acia_txd_w(int state);
	void acia_rts_w(int state);

	// Clocks
	void write_f1_clock(int state);
	void write_f3_clock(int state);
	void write_f5_clock(int state);
	void write_f6_clock(int state);
	void write_f7_clock(int state);
	void write_f8_clock(int state);
	void write_f9_clock(int state);
	void write_f11_clock(int state);
	void write_f13_clock(int state);

	u8 m_dsr;

	devcb_write_line m_rs232_conn_txd_handler;
	devcb_write_line m_rs232_conn_dtr_handler;
	devcb_write_line m_rs232_conn_rts_handler;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u8 m_framecnt;
	u16 m_cursor_addr;
	u8 m_inv_video;
	u8 m_special_char_disp;

	void sys_timer_w(int state);
	u8 m_sys_timer_count;

	void pia_cfg_cb2_w(int state);

	u8 pia_kbd_pa_r();
	u8 pia_kbd_pb_r();
	void pia_kbd_pb_w(u8 data);
	u8 m_term_data;

	u8 pia_disp_pb_r();
	void pia_disp_pa_w(u8 data);
	void pia_disp_pb_w(u8 data);

	u8 m_kbd_start_holdoff;

	// Keyboard

	static u8 const translation_table[][5][17];
	static bool const caps_table[5][17];
	void kbd_start_processing();
	void kbd_reset_state();
	void kbd_repeat_start(u8 row, u8 column);
	void kbd_repeat_restart();
	void kbd_repeat_stop();
	void kbd_send_translated(u8 code);

	TIMER_CALLBACK_MEMBER(kbd_scan_row);
	TIMER_CALLBACK_MEMBER(kbd_repeat);

	emu_timer       *m_kbd_scan_timer;
	emu_timer       *m_kbd_repeat_timer;
	required_ioport m_kbd_modifiers;
	required_ioport_array<5> m_kbd_rows;
	ioport_value    m_kbd_states[5];
	u8              m_kbd_next_row;
	u8              m_kbd_processing;
	u8              m_kbd_repeat_row;
	u8              m_kbd_repeat_column;
	u16             m_kbd_last_modifiers;

};

DECLARE_DEVICE_TYPE(EXORTERM155, exorterm155_device)

#endif // MAME_MACHINE_EXORTERM_H
