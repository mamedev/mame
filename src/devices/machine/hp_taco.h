// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_taco.h

    HP TApe COntroller (5006-3012)

*********************************************************************/

#ifndef MAME_MACHINE_HP_TACO_H
#define MAME_MACHINE_HP_TACO_H

#pragma once

#include "formats/hti_tape.h"
#include "machine/hp_dc100_tape.h"

class hp_taco_device : public device_t
{
public:
	// construction/destruction
	hp_taco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }
	auto flg() { return m_flg_handler.bind(); }
	auto sts() { return m_sts_handler.bind(); }

	// Set unit name
	void set_name(const std::string& name);

	// Register read/write
	void reg_w(offs_t offset, uint16_t data);
	uint16_t reg_r(offs_t offset);

	// Flag & status read
	int flg_r();
	int sts_r();

	void cart_out_w(int state);
	void hole_w(int state);
	void tacho_tick_w(int state);
	void motion_w(int state);
	void rd_bit_w(int state);
	int wr_bit_r();

protected:
	hp_taco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(gap_timer_tick);
	TIMER_CALLBACK_MEMBER(evd_timer_tick);
	TIMER_CALLBACK_MEMBER(error_timer_tick);

private:
	required_device<hp_dc100_tape_device> m_tape;

	devcb_write_line m_irq_handler;
	devcb_write_line m_flg_handler;
	devcb_write_line m_sts_handler;

	// Registers
	uint16_t m_data_reg;
	uint16_t m_cmd_reg;
	uint16_t m_status_reg;
	uint16_t m_tach_reg;
	uint16_t m_checksum_reg;
	uint16_t m_threshold_reg;

	// State
	bool m_irq;
	bool m_flg;
	bool m_sts;
	bool m_error;
	bool m_gap_in_read;

	// Command FSM state
	typedef enum {
		CMD_IDLE,
		CMD_PH0,
		CMD_PH1,
		CMD_PH2,
		CMD_STOPPING
	} cmd_state_t;
	cmd_state_t m_cmd_state;

	// Timers
	emu_timer *m_gap_timer;
	emu_timer *m_evd_timer;
	emu_timer *m_error_timer;

	// Reading & writing
	uint16_t m_working_reg;
	unsigned m_bit_idx;

	void clear_state();
	void irq_w(bool state);
	void sts_w(bool state);
	void set_error(bool error , bool gap_in_read);
	hti_format_t::tape_pos_t min_gap_size() const;
	void set_gap_timer();
	void set_evd_timer();
	void set_tape_present(bool present);
	void send_go();
	void send_stop();
	void end_cmd();
	void irq_and_end();
	bool is_at_slow_speed() const;
	void start_rd();
	void start_wr();
	bool adv_bit_idx();
	void update_checksum(uint16_t data);
	void cmd_fsm();
	static uint8_t get_cmd(uint16_t cmd_reg);
	static bool is_cmd_rd_wr(uint16_t cmd_reg);
	static bool is_cmd_rd(uint16_t cmd_reg);
	static bool is_cmd_wr(uint16_t cmd_reg);
	static bool is_double_hole_cmd(uint16_t cmd_reg);
	void start_cmd_exec(uint16_t new_cmd_reg);
};

// device type definition
DECLARE_DEVICE_TYPE(HP_TACO, hp_taco_device)

#endif // MAME_MACHINE_HP_TACO_H
