// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM530 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM530_H
#define MAME_CPU_SM510_SM530_H

#pragma once

#include "sm510.h"


class sm530_device : public sm511_device
{
public:
	sm530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm530_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_2k(address_map &map);
	void data_64_24x4(address_map &map);

	virtual void device_reset() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;
	virtual void get_opcode_param() override;

	virtual u8 ram_r() override;
	virtual void ram_w(u8 data) override;
};

class sm531_device : public sm530_device
{
public:
	sm531_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);
};



DECLARE_DEVICE_TYPE(SM530, sm530_device)
DECLARE_DEVICE_TYPE(SM531, sm531_device)

#endif // MAME_CPU_SM510_SM530_H
