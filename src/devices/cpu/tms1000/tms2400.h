// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS2400, TMS2470, TMS2600, TMS2670

*/

#ifndef MAME_CPU_TMS1000_TMS2400_H
#define MAME_CPU_TMS1000_TMS2400_H

#pragma once

#include "tms2100.h"


class tms2400_cpu_device : public tms2100_cpu_device
{
public:
	tms2400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms2400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// overrides
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void op_ldx() override;
	virtual void op_txa() override;
};

class tms2470_cpu_device : public tms2400_cpu_device
{
public:
	tms2470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms2600_cpu_device : public tms2400_cpu_device
{
public:
	tms2600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms2600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);
};

class tms2670_cpu_device : public tms2600_cpu_device
{
public:
	tms2670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS2400, tms2400_cpu_device)
DECLARE_DEVICE_TYPE(TMS2470, tms2470_cpu_device)
DECLARE_DEVICE_TYPE(TMS2600, tms2600_cpu_device)
DECLARE_DEVICE_TYPE(TMS2670, tms2670_cpu_device)

#endif // MAME_CPU_TMS1000_TMS2400_H
