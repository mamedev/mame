// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0980, TMS1980

*/

#ifndef MAME_CPU_TMS1000_TMS0980_H
#define MAME_CPU_TMS1000_TMS0980_H

#pragma once

#include "tms0970.h"


class tms0980_cpu_device : public tms0970_cpu_device
{
public:
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_11bit_9(address_map &map);
	void data_144x4(address_map &map);

	// overrides
	virtual u32 decode_fixed(u16 op);
	virtual u32 decode_micro(u8 sel) override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u8 read_k_input() override;
	virtual void set_cki_bus() override;
	virtual u32 read_micro();
	virtual void read_opcode() override;

	virtual void op_comx() override;
};

class tms1980_cpu_device : public tms0980_cpu_device
{
public:
	tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void write_o_output(u8 index) override { tms1k_base_device::write_o_output(index); }
	virtual u8 read_k_input() override { return tms1k_base_device::read_k_input(); }

	virtual void op_setr() override { tms1k_base_device::op_setr(); }
	virtual void op_tdo() override;
};


DECLARE_DEVICE_TYPE(TMS0980, tms0980_cpu_device)
DECLARE_DEVICE_TYPE(TMS1980, tms1980_cpu_device)

#endif // MAME_CPU_TMS1000_TMS0980_H
