// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_CPU_MPK1839_KL1839VM1DASM_H
#define MAME_CPU_MPK1839_KL1839VM1DASM_H

#pragma once

class kl1839vm1_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	kl1839vm1_disassembler() = default;
	virtual ~kl1839vm1_disassembler() = default;

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:

};

#endif // MAME_CPU_MPK1839_KL1839VM1DASM_H
