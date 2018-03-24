// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1100, TMS1170, TMS1300, TMS1370

*/

#ifndef MAME_CPU_TMS1000_TMS1100_H
#define MAME_CPU_TMS1000_TMS1100_H

#pragma once

#include "tms1000.h"


class tms1100_cpu_device : public tms1000_cpu_device
{
public:
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_11bit_8(address_map &map);
	void data_128x4(address_map &map);

	// overrides
	virtual void device_reset() override;

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
