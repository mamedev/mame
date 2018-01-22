// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    esripdsm.c

    Implementation of the Entertainment Sciences
    AM29116-based Real Time Image Processor

***************************************************************************/

#include "emu.h"
#include "esripdsm.h"

u32 esrip_disassembler::opcode_alignment() const
{
	return 1;
}

/***************************************************************************
    DISASSEMBLY HOOK (TODO: FINISH)
***************************************************************************/

offs_t esrip_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
#if 0
	static const char* const jmp_types[] =
	{
		"JCT",
		"JT1",
		"JT2",
		"JT3",
		"JT4",
		"JLBRM",
		"J#HBLANK",
		"JMP",
	};

	static const char* const njmp_types[] =
	{
		"JNCT",
		"JNT1",
		"JNT2",
		"JNT3",
		"JNT4",
		"JNLBRM",
		"J#HBLANK",
		"  ",
	};
#endif

	uint64_t inst = opcodes.r64(pc);

	uint32_t inst_hi = inst >> 32;
	uint32_t inst_lo = inst & 0xffffffff;

	uint16_t ins = (inst_hi >> 16) & 0xffff;
	uint8_t  ctrl = (inst_hi >> 8) & 0xff;
	uint8_t  jmp_dest = (inst_lo >> 8) & 0xff;

	uint8_t jmp_ctrl = (ctrl >> 3) & 0x1f;

	uint8_t ctrl1 = (inst_lo >> 16) & 0xff;
	uint8_t ctrl2 = (inst_lo >> 24) & 0xff;
	uint8_t ctrl3 = (inst_hi) & 0xff;

	util::stream_format(stream, "%04x %c%c%c%c %02x %s%s%s%s%s%s%s%s %c%s%s%s %c%c%c%c%c%c%c%c",
			ins,
			ctrl & 1 ? 'D' : ' ',
			ctrl & 2 ? ' ' : 'Y',
			ctrl & 4 ? 'S' : ' ',
			(~jmp_ctrl & 0x18) ? 'J' : ' ',
			jmp_dest,
			ctrl1 & 0x01 ? "  " : "I ",
			ctrl1 & 0x02 ? "  " : "FL",
			ctrl1 & 0x04 ? "FE" : "  ",
			ctrl1 & 0x08 ? "  " : "FR",
			ctrl1 & 0x10 ? "  " : "IL",
			ctrl1 & 0x20 ? "IE" : "  ",
			ctrl1 & 0x40 ? "  " : "IR",
			ctrl1 & 0x80 ? "  " : "IW",

			ctrl2 & 0x80 ? ' ' : 'O',
			ctrl2 & 0x40 ? "     " : "IXLLD",
			ctrl2 & 0x20 ? "     " : "IADLD",
			ctrl2 & 0x10 ? "     " : "SCALD",

			ctrl3 & 0x01 ? ' ' : '0',
			ctrl3 & 0x02 ? ' ' : '1',
			ctrl3 & 0x04 ? ' ' : '2',
			ctrl3 & 0x08 ? ' ' : '3',
			ctrl3 & 0x10 ? ' ' : '4',
			ctrl3 & 0x20 ? ' ' : '5',
			ctrl3 & 0x40 ? ' ' : '6',
			ctrl3 & 0x80 ? ' ' : '7'
			);

	return 1 | SUPPORTED;
}
