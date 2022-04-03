// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5000 MCU

*/

#ifndef MAME_CPU_RW5000_A5000_H
#define MAME_CPU_RW5000_A5000_H

#pragma once

#include "b5000.h"


class a5000_cpu_device : public b5000_cpu_device
{
public:
	a5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	a5000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_one() override;

	// opcode handlers
	virtual void op_mtd_step() override;
};


DECLARE_DEVICE_TYPE(A5000, a5000_cpu_device)

#endif // MAME_CPU_RW5000_A5000_H
