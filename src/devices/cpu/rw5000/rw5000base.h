// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A/B5000 family MCU cores

  Don't include this file, include the specific device header instead,
  for example b5000.h

*/

#ifndef MAME_CPU_RW5000_RW5000BASE_H
#define MAME_CPU_RW5000_RW5000BASE_H

#pragma once


class rw5000_base_device : public cpu_device
{
public:
	// configuration helpers
	// I/O ports:

	// 4-bit KB inputs
	auto read_kb() { return m_read_kb.bind(); }

	// 1-4 DIN inputs
	auto read_din() { return m_read_din.bind(); }

	// 9(possibly more) strobe outputs
	auto write_str() { return m_write_str.bind(); }

	// 7/8/10 segment outputs
	auto write_seg() { return m_write_seg.bind(); }

	// speaker output line (aka SEG0)
	auto write_spk() { return m_write_spk.bind(); }

protected:
	// construction/destruction
	rw5000_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; } // 4-phase clock
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
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

	int m_prgwidth; // ROM/RAM address size
	int m_datawidth; // "
	u16 m_prgmask; // "
	u16 m_datamask; // "

	void increment_pc();
	virtual bool op_canskip(u8 op) = 0;
	virtual void reset_pc() = 0;

	u16 m_pc;
	u16 m_prev_pc;
	u16 m_s;
	u8 m_op;
	u8 m_prev_op;

	u8 m_a;
	u8 m_bl;
	u8 m_bu;
	u8 m_prev_bl;
	u8 m_prev_bu;
	bool m_bl_delay;
	bool m_bu_delay;
	u8 m_ram_addr;
	u8 m_c;
	u8 m_prev_c;
	u8 m_prev2_c;
	u8 m_prev3_c;
	bool m_sr;
	bool m_skip;
	u16 m_seg;
	bool m_suppress0;

	u8 m_atb_step;
	u8 m_mtd_step;
	u8 m_tra_step;
	u8 m_ret_step;

	virtual void op_atb_step() { ; }
	virtual void op_mtd_step() { ; }
	virtual void op_tra_step() { ; }
	virtual void op_ret_step() { ; }

	// i/o handlers
	devcb_read8 m_read_kb;
	devcb_read8 m_read_din;
	devcb_write16 m_write_str;
	devcb_write16 m_write_seg;
	devcb_write_line m_write_spk;
};


#endif // MAME_CPU_RW5000_RW5000BASE_H
