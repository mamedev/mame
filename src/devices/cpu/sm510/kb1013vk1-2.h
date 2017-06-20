// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  KB1013VK1-2

*/

#ifndef MAME_CPU_SM510_KB1013VK1_2_H
#define MAME_CPU_SM510_KB1013VK1_2_H

#pragma once

#include "sm500.h"


// I/O ports setup

// ..

class kb1013vk12_device : public sm500_device
{
public:
	kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options) override;
	virtual void execute_one() override;

	// opcode handlers
	virtual void op_bs0();
	virtual void op_bs1();
};


DECLARE_DEVICE_TYPE(KB1013VK12, kb1013vk12_device)

#endif // MAME_CPU_SM510_KB1013VK1_2_H
