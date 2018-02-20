// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Nuon disassembler

#ifndef MAME_CPU_NUON_NUONDASM_H
#define MAME_CPU_NUON_NUONDASM_H

#pragma once

class nuon_disassembler : public util::disasm_interface
{
public:
	nuon_disassembler() = default;
	virtual ~nuon_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct reginfo {
		u32 adr;
		const char *name;
	};

	static const reginfo reginfos[];

	std::string s2x(s32 val, int bits) const;
	std::string u2x(u32 val, int bits) const;
	s32 s2i(u32 val, int bits) const;
	std::string cc(u16 val, bool comma) const;
	std::string rx(u16 sel) const;
	std::string xy(u16 sel) const;
	u32 svs(u16 sel) const;

	std::string dec(u16 opc, std::string r = "") const;
	std::string reg(u32 adr) const;

	bool m(u16 val, u16 mask, u16 test) const;
	u32 b(u16 val, int start, int count, int target);

	std::string parse_packet(const data_buffer &opcodes, offs_t &pc, offs_t bpc, bool &cont);
};

#endif

