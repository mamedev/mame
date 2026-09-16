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

#include "machine/pla.h"


class mn1400_base_device : public cpu_device
{
public:
	// configuration helpers
	// I/O ports:

	// 4-bit A/B input ports
	auto read_a() { return m_read_a.bind(); }
	auto read_b() { return m_read_b.bind(); }

	// SNS0/SNS1 input pins
	auto read_sns() { return m_read_sns.bind(); }

	// up to 12-bit C output port
	auto write_c() { return m_write_c.bind(); }
	auto &set_c_mask(u16 mask) { m_c_mask = mask; return *this; }

	// up to 8-bit D output port
	// for 4-bit, it commonly uses D1-D3,D5
	auto write_d() { return m_write_d.bind(); }
	auto &set_d_mask(u8 mask, u32 bits) { m_d_mask = mask; m_d_bits = bits; return *this; }

	// 4-bit E output port
	auto write_e() { return m_write_e.bind(); }

protected:
	// construction/destruction
	mn1400_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 3 - 1) / 3; } // 3-phase clock
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 3); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_one() = 0;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void program_1kx8(address_map &map) ATTR_COLD;
	void program_2kx8(address_map &map) ATTR_COLD;
	void data_64x4(address_map &map) ATTR_COLD;
	void data_128x4(address_map &map) ATTR_COLD;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	optional_device<pla_device> m_opla; // D port output PLA

	int m_icount;
	int m_state_count;

	int m_stack_levels;
	int m_prgwidth; // ROM/RAM address size
	int m_datawidth; // "
	u16 m_prgmask; // "
	u16 m_datamask; // "

	virtual void write_c(u16 data);
	virtual void write_d(u8 data);
	virtual void cycle();
	virtual void increment_pc();
	virtual bool op_has_param(u8 op) = 0;

	u16 m_pc;
	u16 m_prev_pc;
	u8 m_op;
	u8 m_prev_op;
	u8 m_param;
	u8 m_ram_address;
	u16 m_stack[2]; // max 2
	u8 m_sp;

	u8 m_a;
	u8 m_x;
	u8 m_y;
	u8 m_status;
	u16 m_c;
	u8 m_counter;
	bool m_ec;

	enum : u8
	{
		FLAG_Z = 1,
		FLAG_C = 2,
		FLAG_P = 4,
	};

	// i/o handlers
	devcb_read8 m_read_a;
	devcb_read8 m_read_b;
	devcb_read8 m_read_sns;
	devcb_write16 m_write_c;
	devcb_write8 m_write_d;
	devcb_write8 m_write_e;

	u16 m_c_mask;
	u8 m_d_mask;
	u32 m_d_bits;
};


#endif // MAME_CPU_MN1400_MN1400BASE_H
