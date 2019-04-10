// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9825_tape.h

    HP9825 tape sub-system

*********************************************************************/

#ifndef MAME_MACHINE_HP9825_TAPE_H
#define MAME_MACHINE_HP9825_TAPE_H

#pragma once

#include "formats/hti_tape.h"
#include "machine/74123.h"

class hp9825_tape_device : public device_t , public device_image_interface
{
public:
	// construction/destruction
	hp9825_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
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

	DECLARE_READ16_MEMBER(tape_r);
	DECLARE_WRITE16_MEMBER(tape_w);

	auto flg() { return m_flg_handler.bind(); }
	auto sts() { return m_sts_handler.bind(); }
	auto dmar() { return m_dmar_handler.bind(); }
	auto led() { return m_led_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER(short_gap_w);
	DECLARE_WRITE_LINE_MEMBER(long_gap_w);

private:
	devcb_write_line m_flg_handler;
	devcb_write_line m_sts_handler;
	devcb_write_line m_dmar_handler;
	devcb_write_line m_led_handler;

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
	bool m_present;
	bool m_valid_bits;  // U39-5
	uint8_t m_trans_cnt;    // U42
	bool m_short_gap_out;   // U43-13
	bool m_long_gap_out;    // U43-5

	// Timers
	emu_timer *m_bit_timer;
	emu_timer *m_tacho_timer;
	emu_timer *m_hole_timer;
	emu_timer *m_inv_timer;

	// Image of tape
	hti_format_t m_image;
	bool m_image_dirty;

	// Tape motion
	hti_format_t::tape_pos_t m_tape_pos;
	hti_format_t::tape_pos_t m_next_bit_pos;
	hti_format_t::tape_pos_t m_next_tacho_pos;
	hti_format_t::tape_pos_t m_next_hole_pos;
	double m_speed;
	attotime m_start_time;  // Tape moving if != never
	bool m_accelerating;

	// R/W
	enum {
		RW_IDLE,
		RW_READING,
		RW_WRITING,
		RW_WRITING_GAP
	};

	int m_rw_stat;
	hti_format_t::track_iterator_t m_rd_it;
	bool m_rd_it_valid;
	uint32_t m_rw_word; // Need 17 bits because of sync bit
	unsigned m_bit_idx; // 0 is MSB, 15 is LSB, 16 is sync bit, >16 means "looking for sync"
	hti_format_t::tape_pos_t m_gap_start;

	void clear_state();
	image_init_result internal_load(bool is_create);
	void set_flg(bool state);
	void update_sts();
	void update_dmar();
	void set_tape_present(bool present);
	bool is_moving_fwd() const;
	bool is_speed_fast() const;
	bool is_actual_dir_fwd() const;
	double get_speed_set_point() const;
	void start_tape();
	void stop_tape();
	void check_for_speed_change(double prev_set_point);
	void update_speed_pos();
	void set_inv_timer();
	void time_to_distance(hti_format_t::tape_pos_t distance , hti_format_t::tape_pos_t& target_pos , emu_timer *target_timer) const;
	double const_a_space(double a , double t) const;
	hti_format_t::tape_pos_t get_next_hole() const;
	void adjust_tacho_timer();
	void adjust_hole_timer();
	unsigned current_track() const;
	static constexpr hti_format_t::tape_pos_t bit_size(bool bit);
	void start_rd_wr(bool recalc = false);
	void load_rd_word();
	void rd_bit(bool bit);
	void wr_bit(bool bit);
	void stop_rd_wr();
};

// device type definition
DECLARE_DEVICE_TYPE(HP9825_TAPE, hp9825_tape_device)

#endif // MAME_MACHINE_HP9825_TAPE_H
