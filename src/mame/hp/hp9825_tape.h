// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9825_tape.h

    HP9825 tape sub-system

*********************************************************************/

#ifndef MAME_HP_HP9825_TAPE_H
#define MAME_HP_HP9825_TAPE_H

#pragma once

#include "formats/hti_tape.h"
#include "machine/hp_dc100_tape.h"
#include "machine/74123.h"

class hp9825_tape_device : public device_t
{
public:
	// construction/destruction
	hp9825_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t tape_r(offs_t offset);
	void tape_w(offs_t offset, uint16_t data);

	auto flg() { return m_flg_handler.bind(); }
	auto sts() { return m_sts_handler.bind(); }
	auto dmar() { return m_dmar_handler.bind(); }
	auto led() { return m_led_handler.bind(); }
	auto cart_in() { return m_cart_in_handler.bind(); }

	void short_gap_w(int state);
	void long_gap_w(int state);

	void cart_out_w(int state);
	void hole_w(int state);
	void tacho_tick_w(int state);
	void motion_w(int state);
	void rd_bit_w(int state);
	int wr_bit_r();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_flg_handler;
	devcb_write_line m_sts_handler;
	devcb_write_line m_dmar_handler;
	devcb_write_line m_led_handler;
	devcb_write_line m_cart_in_handler;

	required_device<hp_dc100_tape_device> m_tape;
	required_device<ttl74123_device> m_short_gap_timer; // U43a
	required_device<ttl74123_device> m_long_gap_timer;  // U43b

	// Registers
	uint8_t m_cmd_reg;
	uint8_t m_stat_reg;

	// State
	bool m_flg;
	bool m_sts;
	bool m_data_out;    // U38-9
	bool m_data_in; // U13-6
	bool m_exception;   // U4-6
	bool m_search_complete; // U9-6
	bool m_dma_req; // U9-9
	bool m_in_gap;  // U39-4
	bool m_no_go;   // U6-3
	bool m_valid_bits;  // U39-5
	uint8_t m_trans_cnt;    // U42
	bool m_short_gap_out;   // U43-13
	bool m_long_gap_out;    // U43-5

	void clear_state();
	void set_flg(bool state);
	void update_sts();
	void update_dmar();
	bool is_moving_fwd() const;
	bool is_speed_fast() const;
	void check_for_speed_change();
	void start_rd_wr(bool recalc = false);
};

// device type definition
DECLARE_DEVICE_TYPE(HP9825_TAPE, hp9825_tape_device)

#endif // MAME_HP_HP9825_TAPE_H
