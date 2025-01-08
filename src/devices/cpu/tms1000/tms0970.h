// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0950, TMS0970, TMS1990

*/

#ifndef MAME_CPU_TMS1000_TMS0970_H
#define MAME_CPU_TMS1000_TMS0970_H

#pragma once

#include "tms1000.h"


class tms0970_cpu_device : public tms1000_cpu_device
{
public:
	tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms0970_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// overrides
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void write_o_reg(u8 index) override;

	virtual void op_setr() override;
	virtual void op_tdo() override;
};

class tms0950_cpu_device : public tms0970_cpu_device
{
public:
	tms0950_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// overrides
	virtual void device_reset() override { tms1000_cpu_device::device_reset(); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void op_rstr() override { } // assume it has no RSTR or CLO
	virtual void op_clo() override { } // "
};

class tms1990_cpu_device : public tms0970_cpu_device
{
public:
	tms1990_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS0950, tms0950_cpu_device)
DECLARE_DEVICE_TYPE(TMS0970, tms0970_cpu_device)
DECLARE_DEVICE_TYPE(TMS1990, tms1990_cpu_device)

#endif // MAME_CPU_TMS1000_TMS0970_H
