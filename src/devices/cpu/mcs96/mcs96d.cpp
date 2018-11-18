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

std::string mcs96_disassembler::regname8(uint8_t reg, bool is_dest) const
{
	switch(reg) {
	case 0x00:
		return "r0";

	case 0x08:
		return "int_mask";

	case 0x09:
		return "int_pending";

	case 0x1c:
		return "al";

	case 0x1d:
		return "ah";

	case 0x1e:
		return "dl";

	case 0x1f:
		return "dh";

	case 0x20:
		return "bl";

	case 0x21:
		return "bh";

	case 0x22:
		return "cl";

	case 0x23:
		return "ch";

	default:
		return util::string_format("%02x", reg);
	}
}

std::string mcs96_disassembler::regname16(uint8_t reg, bool is_dest) const
{
	switch(reg) {
	case 0x00:
		return "r0";

	case 0x18:
		return "sp";

	case 0x1c:
		return "ax";

	case 0x1e:
		return "dx";

	case 0x20:
		return "bx";

	case 0x22:
		return "cx";

	default:
		return util::string_format("%02x", reg);
		break;
	}
}

std::string mcs96_disassembler::regname_indirect(uint8_t reg) const
{
	if(BIT(reg, 0))
		return util::string_format("[%s]+", regname16(reg & 0xfe, false));
	else
		return util::string_format("[%s]", regname16(reg, false));

}

