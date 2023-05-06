// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita MN1400, MN1405

*/

#ifndef MAME_CPU_MN1400_MN1400_H
#define MAME_CPU_MN1400_MN1400_H

#pragma once

#include "mn1400base.h"

// pinout reference

/*

*/

class mn1400_cpu_device : public mn1400_base_device
{
public:
	mn1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mn1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;
	virtual bool op_has_param(u8 op) override;

	// opcode helpers
	void op_illegal();

	// opcode handlers
};

class mn1405_cpu_device : public mn1400_cpu_device
{
public:
	mn1405_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MN1400, mn1400_cpu_device)
DECLARE_DEVICE_TYPE(MN1405, mn1405_cpu_device)

#endif // MAME_CPU_MN1400_MN1400_H
