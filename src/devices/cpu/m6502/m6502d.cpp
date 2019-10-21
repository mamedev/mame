// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502d.cpp

    MOS Technology 6502, original NMOS variant, disassembler

***************************************************************************/

#include "emu.h"
#include "m6502d.h"
#include "cpu/m6502/m6502d.hxx"

m6502_base_disassembler::m6502_base_disassembler(const disasm_entry *_table) : table(_table)
{
}

u32 m6502_base_disassembler::get_instruction_bank() const
{
	return 0;
}

u32 m6502_base_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t m6502_base_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const disasm_entry &e = table[opcodes.r8(pc) | get_instruction_bank()];
	uint32_t flags = e.flags | SUPPORTED;
	util::stream_format(stream, "%s", e.opcode);

	switch(e.mode) {
	case DASM_non:
		flags |= 1;
		break;

	case DASM_aba:
		util::stream_format(stream, " $%02x%02x", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_abx:
		util::stream_format(stream, " $%02x%02x, x", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_aby:
		util::stream_format(stream, " $%02x%02x, y", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_acc:
		util::stream_format(stream, " a");
		flags |= 1;
		break;

	case DASM_adr:
		util::stream_format(stream, " $%02x%02x", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_bzp:
		util::stream_format(stream, "%d $%02x", (opcodes.r8(pc) >> 4) & 7, params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_iax:
		util::stream_format(stream, " ($%02x%02x, x)", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_idx:
		util::stream_format(stream, " ($%02x, x)", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_idy:
		util::stream_format(stream, " ($%02x), y", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_idz:
		util::stream_format(stream, " ($%02x), z", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_imm:
		util::stream_format(stream, " #$%02x", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_imp:
		flags |= 1;
		break;

	case DASM_ind:
		util::stream_format(stream, " ($%02x%02x)", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_isy:
		util::stream_format(stream, " ($%02x, s), y", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_iw2:
		util::stream_format(stream, " #$%02x%02x", params.r8(pc+2), params.r8(pc+1));
		flags |= 3;
		break;

	case DASM_iw3:
		util::stream_format(stream, " #$%02x%02x%02x", params.r8(pc+3), params.r8(pc+2), params.r8(pc+1));
		flags |= 4;
		break;

	case DASM_rel:
		util::stream_format(stream, " $%04x", (pc & 0xf0000) | uint16_t(pc + 2 + int8_t(params.r8(pc+1))));
		flags |= 2;
		break;

	case DASM_rw2:
		util::stream_format(stream, " $%04x", (pc & 0xf0000) | uint16_t(pc + 2 + int16_t((params.r8(pc+2) << 8) | params.r8(pc+1))));
		flags |= 3;
		break;

	case DASM_zpb:
		util::stream_format(stream, "%d $%02x, $%04x", (opcodes.r8(pc) >> 4) & 7, params.r8(pc+1), (pc & 0xf0000) | uint16_t(pc + 3 + int8_t(params.r8(pc+2))));
		flags |= 3;
		break;

	case DASM_zpg:
		util::stream_format(stream, " $%02x", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_zpi:
		util::stream_format(stream, " ($%02x)", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_zpx:
		util::stream_format(stream, " $%02x, x", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_zpy:
		util::stream_format(stream, " $%02x, y", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_imz:
		util::stream_format(stream, " #$%02x, $%02x", params.r8(pc+1), params.r8(pc+2));
		flags |= 3;
		break;

	case DASM_spg:
		util::stream_format(stream, " \\$%02x", params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_biz:
		util::stream_format(stream, " %d, $%02x", (opcodes.r8(pc) >> 5) & 7, params.r8(pc+1));
		flags |= 2;
		break;

	case DASM_bzr:
		util::stream_format(stream, " %d, $%02x, $%04x", (opcodes.r8(pc) >> 5) & 7, params.r8(pc+1), (pc & 0xf0000) | uint16_t(pc + 3 + int8_t(params.r8(pc+2))));
		flags |= 3;
		break;

	case DASM_bar:
		util::stream_format(stream, " %d, a, $%04x", (opcodes.r8(pc) >> 5) & 7, (pc & 0xf0000) | uint16_t(pc + 3 + int8_t(params.r8(pc+1))));
		flags |= 2;
		break;

	case DASM_bac:
		util::stream_format(stream, " %d, a", (opcodes.r8(pc) >> 5) & 7);
		flags |= 1;
		break;

	case DASM_xa3:
		util::stream_format(stream, " #$%02x%02x%02x", params.r8(pc+1), params.r8(pc+3), params.r8(pc+2));
		flags |= 4;
		break;

	default:
		fprintf(stderr, "Unhandled dasm mode %d\n", e.mode);
		abort();
	}
	return flags;
}

m6502_disassembler::m6502_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
