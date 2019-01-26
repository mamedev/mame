// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC

 *****************************************************************************/

#ifndef MAME_CPU_MCS51_AXC51_CORE_DASM_H
#define MAME_CPU_MCS51_AXC51_CORE_DASM_H

#pragma once

#include "mcs51dasm.h"

class axc51core_disassembler : public mcs51_disassembler
{
public:
	axc51core_disassembler();
	virtual ~axc51core_disassembler() = default;

	static const mem_info axc51core_names[];

protected:
	virtual offs_t disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op) override;
};


#endif // MAME_CPU_MCS51_AXC51_CORE_DASM_H