std::string mcs96_disassembler::regname_indexed(uint8_t reg, int8_t delta) const
{
	if(reg == 0x00) {
		if(delta < 0)
			return util::string_format("%04x", uint8_t(delta) | 0xff00);
		else
			return util::string_format("%02x", delta);
	} else {
		if(delta < 0)
			return util::string_format("-%02x[%s]", -delta, regname16(reg, false));
		else
			return util::string_format("%02x[%s]", delta, regname16(reg, false));
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

	case DASM_rel8:
		util::stream_format(stream, " %04x", (pc+2+int8_t(opcodes.r8(pc+1))) & 0xffff);
		flags |= 2;
		break;

	case DASM_rel11: {
		int delta = ((opcodes.r8(pc) << 8) | opcodes.r8(pc+1)) & 0x7ff;
		if(delta & 0x400)
			delta -= 0x800;
		util::stream_format(stream, " %04x", (pc+2+delta) & 0xffff);
		flags |= 2;
		break;
	}

	case DASM_rel16:
		util::stream_format(stream, " %04x", (pc+3+opcodes.r16(pc+1)) & 0xffff);
		flags |= 3;
		break;

	case DASM_rrel8:
		util::stream_format(stream, " %s, %04x", regname8(opcodes.r8(pc+1), true), (pc+3+int8_t(opcodes.r8(pc+2))) & 0xffff);
		flags |= 3;
		break;

	case DASM_brrel8:
		util::stream_format(stream, " %s, %d, %04x", regname8(opcodes.r8(pc+1), false), opcodes.r8(pc) & 7, (pc+3+int8_t(opcodes.r8(pc+2))) & 0xffff);
		flags |= 3;
		break;

	case DASM_wrrel8:
		util::stream_format(stream, " %s, %04x", regname16(opcodes.r8(pc+1), true), (pc+3+int8_t(opcodes.r8(pc+2))) & 0xffff);
		flags |= 3;
		break;

	case DASM_direct_1b:
		util::stream_format(stream, " %s", regname8(opcodes.r8(pc+1), true));
		flags |= 2;
		break;

	case DASM_direct_1w:
		util::stream_format(stream, " %s", regname16(opcodes.r8(pc+1), opcodes.r8(pc) == 0x01 || opcodes.r8(pc) == 0xcc));
		flags |= 2;
		break;

	case DASM_direct_2b:
		util::stream_format(stream, " %s, %s", regname8(opcodes.r8(pc+2), opcodes.r8(pc) == 0xb0), regname8(opcodes.r8(pc+1), opcodes.r8(pc) == 0xc4));
		flags |= 3;
		break;

	case DASM_direct_2e:
		util::stream_format(stream, " %s, %s", regname16(opcodes.r8(pc+2), (opcodes.r8(pc) & 0xef) == 0xac), regname8(opcodes.r8(pc+1), opcodes.r8(pc) == 0x0f));
		flags |= 3;
		break;

	case DASM_direct_2w:
		util::stream_format(stream, " %s, %s", regname16(opcodes.r8(pc+2), opcodes.r8(pc) == 0xa0), regname16(opcodes.r8(pc+1), opcodes.r8(pc) == 0xc0));
		flags |= 3;
		break;

	case DASM_direct_3b:
		util::stream_format(stream, " %s, %s, %s", regname8(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), regname8(opcodes.r8(pc+1), false));
		flags |= 4;
		break;

	case DASM_direct_3e:
		util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), regname8(opcodes.r8(pc+1), false));
		flags |= 4;
		break;

	case DASM_direct_3w:
		util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+3), true), regname16(opcodes.r8(pc+2), false), regname16(opcodes.r8(pc+1), false));
		flags |= 4;
		break;

	case DASM_immed_1b:
		util::stream_format(stream, " #%02x", opcodes.r8(pc+1));
		flags |= 2;
		break;

	case DASM_immed_2b:
		util::stream_format(stream, " %s, #%02x", regname8(opcodes.r8(pc+2), opcodes.r8(pc) == 0xb1), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_2e:
		util::stream_format(stream, " %s, #%02x", regname16(opcodes.r8(pc+2), (opcodes.r8(pc) & 0xef) == 0xad), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_or_reg_2b:
		if(opcodes.r8(pc+1) >= 0x10)
			util::stream_format(stream, " %s, %s", regname8(opcodes.r8(pc+2), false), regname8(opcodes.r8(pc+1), false));
		else
			util::stream_format(stream, " %s, #%02x", regname8(opcodes.r8(pc+2), false), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_3b:
		util::stream_format(stream, " %s, %s, #%02x", regname8(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), opcodes.r8(pc+1));
		flags |= 4;
		break;

	case DASM_immed_3e:
		util::stream_format(stream, " %s, %s, #%02x", regname16(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), opcodes.r8(pc+1));
		flags |= 4;
		break;

	case DASM_immed_1w:
		util::stream_format(stream, " #%04x", opcodes.r16(pc+1));
		flags |= 3;
		break;

	case DASM_immed_2w:
		util::stream_format(stream, " %s, #%04x", regname16(opcodes.r8(pc+3), opcodes.r8(pc) == 0xa1), opcodes.r16(pc+1));
		flags |= 4;
		break;

	case DASM_immed_or_reg_2w:
		if(opcodes.r8(pc+1) >= 0x10)
			util::stream_format(stream, " %s, %s", regname16(opcodes.r8(pc+2), false), regname8(opcodes.r8(pc+1), false));
		else
			util::stream_format(stream, " %s, #%02x", regname16(opcodes.r8(pc+2), false), opcodes.r8(pc+1));
		flags |= 3;
		break;

	case DASM_immed_3w:
		util::stream_format(stream, " %s, %s, #%04x", regname16(opcodes.r8(pc+4), true), regname16(opcodes.r8(pc+3), false), opcodes.r16(pc+1));
		flags |= 5;
		break;

	case DASM_indirect_1n:
		util::stream_format(stream, " [%s]", regname16(opcodes.r8(pc+1), false));
		flags |= 2;
		break;

	case DASM_indirect_1w:
		util::stream_format(stream, " %s", regname_indirect(opcodes.r8(pc+1)));
		flags |= 2;
		break;

	case DASM_indirect_2b:
		util::stream_format(stream, " %s, %s", regname8(opcodes.r8(pc+2), opcodes.r8(pc) == 0xb2), regname_indirect(opcodes.r8(pc+1)));
		flags |= 3;
		break;

	case DASM_indirect_2w:
		util::stream_format(stream, " %s, %s", regname16(opcodes.r8(pc+2), opcodes.r8(pc) == 0xa2), regname_indirect(opcodes.r8(pc+1)));
		flags |= 3;
		break;

	case DASM_indirect_3b:
		util::stream_format(stream, " %s, %s, %s", regname8(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), regname_indirect(opcodes.r8(pc+1)));
		flags |= 4;
		break;

	case DASM_indirect_3e:
		util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+3), true), regname8(opcodes.r8(pc+2), false), regname_indirect(opcodes.r8(pc+1)));
		flags |= 4;
		break;

	case DASM_indirect_3w:
		util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+3), true), regname16(opcodes.r8(pc+2), false), regname_indirect(opcodes.r8(pc+1)));
		flags |= 4;
		break;

	case DASM_indexed_1w:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %04x", opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %04x[%s]", opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 4;
		} else {
			util::stream_format(stream, " %s", regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 3;
		}
		break;

	case DASM_indexed_2b: {
		bool is_dest = opcodes.r8(pc) == 0xb3;
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %04x", regname8(opcodes.r8(pc+4), is_dest), opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %s, %04x[%s]", regname8(opcodes.r8(pc+4), is_dest), opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 5;
		} else {
			util::stream_format(stream, " %s, %s", regname8(opcodes.r8(pc+3), is_dest), regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 4;
		}
		break;
	}

	case DASM_indexed_2w: {
		bool is_dest = opcodes.r8(pc) == 0xa3 || (opcodes.r8(pc) & 0xef) == 0xaf;
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %04x", regname16(opcodes.r8(pc+4), is_dest), opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %s, %04x[%s]", regname16(opcodes.r8(pc+4), is_dest), opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 5;
		} else {
			util::stream_format(stream, " %s, %s", regname16(opcodes.r8(pc+3), is_dest), regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 4;
		}
		break;
	}

	case DASM_indexed_3b:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %s, %04x", regname8(opcodes.r8(pc+5), true), regname8(opcodes.r8(pc+4), false), opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %s, %s, %04x[%s]", regname8(opcodes.r8(pc+5), true), regname8(opcodes.r8(pc+4), false), opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 6;
		} else {
			util::stream_format(stream, " %s, %s, %s", regname8(opcodes.r8(pc+4), true), regname8(opcodes.r8(pc+3), false), regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 5;
		}
		break;

	case DASM_indexed_3e:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %s, %04x", regname16(opcodes.r8(pc+5), true), regname8(opcodes.r8(pc+4), false), opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %s, %s, %04x[%s]", regname16(opcodes.r8(pc+5), true), regname8(opcodes.r8(pc+4), false), opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 6;
		} else {
			util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+4), true), regname8(opcodes.r8(pc+3), false), regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 5;
		}
		break;

	case DASM_indexed_3w:
		if(opcodes.r8(pc+1) & 0x01) {
			if(opcodes.r8(pc+1) == 0x01)
				util::stream_format(stream, " %s, %s, %04x", regname16(opcodes.r8(pc+5), true), regname16(opcodes.r8(pc+4), false), opcodes.r16(pc+2));
			else
				util::stream_format(stream, " %s, %s, %04x[%s]", regname16(opcodes.r8(pc+5), true), regname16(opcodes.r8(pc+4), false), opcodes.r16(pc+2), regname16(opcodes.r8(pc+1)-1, false));
			flags |= 6;
		} else {
			util::stream_format(stream, " %s, %s, %s", regname16(opcodes.r8(pc+4), true), regname16(opcodes.r8(pc+3), false), regname_indexed(opcodes.r8(pc+1), opcodes.r8(pc+2)));
			flags |= 5;
		}
		break;

	default:
		fprintf(stderr, "Unhandled dasm mode %d\n", e.mode);
		abort();
	};

	return flags+off;
}
