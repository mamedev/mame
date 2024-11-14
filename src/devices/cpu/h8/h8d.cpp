// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8d.cpp

    H8-300 base cpu emulation, disassembler

***************************************************************************/

#include "emu.h"
#include "h8d.h"
#include "cpu/h8/h8d.hxx"

h8_disassembler::h8_disassembler(const disasm_entry *table, bool advanced) : m_table(table), m_advanced(advanced)
{
}

h8_disassembler::h8_disassembler() : h8_disassembler(disasm_entries, false)
{
}

u32 h8_disassembler::opcode_alignment() const
{
	return 2;
}

void h8_disassembler::disassemble_am(std::ostream &stream, int am, offs_t pc, const data_buffer &opcodes, u32 opcode, int slot, int offset)
{
	static const char *const r8_names[16] = {
		"r0h", "r1h",  "r2h", "r3h",  "r4h", "r5h",  "r6h", "r7h",
		"r0l", "r1l",  "r2l", "r3l",  "r4l", "r5l",  "r6l", "r7l"
	};

	static const char *const r16_names[16] = {
		"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
		"e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
	};

	static const char *const r32_names[8] = {
		"er0", "er1", "er2", "er3", "er4", "er5", "er6", "sp",
	};

	offs_t epc = pc + offset;

	switch(am) {
	case DASM_r8l:
		util::stream_format(stream, "%s", r8_names[opcode & 15]);
		break;

	case DASM_r8h:
		util::stream_format(stream, "%s", r8_names[(opcode >> 4) & 15]);
		break;

	case DASM_r8u:
		util::stream_format(stream, "%s", r8_names[(opcode >> 8) & 15]);
		break;

	case DASM_r16l:
		util::stream_format(stream, "%s", r16_names[opcode & 15]);
		break;

	case DASM_r16h:
		util::stream_format(stream, "%s", r16_names[(opcode >> 4) & 15]);
		break;

	case DASM_r32l:
		util::stream_format(stream, "%s", r32_names[opcode & 7]);
		break;

	case DASM_r32h:
		util::stream_format(stream, "%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ih:
		util::stream_format(stream, "@%s", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ihh:
		util::stream_format(stream, "@%s", r16_names[(opcode >> 20) & 7]);
		break;

	case DASM_pr16h:
		util::stream_format(stream, "@-%s", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16ph:
		util::stream_format(stream, "@%s+", r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r16d16h:
		util::stream_format(stream, "@(h'%x, %s)", opcodes.r16(epc-2), r16_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32ih:
		util::stream_format(stream, "@%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32ihh:
		util::stream_format(stream, "@%s", r32_names[(opcode >> 20) & 7]);
		break;

	case DASM_pr32h:
		util::stream_format(stream, "@-%s", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32pl:
		util::stream_format(stream, "@%s+", r32_names[opcode & 7]);
		break;

	case DASM_r32ph:
		util::stream_format(stream, "@%s+", r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32d16h:
		util::stream_format(stream, "@(h'%x, %s)", opcodes.r16(epc-2), r32_names[(opcode >> 4) & 7]);
		break;

	case DASM_r32d32hh:
		util::stream_format(stream, "@(h'%x, %s)", opcodes.r32(epc-4), r32_names[(opcode >> 20) & 7]);
		break;

	case DASM_psp:
		util::stream_format(stream, "@-sp");
		break;

	case DASM_spp:
		util::stream_format(stream, "@sp+");
		break;

	case DASM_r32n2l:
		util::stream_format(stream, "%s-%s", r32_names[opcode & 6], r32_names[(opcode & 6) + 1]);
		break;

	case DASM_r32n3l:
		util::stream_format(stream, "%s-%s", r32_names[opcode & 4], r32_names[(opcode & 4) + 2]);
		break;

	case DASM_r32n4l:
		util::stream_format(stream, "%s-%s", r32_names[opcode & 4], r32_names[(opcode & 4) + 3]);
		break;

	case DASM_abs8:
		if(m_advanced)
			util::stream_format(stream, "@h'%06x", 0xffff00 | opcodes.r8(pc+1));
		else
			util::stream_format(stream, "@h'%04x", 0xff00 | opcodes.r8(pc+1));
		break;

	case DASM_abs16:
		if(slot == 2)
		{
			if(m_advanced)
				util::stream_format(stream, "@h'%06x", s32(s16(opcodes.r16(epc-4))) & 0xffffff);
			else
				util::stream_format(stream, "@h'%04x", opcodes.r16(epc-4));
		}
		else
		{
			if(m_advanced)
				util::stream_format(stream, "@h'%06x", s32(s16(opcodes.r16(epc-2))) & 0xffffff);
			else
				util::stream_format(stream, "@h'%04x", opcodes.r16(epc-2));
		}
		break;

	case DASM_abs32:
		if(slot == 3)
			util::stream_format(stream, "@h'%06x", opcodes.r32(epc-6));
		else
			util::stream_format(stream, "@h'%06x", opcodes.r32(epc-4));
		break;

	case DASM_abs8i:
		util::stream_format(stream, "@h'%02x", opcodes.r8(pc+1));
		break;

	case DASM_abs16e:
		util::stream_format(stream, "h'%04x", opcodes.r16(pc+2));
		break;

	case DASM_abs22e:
		util::stream_format(stream, "h'%06x", opcodes.r32(pc) & 0x3fffff);
		break;

	case DASM_abs24e:
		util::stream_format(stream, "h'%06x", opcodes.r32(pc) & 0xffffff);
		break;

	case DASM_rel8:
		if(m_advanced)
			util::stream_format(stream, "h'%06x", (pc + 2 + s8(opcodes.r8(pc+1))) & 0xffffff);
		else
			util::stream_format(stream, "h'%04x", (pc + 2 + s8(opcodes.r8(pc+1))) & 0xffff);
		break;

	case DASM_rel16:
		if(m_advanced)
			util::stream_format(stream, "h'%06x", (pc + 4 + s16(opcodes.r16(pc+2))) & 0xffffff);
		else
			util::stream_format(stream, "h'%04x", (pc + 4 + s16(opcodes.r16(pc+2))) & 0xffff);
		break;

	case DASM_one:
		util::stream_format(stream, "#1");
		break;

	case DASM_two:
		util::stream_format(stream, "#2");
		break;

	case DASM_four:
		util::stream_format(stream, "#4");
		break;

	case DASM_imm2:
		util::stream_format(stream, "#%x", (opcode >> 4) & 3);
		break;

	case DASM_imm3:
		util::stream_format(stream, "#%x", (opcode >> 4) & 7);
		break;

	case DASM_imm6l:
		util::stream_format(stream, "#%x", opcode & 0x3f);
		break;

	case DASM_imm8:
		util::stream_format(stream, "#h'%02x", opcodes.r8(epc-1));
		break;

	case DASM_imm16:
		util::stream_format(stream, "#h'%04x", opcodes.r16(pc+2));
		break;

	case DASM_imm32:
		util::stream_format(stream, "#h'%08x", opcodes.r32(pc+2));
		break;

	case DASM_ccr:
		util::stream_format(stream, "ccr");
		break;

	case DASM_exr:
		util::stream_format(stream, "exr");
		break;

	case DASM_bankl:
		util::stream_format(stream, "bankl");
		break;

	case DASM_bankh:
		util::stream_format(stream, "bankh");
		break;

	case DASM_macl:
		util::stream_format(stream, "macl");
		break;

	case DASM_mach:
		util::stream_format(stream, "mach");
		break;

	default:
		util::stream_format(stream, "<%d>", am);
		break;
	}
}

offs_t h8_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 slot[5];
	slot[0] = opcodes.r16(pc);
	slot[1] = opcodes.r32(pc);
	slot[2] = (opcodes.r16(pc) << 16) | opcodes.r16(pc+4);
	slot[3] = (opcodes.r16(pc) << 16) | opcodes.r16(pc+6);
	slot[4] = opcodes.r32(pc+2);

	int inst;
	for(inst=0;; inst++) {
		const disasm_entry &e = m_table[inst];
		if((slot[e.m_slot] & e.m_mask) == e.m_val && (slot[0] & e.m_mask0) == e.m_val0)
			break;
	}
	const disasm_entry &e = m_table[inst];
	if(e.m_am1 == DASM_none)
		stream << e.m_opcode;
	else {
		util::stream_format(stream, "%-08s", e.m_opcode);
		disassemble_am(stream, e.m_am1, pc, opcodes, slot[e.m_slot], e.m_slot, e.m_flags & LENGTHMASK);
		if(e.m_am2 != DASM_none) {
			stream << ", ";
			disassemble_am(stream, e.m_am2, pc, opcodes, slot[e.m_slot], e.m_slot, e.m_flags & LENGTHMASK);
		}
	}
	return e.m_flags | SUPPORTED;
}
