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
	virtual u16 reset_vector() = 0;

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
	bool m_skip;

	u8 m_atbz_step;
	u8 m_tkbs_step;
	u8 m_tra0_step;
	u8 m_tra1_step;
	u8 m_ret_step;

	virtual void op_atbz(u8 step) { ; }
	virtual void op_tkbs(u8 step) { ; }
	virtual void op_tra0(u8 step) { ; }
	virtual void op_tra1(u8 step) { ; }
	virtual void op_ret(u8 step) { ; }

	// i/o handlers
};


#endif // MAME_CPU_B5000_B5000BASE_H
