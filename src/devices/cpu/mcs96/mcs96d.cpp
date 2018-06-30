// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    mcs96.h

    MCS96, 8098/8398/8798 branch

***************************************************************************/

#include "emu.h"
#include "mcs96d.h"

mcs96_disassembler::mcs96_disassembler(const disasm_entry *entries) : m_entries(entries)
{
}

u32 mcs96_disassembler::opcode_alignment() const
{
	return 1;
}

std::string mcs96_disassembler::regname(uint8_t reg)
{
	switch(reg) {
	case 0x18:
		return "sp";
		break;

	case 0x19:
		return "sph";
		break;

	default:
		return util::string_format("%02x", reg);
		break;
	}
}

offs_t mcs96_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	bool prefix_fe = false;
	int off = 0;
	if(opcodes.r8(pc) == 0xfe && m_entries[opcodes.r8(pc+1)].opcode_fe) {
		prefix_fe = true;
		pc++;
		off++;
	}
	const disasm_entry &e = m_entries[opcodes.r8(pc)];
	uint32_t flags = e.flags | SUPPORTED;
	util::stream_format(stream, "%s", prefix_fe ? e.opcode_fe : e.opcode);

	switch(e.mode) {
	case DASM_none:
		flags |= 1;
		break;

	case DASM_nop_2:
		util::stream_format(stream, " %02x", opcodes.r8(pc+1));
		flags |= 2;
		break;

	case DASM_rel8: {
		int delta = opcodes.r8(pc+1);
		if(delta & 0x80)
			delta -= 0x100;
		util::stream_format(stream, " %04x", (pc+2+delta) & 0xffff);
		flags |= 2;
		break;
	}

	case DASM_rel11: {
		int delta = ((opcodes.r8(pc) << 8) | opcodes.r8(pc+1)) & 0x7ff;
		if(delta & 0x400)
			delta -= 0x800;
		util::stream_format(stream, " %04x", (pc+2+delta) & 0xffff);
		flags |= 2;
		break;
	}

	case DASM_rel16: {
		int delta = opcodes.r8(pc+1) | (opcodes.r8(pc+2) << 8);
		util::stream_format(stream, " %04x", (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_rrel8: {
		int delta = opcodes.r8(pc+2);
		if(delta & 0x80)
			delta -= 0x100;
		util::stream_format(stream, " %s, %04x", regname(opcodes.r8(pc+1)), (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_brrel8: {
		int delta = opcodes.r8(pc+2);
		if(delta & 0x80)
			delta -= 0x100;
		util::stream_format(stream, " %d, %s, %04x", opcodes.r8(pc) & 7, regname(opcodes.r8(pc+1)), (pc+3+delta) & 0xffff);
		flags |= 3;
		break;
	}

	case DASM_direct_1:
		util::stream_format(stream, " %s", regname(opcodes.r8(pc+1)));
		flags |= 2;
		break;

	case DASM_direct_2:
		util::stream_format(stream, " %s, %s", regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)));
		flags |= 3;
		break;

	case DASM_direct_3:
		util::stream_format(stream, " %s, %s, %s", regname(opcodes.r8(pc+3)), regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)));
		flags |= 4;
		break;

	case DASM_immed_1b:
		util::stream_format(stream, " #%02x", opcodes.r8(pc+1));
		flags |= 2;
		break;

	case DASM_immed_2b:
		util::stream_format(stream, " %s, #%02x", regname(opcodes.r8(pc+2)), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_or_reg_2b:
		if(opcodes.r8(pc+1) >= 0x10)
			util::stream_format(stream, " %s, %s", regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)));
		else
			util::stream_format(stream, " %s, #%02x", regname(opcodes.r8(pc+2)), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_3b:
		util::stream_format(stream, " %s, %s, #%02x", regname(opcodes.r8(pc+3)), regname(opcodes.r8(pc+2)), opcodes.r8(pc+1));
		flags |= 4;
		break;

	case DASM_immed_1w:
		util::stream_format(stream, " #%02x%02x", opcodes.r8(pc+2), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_2w:
		util::stream_format(stream, " %s, #%02x%02x", regname(opcodes.r8(pc+3)), opcodes.r8(pc+2), opcodes.r8(pc+1));
		flags |= 4;
		break;

	case DASM_immed_3w:
		util::stream_format(stream, " %s, %s, #%02x%02x", regname(opcodes.r8(pc+4)), regname(opcodes.r8(pc+3)), opcodes.r8(pc+2), opcodes.r8(pc+1));
		flags |= 5;
		break;

	case DASM_indirect_1n:
		util::stream_format(stream, " [%s]", regname(opcodes.r8(pc+1)));
		flags |= 2;
		break;

	case DASM_indirect_1:
		if(opcodes.r8(pc+1) & 0x01) {
			util::stream_format(stream, " [%s]+", regname(opcodes.r8(pc+1)-1));
			flags |= 2;
		} else {
			util::stream_format(stream, " [%s]", regname(opcodes.r8(pc+1)));
			flags |= 2;
		}
		break;

	case DASM_indirect_2:
		if(opcodes.r8(pc+1) & 0x01) {
			util::stream_format(stream, " %s, [%s]+", regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)-1));
			flags |= 3;
		} else {
			util::stream_format(stream, " %s, [%s]", regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)));
			flags |= 3;
		}
		break;

	case DASM_indirect_3:
		if(opcodes.r8(pc+1) & 0x01) {
			util::stream_format(stream, " %s, %s, [%s]+", regname(opcodes.r8(pc+3)), regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)-1));
			flags |= 4;
		} else {
			util::stream_format(stream, " %s, %s, [%s]", regname(opcodes.r8(pc+3)), regname(opcodes.r8(pc+2)), regname(opcodes.r8(pc+1)));
			flags |= 4;
		}
		break;

	case DASM_indexed_1:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %02x%02x", opcodes.r8(pc+3), opcodes.r8(pc+2));
			else
				util::stream_format(stream, " %02x%02x[%s]", opcodes.r8(pc+3), opcodes.r8(pc+2), regname(opcodes.r8(pc+1)-1));
			flags |= 4;
		} else {
			int delta = opcodes.r8(pc+2);
			if(delta & 0x80)
				delta -= 0x100;
			if(opcodes.r8(pc+1) == 0x00) {
				if(delta < 0)
					util::stream_format(stream, " %04x", delta & 0xffff);
				else
					util::stream_format(stream, " %02x", delta);
			} else {
				if(delta < 0)
					util::stream_format(stream, " -%02x[%s]", -delta, regname(opcodes.r8(pc+1)));
				else
					util::stream_format(stream, " %02x[%s]", delta, regname(opcodes.r8(pc+1)));
			}
			flags |= 3;
		}
		break;

	case DASM_indexed_2:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %02x%02x", regname(opcodes.r8(pc+4)), opcodes.r8(pc+3), opcodes.r8(pc+2));
			else
				util::stream_format(stream, " %s, %02x%02x[%s]", regname(opcodes.r8(pc+4)), opcodes.r8(pc+3), opcodes.r8(pc+2), regname(opcodes.r8(pc+1)-1));
			flags |= 5;
		} else {
			int delta = opcodes.r8(pc+2);
			if(delta & 0x80)
				delta -= 0x100;
			if(opcodes.r8(pc+1) == 0x00) {
				if(delta < 0)
					util::stream_format(stream, " %s, %04x", regname(opcodes.r8(pc+3)), delta & 0xffff);
				else
					util::stream_format(stream, " %s, %02x", regname(opcodes.r8(pc+3)), delta);
			} else {
				if(delta < 0)
					util::stream_format(stream, " %s, -%02x[%s]", regname(opcodes.r8(pc+3)), -delta, regname(opcodes.r8(pc+1)));
				else
					util::stream_format(stream, " %s, %02x[%s]", regname(opcodes.r8(pc+3)), delta, regname(opcodes.r8(pc+1)));
			}
			flags |= 4;
		}
		break;

	case DASM_indexed_3:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %s, %02x%02x", regname(opcodes.r8(pc+5)),  regname(opcodes.r8(pc+4)), opcodes.r8(pc+3), opcodes.r8(pc+2));
			else
				util::stream_format(stream, " %s, %s, %02x%02x[%s]", regname(opcodes.r8(pc+5)), regname(opcodes.r8(pc+4)), opcodes.r8(pc+3), opcodes.r8(pc+2), regname(opcodes.r8(pc+1)-1));
			flags |= 6;
		} else {
			int delta = opcodes.r8(pc+2);
			if(delta & 0x80)
				delta -= 0x100;
			if(opcodes.r8(pc+1) == 0x00) {
				if(delta < 0)
					util::stream_format(stream, " %s, %s, %04x", regname(opcodes.r8(pc+4)), regname(opcodes.r8(pc+3)), delta & 0xffff);
				else
					util::stream_format(stream, " %s, %s, %02x", regname(opcodes.r8(pc+4)), regname(opcodes.r8(pc+3)), delta);
			} else {
				if(delta < 0)
					util::stream_format(stream, " %s, %s, -%02x[%s]", regname(opcodes.r8(pc+4)), regname(opcodes.r8(pc+3)), -delta, regname(opcodes.r8(pc+1)));
				else
					util::stream_format(stream, " %s, %s, %02x[%s]", regname(opcodes.r8(pc+4)), regname(opcodes.r8(pc+3)), delta, regname(opcodes.r8(pc+1)));
			}
			flags |= 5;
		}
		break;

	default:
		fprintf(stderr, "Unhandled dasm mode %d\n", e.mode);
		abort();
	};

	return flags+off;
}
