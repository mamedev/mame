// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000, TMS1070, TMS1040, TMS1200, TMS1270, TMS1700, TMS1730

*/

#ifndef MAME_CPU_TMS1000_TMS1000_H
#define MAME_CPU_TMS1000_TMS1000_H

#pragma once

#include "tms1k_base.h"


// pinout reference

/*
            ____   ____
     R8  1 |*   \_/    | 28 R7
     R9  2 |           | 27 R6
    R10  3 |           | 26 R5
    Vdd  4 |           | 25 R4
     K1  5 |           | 24 R3
     K2  6 |           | 23 R2
     K4  7 |  TMS1000  | 22 R1
     K8  8 |           | 21 R0
   INIT  9 |           | 20 Vss
     O7 10 |           | 19 OSC2
     O6 11 |           | 18 OSC1
     O5 12 |           | 17 O0
     O4 13 |           | 16 O1
     O3 14 |___________| 15 O2

  note: TMS1070 is same as TMS1000, except pins 20 and 21 are swapped.

            ____   ____                      ____   ____
     R8  1 |*   \_/    | 40 R7        R8  1 |*   \_/    | 40 NC
     R9  2 |           | 39 R6        R9  2 |           | 39 R7
    R10  3 |           | 38 R5       R10  3 |           | 38 R6
    R11  4 |           | 37 R4       R11  4 |           | 37 R5
    R12  5 |           | 36 R3       R12  5 |           | 36 R4
    Vdd  6 |           | 35 NC       Vdd  6 |           | 35 R3
     K1  7 |           | 34 NC        K1  7 |           | 34 NC
     K2  8 |           | 33 NC        K2  8 |           | 33 NC
     K4  9 |           | 32 NC        K4  9 |           | 32 NC
     K8 10 |  TMS1200  | 31 R2        K8 10 |  TMS1270  | 31 NC
   INIT 11 |           | 30 R1      INIT 11 |           | 30 R2
     O7 12 |           | 29 R0        NC 12 |           | 29 R1
     NC 13 |           | 28 Vss       NC 13 |           | 28 Vss
     NC 14 |           | 27 OSC2      O7 14 |           | 27 R0
     NC 15 |           | 26 OSC1      O6 15 |           | 26 OSC2
     O6 16 |           | 25 O0        O5 16 |           | 25 OSC1
     O5 17 |           | 24 O1        O9 17 |           | 24 O0
     O4 18 |           | 23 O2        O4 18 |           | 23 O1
     O3 19 |           | 22 NC        O3 19 |           | 22 O2
     NC 20 |___________| 21 NC        O8 20 |___________| 21 NC

*/


class tms1000_cpu_device : public tms1k_base_device
{
public:
	tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	void rom_9bitm(address_map &map) ATTR_COLD;
	void ram_32x4(address_map &map) ATTR_COLD;

	// overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 decode_micro(offs_t offset);
};

class tms1040_cpu_device : public tms1000_cpu_device
{
public:
	tms1040_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms1200_cpu_device : public tms1000_cpu_device
{
public:
	tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class tms1070_cpu_device : public tms1000_cpu_device
{
public:
	tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1070_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void write_o_output(u16 data) override { tms1000_cpu_device::write_o_output(bitswap<10>(data,0,1,9,8,7,6,5,4,3,2)); }
};

class tms1270_cpu_device : public tms1070_cpu_device
{
public:
	tms1270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class tms1700_cpu_device : public tms1000_cpu_device
{
public:
	tms1700_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms1730_cpu_device : public tms1000_cpu_device
{
public:
	tms1730_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS1000, tms1000_cpu_device)
DECLARE_DEVICE_TYPE(TMS1040, tms1040_cpu_device)
DECLARE_DEVICE_TYPE(TMS1070, tms1070_cpu_device)
DECLARE_DEVICE_TYPE(TMS1200, tms1200_cpu_device)
DECLARE_DEVICE_TYPE(TMS1270, tms1270_cpu_device)
DECLARE_DEVICE_TYPE(TMS1700, tms1700_cpu_device)
DECLARE_DEVICE_TYPE(TMS1730, tms1730_cpu_device)

#endif // MAME_CPU_TMS1000_TMS1000_H
