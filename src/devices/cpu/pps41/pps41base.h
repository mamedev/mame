// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell PPS-4/1 MCU cores

  Don't include this file, include the specific device header instead,
  for example mm76.h

*/

#ifndef MAME_CPU_PPS41_PPS41BASE_H
#define MAME_CPU_PPS41_PPS41BASE_H

#pragma once

#include "machine/pla.h"

enum
{
	PPS41_INPUT_LINE_INT0 = 0,
	PPS41_INPUT_LINE_INT1
};


class pps41_base_device : public cpu_device
{
public:
	// configuration helpers
	// I/O ports:

	// 8-bit P(parallel) input
	auto read_p() { return m_read_p.bind(); }

	// 10/12-bit D(discrete) I/O
	auto read_d() { return m_read_d.bind(); }
	auto write_d() { return m_write_d.bind(); }

	// 8/10/14-bit R I/O
	auto read_r() { return m_read_r.bind(); }
	auto write_r() { return m_write_r.bind(); }

	// serial data/clock
	auto read_sdi() { return m_read_sdi.bind(); }
	auto write_sdo() { return m_write_sdo.bind(); }
	auto write_ssc() { return m_write_ssc.bind(); }

	// speaker output
	auto write_spk() { return m_write_spk.bind(); }

	// I/O access
	u16 d_output_r() { return m_d_output; }
	u16 r_output_r() { return m_r_output; }
	int sdo_r() { return BIT(m_s, 3); }
	void ssc_w(int state);

protected:
	// construction/destruction
	pps41_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;
	virtual void execute_one() = 0;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_icount;
	int m_state_count;

	// fixed settings or mask options
	int m_prgwidth; // ROM/RAM address size
	int m_datawidth; // "
	u16 m_prgmask; // "
	u16 m_datamask; // "

	optional_device<pla_device> m_opla; // segment output PLA

	// i/o handlers
	devcb_read8 m_read_p;
	devcb_read16 m_read_d;
	devcb_write16 m_write_d;
	devcb_read16 m_read_r;
	devcb_write16 m_write_r;
	devcb_read_line m_read_sdi;
	devcb_write_line m_write_sdo;
	devcb_write_line m_write_ssc;
	devcb_write8 m_write_spk;

	// internal state, regs
	u16 m_pc;
	u16 m_prev_pc;
	u8 m_op;
	u8 m_prev_op;
	u8 m_prev2_op;
	u8 m_prev3_op;
	int m_stack_levels;
	u16 m_stack[2]; // max 2

	u8 m_a;
	u8 m_b;
	u8 m_prev_b;
	u8 m_prev2_b;
	u8 m_ram_addr;
	bool m_ram_delay;
	bool m_sag;
	int m_c;
	int m_prev_c;
	int m_c_in;
	bool m_c_delay;
	u8 m_x;
	bool m_skip;
	int m_skip_count;

	u8 m_s;
	int m_sclock_in;
	int m_sclock_count;

	int m_d_pins;
	u16 m_d_mask;
	u16 m_d_output;
	int m_r_pins;
	u16 m_r_mask;
	u16 m_r_output;
	int m_int_line[2];
	int m_int_ff[2];

	// misc handlers
	void set_d_pins(u8 p) { m_d_pins = p; m_d_mask = (1 << p) - 1; }
	void set_r_pins(u8 p) { m_r_pins = p; m_r_mask = (1 << p) - 1; }
	virtual bool op_is_tr(u8 op) = 0;

	void serial_shift(int state);
	void serial_clock();
	virtual void cycle();
	void increment_pc();
};


#endif // MAME_CPU_PPS41_PPS41BASE_H
