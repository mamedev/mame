// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp2640_tape.h

    Tape subsystem of HP264x terminals

*********************************************************************/

#ifndef MAME_HP_HP2640_TAPE_H
#define MAME_HP_HP2640_TAPE_H

#pragma once

#include "machine/hp_dc100_tape.h"

class hp2640_tape_device : public device_t
{
public:
	// construction/destruction
	hp2640_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq() { return m_irq_handler.bind(); }
	auto led_0() { return m_led0_handler.bind(); }
	auto led_1() { return m_led1_handler.bind(); }

	// I/O
	void    command_w(uint8_t cmd);
	uint8_t status_r ();
	void    data_w   (uint8_t data);
	uint8_t data_r   ();
	uint8_t poll_r   () const;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(gap_timer_tick);
	TIMER_CALLBACK_MEMBER(cell_timer_tick);

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_led0_handler;
	devcb_write_line m_led1_handler;

	// 0 is left hand drive
	// 1 is right hand drive
	required_device_array<hp_dc100_tape_device , 2> m_drives;

	// State
	uint8_t m_selected_drive;   // U35-10
	uint8_t m_cmd_reg;          // U35 & others
	uint8_t m_data_rd;          // U32 & U33
	uint8_t m_data_sr;          // U42 & U43
	uint8_t m_modulus;          // U49 & U59
	uint8_t m_cell_cnt;         // U210
	uint8_t m_bit_cnt;          // U310
	bool m_tach_div2;           // U19-6
	bool m_tach_latch;          // U21-10
	bool m_hole_latch;          // U21-6
	bool m_byte_ready;          // U19-9
	bool m_irq;                 // U48-8
	bool m_bit_sync;            // U110-6
	bool m_wr_bit;              // U110-10
	bool m_last_rd_bit;
	bool m_isf;                 // U36-6
	bool m_gap;
	bool m_prev_gap;
	bool m_rip;
	hp_dc100_tape_device::tape_op_t m_current_op;

	// Timers
	emu_timer *m_gap_timer;
	emu_timer *m_cell_timer;

	void hole_w(unsigned drive , int state);
	void tacho_tick_w(unsigned drive , int state);
	void motion_w(unsigned drive , int state);
	void rd_bit_w(unsigned drive , int state);
	int wr_bit_r(unsigned drive);
	void update_irq();
	bool set_speed();
	void start_rd_wr(bool recalc = false);
	void load_modulus();
	void restart_cell_cnt();
	void stop_cell_cnt();
	void set_gap();
	void load_gap_timer();
};

// device type definition
DECLARE_DEVICE_TYPE(HP2640_TAPE, hp2640_tape_device)

#endif /* MAME_HP_HP2640_TAPE_H */
