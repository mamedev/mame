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

	axc51core_disassembler(const mem_info* names) : mcs51_disassembler(names) {}

	virtual ~axc51core_disassembler() = default;

	static const mem_info axc51core_names[];

protected:
	virtual offs_t disassemble_op(std::ostream &stream, unsigned PC, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint8_t op) override;

private:
	offs_t disassemble_extended_a5(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_0e(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_0f(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_d0(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);
	offs_t disassemble_extended_a5_d1(std::ostream& stream, unsigned PC, offs_t pc, const data_buffer& opcodes, const data_buffer& params);

};

class ax208_disassembler : public axc51core_disassembler
{
public:
	ax208_disassembler();
	virtual ~ax208_disassembler() = default;

	struct ax208_bios_info {
		int addr;
		const char *name;
	};

	static const ax208_bios_info bios_call_names[];

protected:
	virtual void disassemble_op_ljmp(std::ostream& stream, unsigned &PC, const data_buffer& params) override;
	virtual void disassemble_op_lcall(std::ostream& stream, unsigned &PC, const data_buffer& params) override;
};



#endif // MAME_CPU_MCS51_AXC51_CORE_DASM_H
