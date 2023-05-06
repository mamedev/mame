// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita (Panasonic) MN1400 family MCU cores

  Don't include this file, include the specific device header instead,
  for example mn1400.h

*/

#ifndef MAME_CPU_MN1400_MN1400BASE_H
#define MAME_CPU_MN1400_MN1400BASE_H

#pragma once


class mn1400_base_device : public cpu_device
{
public:
	// configuration helpers
	// I/O ports:

protected:
	// construction/destruction
	mn1400_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 3 - 1) / 3; } // 3-phase clock
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 3); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_one() = 0;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	void program_1kx8(address_map &map);
	void program_2kx8(address_map &map);
	void data_64x4(address_map &map);
	void data_128x4(address_map &map);

	int m_icount;
	int m_state_count;

	int m_stack_levels;
	int m_prgwidth; // ROM/RAM address size
	int m_datawidth; // "
	u16 m_prgmask; // "
	u16 m_datamask; // "

	virtual void cycle();
	virtual void increment_pc();
	virtual bool op_has_param(u8 op) = 0;

	u16 m_pc;
	u16 m_prev_pc;
	u8 m_op;
	u8 m_prev_op;
	u8 m_param;

	u8 m_a;
	u8 m_x;
	u8 m_y;

	// i/o handlers
};


#endif // MAME_CPU_MN1400_MN1400BASE_H
