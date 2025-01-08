// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9845_printer.h

    HP9845 internal printer HLE

*********************************************************************/

#ifndef MAME_HP_HP9845_PRINTER_H
#define MAME_HP_HP9845_PRINTER_H

#pragma once

#include "imagedev/bitbngr.h"

class hp9845_printer_device : public device_t
{
public:
	// construction/destruction
	hp9845_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq() { return m_irl_handler.bind(); }
	auto flg() { return m_flg_handler.bind(); }
	auto sts() { return m_sts_handler.bind(); }

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

	// PPU access
	uint16_t printer_r(offs_t offset);
	void printer_w(offs_t offset, uint16_t data);

private:
	devcb_write_line m_irl_handler;
	devcb_write_line m_flg_handler;
	devcb_write_line m_sts_handler;

	required_device<bitbanger_device> m_prt_graph_out;
	required_device<bitbanger_device> m_prt_alpha_out;
	required_region_ptr<uint8_t> m_prt_chargen;

	emu_timer *m_timer;

	// Size of various buffers
	static constexpr unsigned REDEF_CH_COUNT = 9;
	static constexpr unsigned REDEF_BUFF_LEN = 77;

	// State
	bool m_display_mode;
	bool m_shifted;
	bool m_current_u_l;
	bool m_current_big;
	bool m_ibf;
	bool m_inten;
	bool m_busy;
	uint8_t m_ib;
	uint8_t m_pos;
	uint8_t m_line[ 80 ];
	uint8_t m_attrs[ 80 ];
	uint8_t m_redef_count;
	uint8_t m_redef_idx;
	uint8_t m_redef_chars[ REDEF_CH_COUNT ];
	uint8_t m_replace_count;
	uint8_t m_redef_buff[ REDEF_BUFF_LEN ];
	uint8_t m_next_replace;
	uint8_t m_rep_str_len;
	uint8_t m_rep_str_ptr;
	uint8_t m_octal_accum;
	int m_fsm_state;
	unsigned m_cur_line;

	// FSM states
	enum {
		FSM_NORMAL_TEXT,
		FSM_AFTER_CR,
		FSM_AFTER_ESC,
		FSM_AFTER_ESC_AMP,
		FSM_COLLECT_ESC_QMARK,
		FSM_AFTER_ESC_AMP_K,
		FSM_AFTER_ESC_AMP_K_01,
		FSM_AFTER_ESC_AMP_D,
		FSM_AFTER_ESC_AMP_N,
		FSM_AFTER_ESC_AMP_O,
		FSM_AFTER_ESC_AMP_O_C,
		FSM_AFTER_ESC_AMP_O_C_L,
		FSM_AFTER_ESC_AMP_L,
		FSM_AFTER_ESC_AMP_N_C,
		FSM_WAIT_ESC_Z
	};

	void state_reset();
	void insert_char(uint8_t ch);
	void start_new_line();
	static uint8_t get_ch_matrix_line(const uint8_t *matrix_base , unsigned line_no , const uint8_t *seq);
	unsigned print_560_pixels(unsigned line_no , const uint8_t *pixels);
	uint8_t get_ch_pixels(uint8_t ch , uint8_t attrs , unsigned matrix_line) const;
	static attotime burn_time(unsigned pixel_count);
	void print_line();
	void print_graphic_line();
	void crlf();
	void set_tab();
	void clear_tabs();
	void move_to_next_tab();
	void update_flg();
	bool is_ch_redef(uint8_t ch , unsigned& redef_number) const;
	uint8_t allocate_ch_redef(uint8_t& idx);
	bool is_ch_replaced(uint8_t ch , uint8_t& len , uint8_t& ptr) const;
	uint8_t free_redef_space() const;
	uint8_t apply_shifting(uint8_t ch) const;
	bool parse_octal(uint8_t ch);
	bool parse_ch(uint8_t ch);
	void update_fsm();
};

// device type definition
DECLARE_DEVICE_TYPE(HP9845_PRINTER, hp9845_printer_device)

#endif // MAME_HP_HP9845_PRINTER_H
