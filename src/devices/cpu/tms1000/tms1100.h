// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1100, TMS1170, TMS1300, TMS1370

*/

#ifndef MAME_CPU_TMS1000_TMS1100_H
#define MAME_CPU_TMS1000_TMS1100_H

#pragma once

#include "tms1000.h"


// pinout reference

/*
            ____   ____
     R8  1 |*   \_/    | 28 R7
     R9  2 |           | 27 R6
    R10  3 |           | 26 R5
    Vdd  4 |           | 25 R4
     K1  5 |           | 24 R3
     K2  6 |           | 23 R2
     K4  7 |  TMS1100  | 22 R1
     K8  8 |  TMS1170  | 21 R0
   INIT  9 |           | 20 Vss
     O7 10 |           | 19 OSC2
     O6 11 |           | 18 OSC1
     O5 12 |           | 17 O0
     O4 13 |           | 16 O1
     O3 14 |___________| 15 O2

            ____   ____
    R11  1 |*   \_/    | 40 R10
    R12  2 |           | 39 R9
    R13  3 |           | 38 R8
    R14  4 |           | 37 R7
    R15  5 |           | 36 R6
    Vdd  6 |           | 35 NC
     K1  7 |           | 34 R5
     K2  8 |           | 33 R4
     K4  9 |           | 32 R3
     K8 10 |  TMS1300  | 31 R2
   INIT 11 |  TMS1370  | 30 R1
     O7 12 |           | 29 R0
     NC 13 |           | 28 Vss
     NC 14 |           | 27 OSC2
     NC 15 |           | 26 OSC1
     O6 16 |           | 25 O0
     O5 17 |           | 24 O1
     O4 18 |           | 23 O2
     O3 19 |           | 22 NC
     NC 20 |___________| 21 NC

  note: TMS1100 pinout is the same as TMS1000

*/


class tms1100_cpu_device : public tms1000_cpu_device
{
public:
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
};

class tms1170_cpu_device : public tms1100_cpu_device
{
public:
	tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms1300_cpu_device : public tms1100_cpu_device
{
public:
	tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms1370_cpu_device : public tms1100_cpu_device
{
public:
	tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS1100, tms1100_cpu_device)
DECLARE_DEVICE_TYPE(TMS1170, tms1170_cpu_device)
DECLARE_DEVICE_TYPE(TMS1300, tms1300_cpu_device)
DECLARE_DEVICE_TYPE(TMS1370, tms1370_cpu_device)

#endif // MAME_CPU_TMS1000_TMS1100_H
