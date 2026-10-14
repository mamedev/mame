// license:BSD-3-Clause
// copyright-holders:Paul Priest, R. Belmont
// Konami 056734 (ESC) disassembler

#ifndef MAME_CPU_K056734_K056734D_H
#define MAME_CPU_K056734_K056734D_H

#pragma once

#include <array>

class k056734_disassembler : public util::disasm_interface
{
public:
	// Instruction fetch descrambler: canonical lane k (bits 2k+1:2k) is lane
	// lanes[k] of (stored word ^ xor_mask).
	class descrambler
	{
	public:
		descrambler();
		descrambler(u32 xor_mask, const std::array<u8, 16> &lanes);

		u32 operator()(u32 stored) const
		{
			stored ^= m_xor;
			return m_table[0][BIT(stored, 0, 8)] | m_table[1][BIT(stored, 8, 8)] | m_table[2][BIT(stored, 16, 8)] | m_table[3][BIT(stored, 24, 8)];
		}

	private:
		u32 m_xor;
		u32 m_table[4][256];
	};

	// fields of a descrambled instruction word
	static constexpr unsigned lane(u32 op, unsigned k) { return BIT(op, k * 2, 2); }
	static constexpr unsigned reg_a(u32 op) { return (lane(op, 5) << 4) | (lane(op, 15) << 2) | lane(op, 9); }
	static constexpr unsigned reg_b(u32 op) { return (lane(op, 2) << 4) | (lane(op, 1) << 2) | lane(op, 4); }
	static constexpr unsigned off10(u32 op) { return (lane(op, 6) << 8) | (lane(op, 14) << 6) | (lane(op, 13) << 4) | (lane(op, 12) << 2) | lane(op, 11); }
	static constexpr unsigned imm16(u32 op) { return (reg_b(op) << 10) | off10(op); }
	static constexpr u32 imm24(u32 op) { return (lane(op, 7) << 22) | (reg_a(op) << 16) | imm16(op); }
	static constexpr unsigned reg_d(u32 op) { return (BIT(lane(op, 8), 0) << 4) | (lane(op, 3) << 2) | (BIT(lane(op, 10), 1) << 1) | (BIT(lane(op, 10), 0) ^ 1); }
	static constexpr bool is_branch(u32 op) { return lane(op, 0) == 0 && lane(op, 5) == 2 && lane(op, 7) == 2 && lane(op, 9) == 2 && lane(op, 15) == 0; }

	// opcode lane digits, lanes 10, 8, 3, 0 and 10, 8, 7, 3, 0
	static constexpr u8 op4(unsigned l10, unsigned l8, unsigned l3, unsigned l0) { return (l10 << 6) | (l8 << 4) | (l3 << 2) | l0; }
	static constexpr u16 op5(unsigned l10, unsigned l8, unsigned l7, unsigned l3, unsigned l0) { return (l10 << 8) | (l8 << 6) | (l7 << 4) | (l3 << 2) | l0; }
	static constexpr u8 op4(u32 op) { return op4(lane(op, 10), lane(op, 8), lane(op, 3), lane(op, 0)); }
	static constexpr u16 op5(u32 op) { return op5(lane(op, 10), lane(op, 8), lane(op, 7), lane(op, 3), lane(op, 0)); }

	k056734_disassembler() = default;
	k056734_disassembler(const descrambler &fetch) : m_fetch(fetch) { }

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	descrambler m_fetch;
};

#endif // MAME_CPU_K056734_K056734D_H
