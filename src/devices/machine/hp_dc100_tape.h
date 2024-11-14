// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp_dc100_tape.h

    HP DC100 tape cartridge & drive

*********************************************************************/

#ifndef MAME_MACHINE_HP_DC100_TAPE_H
#define MAME_MACHINE_HP_DC100_TAPE_H

#pragma once

#include "imagedev/magtape.h"

#include "formats/hti_tape.h"

class hp_dc100_tape_device : public microtape_image_device
{
public:
	// Construction
	hp_dc100_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;
	virtual const char *file_extensions() const noexcept override;

	// **** Units ****
	// Tape position or distance    1/(968 * 1024) in (see hti_format_t::tape_pos_t)
	// Speed                        in/s
	// Acceleration                 in/s^2

	// Settings
	void set_acceleration(double accel);
	void set_set_points(double slow_sp , double fast_sp);
	void set_tick_size(hti_format_t::tape_pos_t size);
	void set_image_format(hti_format_t::image_format_t fmt);
	void set_go_threshold(double threshold);
	void set_name(const std::string& name);

	// Commands
	void set_track_no(unsigned track);

	enum tape_speed_t {
		SP_STOP,    // Stop
		SP_SLOW,    // Slow speed
		SP_FAST     // Fast speed
	};
	bool set_speed_setpoint(tape_speed_t speed , bool fwd);

	enum tape_op_t {
		OP_IDLE,    // No R/W
		OP_READ,    // Read from tape
		OP_WRITE,   // Write to tape
		OP_ERASE    // Erase tape
	};
	void set_op(tape_op_t op , bool force = false);

	void update_speed_pos();

	// State access
	hti_format_t::tape_pos_t get_pos() const { return m_tape_pos; }
	hti_format_t::tape_pos_t get_approx_pos() const;
	double get_speed() const { return m_speed; }
	double get_speed_setpoint() const { return m_set_point; }
	unsigned get_track_no() const { return m_track; }
	tape_op_t get_op() const { return m_current_op; }
	bool has_started() const { return !m_start_time.is_never(); }
	bool is_moving() const { return fabs(m_speed) >= m_go_threshold; }
	bool is_accelerating() const { return m_accelerating; }
	bool is_moving_fwd() const { return m_speed == 0.0 ? m_set_point >= 0.0 : m_speed > 0.0; }
	bool cart_out_r() const { return !m_present; }
	bool wpr_r() const { return !m_present || is_readonly(); }
	bool is_above_threshold() const { return fabs(m_speed) >= m_slow_set_point; }
	bool gap_reached(hti_format_t::tape_pos_t min_gap_size);

	void time_to_next_gap(hti_format_t::tape_pos_t min_gap_size , bool new_gap , emu_timer *target_timer);

	// Callbacks
	// Cartridge is out (1)
	auto cart_out() { return m_cart_out_handler.bind(); }
	// On a hole (1)
	auto hole() { return m_hole_handler.bind(); }
	// Tachometer tick (1)
	auto tacho_tick() { return m_tacho_tick_handler.bind(); }
	// Motion event (1)
	// One of the following:
	// 1. "In motion" threshold crossed
	// 2. Slow speed threshold crossed
	// 3. Accelerated phase ended (== set point reached)
	auto motion_event() { return m_motion_handler.bind(); }
	// Read bit
	auto rd_bit() { return m_rd_bit_handler.bind(); }
	// Bit to write
	auto wr_bit() { return m_wr_bit_handler.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bit_timer_tick);
	TIMER_CALLBACK_MEMBER(tacho_timer_tick);
	TIMER_CALLBACK_MEMBER(hole_timer_tick);
	TIMER_CALLBACK_MEMBER(motion_timer_tick);

private:
	devcb_write_line m_cart_out_handler;
	devcb_write_line m_hole_handler;
	devcb_write_line m_tacho_tick_handler;
	devcb_write_line m_motion_handler;
	devcb_write_line m_rd_bit_handler;
	devcb_read_line m_wr_bit_handler;

	// Settings
	double m_acceleration;
	double m_slow_set_point;
	double m_fast_set_point;
	hti_format_t::tape_pos_t m_tick_size;
	double m_go_threshold;
	std::string m_unit_name;

	// State
	hti_format_t::tape_pos_t m_tape_pos;
	tape_speed_t m_tape_speed;
	double m_set_point;
	double m_speed;
	attotime m_start_time;  // Tape moving if != never
	bool m_accelerating;
	unsigned m_track;
	tape_op_t m_current_op;
	bool m_in_set_op;
	bool m_present;
	hti_format_t::track_iterator_t m_rd_it;
	bool m_rd_it_valid;
	hti_format_t::tape_word_t m_rw_word;
	int m_bit_idx;
	hti_format_t::tape_pos_t m_rw_pos;  // Start position of current R/W word or start position of gap when erasing
	hti_format_t::tape_pos_t m_gap_detect_start;

	// Timers
	emu_timer *m_bit_timer;
	emu_timer *m_tacho_timer;
	emu_timer *m_hole_timer;
	emu_timer *m_motion_timer;
	hti_format_t::tape_pos_t m_next_bit_pos;
	hti_format_t::tape_pos_t m_next_tacho_pos;
	hti_format_t::tape_pos_t m_next_hole_pos;

	// Image of tape
	hti_format_t m_image;
	bool m_image_dirty;

	void clear_state();
	std::pair<std::error_condition, std::string> internal_load(bool is_create);
	void set_tape_present(bool present);
	double compute_set_point(tape_speed_t speed , bool fwd) const;
	void start_tape();
	void stop_tape();
	double const_a_space(double a , double t) const;
	attotime time_to_threshold(double threshold , bool zero_allowed) const;
	void set_motion_timer();
	void time_to_distance(hti_format_t::tape_pos_t distance , hti_format_t::tape_pos_t& target_pos , emu_timer *target_timer) const;
	void adjust_tacho_timer();
	void adjust_hole_timer();
	void stop_op();
	void load_rd_word();
	void store_wr_word();
};

// device type definition
DECLARE_DEVICE_TYPE(HP_DC100_TAPE, hp_dc100_tape_device)

#endif // MAME_MACHINE_HP_DC100_TAPE_H
