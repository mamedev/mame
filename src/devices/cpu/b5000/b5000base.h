// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 family MCU cores

*/

#ifndef MAME_CPU_B5000_B5000BASE_H
#define MAME_CPU_B5000_B5000BASE_H

#pragma once


class b5000_base_device : public cpu_device
{
public:
	// ...

protected:
	// construction/destruction
	b5000_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; } // 4-phase clock
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
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

	int m_icount;

	int m_prgwidth; // ROM/RAM address size
	int m_datawidth; // "
	u16 m_prgmask; // "
	u16 m_datamask; // "

	void cycle();
	void increment_pc();

	u16 m_pc;
	u16 m_prev_pc;
	u8 m_op;

	bool m_skip;

	// i/o handlers
};


#endif // MAME_CPU_B5000_B5000BASE_H
