// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    1ma6.h

    HP-85 tape controller (1MA6-0001)

*********************************************************************/

#ifndef MAME_MACHINE_1MA6_H
#define MAME_MACHINE_1MA6_H

#pragma once

#include "formats/hti_tape.h"

class hp_1ma6_device : public device_t ,
					   public device_image_interface
{
public:
	// construction/destruction
	hp_1ma6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Register read/write
	DECLARE_WRITE8_MEMBER(reg_w);
	DECLARE_READ8_MEMBER(reg_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_image_interface overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;
	virtual iodevice_t image_type()  const override { return IO_MAGTAPE; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override;

private:
	// Registers
	uint8_t m_data_reg;
	uint8_t m_status_reg;
	uint8_t m_control_reg;

	// Tape position & motion
	hti_format_t::tape_pos_t m_tape_pos;
	hti_format_t::tape_pos_t m_prev_pos;
	attotime m_start_time;  // Tape moving if != never

	// Timers
	emu_timer *m_tape_timer;
	emu_timer *m_hole_timer;

	// Content of tape
	hti_format_t m_image;
	bool m_image_dirty;

	// Reading & writing
	hti_format_t::tape_pos_t m_rw_pos;
	hti_format_t::tape_word_t m_next_word;
	hti_format_t::track_iterator_t m_rd_it;
	bool m_rd_it_valid;

	// Gap detection
	hti_format_t::tape_pos_t m_gap_detect_start;

	// Command FSM state
	typedef enum {
		CMD_IDLE,
		CMD_STOPPING,
		CMD_RD_WAIT_SYNC,
		CMD_RD_MSB,
		CMD_RD_LSB,
		CMD_WR_SYNC,
		CMD_WR_MSB,
		CMD_WR_LSB,
		CMD_WR_GAP,
		CMD_FAST_FWD_REV
	} cmd_state_t;
	cmd_state_t m_cmd_state;

	// Cartridge-in status
	bool m_cartridge_in;

	void clear_state();
	unsigned current_track() const;
	unsigned speed_to_tick_freq(void) const;
	attotime time_to_distance(hti_format_t::tape_pos_t distance) const;
	attotime time_to_target(hti_format_t::tape_pos_t target) const;
	attotime time_to_next_hole(void) const;
	attotime time_to_rd_next_word(hti_format_t::tape_pos_t& word_rd_pos) const;
	hti_format_t::tape_pos_t current_tape_pos() const;
	void update_tape_pos();
	bool is_reading() const;
	bool is_writing() const;
	bool is_moving_fwd() const;
	void set_cartridge_in(bool in);
	void start_tape();
	void stop_tape();
	attotime advance_rd_state();
	void start_cmd_exec(uint8_t new_ctl_reg);
	image_init_result internal_load(bool is_create);
};

// device type definition
DECLARE_DEVICE_TYPE(HP_1MA6, hp_1ma6_device)

#endif /* MAME_MACHINE_1MA6_H */
