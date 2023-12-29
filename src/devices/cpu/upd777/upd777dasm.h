// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_UPD777_UPD777DASM_H
#define MAME_CPU_UPD777_UPD777DASM_H

#pragma once


class upd777_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	upd777_disassembler();

	// the ROMs are for some reason already in execution order, not address order, so build up a table of addresses
	static void populate_addr_table(u16 *table)
	{
		int count = 0;
		for (int upper = 0; upper < 0x800; upper += 0x80)
		{
			int out = 0;
			for (int i = 0; i < 127; i++) // 127 not 128, because last address is just 0 again and doesn't exist
			{
				int top1 = (out & 0x40) >> 6;
				int top2 = (out & 0x20) >> 5;
				int nor = (top1 ^ top2) ^ 1;
				int full = upper | out;
				table[count++] = full;
				out = (out << 1) | nor;
				out &= 0x7f;
			}
		}

	}

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	u16 get_table(u16 addr);
	u16 m_table[0xfe0/2];
};

#endif // MAME_CPU_UPD777_UPD777DASM_H
