// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0980, TMS1980

*/

#ifndef MAME_CPU_TMS1000_TMS0980_H
#define MAME_CPU_TMS1000_TMS0980_H

#pragma once

#include "tms0970.h"


// pinout reference

/*
            ____   ____
     R2  1 |*   \_/    | 28 R3
     R1  2 |           | 27 R4
     R0  3 |           | 26 R5
      ?  4 |           | 25 R6
    Vdd  5 |           | 24 R7
     K3  6 |           | 23 R8
     K8  7 |  TMS0980  | 22 ?
     K4  8 |           | 21 ?
     K2  9 |           | 20 Vss
     K1 10 |           | 19 ?
     O7 11 |           | 18 O0
     O6 12 |           | 17 O1
     O5 13 |           | 16 O2
     O4 14 |___________| 15 O3

  note: TMS0980 official pin names for R0-R8 is D9-D1, O0-O7 is S(A-G,DP)

*/


class tms0980_cpu_device : public tms0970_cpu_device
{
public:
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	void ram_144x4(address_map &map) ATTR_COLD;

	// overrides
	virtual u32 decode_fixed(offs_t offset);
	virtual u32 decode_micro(offs_t offset) override;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u8 read_k_input() override;
	virtual void set_cki_bus() override;
	virtual u32 read_micro();
	virtual void read_opcode() override;

	virtual void op_comx() override;
	virtual void op_xda() override;
	virtual void op_off() override;
	virtual void op_seac() override;
	virtual void op_reac() override;
	virtual void op_sal() override;
	virtual void op_sbl() override;
};

class tms1980_cpu_device : public tms0980_cpu_device
{
public:
	tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void write_o_reg(u8 index) override { tms1k_base_device::write_o_reg(index); }

	virtual void op_setr() override { tms1k_base_device::op_setr(); }
	virtual void op_tdo() override;
};


DECLARE_DEVICE_TYPE(TMS0980, tms0980_cpu_device)
DECLARE_DEVICE_TYPE(TMS1980, tms1980_cpu_device)

#endif // MAME_CPU_TMS1000_TMS0980_H
