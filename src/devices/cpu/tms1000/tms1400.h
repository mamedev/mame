// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1400, TMS1470, TMS1600, TMS1670

*/

#ifndef MAME_CPU_TMS1000_TMS1400_H
#define MAME_CPU_TMS1000_TMS1400_H

#pragma once

#include "tms1100.h"


class tms1400_cpu_device : public tms1100_cpu_device
{
public:
	tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_12bit_8(address_map &map);

	// overrides
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void op_br() override { op_br3(); } // 3-level stack
	virtual void op_call() override { op_call3(); } // "
	virtual void op_retn() override { op_retn3(); } // "

	virtual void op_setr() override { tms1k_base_device::op_setr(); } // no anomaly with MSB of X register
	virtual void op_rstr() override { tms1k_base_device::op_rstr(); } // "
};

class tms1470_cpu_device : public tms1400_cpu_device
{
public:
	tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class tms1600_cpu_device : public tms1400_cpu_device
{
public:
	tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);
};

class tms1670_cpu_device : public tms1600_cpu_device
{
public:
	tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS1400, tms1400_cpu_device)
DECLARE_DEVICE_TYPE(TMS1470, tms1470_cpu_device)
DECLARE_DEVICE_TYPE(TMS1600, tms1600_cpu_device)
DECLARE_DEVICE_TYPE(TMS1670, tms1670_cpu_device)

#endif // MAME_CPU_TMS1000_TMS1400_H
