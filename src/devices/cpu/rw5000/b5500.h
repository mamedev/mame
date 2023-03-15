// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5500 MCU

*/

#ifndef MAME_CPU_RW5000_B5500_H
#define MAME_CPU_RW5000_B5500_H

#pragma once

#include "a5500.h"


class b5500_cpu_device : public a5500_cpu_device
{
public:
	b5500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_one() override;
};


DECLARE_DEVICE_TYPE(B5500, b5500_cpu_device)

#endif // MAME_CPU_RW5000_B5500_H
