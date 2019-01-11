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

class hp_taco_device : public device_t ,
						public device_image_interface
{
public:
	// construction/destruction
	hp_taco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }
	auto flg() { return m_flg_handler.bind(); }
	auto sts() { return m_sts_handler.bind(); }

	// Register read/write
	DECLARE_WRITE16_MEMBER(reg_w);
	DECLARE_READ16_MEMBER(reg_r);

	// Flag & status read
	DECLARE_READ_LINE_MEMBER(flg_r);
	DECLARE_READ_LINE_MEMBER(sts_r);

		// device_image_interface overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;
	virtual iodevice_t image_type() const override { return IO_MAGTAPE; }
	virtual bool is_readable() const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override;

protected:
	hp_taco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_flg_handler;
	devcb_write_line m_sts_handler;

	// Registers
	uint16_t m_data_reg;
	bool m_data_reg_full;
	uint16_t m_cmd_reg;
	uint16_t m_status_reg;
	uint16_t m_tach_reg;
	hti_format_t::tape_pos_t m_tach_reg_ref;
	bool m_tach_reg_frozen;
	uint16_t m_checksum_reg;
	bool m_clear_checksum_reg;
	uint16_t m_timing_reg;

	// State
	bool m_irq;
	bool m_flg;
	bool m_sts;

	// Command FSM state
	typedef enum {
		CMD_IDLE,
		CMD_INVERTING,
		CMD_PH0,
		CMD_PH1,
		CMD_PH2,
		CMD_END,
		CMD_STOPPING
	} cmd_state_t;
	cmd_state_t m_cmd_state;

	// Tape position & motion
	hti_format_t::tape_pos_t m_tape_pos;
	attotime m_start_time;  // Tape moving if != never
	bool m_tape_fwd;
	bool m_tape_fast;

	// Timers
	emu_timer *m_tape_timer;
	emu_timer *m_hole_timer;
	emu_timer *m_timeout_timer;

	// Content of tape
	hti_format_t m_image;
	bool m_image_dirty;

	// Reading & writing
	bool m_tape_wr;
	hti_format_t::tape_pos_t m_rw_pos;
	uint16_t m_next_word;
	hti_format_t::track_iterator_t m_rd_it;
	bool m_rd_it_valid;

	// Gap detection
	hti_format_t::tape_pos_t m_gap_detect_start;

	void clear_state(void);
	void irq_w(bool state);
	void set_error(bool state);
	bool is_braking(void) const;
	unsigned speed_to_tick_freq(void) const;
	hti_format_t::tape_pos_t current_tape_pos(void) const;
	void update_tape_pos(void);
	void update_tach_reg(void);
	void freeze_tach_reg(bool freeze);
	attotime time_to_distance(hti_format_t::tape_pos_t distance) const;
	attotime time_to_target(hti_format_t::tape_pos_t target) const;
	attotime time_to_stopping_pos(void) const;
	bool start_tape_cmd(uint16_t cmd_reg , uint16_t must_be_1 , uint16_t must_be_0);
	void stop_tape(void);
	unsigned current_track(void);
	attotime fetch_next_wr_word(void);
	attotime time_to_rd_next_word(hti_format_t::tape_pos_t& word_rd_pos);
	hti_format_t::tape_pos_t min_gap_size(void) const;
	void set_tape_present(bool present);
	attotime time_to_next_hole(void) const;
	attotime time_to_tach_pulses(void) const;
	void terminate_cmd_now(void);
	void set_data_timeout(bool long_timeout);
	void cmd_fsm(void);
	void start_cmd_exec(uint16_t new_cmd_reg);
	image_init_result internal_load(bool is_create);
};

// device type definition
DECLARE_DEVICE_TYPE(HP_TACO, hp_taco_device)

#endif // MAME_MACHINE_HP_TACO_H
