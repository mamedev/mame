// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*****************************************************************************
 *
 *   4004dasm.cpp
 *
 *   Intel MCS-40 CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"
#include "mcs40dasm.h"


#define OP(fmt, lvl, name) { format::fmt, level::lvl, #name, nullptr }
#define OPX(tbl) { format::EXT, level::I4004, nullptr, f_opx_##tbl }

mcs40_disassembler::op const mcs40_disassembler::f_opx_0[16] = {
		OP(SIMPLE, I4004, nop), OP(SIMPLE, I4040, hlt), OP(SIMPLE, I4040, bbs), OP(SIMPLE, I4040, lcr),
		OP(SIMPLE, I4040, or4), OP(SIMPLE, I4040, or5), OP(SIMPLE, I4040, an6), OP(SIMPLE, I4040, an7),
		OP(SIMPLE, I4040, db0), OP(SIMPLE, I4040, db1), OP(SIMPLE, I4040, sb0), OP(SIMPLE, I4040, sb1),
		OP(SIMPLE, I4040, ein), OP(SIMPLE, I4040, din), OP(SIMPLE, I4040, rpm), OP(ILL,    I4004, ill) };

mcs40_disassembler::op const mcs40_disassembler::f_opx_2[16] = {
		OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR, I4004, src) };

mcs40_disassembler::op const mcs40_disassembler::f_opx_3[16] = {
		OP(PAIR, I4004, fin), OP(PAIR, I4004, jin), OP(PAIR, I4004, fin), OP(PAIR, I4004, jin),
		OP(PAIR, I4004, fin), OP(PAIR, I4004, jin), OP(PAIR, I4004, fin), OP(PAIR, I4004, jin),
		OP(PAIR, I4004, fin), OP(PAIR, I4004, jin), OP(PAIR, I4004, fin), OP(PAIR, I4004, jin),
		OP(PAIR, I4004, fin), OP(PAIR, I4004, jin), OP(PAIR, I4004, fin), OP(PAIR, I4004, jin) };

mcs40_disassembler::op const mcs40_disassembler::f_opx_io[16] = {
		OP(SIMPLE, I4004, wrm), OP(SIMPLE, I4004, wmp), OP(SIMPLE, I4004, wrr), OP(SIMPLE, I4004, wpm),
		OP(SIMPLE, I4004, wr0), OP(SIMPLE, I4004, wr1), OP(SIMPLE, I4004, wr2), OP(SIMPLE, I4004, wr3),
		OP(SIMPLE, I4004, sbm), OP(SIMPLE, I4004, rdm), OP(SIMPLE, I4004, rdr), OP(SIMPLE, I4004, adm),
		OP(SIMPLE, I4004, rd0), OP(SIMPLE, I4004, rd1), OP(SIMPLE, I4004, rd2), OP(SIMPLE, I4004, rd3) };

mcs40_disassembler::op const mcs40_disassembler::f_opx_f[16] = {
		OP(SIMPLE, I4004, clb), OP(SIMPLE, I4004, clc), OP(SIMPLE, I4004, iac), OP(SIMPLE, I4004, cmc),
		OP(SIMPLE, I4004, cma), OP(SIMPLE, I4004, ral), OP(SIMPLE, I4004, rar), OP(SIMPLE, I4004, tcc),
		OP(SIMPLE, I4004, dac), OP(SIMPLE, I4004, tcs), OP(SIMPLE, I4004, stc), OP(SIMPLE, I4004, daa),
		OP(SIMPLE, I4004, kbp), OP(SIMPLE, I4004, dcl), OP(ILL,    I4004, ill), OP(ILL,    I4004, ill) };

mcs40_disassembler::op const mcs40_disassembler::f_ops[16] = {
		OPX(0),                  OP(COND,    I4004, jcn), OPX(2),                  OPX(3),
		OP(ABS,     I4004, jun), OP(ABS,     I4004, jms), OP(REG,     I4004, inc), OP(REGPAGE, I4004, isz),
		OP(REG,     I4004, add), OP(REG,     I4004, sub), OP(REG,     I4004, ld ), OP(REG,     I4004, xch),
		OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, ldm), OPX(io),                 OPX(f)                  };

char const *const mcs40_disassembler::f_cond[16] = {
		"$0", "nt", "c",  "$3", "z",  "$5", "$6", "$7",
		"$8", "t",  "nc", "$b", "nz", "$d", "$e", "$f" };

u32 mcs40_disassembler::opcode_alignment() const
{
	return 1;
}

mcs40_disassembler::mcs40_disassembler(level lvl, unsigned pcmask) : m_lvl(lvl), m_pcmask(pcmask)
{
	(void)m_pcmask;
}

offs_t mcs40_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t npc(pc + 1);
	u8 const opcode(opcodes.r8(pc));
	op const &base_op(f_ops[(opcode >> 4) & 0x0f]);
	op const &ext_op((base_op.m_format == format::EXT) ? base_op.m_ext[opcode & 0x0f] : base_op);
	op const desc((ext_op.m_level > m_lvl) ? f_opx_f[0x0f] : ext_op);

	u8 const imm4(opcode & 0x0f);
	u8 const pair(opcode & 0x0e);
	switch (desc.m_format)
	{
	case format::ILL:
		util::stream_format(stream, "%-3s $%02x", desc.m_name, opcode);
		break;
	case format::SIMPLE:
		util::stream_format(stream, "%s", desc.m_name);
		break;
	case format::IMM4:
		util::stream_format(stream, "%-3s $%01x", desc.m_name, imm4);
		break;
	case format::REG:
		util::stream_format(stream, "%-3s r%01x", desc.m_name, imm4);
		break;
	case format::REGPAGE:
		npc++;
		util::stream_format(stream, "%-3s r%01x,$%03x", desc.m_name, imm4, opcodes.r8(pc+1) | (npc & 0x0f00U));
		break;
	case format::PAIR:
		util::stream_format(stream, "%-3s r%01xr%01x", desc.m_name, pair, pair + 1U);
		break;
	case format::PAIRIMM:
		npc++;
		util::stream_format(stream, "%-3s r%01xr%01x,$%02x", desc.m_name, pair, pair + 1, opcodes.r8(pc+1));
		break;
	case format::ABS:
		npc++;
		util::stream_format(stream, "%-3s $%03x", desc.m_name, ((u16(opcode) & 0x0fU) << 8) | opcodes.r8(pc+1));
		break;
	case format::PAGE:
		npc++;
		util::stream_format(stream, "%-3s $%03x", desc.m_name, opcodes.r8(pc+1) | (npc & 0x0f00U));
		break;
	case format::COND:
		npc++;
		util::stream_format(stream, "%-3s %s,$%03x", desc.m_name, f_cond[imm4], opcodes.r8(pc+1) | (npc & 0x0f00U));
		break;
	default:
		throw false;
	}

	offs_t flags(0U);
	if (format::ILL != desc.m_format)
	{
		if (0x50U == (opcode & 0xf0U)) // JMS
			flags = STEP_OVER;
		else if ((0xc0 == (opcode & 0xf0)) || (0x02 == opcode)) // BBL/BBS
			flags = STEP_OUT;
	}

	return (npc - pc) | flags | SUPPORTED;
}

i4004_disassembler::i4004_disassembler() : mcs40_disassembler(level::I4004, 0x0fffU)
{
}


i4040_disassembler::i4040_disassembler() : mcs40_disassembler(level::I4040, 0x1fffU)
{
}
