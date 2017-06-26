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

namespace {

enum class format
{
	ILL,
	SIMPLE,
	IMM4,
	REG,
	REGPAGE,
	PAIR,
	PAIRIMM,
	ABS,
	PAGE,
	COND
};

enum class level
{
	I4004,
	I4040
};

struct op
{
	format m_format;
	level m_level;
	char const *m_name;
};

#define OP(fmt, lvl, name) { format::fmt, level::lvl, #name }

op const f_ops[256] = {
		OP(SIMPLE,  I4004, nop), OP(SIMPLE,  I4040, hlt), OP(SIMPLE,  I4040, bbs), OP(SIMPLE,  I4040, lcr),
		OP(SIMPLE,  I4040, or4), OP(SIMPLE,  I4040, or5), OP(SIMPLE,  I4040, an6), OP(SIMPLE,  I4040, an7),
		OP(SIMPLE,  I4040, db0), OP(SIMPLE,  I4040, db1), OP(SIMPLE,  I4040, sb0), OP(SIMPLE,  I4040, sb1),
		OP(SIMPLE,  I4040, ein), OP(SIMPLE,  I4040, din), OP(SIMPLE,  I4040, rpm), OP(ILL,     I4004, ill),

		OP(COND,    I4004, jcn), OP(PAGE,    I4004, jnt), OP(PAGE,    I4004, jc ), OP(COND,    I4004, jcn),
		OP(PAGE,    I4004, jz ), OP(COND,    I4004, jcn), OP(COND,    I4004, jcn), OP(COND,    I4004, jcn),
		OP(COND,    I4004, jcn), OP(PAGE,    I4004, jt),  OP(PAGE,    I4004, jnc), OP(COND,    I4004, jcn),
		OP(PAGE,    I4004, jnz), OP(COND,    I4004, jcn), OP(COND,    I4004, jcn), OP(COND,    I4004, jcn),

		OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src),
		OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src), OP(PAIRIMM, I4004, fim), OP(PAIR,    I4004, src),

		OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin), OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin),
		OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin), OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin),
		OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin), OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin),
		OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin), OP(PAIR,    I4004, fin), OP(PAIR,    I4004, jin),

		OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun),
		OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun),
		OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun),
		OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun), OP(ABS,     I4004, jun),

		OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms),
		OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms),
		OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms),
		OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms), OP(ABS,     I4004, jms),

		OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc),
		OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc),
		OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc),
		OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc), OP(REG,     I4004, inc),

		OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz),
		OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz),
		OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz),
		OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz), OP(REGPAGE, I4004, isz),

		OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add),
		OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add),
		OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add),
		OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add), OP(REG,     I4004, add),

		OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub),
		OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub),
		OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub),
		OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub), OP(REG,     I4004, sub),

		OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ),
		OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ),
		OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ),
		OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ), OP(REG,     I4004, ld ),

		OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch),
		OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch),
		OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch),
		OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch), OP(REG,     I4004, xch),

		OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl),
		OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl),
		OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl),
		OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl), OP(IMM4,    I4004, bbl),

		OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm),
		OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm),
		OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm),
		OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm), OP(IMM4,    I4004, ldm),

		OP(SIMPLE,  I4004, wrm), OP(SIMPLE,  I4004, wmp), OP(SIMPLE,  I4004, wrr), OP(SIMPLE,  I4004, wpm),
		OP(SIMPLE,  I4004, wr0), OP(SIMPLE,  I4004, wr1), OP(SIMPLE,  I4004, wr2), OP(SIMPLE,  I4004, wr3),
		OP(SIMPLE,  I4004, sbm), OP(SIMPLE,  I4004, rdm), OP(SIMPLE,  I4004, rdr), OP(SIMPLE,  I4004, adm),
		OP(SIMPLE,  I4004, rd0), OP(SIMPLE,  I4004, rd1), OP(SIMPLE,  I4004, rd2), OP(SIMPLE,  I4004, rd3),

		OP(SIMPLE,  I4004, clb), OP(SIMPLE,  I4004, clc), OP(SIMPLE,  I4004, iac), OP(SIMPLE,  I4004, cmc),
		OP(SIMPLE,  I4004, cma), OP(SIMPLE,  I4004, ral), OP(SIMPLE,  I4004, rar), OP(SIMPLE,  I4004, tcc),
		OP(SIMPLE,  I4004, dac), OP(SIMPLE,  I4004, tcs), OP(SIMPLE,  I4004, stc), OP(SIMPLE,  I4004, daa),
		OP(SIMPLE,  I4004, kbp), OP(SIMPLE,  I4004, dcl), OP(ILL,     I4004, ill), OP(ILL,     I4004, ill) };

offs_t disassemble(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		u8 const *oprom,
		u8 const *opram,
		int options,
		level lvl,
		unsigned pcmask)
{
	offs_t npc(pc + 1);
	u8 const opcode(oprom[0]);
	op const &desc(f_ops[(f_ops[opcode].m_level > lvl) ? 0xffU : opcode]);

	switch (desc.m_format)
	{
	case format::ILL:
		util::stream_format(stream, "%-3s $%02x", desc.m_name, opcode);
		break;
	case format::SIMPLE:
		util::stream_format(stream, "%s", desc.m_name);
		break;
	case format::IMM4:
	case format::REG:
		util::stream_format(stream, "%-3s $%01x", desc.m_name, opcode & 0x0fU);
		break;
	case format::REGPAGE:
	case format::COND:
		npc++;
		util::stream_format(stream, "%-3s $%01x,$%03x", desc.m_name, opcode & 0x0fU, opram[1] | (npc & 0x0f00U));
		break;
	case format::PAIR:
		util::stream_format(stream, "%-3s $%01x", desc.m_name, opcode & 0x0eU);
		break;
	case format::PAIRIMM:
		npc++;
		util::stream_format(stream, "%-3s $%01x,$%02x", desc.m_name, opcode & 0x0eU, opram[1]);
		break;
	case format::ABS:
		npc++;
		util::stream_format(stream, "%-3s $%03x", desc.m_name, ((u16(opcode) & 0x0fU) << 8) | opram[1]);
		break;
	case format::PAGE:
		npc++;
		util::stream_format(stream, "%-3s $%03x", desc.m_name, opram[1] | (npc & 0x0f00U));
		break;
	}

	offs_t flags(0U);
	if (format::ILL != desc.m_format)
	{
		if (0x50U == (opcode & 0xf0U)) // JMS
			flags = DASMFLAG_STEP_OVER;
		else if ((0xc0 == (opcode & 0xf0)) || (0x02 == opcode)) // BBL/BBS
			flags = DASMFLAG_STEP_OUT;
	}

	return (npc - pc) | flags | DASMFLAG_SUPPORTED;
}

} // anonymous namespace


CPU_DISASSEMBLE(i4004) { return disassemble(device, stream, pc, oprom, opram, options, level::I4004, 0x0fffU); }
CPU_DISASSEMBLE(i4040) { return disassemble(device, stream, pc, oprom, opram, options, level::I4040, 0x1fffU); }
