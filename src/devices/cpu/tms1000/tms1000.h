// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000, TMS1070, TMS1040, TMS1200

*/

#ifndef MAME_CPU_TMS1000_TMS1000_H
#define MAME_CPU_TMS1000_TMS1000_H

#pragma once

#include "tms1k_base.h"


class tms1000_cpu_device : public tms1k_base_device
{
public:
	tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_10bit_8(address_map &map);
	void data_64x4(address_map &map);
	void program_9bit_8(address_map &map);
	void data_32x4(address_map &map);

	// overrides
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 decode_micro(u8 sel);
};

class tms1070_cpu_device : public tms1000_cpu_device
{
public:
	tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
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


class mc141000_cpu_device : public tms1000_cpu_device
{
public:
	mc141000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mc141200_cpu_device : public tms1000_cpu_device
{
public:
	mc141200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS1000, tms1000_cpu_device)
DECLARE_DEVICE_TYPE(TMS1070, tms1070_cpu_device)
DECLARE_DEVICE_TYPE(TMS1040, tms1040_cpu_device)
DECLARE_DEVICE_TYPE(TMS1200, tms1200_cpu_device)
DECLARE_DEVICE_TYPE(TMS1700, tms1700_cpu_device)
DECLARE_DEVICE_TYPE(TMS1730, tms1730_cpu_device)
DECLARE_DEVICE_TYPE(MC141000, mc141000_cpu_device)
DECLARE_DEVICE_TYPE(MC141200, mc141200_cpu_device)

#endif // MAME_CPU_TMS1000_TMS1000_H
